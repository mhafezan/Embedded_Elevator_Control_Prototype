/**
 * @file stm32_main.c
 * @brief STM32 Elevator Controller Firmware Entry Point
 *
 * This file shows how to run the shared elevator controller on STM32 hardware.
 * It keeps interrupt work short, samples/debounces GPIO every millisecond, runs
 * the controller at a deterministic interval, and sends safe motor/door commands
 * through the board abstraction in stm32_gpio_config.c.
 *
 * Hardware profile used by the default configuration:
 * - Cabin request buttons: PA0-PA7, PB0-PB1, active-low with pull-ups
 * - Safety inputs: PC0-PC3, active-low fail-safe wiring with pull-ups
 * - Motor/door command outputs: PD0-PD2, active-high logic-level commands
 * - Hall request shortcut inputs: PE0 and PE1, active-low with pull-ups
 * - Optional floor sensors: GPIOF pins, disabled by default
 *
 * Import this file, stm32_gpio_config.c/.h, elevator_controller.c/.h into a
 * STM32CubeIDE project generated for your exact board and MCU family.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "elevator_controller.h"
#include "stm32_gpio_config.h"

#ifndef ELEVATOR_ENABLE_IWDG
#define ELEVATOR_ENABLE_IWDG 0
#endif

/* ============================================================================
 * PRIVATE DATA
 * ============================================================================ */

static ElevatorController g_elevator_controller;
static volatile uint32_t g_update_counter_ms = 0;
static volatile bool g_controller_update_due = false;
static volatile bool g_firmware_initialized = false;

#if ELEVATOR_ENABLE_IWDG
static IWDG_HandleTypeDef g_iwdg;
#endif

/* ============================================================================
 * PRIVATE FUNCTION PROTOTYPES
 * ============================================================================ */

void SystemClock_Config(void);
void SysTick_Handler(void);
void Error_Handler(void);

static ElevatorInputs App_ReadInputs(void);
static void App_UpdateOutputs(const ElevatorController *controller,
                              const ElevatorSafetyInputs *safety_inputs);
static void App_PrintStatus(const ElevatorController *controller);
static void App_WatchdogInit(void);
static void App_WatchdogRefresh(void);

/* ============================================================================
 * INPUT / OUTPUT ADAPTERS
 * ============================================================================ */

static ElevatorInputs App_ReadInputs(void)
{
    ElevatorInputs inputs;
    ElevatorSafetyInputs safety_inputs = Elevator_GPIO_ReadSafetyInputs();

    inputs.cabin_request_floor = Elevator_GPIO_GetCabinRequestFloor();
    inputs.hall_up_request_floor = Elevator_GPIO_GetHallUpRequestFloor();
    inputs.hall_down_request_floor = Elevator_GPIO_GetHallDownRequestFloor();

    inputs.door_obstruction = safety_inputs.door_obstruction;
    inputs.emergency_stop = safety_inputs.emergency_stop;
    inputs.upper_limit_switch = safety_inputs.upper_limit_switch;
    inputs.lower_limit_switch = safety_inputs.lower_limit_switch;
    inputs.measured_floor = Elevator_GPIO_GetMeasuredFloor();

    return inputs;
}

static void App_UpdateOutputs(const ElevatorController *controller,
                              const ElevatorSafetyInputs *safety_inputs)
{
    ElevatorOutputDirection requested_direction = ELEVATOR_OUTPUT_STOPPED;

    if (safety_inputs->emergency_stop ||
        (safety_inputs->upper_limit_switch && controller->motor == MOTOR_UP) ||
        (safety_inputs->lower_limit_switch && controller->motor == MOTOR_DOWN))
    {
        Elevator_GPIO_AllOutputsOff();
        return;
    }

    switch (controller->motor)
    {
    case MOTOR_UP:
        requested_direction = ELEVATOR_OUTPUT_MOVING_UP;
        break;

    case MOTOR_DOWN:
        requested_direction = ELEVATOR_OUTPUT_MOVING_DOWN;
        break;

    case MOTOR_STOP:
    default:
        requested_direction = ELEVATOR_OUTPUT_STOPPED;
        break;
    }

    (void)Elevator_GPIO_SetMotorDirection(requested_direction);
    Elevator_GPIO_SetDoorOpen(controller->door_open);
}

