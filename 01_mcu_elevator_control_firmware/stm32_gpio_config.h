/**
 * @file stm32_gpio_config.h
 * @brief STM32 GPIO and hardware abstraction for the elevator controller.
 *
 * This module keeps board-specific pin mapping, input polarity, debouncing,
 * optional floor-position sensors, and safe output control separate from the
 * elevator state-machine logic.
 *
 * Import this header into a STM32CubeIDE project generated for your exact MCU
 * family. Change ELEVATOR_STM32_HAL_HEADER or this include if your project uses
 * another STM32 HAL family header.
 */

#ifndef STM32_GPIO_CONFIG_H
#define STM32_GPIO_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#ifndef ELEVATOR_STM32_HAL_HEADER
#define ELEVATOR_STM32_HAL_HEADER "stm32f4xx_hal.h"
#endif
#include ELEVATOR_STM32_HAL_HEADER

/* ============================================================================
 * ELEVATOR SYSTEM CONFIGURATION
 * ============================================================================ */

#define ELEVATOR_NUM_FLOORS        10U
#define ELEVATOR_MIN_FLOOR         1U
#define ELEVATOR_MAX_FLOOR         10U
#define ELEVATOR_UPDATE_INTERVAL   1000U /* milliseconds */
#define ELEVATOR_DEBOUNCE_TICKS    20U   /* 20 ms when sampled from SysTick */

/*
 * Set to 1 when real floor sensors are wired and the pins below are valid for
 * your board. When 0, the shared simulation controller still advances one floor
 * per update cycle.
 */
#ifndef ELEVATOR_ENABLE_FLOOR_SENSORS
#define ELEVATOR_ENABLE_FLOOR_SENSORS 0
#endif

/* Input polarity. GPIO_PULLUP + normally-open switch to GND is active-low. */
#define ELEVATOR_INPUT_ACTIVE_HIGH  GPIO_PIN_SET
#define ELEVATOR_INPUT_ACTIVE_LOW   GPIO_PIN_RESET
#define ELEVATOR_BUTTON_ACTIVE_STATE ELEVATOR_INPUT_ACTIVE_LOW
#define ELEVATOR_SAFETY_ACTIVE_STATE ELEVATOR_INPUT_ACTIVE_LOW
#define ELEVATOR_FLOOR_SENSOR_ACTIVE_STATE ELEVATOR_INPUT_ACTIVE_LOW

/* ============================================================================
 * CABIN REQUEST BUTTON PINS (Inputs, pull-up, active-low)
 * ============================================================================ */

#define CABIN_FLOOR1_PORT          GPIOA
#define CABIN_FLOOR1_PIN           GPIO_PIN_0
#define CABIN_FLOOR2_PORT          GPIOA
#define CABIN_FLOOR2_PIN           GPIO_PIN_1
#define CABIN_FLOOR3_PORT          GPIOA
#define CABIN_FLOOR3_PIN           GPIO_PIN_2
#define CABIN_FLOOR4_PORT          GPIOA
#define CABIN_FLOOR4_PIN           GPIO_PIN_3
#define CABIN_FLOOR5_PORT          GPIOA
#define CABIN_FLOOR5_PIN           GPIO_PIN_4
#define CABIN_FLOOR6_PORT          GPIOA
#define CABIN_FLOOR6_PIN           GPIO_PIN_5
#define CABIN_FLOOR7_PORT          GPIOA
#define CABIN_FLOOR7_PIN           GPIO_PIN_6
#define CABIN_FLOOR8_PORT          GPIOA
#define CABIN_FLOOR8_PIN           GPIO_PIN_7
#define CABIN_FLOOR9_PORT          GPIOB
#define CABIN_FLOOR9_PIN           GPIO_PIN_0
#define CABIN_FLOOR10_PORT         GPIOB
#define CABIN_FLOOR10_PIN          GPIO_PIN_1

/* ============================================================================
 * SAFETY INPUT PINS (Inputs, pull-up, active-low fail-safe wiring)
 * ============================================================================ */

#define EMERGENCY_STOP_PORT        GPIOC
#define EMERGENCY_STOP_PIN         GPIO_PIN_0
#define DOOR_OBSTRUCTION_PORT      GPIOC
#define DOOR_OBSTRUCTION_PIN       GPIO_PIN_1
#define UPPER_LIMIT_SWITCH_PORT    GPIOC
#define UPPER_LIMIT_SWITCH_PIN     GPIO_PIN_2
#define LOWER_LIMIT_SWITCH_PORT    GPIOC
#define LOWER_LIMIT_SWITCH_PIN     GPIO_PIN_3

