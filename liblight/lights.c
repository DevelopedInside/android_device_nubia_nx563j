/*
 * Copyright (C) 2008 The Android Open Source Project
 * Copyright (C) 2014 The Linux Foundation. All rights reserved.
 * Copyright (C) 2016 The CyanogenMod Project
 * Copyright (C) 2018 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "lightsHAL"

#include <cutils/log.h>
#include <cutils/properties.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <math.h>

#include <sys/ioctl.h>
#include <sys/types.h>

#include <hardware/lights.h>

/******************************************************************************/

static pthread_once_t g_init = PTHREAD_ONCE_INIT;
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
static struct light_state_t g_attention;
static struct light_state_t g_notification;
static struct light_state_t g_battery;
static struct light_state_t g_buttons;

// Display
char const* const LCD_FILE = "/sys/class/leds/lcd-backlight/brightness";

char const* const LCD_MAX_BRIGHTNESS_FILE = "/sys/class/leds/lcd-backlight/max_brightness";

// Nubia LED
char const* const LED_BRIGHTNESS = "/sys/class/leds/nubia_led/brightness";

char const* const LED_BLINK_MODE = "/sys/class/leds/nubia_led/blink_mode";

char const* const LED_CHANNEL = "/sys/class/leds/nubia_led/outn";

char const* const LED_GRADE = "/sys/class/leds/nubia_led/grade_parameter";

char const* const LED_FADE = "/sys/class/leds/nubia_led/fade_parameter";

// Battery
char const* const BATTERY_CAPACITY = "/sys/class/power_supply/battery/capacity";

char const* const BATTERY_CHARGING_STATUS = "/sys/class/power_supply/battery/status";

// Blink mode
#define BLINK_MODE_ON 1
#define BLINK_MODE_OFF 2
#define BLINK_MODE_BREATH 3
#define BLINK_MODE_BREATH_ONCE 6

// Events
#define BREATH_SOURCE_NOTIFICATION 0x01
#define BREATH_SOURCE_BATTERY 0x02
#define BREATH_SOURCE_BUTTONS 0x04
#define BREATH_SOURCE_ATTENTION 0x08

// Outn channels
#define LED_CHANNEL_HOME 16
#define LED_CHANNEL_BUTTON 8

// Grade values
#define LED_GRADE_BUTTON 8
#define LED_GRADE_HOME 8
#define LED_GRADE_HOME_BATTERY_LOW 0
#define LED_GRADE_HOME_NOTIFICATION 6
#define LED_GRADE_HOME_BATTERY 6

// Max display brightness
#define DEFAULT_MAX_BRIGHTNESS 255

int max_brightness;

int initialized = 0;

/**
 * device methods
 */

void init_globals(void)
{
    // init the mutex
    pthread_mutex_init(&g_lock, NULL);
}

static int read_int(char const* path, int* value)
{
    int fd;
    static int already_warned = 0;

    fd = open(path, O_RDONLY);
    if (fd >= 0)
    {
        char buffer[20];
        int amt = read(fd, buffer, 20);
        sscanf(buffer, "%d\n", value);
        close(fd);
        return amt == -1 ? -errno : 0;
    }
    else
    {
        if (already_warned == 0)
        {
            ALOGD("read_int failed to open %s\n", path);
            already_warned = 1;
        }
        return -errno;
    }
}

static int write_int(char const* path, int value)
{
    int fd;
    static int already_warned = 0;

    fd = open(path, O_RDWR);
    if (fd >= 0)
    {
        char buffer[20];
        int bytes = snprintf(buffer, sizeof(buffer), "%d\n", value);
        ssize_t amt = write(fd, buffer, (size_t)bytes);
        close(fd);
        return amt == -1 ? -errno : 0;
    }
    else
    {
        if (already_warned == 0)
        {
            ALOGE("write_int failed to open %s\n", path);
            already_warned = 1;
        }
        return -errno;
    }
}

