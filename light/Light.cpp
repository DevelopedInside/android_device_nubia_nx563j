#define LOG_TAG "LightService"

#include <log/log.h>

#include "Light.h"

#include <fstream>

// Display
#define LCD_FILE "/sys/class/leds/lcd-backlight/brightness"


// Nubia LED
#define LED_BRIGHTNESS "/sys/class/leds/nubia_led/brightness"
#define LED_BLINK_MODE "/sys/class/leds/nubia_led/blink_mode"
#define LED_CHANNEL "/sys/class/leds/nubia_led/outn"
#define LED_GRADE "/sys/class/leds/nubia_led/grade_parameter"
#define LED_FADE "/sys/class/leds/nubia_led/fade_parameter"

// Battery
#define BATTERY_CAPACITY "/sys/class/power_supply/battery/capacity"
#define BATTERY_CHARGING_STATUS "/sys/class/power_supply/battery/status"
#define BATTERY_STATUS_DISCHARGING  "Discharging"
#define BATTERY_STATUS_NOT_CHARGING "Not charging"
#define BATTERY_STATUS_CHARGING     "Charging"


// Blink mode
#define BLINK_MODE_ON 1
#define BLINK_MODE_OFF 2
#define BLINK_MODE_BREATH 3
#define BLINK_MODE_BREATH_ONCE 6

// Outn channels
#define LED_CHANNEL_HOME 16
#define LED_CHANNEL_BUTTON 8

// Grade values
#define LED_GRADE_BUTTON 8
#define LED_GRADE_HOME 8
#define LED_GRADE_HOME_BATTERY_LOW 0
#define LED_GRADE_HOME_BATTERY_FULL 6 
#define LED_GRADE_HOME_NOTIFICATION 6
#define LED_GRADE_HOME_BATTERY 6
#define LED_GRADE_HOME_ATTENTION 6

#define MAX_LED_BRIGHTNESS 255
#define MAX_LCD_BRIGHTNESS 4095

