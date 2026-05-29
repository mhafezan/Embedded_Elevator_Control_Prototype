/**
 * @file stm32_main.c
 * @brief STM32 Elevator Controller Firmware
 *
 * This firmware runs on an STM32 microcontroller and reads floor request buttons
 * and safety inputs from GPIO pins. The elevator controller logic is updated
 * periodically in the main loop, and motor commands are sent to output pins.
 *
 * Hardware Configuration (STM32F4xx/STM32H7xx):
 * ===============================================
 * CABIN REQUEST BUTTONS (10 Floors):
 *   PA0  - Floor 1 button  (Cabin Request)
 *   PA1  - Floor 2 button  (Cabin Request)
 *   PA2  - Floor 3 button  (Cabin Request)
 *   PA3  - Floor 4 button  (Cabin Request)
 *   PA4  - Floor 5 button  (Cabin Request)
 *   PA5  - Floor 6 button  (Cabin Request)
 *   PA6  - Floor 7 button  (Cabin Request)
 *   PA7  - Floor 8 button  (Cabin Request)
 *   PB0  - Floor 9 button  (Cabin Request)
 *   PB1  - Floor 10 button (Cabin Request)
 *
 * SAFETY INPUTS:
 *   PC0  - Emergency Stop Button      (Active High: 1 = Emergency)
 *   PC1  - Door Obstruction Sensor    (Active High: 1 = Obstructed)
 *   PC2  - Upper Limit Switch         (Active High: 1 = At Top)
 *   PC3  - Lower Limit Switch         (Active High: 1 = At Bottom)
 *
 * MOTOR CONTROL OUTPUTS:
 *   PD0  - Motor UP    Control   (GPIO Output: 1 = Moving Up)
 *   PD1  - Motor DOWN  Control   (GPIO Output: 1 = Moving Down)
 *   PD2  - Door Control          (GPIO Output: 1 = Door Open)
 *
 * HALL REQUESTS (Future Extension):
 *   PE0  - Hall UP Request (Floor selections on outside panels)
 *   PE1  - Hall DOWN Request
 *
 * Update Rate: 1 second (configured via SysTick or Timer)
 */

#include "stm32f4xx_hal.h"  /* Adjust to your STM32 variant (stm32h7xx_hal.h, etc.) */
#include <stdbool.h>
#include "elevator_controller.h"

/* ============================================================================
 * GPIO PIN DEFINITIONS
 * ============================================================================ */

/* Cabin Request Buttons - Port A (PA0 to PA7) and Port B (PB0 to PB1) */
#define CABIN_FLOOR1_PORT     GPIOA
#define CABIN_FLOOR1_PIN      GPIO_PIN_0
#define CABIN_FLOOR2_PORT     GPIOA
#define CABIN_FLOOR2_PIN      GPIO_PIN_1
#define CABIN_FLOOR3_PORT     GPIOA
#define CABIN_FLOOR3_PIN      GPIO_PIN_2
#define CABIN_FLOOR4_PORT     GPIOA
#define CABIN_FLOOR4_PIN      GPIO_PIN_3
#define CABIN_FLOOR5_PORT     GPIOA
#define CABIN_FLOOR5_PIN      GPIO_PIN_4
#define CABIN_FLOOR6_PORT     GPIOA
#define CABIN_FLOOR6_PIN      GPIO_PIN_5
#define CABIN_FLOOR7_PORT     GPIOA
#define CABIN_FLOOR7_PIN      GPIO_PIN_6
#define CABIN_FLOOR8_PORT     GPIOA
#define CABIN_FLOOR8_PIN      GPIO_PIN_7
#define CABIN_FLOOR9_PORT     GPIOB
#define CABIN_FLOOR9_PIN      GPIO_PIN_0
#define CABIN_FLOOR10_PORT    GPIOB
#define CABIN_FLOOR10_PIN     GPIO_PIN_1

/* Safety Input Pins - Port C */
#define EMERGENCY_STOP_PORT      GPIOC
#define EMERGENCY_STOP_PIN       GPIO_PIN_0
#define DOOR_OBSTRUCTION_PORT    GPIOC
#define DOOR_OBSTRUCTION_PIN     GPIO_PIN_1
#define UPPER_LIMIT_SWITCH_PORT  GPIOC
#define UPPER_LIMIT_SWITCH_PIN   GPIO_PIN_2
#define LOWER_LIMIT_SWITCH_PORT  GPIOC
#define LOWER_LIMIT_SWITCH_PIN   GPIO_PIN_3

