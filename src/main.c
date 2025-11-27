/**
 * @file      main.c
 * @author    Theodore Bardy
 *
 * @note      This file is part of Witekio's Zephyr Demo project
 * @brief     Entry point of the Witekio's Zephyr Demo
 */

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#include <esp_sleep.h>

#include "led.h"
#include "wifi_agent.h"
#include "ota_agent.h"

#define BT0_NODE DT_ALIAS(bt0)
#if !DT_NODE_HAS_STATUS_OKAY(BT0_NODE)
#error Unsupported board: bt0 devicetree alias is not defined
#endif
static const struct gpio_dt_spec bt0 = GPIO_DT_SPEC_GET(BT0_NODE, gpios);
static struct gpio_callback bt0_cb_data;
K_SEM_DEFINE(button_pressed_sem, 0, 1);


static void cooldown_expired(struct k_work *work)
{
    ARG_UNUSED(work);
    k_sem_give(&button_pressed_sem);
}
static K_WORK_DELAYABLE_DEFINE(cooldown_work, cooldown_expired);

void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    // Debounce the button press by scheduling a delayed work
    k_work_reschedule(&cooldown_work, K_MSEC(15));
}

#define APP_THREAD_STACK_SIZE (2048)
#define APP_THREAD_PRIORITY   (5)

static void enter_deep_sleep(void)
{
    LOG_INF("Preparing for deep sleep.\n");

    esp_sleep_enable_ext0_wakeup(bt0.pin, 1);

    LOG_INF("Entering deep sleep NOW.\n");
    k_msleep(2000);
    esp_deep_sleep_start();

    // Code never returns here
}

void
app_thread(void *arg1, void *arg2, void *arg3)
{
    if (!gpio_is_ready_dt(&bt0))
    {
        LOG_ERR("Error: button device %s is not ready", bt0.port->name);
        goto ERROR;
    }
    int ret = gpio_pin_configure_dt(&bt0, GPIO_INPUT);
    if (ret != 0)
    {
        LOG_ERR("Error %d: failed to configure %s pin %d",
                ret,
                bt0.port->name,
                bt0.pin);
        goto ERROR;
    }
    ret = gpio_pin_interrupt_configure_dt(&bt0, GPIO_INT_LEVEL_ACTIVE);
    if (ret != 0)
    {
        LOG_ERR("Error %d: failed to configure interrupt on %s pin %d",
                ret,
                bt0.port->name,
                bt0.pin);
        goto ERROR;
    }
    gpio_init_callback(&bt0_cb_data, button_pressed, BIT(bt0.pin));
    gpio_add_callback(bt0.port, &bt0_cb_data);
    ui_led_set(UI_LED_COLOR_OFF);

    while (1)
    {
        k_sem_take(&button_pressed_sem, K_FOREVER);
        LOG_INF("Button pressed, connect to Wi-Fi update...");
        wifi_agent_connect();

        k_sem_take(&button_pressed_sem, K_FOREVER);
        LOG_INF("Button pressed again, putting device to sleep...");
        wifi_agent_disconnect();
        k_msleep(2000);
        ui_led_set(UI_LED_COLOR_OFF);

        enter_deep_sleep();
    }

ERROR:
    while (1)
    {
        // Inifinite loop on error
        k_sleep(K_FOREVER);
        z_fatal_error(K_ERR_CPU_EXCEPTION, 0);
    }    
}
K_THREAD_DEFINE(app_thread_id,
                APP_THREAD_STACK_SIZE,
                app_thread,
                NULL,
                NULL,
                NULL,
                APP_THREAD_PRIORITY,
                0,
                0);

int
main (void)
{
    LOG_INF("Witekio Zephyr's app running on %s", CONFIG_BOARD_TARGET);

    // Initialize subsystems
    wifi_agent_init();
    ota_agent_init();
    ui_led_init();

    while (1)
    {
        k_sleep(K_FOREVER);
    }

    return 0;
}
