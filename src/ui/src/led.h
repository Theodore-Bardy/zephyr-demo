/**
 * @file      led.h
 * @author    Theodore Bardy
 *
 * @note      This file is part of Witekio's Zephyr Demo project
 * @brief     LED interface
 */

#ifndef LED_H
#define LED_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    typedef enum
    {
        UI_LED_COLOR_OFF,
        UI_LED_COLOR_ON,
#ifdef CONFIG_LED_STRIP
        UI_LED_COLOR_RED,
        UI_LED_COLOR_ORANGE,
        UI_LED_COLOR_YELLOW,
        UI_LED_COLOR_GREEN,
        UI_LED_COLOR_CYAN,
        UI_LED_COLOR_BLUE,
        UI_LED_COLOR_PURPLE,
        UI_LED_COLOR_MAGENTA,
        UI_LED_COLOR_COUNT
#endif // CONFIG_LED_STRIP
    } ui_led_tone_t;

    /**
     * @brief Initializes the LED interface
     * @return true if initialization success, false otherwise
     */
    bool ui_led_init(void);

    /**
     * @brief Sets the LED
     * @param tone The tone to set
     * @return true if the tone was set successfully, false otherwise
     */
    bool ui_led_set(ui_led_tone_t tone);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // LED_H