/* Motor Control Output Pins - Port D */
#define MOTOR_UP_PORT         GPIOD
#define MOTOR_UP_PIN          GPIO_PIN_0
#define MOTOR_DOWN_PORT       GPIOD
#define MOTOR_DOWN_PIN        GPIO_PIN_1
#define DOOR_CONTROL_PORT     GPIOD
#define DOOR_CONTROL_PIN      GPIO_PIN_2

/* Hall Request Input Pins - Port E (Future Extension) */
#define HALL_UP_PORT          GPIOE
#define HALL_UP_PIN           GPIO_PIN_0
#define HALL_DOWN_PORT        GPIOE
#define HALL_DOWN_PIN         GPIO_PIN_1

/* ============================================================================
 * GLOBAL VARIABLES
 * ============================================================================ */

static ElevatorController g_elevator_controller;
static uint32_t g_update_counter = 0;
static const uint32_t UPDATE_INTERVAL = 1000; /* Update every 1000ms */

/* ============================================================================
 * FUNCTION PROTOTYPES
 * ============================================================================ */

void SystemClock_Config(void);
void GPIO_Init(void);
void UART_Init(void);
void SysTick_Handler(void);

static void HAL_GPIO_SetPin(GPIO_TypeDef *gpio_port, uint16_t pin);
static void HAL_GPIO_ResetPin(GPIO_TypeDef *gpio_port, uint16_t pin);
static GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *gpio_port, uint16_t pin);
static ElevatorInputs HAL_ReadInputs(void);
static void HAL_UpdateMotorOutputs(const ElevatorController *controller);
static void HAL_UART_PrintStatus(const ElevatorController *controller);

/* ============================================================================
 * HELPER FUNCTIONS - GPIO PIN CONTROL
 * ============================================================================ */

/**
 * @brief Set a GPIO pin to HIGH (1)
 */
static void HAL_GPIO_SetPin(GPIO_TypeDef *gpio_port, uint16_t pin)
{
    gpio_port->BSRR = pin;
}

/**
 * @brief Reset a GPIO pin to LOW (0)
 */
static void HAL_GPIO_ResetPin(GPIO_TypeDef *gpio_port, uint16_t pin)
{
    gpio_port->BSRR = (uint32_t)pin << 16U;
}

/**
 * @brief Read a GPIO input pin state
 * @return GPIO_PIN_SET (1) or GPIO_PIN_RESET (0)
 */
static GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *gpio_port, uint16_t pin)
{
    return (GPIO_PinState)((gpio_port->IDR & pin) >> __builtin_ctz(pin));
}

/* ============================================================================
 * INPUT READING FUNCTIONS
 * ============================================================================ */

/**
 * @brief Read all inputs from GPIO pins
 * @return ElevatorInputs structure with current button and sensor states
 */
static ElevatorInputs HAL_ReadInputs(void)
{
    ElevatorInputs inputs;

    /* Read cabin request buttons (PA0-PA7, PB0-PB1) */
    inputs.cabin_request_floor = 0;

    if (HAL_GPIO_ReadPin(CABIN_FLOOR1_PORT, CABIN_FLOOR1_PIN) == GPIO_PIN_SET)
        inputs.cabin_request_floor = 1;
    else if (HAL_GPIO_ReadPin(CABIN_FLOOR2_PORT, CABIN_FLOOR2_PIN) == GPIO_PIN_SET)
        inputs.cabin_request_floor = 2;
    else if (HAL_GPIO_ReadPin(CABIN_FLOOR3_PORT, CABIN_FLOOR3_PIN) == GPIO_PIN_SET)
        inputs.cabin_request_floor = 3;
    else if (HAL_GPIO_ReadPin(CABIN_FLOOR4_PORT, CABIN_FLOOR4_PIN) == GPIO_PIN_SET)
        inputs.cabin_request_floor = 4;
    else if (HAL_GPIO_ReadPin(CABIN_FLOOR5_PORT, CABIN_FLOOR5_PIN) == GPIO_PIN_SET)
        inputs.cabin_request_floor = 5;
    else if (HAL_GPIO_ReadPin(CABIN_FLOOR6_PORT, CABIN_FLOOR6_PIN) == GPIO_PIN_SET)
        inputs.cabin_request_floor = 6;
    else if (HAL_GPIO_ReadPin(CABIN_FLOOR7_PORT, CABIN_FLOOR7_PIN) == GPIO_PIN_SET)
        inputs.cabin_request_floor = 7;
    else if (HAL_GPIO_ReadPin(CABIN_FLOOR8_PORT, CABIN_FLOOR8_PIN) == GPIO_PIN_SET)
        inputs.cabin_request_floor = 8;
    else if (HAL_GPIO_ReadPin(CABIN_FLOOR9_PORT, CABIN_FLOOR9_PIN) == GPIO_PIN_SET)
        inputs.cabin_request_floor = 9;
    else if (HAL_GPIO_ReadPin(CABIN_FLOOR10_PORT, CABIN_FLOOR10_PIN) == GPIO_PIN_SET)
        inputs.cabin_request_floor = 10;

    /* Read hall request buttons (currently set to 0, extend as needed) */
    inputs.hall_up_request_floor = 0;
    inputs.hall_down_request_floor = 0;

    /* Read safety input pins (PC0-PC3) */
    inputs.emergency_stop = (HAL_GPIO_ReadPin(EMERGENCY_STOP_PORT, EMERGENCY_STOP_PIN) == GPIO_PIN_SET);
    inputs.door_obstruction = (HAL_GPIO_ReadPin(DOOR_OBSTRUCTION_PORT, DOOR_OBSTRUCTION_PIN) == GPIO_PIN_SET);
    inputs.upper_limit_switch = (HAL_GPIO_ReadPin(UPPER_LIMIT_SWITCH_PORT, UPPER_LIMIT_SWITCH_PIN) == GPIO_PIN_SET);
    inputs.lower_limit_switch = (HAL_GPIO_ReadPin(LOWER_LIMIT_SWITCH_PORT, LOWER_LIMIT_SWITCH_PIN) == GPIO_PIN_SET);

    return inputs;
}