static int write_str(char const* path, char* value)
{
    int fd;
    static int already_warned = 0;

    fd = open(path, O_RDWR);
    if (fd >= 0)
    {
        char buffer[1024];
        int bytes = snprintf(buffer, sizeof(buffer), "%s\n", value);
        ssize_t amt = write(fd, buffer, (size_t)bytes);
        close(fd);
        return amt == -1 ? -errno : 0;
    }
    else
    {
        if (already_warned == 0)
        {
            ALOGE("write_int failed to open %s\n", path);
            already_warned = 1;
        }
        return -errno;
    }
}

static int is_lit(struct light_state_t const* state)
{
    return state->color & 0x00ffffff;
}

static int rgb_to_brightness(struct light_state_t const* state)
{
    int color = state->color & 0x00ffffff;
    return ((77 * ((color >> 16) & 0x00ff)) + (150 * ((color >> 8) & 0x00ff))
               + (29 * (color & 0x00ff)))
        >> 8;
}

static int set_light_backlight(struct light_device_t* dev, struct light_state_t const* state)
{
    int err = 0;
    int brightness = rgb_to_brightness(state);
    if (!dev)
    {
        return -1;
    }

    // If max panel brightness is not the default (255),
    // apply linear scaling across the accepted range.
    if (max_brightness != DEFAULT_MAX_BRIGHTNESS)
    {
        int old_brightness = brightness;
        brightness = brightness * max_brightness / DEFAULT_MAX_BRIGHTNESS;
        ALOGV("%s: scaling brightness %d => %d\n", __func__, old_brightness, brightness);
    }

    pthread_mutex_lock(&g_lock);
    err = write_int(LCD_FILE, brightness);
    pthread_mutex_unlock(&g_lock);
    return err;
}

static int set_breathing_light_locked(
    int event_source, struct light_device_t* dev, struct light_state_t const* state)
{
    int brightness, blink;
    int onMS, offMS;

    if (!dev)
    {
        return -1;
    }

    switch (state->flashMode)
    {
        case LIGHT_FLASH_TIMED:
            onMS = state->flashOnMS;
            offMS = state->flashOffMS;
            break;
        case LIGHT_FLASH_NONE:
        default:
            onMS = 0;
            offMS = 0;
            break;
    }

    ALOGD("set_breathing_light_locked mode %d, colorRGB=%08X, onMS=%d, offMS=%d\n",
        state->flashMode, state->color, onMS, offMS);

    brightness = rgb_to_brightness(state);
    blink = onMS > 0 && offMS > 0;

    if (blink)
    {
        char buffer[25];
        if (onMS == 1)
        { // Always
            onMS = -1;
        }
        else if (onMS > 1 && onMS <= 250)
        { // Very fast
            onMS = 0;
        }
        else if (onMS > 250 && onMS <= 500)
        { // Fast
            onMS = 1;
        }
        else if (onMS > 500 && onMS <= 1000)
        { // Normal
            onMS = 2;
        }
        else if (onMS > 1000 && onMS <= 2000)
        { // Long
            onMS = 3;
        }
        else if (onMS > 2000 && onMS <= 5000)
        { // Very long
            onMS = 4;
        }
        else if (onMS > 5000)
        {
            onMS = 5;
        }

        // We can not keep the notification button is constantly
        // illuminated. Therefore, disable it.
        if (onMS != -1)
        {
            if (offMS > 1 && offMS <= 250)
            { // Very fast
                offMS = 1;
            }
            else if (offMS > 250 && offMS <= 500)
            { // Fast
                offMS = 2;
            }
            else if (offMS > 500 && offMS <= 1000)
            { // Normal
                offMS = 3;
            }
            else if (offMS > 1000 && offMS <= 2000)
            { // Long
                offMS = 4;
            }
            else if (offMS > 2000 && offMS <= 5000)
            { // Very long
                offMS = 5;
            }
            else if (onMS > 5000)
            {
                offMS = 6;
            }
        }
        else
        {
            offMS = 0;
        }

        snprintf(buffer, sizeof(buffer), "%d %d %d\n", offMS, onMS, onMS);
        ALOGD(
            "fade_time(offMS)=%d fullon_time(onMS)=%d fulloff_time(onMS)=%d\n", offMS, onMS, onMS);
        write_int(LED_CHANNEL, LED_CHANNEL_HOME);
        write_int(LED_GRADE, LED_GRADE_HOME_NOTIFICATION);
        write_str(LED_FADE, buffer);
        write_int(LED_BLINK_MODE, BLINK_MODE_BREATH);
    }
    else
    {
        if (brightness <= 0)
        {
            // Disable Home LED
            write_int(LED_CHANNEL, LED_CHANNEL_HOME);
            write_int(LED_GRADE, 0);
            write_str(LED_FADE, "0 0 0");
            write_int(LED_BLINK_MODE, BLINK_MODE_OFF);
        }
        else
        {
            if (event_source == BREATH_SOURCE_BUTTONS)
            {
                write_int(LED_CHANNEL, LED_CHANNEL_HOME);
                write_int(LED_GRADE, LED_GRADE_BUTTON);
                write_str(LED_FADE, "1 0 0");
                write_int(LED_BLINK_MODE, BLINK_MODE_BREATH_ONCE);
            }
            else if (event_source == BREATH_SOURCE_BATTERY)
            {
                int grade;
                int blink_mode;

                // can't get battery info from state, getting it from sysfs
                int is_charging = 0;
                int capacity = 0;
                char charging_status[15];
                FILE* fp = fopen(BATTERY_CHARGING_STATUS, "rb");
                fgets(charging_status, 14, fp);
                fclose(fp);
                if (strstr(charging_status, "Charging") != NULL
                    || strstr(charging_status, "Full") != NULL)
                {
                    is_charging = 1;
                }
                read_int(BATTERY_CAPACITY, &capacity);
                if (is_charging == 0)
                {
                    // battery low
                    grade = LED_GRADE_HOME_BATTERY_LOW;
                    blink_mode = BLINK_MODE_BREATH;
                }
                else
                {
                    grade = LED_GRADE_HOME_BATTERY;
                    if (capacity < 90)
                    {
                        // battery chagring
                        blink_mode = BLINK_MODE_BREATH;
                    }
                    else
                    {
                        // battery full
                        blink_mode = BLINK_MODE_BREATH_ONCE;
                    }
                }
                write_int(LED_CHANNEL, LED_CHANNEL_HOME);
                write_int(LED_GRADE, grade);
                write_str(LED_FADE, "3 0 4");
                write_int(LED_BLINK_MODE, blink_mode);
            }
        }
    }
    return 0;
}