/* ============================================================================
 * MOTOR / DOOR OUTPUT PINS (Outputs, push-pull, active-high command signals)
 * ============================================================================ */

#define MOTOR_UP_PORT              GPIOD
#define MOTOR_UP_PIN               GPIO_PIN_0
#define MOTOR_DOWN_PORT            GPIOD
#define MOTOR_DOWN_PIN             GPIO_PIN_1
#define DOOR_CONTROL_PORT          GPIOD
#define DOOR_CONTROL_PIN           GPIO_PIN_2
#define MOTOR_DIRECTION_CHANGE_DEADTIME_MS 100U

/* ============================================================================
 * HALL REQUEST PINS (Inputs, pull-up, active-low)
 *
 * This simple hardware profile provides one UP and one DOWN request input. If
 * your panel has one hall button per floor, expand these definitions into arrays
 * the same way cabin buttons are represented in stm32_gpio_config.c.
 * ============================================================================ */

#define HALL_UP_PORT               GPIOE
#define HALL_UP_PIN                GPIO_PIN_0
#define HALL_DOWN_PORT             GPIOE
#define HALL_DOWN_PIN              GPIO_PIN_1
#define HALL_UP_FLOOR              ELEVATOR_MIN_FLOOR
#define HALL_DOWN_FLOOR            ELEVATOR_MAX_FLOOR

/* ============================================================================
 * OPTIONAL FLOOR SENSOR PINS (Inputs, pull-up, active-low)
 *
 * Default mapping uses GPIOF for clarity. Verify the selected MCU/package has
 * these pins, then set ELEVATOR_ENABLE_FLOOR_SENSORS to 1 in your project.
 * ============================================================================ */

#define FLOOR_SENSOR1_PORT         GPIOF
#define FLOOR_SENSOR1_PIN          GPIO_PIN_0
#define FLOOR_SENSOR2_PORT         GPIOF
#define FLOOR_SENSOR2_PIN          GPIO_PIN_1
#define FLOOR_SENSOR3_PORT         GPIOF
#define FLOOR_SENSOR3_PIN          GPIO_PIN_2
#define FLOOR_SENSOR4_PORT         GPIOF
#define FLOOR_SENSOR4_PIN          GPIO_PIN_3
#define FLOOR_SENSOR5_PORT         GPIOF
#define FLOOR_SENSOR5_PIN          GPIO_PIN_4
#define FLOOR_SENSOR6_PORT         GPIOF
#define FLOOR_SENSOR6_PIN          GPIO_PIN_5
#define FLOOR_SENSOR7_PORT         GPIOF
#define FLOOR_SENSOR7_PIN          GPIO_PIN_6
#define FLOOR_SENSOR8_PORT         GPIOF
#define FLOOR_SENSOR8_PIN          GPIO_PIN_7
#define FLOOR_SENSOR9_PORT         GPIOF
#define FLOOR_SENSOR9_PIN          GPIO_PIN_8
#define FLOOR_SENSOR10_PORT        GPIOF
#define FLOOR_SENSOR10_PIN         GPIO_PIN_9

/* ============================================================================
 * DATA TYPES
 * ============================================================================ */

typedef struct
{
    bool emergency_stop;
    bool door_obstruction;
    bool upper_limit_switch;
    bool lower_limit_switch;
} ElevatorSafetyInputs;

typedef enum
{
    ELEVATOR_OUTPUT_STOPPED = 0,
    ELEVATOR_OUTPUT_MOVING_UP,
    ELEVATOR_OUTPUT_MOVING_DOWN
} ElevatorOutputDirection;

/* ============================================================================
 * FUNCTION DECLARATIONS
 * ============================================================================ */

void Elevator_GPIO_Init(void);
void Elevator_GPIO_DebounceTick(void);
void Elevator_GPIO_AllOutputsOff(void);

int Elevator_GPIO_GetCabinRequestFloor(void);
int Elevator_GPIO_GetHallUpRequestFloor(void);
int Elevator_GPIO_GetHallDownRequestFloor(void);
int Elevator_GPIO_GetMeasuredFloor(void);

ElevatorSafetyInputs Elevator_GPIO_ReadSafetyInputs(void);

bool Elevator_GPIO_SetMotorDirection(ElevatorOutputDirection direction);
void Elevator_GPIO_SetDoorOpen(bool door_open);
void Elevator_GPIO_ServiceOutputs(void);

#ifdef __cplusplus
}
#endif

#endif /* STM32_GPIO_CONFIG_H */