/* ============================================================================
 * OUTPUT CONTROL FUNCTIONS
 * ============================================================================ */

/**
 * @brief Update motor and door control outputs based on controller state
 */
static void HAL_UpdateMotorOutputs(const ElevatorController *controller)
{
    /* Control Motor UP pin (PD0) */
    if (controller->motor == MOTOR_UP)
    {
        HAL_GPIO_SetPin(MOTOR_UP_PORT, MOTOR_UP_PIN);
    }
    else
    {
        HAL_GPIO_ResetPin(MOTOR_UP_PORT, MOTOR_UP_PIN);
    }

    /* Control Motor DOWN pin (PD1) */
    if (controller->motor == MOTOR_DOWN)
    {
        HAL_GPIO_SetPin(MOTOR_DOWN_PORT, MOTOR_DOWN_PIN);
    }
    else
    {
        HAL_GPIO_ResetPin(MOTOR_DOWN_PORT, MOTOR_DOWN_PIN);
    }

    /* Control Door pin (PD2) */
    if (controller->door_open)
    {
        HAL_GPIO_SetPin(DOOR_CONTROL_PORT, DOOR_CONTROL_PIN);
    }
    else
    {
        HAL_GPIO_ResetPin(DOOR_CONTROL_PORT, DOOR_CONTROL_PIN);
    }
}

/* ============================================================================
 * UART DEBUGGING FUNCTIONS
 * ============================================================================ */

/**
 * @brief Print controller status via UART (optional for debugging)
 */
static void HAL_UART_PrintStatus(const ElevatorController *controller)
{
    /* Note: UART is optional and may not be needed in production.
     * Remove or conditionally compile this function as needed.
     * Requires UART initialization and retarget of printf().
     */

#ifdef DEBUG_UART_ENABLED
    printf("Floor: %d | Target: %d | State: %s | Motor: %s | Door: %s\n",
           controller->current_floor,
           controller->target_floor,
           Elevator_StateToString(controller->state),
           Elevator_MotorToString(controller->motor),
           controller->door_open ? "OPEN" : "CLOSED");
#endif
}

/* ============================================================================
 * GPIO INITIALIZATION
 * ============================================================================ */

/**
 * @brief Initialize GPIO pins for inputs and outputs
 *
 * Input Configuration (Pull-up):
 *   - PA0-PA7: Cabin request buttons (Floor 1-8)
 *   - PB0-PB1: Cabin request buttons (Floor 9-10)
 *   - PC0-PC3: Safety inputs
 *   - PE0-PE1: Hall request buttons
 *
 * Output Configuration (Push-Pull):
 *   - PD0: Motor UP control
 *   - PD1: Motor DOWN control
 *   - PD2: Door control
 */
void GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable GPIO port clocks */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();

    /* ========== CONFIGURE INPUT PINS (PA0-PA7) - Cabin Buttons Floor 1-8 ========== */
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |
                          GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP; /* Pull-up: Button press pulls to GND */
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* ========== CONFIGURE INPUT PINS (PB0-PB1) - Cabin Buttons Floor 9-10 ========== */
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* ========== CONFIGURE INPUT PINS (PC0-PC3) - Safety Inputs ========== */
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* ========== CONFIGURE OUTPUT PINS (PD0-PD2) - Motor & Door Control ========== */
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; /* Push-Pull output */
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* ========== CONFIGURE INPUT PINS (PE0-PE1) - Hall Requests ========== */
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    /* Initialize all outputs to LOW (motors off, door closed) */
    HAL_GPIO_ResetPin(MOTOR_UP_PORT, MOTOR_UP_PIN);
    HAL_GPIO_ResetPin(MOTOR_DOWN_PORT, MOTOR_DOWN_PIN);
    HAL_GPIO_ResetPin(DOOR_CONTROL_PORT, DOOR_CONTROL_PIN);
}