static void handle_breathing_light_locked(struct light_device_t* dev)
{
    if (is_lit(&g_attention))
    {
        set_breathing_light_locked(BREATH_SOURCE_ATTENTION, dev, &g_attention);
    }
    else if (is_lit(&g_notification))
    {
        set_breathing_light_locked(BREATH_SOURCE_NOTIFICATION, dev, &g_notification);
    }
    else if (is_lit(&g_buttons))
    {
        set_breathing_light_locked(BREATH_SOURCE_BUTTONS, dev, &g_buttons);
    }
    else
    {
        set_breathing_light_locked(BREATH_SOURCE_BATTERY, dev, &g_battery);
    }
}

static int set_light_buttons(struct light_device_t* dev, struct light_state_t const* state)
{
    int brightness = rgb_to_brightness(state);

    if (!dev)
    {
        return -1;
    }
    pthread_mutex_lock(&g_lock);

    g_buttons = *state;

    if (brightness <= 0)
    {
        // Disable buttons
        write_int(LED_CHANNEL, LED_CHANNEL_BUTTON);
        write_int(LED_BLINK_MODE, BLINK_MODE_OFF);
        write_int(LED_BRIGHTNESS, 0);

        handle_breathing_light_locked(dev);
    }
    else
    {
        if (initialized == 0)
        {
            // Kill buttons
            write_int(LED_CHANNEL, LED_CHANNEL_BUTTON);
            write_str(LED_FADE, "0 0 0");
            write_int(LED_BLINK_MODE, BLINK_MODE_BREATH); // Disable all buttons keys (?)
            write_int(LED_BRIGHTNESS, 0); // Disable left key
            initialized = 1;
        }

        // Set home button
        handle_breathing_light_locked(dev);

        // Set buttons
        write_int(LED_CHANNEL, LED_CHANNEL_BUTTON);
        write_int(LED_BRIGHTNESS, brightness);
        write_int(LED_BLINK_MODE, BLINK_MODE_BREATH_ONCE);
    }
    pthread_mutex_unlock(&g_lock);
    return 0;
}

