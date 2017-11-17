/*
 * Copyright (C) 2008 The Android Open Source Project
 * Copyright (C) 2014 The Linux Foundation. All rights reserved.
 * Copyright (C) 2016 The CyanogenMod Project
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

#define LOG_TAG "lightHAL"

/******************************************************************************/

#define BLINK_MODE_ON   "6"
#define BLINK_MODE_OFF  "2"

#define CHANNEL_BUTTONS 8
#define CHANNEL_RED     16

#define BRIGHTNESS_BUTTONS  3
#define BRIGHTNESS_RED      8

static pthread_once_t g_init = PTHREAD_ONCE_INIT;
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;

static struct light_state_t g_attention;
static struct light_state_t g_notification;
static struct light_state_t g_battery;
static struct light_state_t g_buttons;

#define BREATH_SOURCE_NOTIFICATION  0x01
#define BREATH_SOURCE_BATTERY       0x02
#define BREATH_SOURCE_BUTTONS       0x04
#define BREATH_SOURCE_ATTENTION     0x08
#define BREATH_SOURCE_NONE          0xFF

static int active_states = 0;

static int last_state = BREATH_SOURCE_NONE;

static int g_breathing = 0;


char const*const LCD_FILE
        = "/sys/class/leds/lcd-backlight/brightness";


char const*const BREATH_RED_LED
        = "sys/class/leds/nubia_led/blink_mode";

char const*const BREATH_RED_OUTN
        = "/sys/class/leds/nubia_led/outn";

char const*const BREATH_RED_FADE
        = "/sys/class/leds/nubia_led/fade_parameter";

char const*const BREATH_RED_GRADE
        = "/sys/class/leds/nubia_led/grade_parameter";

char const*const BREATH_RED_DUTY_PCTS
        = "/sys/class/leds/nubia_led/duty_pcts";

char const*const BREATH_RED_START_IDX
        = "/sys/class/leds/nubia_led/start_idx";

char const*const BREATH_RED_PAUSE_LO
        = "/sys/class/leds/nubia_led/pause_lo";

char const*const BREATH_RED_PAUSE_HI
        = "/sys/class/leds/nubia_led/pause_hi";

char const*const BREATH_RED_RAMP_STEP_MS
        = "/sys/class/leds/nubia_led/ramp_step_ms";


char const*const BATTERY_CAPACITY
        = "/sys/class/power_supply/battery/capacity";

char const*const BATTERY_IS_CHARGING
        = "/sys/class/power_supply/battery/status";


#define RAMP_SIZE 8
static int BRIGHTNESS_RAMP[RAMP_SIZE]
        = { 0, 12, 25, 37, 50, 72, 85, 100 };
#define RAMP_STEP_DURATION 50

/**
 * Device methods
 */

static void init_globals(void)
{
    // Init the mutex
    pthread_mutex_init(&g_lock, NULL);
}