/* ============================================================================
 * UART INITIALIZATION (Optional for Debug)
 * ============================================================================ */

void UART_Init(void)
{
    /* Optional: Configure UART for debugging output
     * This would typically use USART2 or USART3 and requires
     * retargeting of printf() to work correctly.
     * For now, this is a placeholder.
     */
}

/* ============================================================================
 * SYSTEM CLOCK CONFIGURATION
 * ============================================================================ */

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /* Initialize the RCC Oscillators according to the specified parameters */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 16;
    RCC_OscInitStruct.PLL.PLLN = 360;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    /* Activate the Over-Drive to reach the 180 Mhz Frequency */
    HAL_PWREx_EnableOverDrive();

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                   RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
}

/* ============================================================================
 * SYSTICK INTERRUPT HANDLER
 * ============================================================================ */

/**
 * @brief SysTick interrupt handler called every 1ms
 * Accumulates time and triggers controller update every UPDATE_INTERVAL ms
 */
void SysTick_Handler(void)
{
    HAL_IncTick();

    g_update_counter++;
    if (g_update_counter >= UPDATE_INTERVAL)
    {
        g_update_counter = 0;

        /* Read inputs from GPIO pins */
        ElevatorInputs inputs = HAL_ReadInputs();

        /* Update elevator controller logic */
        Elevator_Update(&g_elevator_controller, inputs);

        /* Update output pins based on new controller state */
        HAL_UpdateMotorOutputs(&g_elevator_controller);

        /* Optional: Print status via UART (if enabled) */
        HAL_UART_PrintStatus(&g_elevator_controller);
    }
}

/* ============================================================================
 * MAIN FUNCTION
 * ============================================================================ */

int main(void)
{
    /* Reset all peripherals and initialize the Flash interface */
    HAL_Init();

    /* Configure the system clock */
    SystemClock_Config();

    /* Initialize GPIO pins */
    GPIO_Init();

    /* Initialize UART for debugging (optional) */
    /* UART_Init(); */

    /* Initialize elevator controller */
    Elevator_Init(&g_elevator_controller);

    /* Configure SysTick to generate interrupt every 1ms */
    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

    /* Main loop: The controller update happens in SysTick_Handler() every 1 second */
    while (1)
    {
        /* Application can perform other non-blocking tasks here */
        /* All elevator control logic runs in the SysTick interrupt handler */
        /* This ensures deterministic timing and responsive input handling */

        __WFI(); /* Wait for Interrupt - reduces power consumption */
    }

    return 0;
}

/* ============================================================================
 * NOTES FOR STM32CubeIDE INTEGRATION
 * ============================================================================
 *
 * 1. GPIO PIN SELECTION:
 *    - Cabin buttons: PA0-PA7, PB0-PB1 (10 pins for 10 floors)
 *    - Safety inputs: PC0-PC3 (4 pins)
 *    - Motor outputs: PD0-PD2 (3 pins)
 *    - Hall requests: PE0-PE1 (2 pins - for future use)
 *
 * 2. BUTTON DEBOUNCING:
 *    To improve reliability, consider adding software debouncing:
 *    - Add a delay before reading the pin again
 *    - Sample the pin multiple times and check consistency
 *    - Use a debounce counter for each button
 *
 * 3. INTERRUPT-DRIVEN INPUT HANDLING (Optional):
 *    Instead of polling in SysTick, you can enable GPIO EXTI interrupts:
 *    - Configure PA0-PA7, PB0-PB1, PC0-PC3 as EXTI inputs
 *    - Set interrupt priority below SysTick
 *    - Store button states in the interrupt handler
 *    - Read stored states in main SysTick handler
 *
 * 4. PWM FOR MOTOR CONTROL (Optional):
 *    For more realistic motor control with variable speed:
 *    - Use Timer PWM output on PD0, PD1 instead of simple GPIO
 *    - Modulate PWM duty cycle based on desired speed
 *    - Allow smooth acceleration/deceleration
 *
 * 5. COMPILATION IN STM32CubeIDE:
 *    - Include this file in your STM32 project
 *    - Link against elevator_controller.c
 *    - Ensure HAL libraries are properly included
 *    - Disable SysTick_Handler in startup code if using this custom handler
 *
 * ============================================================================
 */
