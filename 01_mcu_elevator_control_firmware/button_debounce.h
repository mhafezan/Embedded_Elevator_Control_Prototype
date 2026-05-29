/**
 * @file button_debounce.h
 * @brief Button Debouncing Module for STM32 Elevator Controller
 *
 * This module provides robust button debouncing to eliminate false triggers
 * from electrical noise and switch bouncing.
 *
 * Each button is sampled multiple times and only registered as pressed when
 * all samples are consistent, ensuring reliable input reading.
 */

#ifndef BUTTON_DEBOUNCE_H
#define BUTTON_DEBOUNCE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"

/* ============================================================================
 * DEBOUNCE CONFIGURATION
 * ============================================================================ */

#define DEBOUNCE_SAMPLES    3    /* Number of consecutive samples to confirm button press */
#define DEBOUNCE_SAMPLE_MS  10   /* Milliseconds between each sample */
#define DEBOUNCE_TIMER_MS   (DEBOUNCE_SAMPLES * DEBOUNCE_SAMPLE_MS)

/* ============================================================================
 * BUTTON STATE MACHINE
 * ============================================================================ */

typedef enum
{
    BTN_STATE_RELEASED = 0,
    BTN_STATE_PRESSING,
    BTN_STATE_PRESSED,
    BTN_STATE_RELEASING
} ButtonState;

typedef struct
{
    GPIO_TypeDef *gpio_port;
    uint16_t gpio_pin;
    ButtonState state;
    uint8_t sample_count;
    uint16_t timer_counter;
    bool pressed;      /* Current debounced state */
    bool pressed_edge; /* Edge detector: went from released to pressed */
} ButtonDebouncer;

/* ============================================================================
 * FUNCTION DECLARATIONS
 * ============================================================================ */

/**
 * @brief Initialize a button debouncer
 * @param debouncer Pointer to ButtonDebouncer structure
 * @param gpio_port GPIO port where button is connected
 * @param gpio_pin GPIO pin number
 */
void ButtonDebounce_Init(ButtonDebouncer *debouncer, GPIO_TypeDef *gpio_port, uint16_t gpio_pin);

/**
 * @brief Update button debouncer state machine
 * Should be called periodically (e.g., every 1ms from SysTick)
 * @param debouncer Pointer to ButtonDebouncer structure
 */
void ButtonDebounce_Update(ButtonDebouncer *debouncer);

/**
 * @brief Check if button is currently pressed (debounced)
 * @param debouncer Pointer to ButtonDebouncer structure
 * @return true if button is pressed, false otherwise
 */
bool ButtonDebounce_IsPressed(const ButtonDebouncer *debouncer);

/**
 * @brief Check for button press edge (press detected)
 * Returns true only once per press cycle
 * @param debouncer Pointer to ButtonDebouncer structure
 * @return true if button was just pressed, false otherwise
 */
bool ButtonDebounce_GetPressEdge(ButtonDebouncer *debouncer);

/**
 * @brief Reset button state (useful after handling a press)
 * @param debouncer Pointer to ButtonDebouncer structure
 */
void ButtonDebounce_Reset(ButtonDebouncer *debouncer);

#ifdef __cplusplus
}
#endif

#endif /* BUTTON_DEBOUNCE_H */