static void App_PrintStatus(const ElevatorController *controller)
{
#ifdef DEBUG_UART_ENABLED
    printf("Floor: %d | Target: %d | State: %s | Motor: %s | Door: %s\r\n",
           controller->current_floor,
           controller->target_floor,
           Elevator_StateToString(controller->state),
           Elevator_MotorToString(controller->motor),
           controller->door_open ? "OPEN" : "CLOSED");
#else
    (void)controller;
#endif
}

/* ============================================================================
 * WATCHDOG SUPPORT
 * ============================================================================ */

static void App_WatchdogInit(void)
{
#if ELEVATOR_ENABLE_IWDG
    g_iwdg.Instance = IWDG;
    g_iwdg.Init.Prescaler = IWDG_PRESCALER_64;
    g_iwdg.Init.Reload = 2500;

    if (HAL_IWDG_Init(&g_iwdg) != HAL_OK)
    {
        Error_Handler();
    }
#endif
}

static void App_WatchdogRefresh(void)
{
#if ELEVATOR_ENABLE_IWDG
    if (HAL_IWDG_Refresh(&g_iwdg) != HAL_OK)
    {
        Error_Handler();
    }
#endif
}

/* ============================================================================
 * SYSTEM CLOCK CONFIGURATION
 * ============================================================================ */

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /*
     * Default STM32F4-style clock tree. Replace with the STM32CubeMX-generated
     * SystemClock_Config() for your exact board before flashing real hardware.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 16;
    RCC_OscInitStruct.PLL.PLLN = 360;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 7;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

#if defined(PWR_CR_ODEN)
    if (HAL_PWREx_EnableOverDrive() != HAL_OK)
    {
        Error_Handler();
    }
#endif

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                   RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
    {
        Error_Handler();
    }
}

/* ============================================================================
 * SYSTICK INTERRUPT HANDLER
 * ============================================================================ */

void SysTick_Handler(void)
{
    HAL_IncTick();

    if (!g_firmware_initialized)
    {
        return;
    }

    Elevator_GPIO_DebounceTick();
    Elevator_GPIO_ServiceOutputs();

    g_update_counter_ms++;
    if (g_update_counter_ms >= ELEVATOR_UPDATE_INTERVAL)
    {
        g_update_counter_ms = 0;
        g_controller_update_due = true;
    }
}

/* ============================================================================
 * MAIN FUNCTION
 * ============================================================================ */

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    Elevator_GPIO_Init();
    Elevator_Init(&g_elevator_controller);
    App_WatchdogInit();
    g_firmware_initialized = true;

    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000U);
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

    while (1)
    {
        if (g_controller_update_due)
        {
            __disable_irq();
            g_controller_update_due = false;
            __enable_irq();

            ElevatorInputs inputs = App_ReadInputs();
            ElevatorSafetyInputs safety_inputs = Elevator_GPIO_ReadSafetyInputs();

            Elevator_Update(&g_elevator_controller, inputs);
            App_UpdateOutputs(&g_elevator_controller, &safety_inputs);
            App_PrintStatus(&g_elevator_controller);
            App_WatchdogRefresh();
        }

        __WFI();
    }
}

/* ============================================================================
 * ERROR HANDLER
 * ============================================================================ */

void Error_Handler(void)
{
    __disable_irq();
    Elevator_GPIO_AllOutputsOff();

    while (1)
    {
        /* Stay in a safe stopped state. A hardware watchdog can reset the MCU. */
    }
}

/* ============================================================================
 * STM32CUBEIDE INTEGRATION NOTES
 * ============================================================================
 * 1. Generate a CubeIDE project for the exact STM32 part/board.
 * 2. Replace SystemClock_Config() with the CubeMX-generated clock function.
 * 3. Add elevator_controller.c and stm32_gpio_config.c to the build.
 * 4. Set ELEVATOR_STM32_HAL_HEADER if the project does not use stm32f4xx_hal.h.
 * 5. Verify every GPIO pin against the schematic and package pinout.
 * 6. Enable ELEVATOR_ENABLE_FLOOR_SENSORS only after wiring real floor sensors.
 * 7. Drive motors through an isolated driver/relay stage with hardware interlocks.
 * 8. Wire emergency stop and final limit switches into hardware safety circuits;
 *    software handling is only a secondary layer of protection.
 * 9. Enable DEBUG_UART_ENABLED only after retargeting printf() to a UART.
 * 10. Enable ELEVATOR_ENABLE_IWDG after validating the independent watchdog
 *     timeout for the selected clock configuration.
 * ============================================================================ */
