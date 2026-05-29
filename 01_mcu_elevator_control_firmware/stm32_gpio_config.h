/**
 * @file stm32_gpio_config.h
 * @brief STM32 GPIO Pin Configuration Header
 *
 * This header centralizes all GPIO pin definitions for the elevator controller,
 * making it easy to reconfigure pins without modifying the main firmware.
 *
 * Supports STM32F4xx, STM32H7xx, and other STM32 variants.
 */

#ifndef STM32_GPIO_CONFIG_H
#define STM32_GPIO_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h" /* Adjust based on your STM32 variant */

/* ============================================================================
 * ELEVATOR SYSTEM CONFIGURATION
 * ============================================================================ */

#define ELEVATOR_NUM_FLOORS        10
#define ELEVATOR_MIN_FLOOR         1
#define ELEVATOR_MAX_FLOOR         10
#define ELEVATOR_UPDATE_INTERVAL   1000 /* milliseconds */

/* ============================================================================
 * CABIN REQUEST BUTTON PINS (Inputs)
 * ============================================================================
 * These pins connect to buttons inside the elevator cabin for floor selection.
 * Each button corresponds to one floor (1-10).
 * Configuration: Input, Pull-Up (Button press pulls to GND)
 */

#define CABIN_BTN_GPIO_PORT        GPIOA
#define CABIN_BTN_FLOOR1_PIN       GPIO_PIN_0
#define CABIN_BTN_FLOOR2_PIN       GPIO_PIN_1
#define CABIN_BTN_FLOOR3_PIN       GPIO_PIN_2
#define CABIN_BTN_FLOOR4_PIN       GPIO_PIN_3
#define CABIN_BTN_FLOOR5_PIN       GPIO_PIN_4
#define CABIN_BTN_FLOOR6_PIN       GPIO_PIN_5
#define CABIN_BTN_FLOOR7_PIN       GPIO_PIN_6
#define CABIN_BTN_FLOOR8_PIN       GPIO_PIN_7

#define CABIN_BTN_GPIO_PORT_B      GPIOB
#define CABIN_BTN_FLOOR9_PIN       GPIO_PIN_0
#define CABIN_BTN_FLOOR10_PIN      GPIO_PIN_1

/* Array of cabin button pins for easy iteration */
static const uint16_t cabin_btn_pins[ELEVATOR_NUM_FLOORS] = {
    GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3, GPIO_PIN_4,
    GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7,
    GPIO_PIN_0, GPIO_PIN_1  /* PB0, PB1 for floors 9, 10 */
};

static const GPIO_TypeDef *cabin_btn_ports[ELEVATOR_NUM_FLOORS] = {
    GPIOA, GPIOA, GPIOA, GPIOA, GPIOA,
    GPIOA, GPIOA, GPIOA,
    GPIOB, GPIOB  /* Floors 9, 10 on Port B */
};

/* ============================================================================
 * SAFETY INPUT PINS (Inputs)
 * ============================================================================
 * These pins connect to safety sensors and emergency switches.
 * Configuration: Input, Pull-Up (Active High: 1 = Triggered)
 */

#define SAFETY_GPIO_PORT           GPIOC

#define EMERGENCY_STOP_PIN         GPIO_PIN_0  /* PC0 - Emergency Stop Button */
#define DOOR_OBSTRUCTION_PIN       GPIO_PIN_1  /* PC1 - Door Obstruction Sensor */
#define UPPER_LIMIT_SWITCH_PIN     GPIO_PIN_2  /* PC2 - Upper Limit (Top Floor) */
#define LOWER_LIMIT_SWITCH_PIN     GPIO_PIN_3  /* PC3 - Lower Limit (Bottom Floor) */

#define SAFETY_ALL_PINS (EMERGENCY_STOP_PIN | DOOR_OBSTRUCTION_PIN | \
                         UPPER_LIMIT_SWITCH_PIN | LOWER_LIMIT_SWITCH_PIN)

/* ============================================================================
 * MOTOR CONTROL OUTPUT PINS (Outputs)
 * ============================================================================
 * These pins control the elevator motor and door mechanism.
 * Configuration: Output, Push-Pull (0 = Inactive, 1 = Active)
 */

#define MOTOR_GPIO_PORT            GPIOD

#define MOTOR_UP_PIN               GPIO_PIN_0  /* PD0 - Motor UP Control */
#define MOTOR_DOWN_PIN             GPIO_PIN_1  /* PD1 - Motor DOWN Control */
#define DOOR_CONTROL_PIN           GPIO_PIN_2  /* PD2 - Door Open Control */

#define MOTOR_ALL_PINS (MOTOR_UP_PIN | MOTOR_DOWN_PIN | DOOR_CONTROL_PIN)

/* ============================================================================
 * HALL REQUEST PINS (Inputs - Future Extension)
 * ============================================================================
 * These pins connect to buttons on outside floor panels for future expansion.
 * Currently not used in basic implementation.
 */

#define HALL_GPIO_PORT             GPIOE

#define HALL_UP_PIN                GPIO_PIN_0  /* PE0 - Hall UP Request */
#define HALL_DOWN_PIN              GPIO_PIN_1  /* PE1 - Hall DOWN Request */

/* ============================================================================
 * FUNCTION DECLARATIONS
 * ============================================================================ */

/**
 * @brief Initialize all GPIO pins for the elevator controller
 * Configures inputs (pull-up) and outputs (push-pull)
 */
void GPIO_Config_Init(void);

/**
 * @brief Set a motor control output pin
 */
void GPIO_Motor_SetPin(GPIO_TypeDef *port, uint16_t pin);

/**
 * @brief Reset a motor control output pin
 */
void GPIO_Motor_ResetPin(GPIO_TypeDef *port, uint16_t pin);

/**
 * @brief Read a GPIO input pin (cabin button or safety input)
 * @return 1 if pin is HIGH, 0 if pin is LOW
 */
uint8_t GPIO_Input_ReadPin(GPIO_TypeDef *port, uint16_t pin);

/**
 * @brief Get which floor button is currently pressed
 * @return Floor number (1-10) if button is pressed, 0 if no button pressed
 */
int GPIO_CabinButton_GetFloor(void);

/**
 * @brief Read all safety inputs
 * @return Bitmask of safety inputs (bit 0=emergency, bit 1=door, bit 2=upper, bit 3=lower)
 */
uint8_t GPIO_SafetyInputs_Read(void);

#ifdef __cplusplus
}
#endif

#endif /* STM32_GPIO_CONFIG_H */