static int write_int(char const* path, int value)
{
    int fd;
    static int already_warned = 0;

    fd = open(path, O_RDWR);
    if (fd >= 0) {
        char buffer[20];
        int bytes = sprintf(buffer, "%d\n", value);
        int amt = write(fd, buffer, bytes);
        close(fd);
        return amt == -1 ? -errno : 0;
    } else {
        if (already_warned == 0) {
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
    if (fd >= 0) {
        char buffer[PAGE_SIZE];
        int bytes = sprintf(buffer, "%s\n", value);
        int amt = write(fd, buffer, bytes);
        close(fd);
        return amt == -1 ? -errno : 0;
    } else {
        if (already_warned == 0) {
            ALOGE("write_str failed to open %s\n", path);
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
    return ((77*((color>>16)&0x00ff))
            + (150*((color>>8)&0x00ff)) + (29*(color&0x00ff))) >> 8;
}

static char* get_scaled_duty_pcts(int brightness)
{
    char *buf = malloc(5 * RAMP_SIZE * sizeof(char));
    char *pad = "";
    int i = 0;

    memset(buf, 0, 5 * RAMP_SIZE * sizeof(char));

    for (i = 0; i < RAMP_SIZE; i++) {
        char temp[5] = "";
        snprintf(temp, sizeof(temp), "%s%d", pad, (BRIGHTNESS_RAMP[i] * brightness / 255));
        strcat(buf, temp);
        pad = ",";
    }
    ALOGV("%s: brightness=%d duty=%s", __func__, brightness, buf);
    return buf;
}

static int set_light_backlight(struct light_device_t* dev,
        struct light_state_t const* state)
{
    int err = 0;
    int brightness = rgb_to_brightness(state);
    pthread_mutex_lock(&g_lock);
    err = write_int(LCD_FILE, brightness);
    pthread_mutex_unlock(&g_lock);
    return err;
}

static int set_breath_light_locked(int event_source,
        struct light_state_t const* state)
{
    int brightness, blink;
    int onMS, offMS, stepDuration, pauseHi;
    unsigned int colorRGB;
    char *duty;

    switch (state->flashMode) {
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

    blink = onMS > 0 && offMS > 0;

    colorRGB = state->color;

    brightness = (colorRGB >> 16) & 0xFF;

    if (brightness > 0) {
        active_states |= event_source;
    } else {
        active_states &= ~event_source;

        write_int(BREATH_RED_OUTN, CHANNEL_BUTTONS);
        write_str(BREATH_RED_FADE, "1 0 0");
        write_str(BREATH_RED_LED, BLINK_MODE_OFF);

        write_int(BREATH_RED_OUTN, CHANNEL_RED);
        write_str(BREATH_RED_FADE, "1 0 0");
        write_str(BREATH_RED_LED, BLINK_MODE_OFF);

        if (active_states == 0) {
            last_state = BREATH_SOURCE_NONE;
            return 0;
        }
    }

    if (active_states & BREATH_SOURCE_NOTIFICATION) {
        state = &g_notification;
        last_state = BREATH_SOURCE_NOTIFICATION;
    } else if (active_states & BREATH_SOURCE_BATTERY) {
        state = &g_battery;
        last_state = BREATH_SOURCE_BATTERY;
    } else if (active_states & BREATH_SOURCE_BUTTONS) {
        if (last_state == BREATH_SOURCE_BUTTONS)
            return 0;

        state = &g_buttons;
        last_state = BREATH_SOURCE_BUTTONS;
    } else if (active_states & BREATH_SOURCE_ATTENTION) {
        state = &g_attention;
        last_state = BREATH_SOURCE_ATTENTION;
    } else {
        last_state = BREATH_SOURCE_NONE;
        ALOGE(" Unknown state");
        return 0;
    }

    if ((active_states & BREATH_SOURCE_BUTTONS) == 0) {
        ALOGE(" Red led on");
        write_int(BREATH_RED_OUTN, CHANNEL_RED);
        write_str(BREATH_RED_FADE, "4 5 0");
        write_str(BREATH_RED_LED, BLINK_MODE_ON);

        if (blink) {
            duty = get_scaled_duty_pcts(brightness);
            stepDuration = RAMP_STEP_DURATION;
            pauseHi = onMS - (stepDuration * RAMP_SIZE * 2);
            if (stepDuration * RAMP_SIZE * 2 > onMS) {
                stepDuration = onMS / (RAMP_SIZE * 2);
                pauseHi = 0;
            }

            write_int(BREATH_RED_START_IDX, 0);
            write_str(BREATH_RED_DUTY_PCTS, duty);
            write_int(BREATH_RED_PAUSE_LO, offMS);
            write_int(BREATH_RED_PAUSE_HI, pauseHi);
            write_int(BREATH_RED_RAMP_STEP_MS, stepDuration);

            free(duty);
        } else {
            write_int(BREATH_RED_GRADE, brightness);
        }
    } else {
        ALOGE(" Button led on");
        write_int(BREATH_RED_OUTN, CHANNEL_BUTTONS);
        write_str(BREATH_RED_FADE, "1 0 0");
        write_int(BREATH_RED_GRADE, BRIGHTNESS_BUTTONS);
        write_str(BREATH_RED_LED, BLINK_MODE_ON);

        write_int(BREATH_RED_OUTN, CHANNEL_RED);
        write_str(BREATH_RED_FADE, "1 0 0");
        write_int(BREATH_RED_GRADE, BRIGHTNESS_RED);
        write_str(BREATH_RED_LED, BLINK_MODE_ON);
    }

    return 0;
}

static int set_light_buttons(struct light_device_t* dev,
        struct light_state_t const* state)
{
    int err = 0;
    int brightness = rgb_to_brightness(state);
    pthread_mutex_lock(&g_lock);
    g_buttons = *state;
    set_breath_light_locked(BREATH_SOURCE_BUTTONS, &g_buttons);
    pthread_mutex_unlock(&g_lock);
    return err;
}

static int set_light_battery(struct light_device_t* dev,
        struct light_state_t const* state)
{
    pthread_mutex_lock(&g_lock);
    g_battery = *state;
    set_breath_light_locked(BREATH_SOURCE_BATTERY, &g_battery);
    pthread_mutex_unlock(&g_lock);
    return 0;
}

static int set_light_notifications(struct light_device_t* dev,
        struct light_state_t const* state)
{
    pthread_mutex_lock(&g_lock);
    g_notification = *state;
    set_breath_light_locked(BREATH_SOURCE_NOTIFICATION, &g_notification);
    pthread_mutex_unlock(&g_lock);
    return 0;
}

static int set_light_attention(struct light_device_t* dev,
        struct light_state_t const* state)
{
    pthread_mutex_lock(&g_lock);
    g_attention = *state;
    pthread_mutex_unlock(&g_lock);
    return 0;
}

/** Close the lights device */
static int close_lights(struct light_device_t *dev)
{
    if (dev) {
        free(dev);
    }
    return 0;
}


/**
 * Module methods
 */

/** Open a new instance of a lights device using name */
static int open_lights(const struct hw_module_t* module, char const* name,
        struct hw_device_t** device)
{
    int (*set_light)(struct light_device_t* dev,
            struct light_state_t const* state);

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

    pthread_once(&g_init, init_globals);

    struct light_device_t *dev = malloc(sizeof(struct light_device_t));
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
    .open =  open_lights,
};

/*
 * The lights Module
 */
struct hw_module_t HAL_MODULE_INFO_SYM = {
    .tag = HARDWARE_MODULE_TAG,
    .version_major = 1,
    .version_minor = 0,
    .id = LIGHTS_HARDWARE_MODULE_ID,
    .name = "Lights Module for Nubia Z11",
    .author = "Parheliamm, XiNGRZ",
    .methods = &lights_module_methods,
};
