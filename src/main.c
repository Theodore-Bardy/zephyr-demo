/**
 * @file main.c
 * @author Theodore Bardy
 *
 * @note This file is part of the Zephyr Demo project from Witekio
 * @brief Entry point of the Zephyr demo application
 */

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/led_strip.h>

#define STRIP_NODE DT_ALIAS(led_strip)
#if DT_NODE_HAS_PROP(DT_ALIAS(led_strip), chain_length)
#define STRIP_NUM_PIXELS DT_PROP(DT_ALIAS(led_strip), chain_length)
#else
#error Unable to determine length of LED strip
#endif

#define RGB(_r, _g, _b) { .r = (_r), .g = (_g), .b = (_b) }

static const struct led_rgb colors[] = {
    RGB(0xFF, 0x00, 0x00), /* red */
    RGB(0xFF, 0xA5, 0x00), /* orange */
    RGB(0xFF, 0xFF, 0x00), /* yellow */
    RGB(0x00, 0xFF, 0x00), /* green */
    RGB(0x00, 0xFF, 0xFF), /* cyan */
    RGB(0x00, 0x00, 0xFF), /* blue */
    RGB(0x80, 0x00, 0x80), /* purple */
    RGB(0xFF, 0x00, 0xFF), /* magenta */
};

static struct led_rgb pixels[STRIP_NUM_PIXELS];
static const struct device *const strip = DEVICE_DT_GET(STRIP_NODE);

#define MAIN_THREAD_STACK_SIZE (512)
#define MAIN_THREAD_PRIORITY   (5)

void main_thread(void *arg1, void *arg2, void *arg3)
{
    size_t strip_color = 0;

    if (!device_is_ready(strip)) {
        LOG_ERR("LED strip is not ready");
    }

    while (1) {
        // Update the strip with the current color
        for (size_t i = 0; i < STRIP_NUM_PIXELS; i++) {
            pixels[i] = colors[strip_color];
        }

        if (led_strip_update_rgb(strip, pixels, STRIP_NUM_PIXELS) < 0) {
            LOG_ERR("Failed to update LED strip");
        }

        // Cycle through the colors
        strip_color = (strip_color + 1) % ARRAY_SIZE(colors);

        // Sleep for a while before updating again
        k_msleep(500);
    }
}
K_THREAD_DEFINE(main_thread_id, MAIN_THREAD_STACK_SIZE, main_thread, NULL, NULL, NULL, MAIN_THREAD_PRIORITY, 0, 0);

int main(void)
{
    LOG_INF("Hello world! %s", CONFIG_BOARD_TARGET);

    while (1) {
        k_sleep(K_FOREVER);
    }

    return 0;
}