static int set_light_battery(struct light_device_t* dev, struct light_state_t const* state)
{
    pthread_mutex_lock(&g_lock);
    g_battery = *state;
    handle_breathing_light_locked(dev);
    pthread_mutex_unlock(&g_lock);
    return 0;
}

static int set_light_notifications(struct light_device_t* dev, struct light_state_t const* state)
{
    pthread_mutex_lock(&g_lock);

    unsigned int brightness;
    unsigned int color;
    unsigned int rgb[3];

    g_notification = *state;

    // If a brightness has been applied by the user
    brightness = (g_notification.color & 0xFF000000) >> 24;
    if (brightness > 0 && brightness < 0xFF)
    {
        // Retrieve each of the RGB colors
        color = g_notification.color & 0x00FFFFFF;
        rgb[0] = (color >> 16) & 0xFF;
        rgb[1] = (color >> 8) & 0xFF;
        rgb[2] = color & 0xFF;

        // Apply the brightness level
        if (rgb[0] > 0)
            rgb[0] = (rgb[0] * brightness) / 0xFF;
        if (rgb[1] > 0)
            rgb[1] = (rgb[1] * brightness) / 0xFF;
        if (rgb[2] > 0)
            rgb[2] = (rgb[2] * brightness) / 0xFF;

        // Update with the new color
        g_notification.color = (rgb[0] << 16) + (rgb[1] << 8) + rgb[2];
    }

    handle_breathing_light_locked(dev);
    pthread_mutex_unlock(&g_lock);
    return 0;
}

static int set_light_attention(struct light_device_t* dev, struct light_state_t const* state)
{
    pthread_mutex_lock(&g_lock);
    g_attention = *state;
    handle_breathing_light_locked(dev);
    pthread_mutex_unlock(&g_lock);
    return 0;
}

/** Close the lights device */
static int close_lights(struct light_device_t* dev)
{
    if (dev)
    {
        free(dev);
    }
    return 0;
}

/******************************************************************************/

/**
 * module methods
 */

/** Open a new instance of a lights device using name */
static int open_lights(
    const struct hw_module_t* module, char const* name, struct hw_device_t** device)
{
    int (*set_light)(struct light_device_t* dev, struct light_state_t const* state);

    if (0 == strcmp(LIGHT_ID_BACKLIGHT, name))
        set_light = set_light_backlight;
    else if (0 == strcmp(LIGHT_ID_BUTTONS, name))
        set_light = set_light_buttons;
    else if (0 == strcmp(LIGHT_ID_BATTERY, name))
        set_light = set_light_battery;
    else if (0 == strcmp(LIGHT_ID_NOTIFICATIONS, name))
        set_light = set_light_notifications;
    else if (0 == strcmp(LIGHT_ID_ATTENTION, name))
        set_light = set_light_attention;
    else
        return -EINVAL;

    read_int(LCD_MAX_BRIGHTNESS_FILE, &max_brightness);
    if (max_brightness < 0)
    {
        ALOGE("%s: failed to read max panel brightness, fallback to 255!\n", __func__);
        max_brightness = DEFAULT_MAX_BRIGHTNESS;
    }

    pthread_once(&g_init, init_globals);

    struct light_device_t* dev = malloc(sizeof(struct light_device_t));

    if (!dev)
        return -ENOMEM;

    memset(dev, 0, sizeof(*dev));

    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = 0;
    dev->common.module = (struct hw_module_t*)module;
    dev->common.close = (int (*)(struct hw_device_t*))close_lights;
    dev->set_light = set_light;

    *device = (struct hw_device_t*)dev;
    return 0;
}

static struct hw_module_methods_t lights_module_methods = {
    .open = open_lights,
};

/*
 * The lights Module
 */
struct hw_module_t HAL_MODULE_INFO_SYM = {
    .tag = HARDWARE_MODULE_TAG,
    .version_major = 1,
    .version_minor = 0,
    .id = LIGHTS_HARDWARE_MODULE_ID,
    .name = "Nubia Z17 Lights Module",
    .author = "The LineageOS Project, BeYkeRYkt",
    .methods = &lights_module_methods,
};
