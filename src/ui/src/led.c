/**
 * @file      led.c
 * @author    Theodore Bardy
 *
 * @note      This file is part of Witekio's Zephyr Demo project
 * @brief     LED interface
 */

#include "led.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ui_led);

#include <zephyr/device.h>

#ifdef CONFIG_LED_STRIP
#include <zephyr/drivers/led_strip.h>

#define STRIP_NODE DT_ALIAS(led_strip)
#if DT_NODE_HAS_PROP(DT_ALIAS(led_strip), chain_length)
#define STRIP_NUM_PIXELS DT_PROP(DT_ALIAS(led_strip), chain_length)
#else
#error Unable to determine length of LED strip
#endif

static struct led_rgb             pixels[STRIP_NUM_PIXELS];
static const struct device *const led_interface = DEVICE_DT_GET(STRIP_NODE);
#endif // CONFIG_LED_STRIP

#define RGB(_r, _g, _b)                 \
    {                                   \
        .r = (_r), .g = (_g), .b = (_b) \
    }

static const struct led_rgb colors[] = {
    RGB(0x00, 0x00, 0x00), /* off */
    RGB(0xFF, 0xFF, 0xFF), /* on */
    RGB(0xFF, 0x00, 0x00), /* red */
    RGB(0xFF, 0xA5, 0x00), /* orange */
    RGB(0xFF, 0xFF, 0x00), /* yellow */
    RGB(0x00, 0xFF, 0x00), /* green */
    RGB(0x00, 0xFF, 0xFF), /* cyan */
    RGB(0x00, 0x00, 0xFF), /* blue */
    RGB(0x80, 0x00, 0x80), /* purple */
    RGB(0xFF, 0x00, 0xFF), /* magenta */
};

bool
ui_led_init(void) {
    if (!device_is_ready(led_interface))
    {
        LOG_ERR("LED interface is not ready");
        return false;
    }
    return true;
}

bool ui_led_set(ui_led_tone_t tone)
{
#ifdef CONFIG_LED_STRIP
    if (tone >= UI_LED_COLOR_COUNT)
    {
        LOG_ERR("Invalid LED tone index: %d", tone);
        return false;
    }

    for (size_t i = 0; i < STRIP_NUM_PIXELS; i++)
    {
        pixels[i] = colors[tone];
    }

    if (led_strip_update_rgb(led_interface, pixels, STRIP_NUM_PIXELS) != 0)
    {
        LOG_ERR("Failed to update LED strip");
        return false;
    }

    return true;
#else // CONFIG_LED_STRIP
    LOG_WRN("LED strip support is not enabled");
    return false;
#endif // CONFIG_LED_STRIP
}