namespace {

using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::light::V2_0::Flash;
using ::android::hardware::light::V2_0::ILight;
using ::android::hardware::light::V2_0::LightState;
using ::android::hardware::light::V2_0::Status;
using ::android::hardware::light::V2_0::Type;

/*
 * Write value to path and close file.
 */
static void set(std::string path, std::string value) {
    std::ofstream file(path);

    if (!file.is_open()) {
        ALOGW("failed to write %s to %s", value.c_str(), path.c_str());
        return;
    }

    file << value;
}

static int readStr(std::string path, char *buffer, size_t size)
{

    std::ifstream file(path);

    if (!file.is_open()) {
        ALOGW("failed to read %s", path.c_str());
        return -1;
    }

    file.read(buffer, size);
    file.close();
    return 1;
}

static int set(std::string path, char *buffer, size_t size)
{

    std::ofstream file(path);

    if (!file.is_open()) {
        ALOGW("failed to write %s", path.c_str());
        return -1;
    }

    file.write(buffer, size);
    file.close();
    return 1;
}

static void set(std::string path, int value) {
    set(path, std::to_string(value));
}

static inline bool isLit(const LightState& state) {
    return state.color & 0x00ffffff;
}

static uint32_t getBrightness(const LightState& state) {
    uint32_t alpha, red, green, blue;

    /*
     * Extract brightness from AARRGGBB.
     */
    alpha = (state.color >> 24) & 0xFF;
    red = (state.color >> 16) & 0xFF;
    green = (state.color >> 8) & 0xFF;
    blue = state.color & 0xFF;

    /*
     * Scale RGB brightness if Alpha brightness is not 0xFF.
     */
    if (alpha != 0xFF) {
        red = red * alpha / 0xFF;
        green = green * alpha / 0xFF;
        blue = blue * alpha / 0xFF;
    }

    return (77 * red + 150 * green + 29 * blue) >> 8;
}

static inline uint32_t scaleBrightness(uint32_t brightness, uint32_t maxBrightness) {
    return brightness * maxBrightness / 0xFF;
}

static inline uint32_t getScaledBrightness(const LightState& state, uint32_t maxBrightness) {
    return scaleBrightness(getBrightness(state), maxBrightness);
}

static void handleBacklight(const LightState& state) {
    uint32_t brightness = getScaledBrightness(state, MAX_LCD_BRIGHTNESS);
    set(LCD_FILE, brightness);
}

int getBatteryStatus(const LightState)
{
    int err;

    char status_str[16];

    err = readStr(BATTERY_CHARGING_STATUS, status_str, sizeof(status_str));
    if (err <= 0) {
        ALOGI("failed to read battery status: %d", err);
        return BATTERY_UNKNOWN;
    }

    ALOGI("battery status: %d, %s", err, status_str);

    char capacity_str[6];
    int capacity;

    err = readStr(BATTERY_CAPACITY, capacity_str, sizeof(capacity_str));
    if (err <= 0) {
        ALOGI("failed to read battery capacity: %d", err);
        return BATTERY_UNKNOWN;
    }

    capacity = atoi(capacity_str);

    ALOGI("battery capacity: %d", capacity);

    if (0 == strncmp(status_str, BATTERY_STATUS_CHARGING, 8)) {
        if (capacity < 90) {
            return BATTERY_CHARGING;
        } else {
            return BATTERY_FULL;
        }
    } else {
        if (capacity < 10) {
            return BATTERY_LOW;
        } else {
            return BATTERY_FREE;
        }
    }
}

static void handleNubiaLed(const LightState& state, int source)
{
    int mode;

    size_t grade = 0;
    char fade[16];
    size_t fade_len = 0;

    ALOGI("setting led %d: %08x, %d, %d", source,
        state.color, state.flashOnMs, state.flashOffMs);

    if (getBrightness(state) > 0) {
        g_ongoing |= source;
        ALOGI("ongoing +: %d = %d", source, g_ongoing);
    } else {
        g_ongoing &= ~source;
        ALOGI("ongoing -: %d = %d", source, g_ongoing);
    }

    /* set side buttons */

    set(LED_CHANNEL, LED_CHANNEL_BUTTON);
    if (g_ongoing & ONGOING_BUTTONS) {
        set(LED_GRADE, LED_GRADE_BUTTON);
        set(LED_BLINK_MODE, BLINK_MODE_ON);
    } else {
        set(LED_BLINK_MODE, BLINK_MODE_OFF);
    }

    /* set middle ring */

    if (g_ongoing & ONGOING_NOTIFICATION) {
        mode = BLINK_MODE_BREATH;
        grade = LED_GRADE_HOME_NOTIFICATION;
    }
    else if (g_ongoing & ONGOING_ATTENTION) {
        mode = BLINK_MODE_BREATH;
        grade = LED_GRADE_HOME_ATTENTION;
    }
    else if (g_battery == BATTERY_CHARGING) {
        mode = BLINK_MODE_BREATH;
        grade = LED_GRADE_HOME_BATTERY;
        fade_len = sprintf(fade, "%d %d %d\n", 3, 0, 4);
    }
    else if (g_battery == BATTERY_FULL) {
        mode = BLINK_MODE_BREATH_ONCE;
        grade = LED_GRADE_HOME_BATTERY_FULL;
        fade_len = sprintf(fade, "%d %d %d\n", 3, 0, 4);
    }
    else if (g_battery == BATTERY_LOW) {
        mode = BLINK_MODE_BREATH;
        grade = LED_GRADE_HOME_BATTERY_LOW;
        fade_len = sprintf(fade, "%d %d %d\n", 3, 0, 4);
    }
    else if (g_ongoing & ONGOING_BUTTONS) {
        mode = BLINK_MODE_ON;
        grade = LED_GRADE_BUTTON;
    }
    else {
        mode = BLINK_MODE_OFF;
    }

    if (state.flashMode == Flash::TIMED) {
        fade_len = sprintf(fade, "%d %d %d\n",
            1, state.flashOnMs / 400, state.flashOffMs / 400);
    }

    set(LED_CHANNEL, LED_CHANNEL_HOME);
    set(LED_GRADE, grade);
    if (fade_len > 0) {
        set(LED_FADE, fade, fade_len);
    }
    set(LED_BLINK_MODE, mode);
}

static void handleButtons(const LightState& state) {
    uint32_t brightness = getScaledBrightness(state, MAX_LED_BRIGHTNESS);
    set(LED_BRIGHTNESS, brightness);
	handleNubiaLed(state, ONGOING_BUTTONS);
}

static void handleNotification(const LightState& state) {
	handleNubiaLed(state, ONGOING_NOTIFICATION);
}

static void handleBattery(const LightState& state){
	g_battery = getBatteryStatus(state);
	handleNubiaLed(state, 0);
}

static void handleAttention(const LightState& state){
	handleNubiaLed(state, ONGOING_ATTENTION);
}
/* Keep sorted in the order of importance. */
static std::vector<LightBackend> backends = {
    { Type::ATTENTION, handleAttention },
    { Type::NOTIFICATIONS, handleNotification },
    { Type::BATTERY, handleBattery },
    { Type::BACKLIGHT, handleBacklight },
    { Type::BUTTONS, handleButtons },
};

}  // anonymous namespace

namespace android {
namespace hardware {
namespace light {
namespace V2_0 {
namespace implementation {

Return<Status> Light::setLight(Type type, const LightState& state) {
    LightStateHandler handler;
    bool handled = false;

    /* Lock global mutex until light state is updated. */
    std::lock_guard<std::mutex> lock(globalLock);

    /* Update the cached state value for the current type. */
    for (LightBackend& backend : backends) {
        if (backend.type == type) {
            backend.state = state;
            handler = backend.handler;
        }
    }

    /* If no handler has been found, then the type is not supported. */
    if (!handler) {
        return Status::LIGHT_NOT_SUPPORTED;
    }

    /* Light up the type with the highest priority that matches the current handler. */
    for (LightBackend& backend : backends) {
        if (handler == backend.handler && isLit(backend.state)) {
            handler(backend.state);
            handled = true;
            break;
        }
    }

    /* If no type has been lit up, then turn off the hardware. */
    if (!handled) {
        handler(state);
    }

    return Status::SUCCESS;
}

Return<void> Light::getSupportedTypes(getSupportedTypes_cb _hidl_cb) {
    std::vector<Type> types;

    for (const LightBackend& backend : backends) {
        types.push_back(backend.type);
    }

    _hidl_cb(types);

    return Void();
}

}  // namespace implementation
}  // namespace V2_0
}  // namespace light
}  // namespace hardware
}  // namespace android
