/**
 * @file stm32_gpio_config.c
 * @brief STM32 GPIO, debouncing, optional floor sensing, and safe outputs.
 */

#include "stm32_gpio_config.h"

/* ============================================================================
 * PRIVATE TYPES
 * ============================================================================ */

typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
    GPIO_PinState active_state;
    GPIO_PinState stable_state;
    GPIO_PinState last_raw_state;
    uint16_t stable_ticks;
} DebouncedInput;

/* ============================================================================
 * PRIVATE DATA
 * ============================================================================ */

static DebouncedInput g_cabin_buttons[ELEVATOR_NUM_FLOORS] = {
    {CABIN_FLOOR1_PORT, CABIN_FLOOR1_PIN, ELEVATOR_BUTTON_ACTIVE_STATE, GPIO_PIN_SET, GPIO_PIN_SET, 0},
    {CABIN_FLOOR2_PORT, CABIN_FLOOR2_PIN, ELEVATOR_BUTTON_ACTIVE_STATE, GPIO_PIN_SET, GPIO_PIN_SET, 0},
    {CABIN_FLOOR3_PORT, CABIN_FLOOR3_PIN, ELEVATOR_BUTTON_ACTIVE_STATE, GPIO_PIN_SET, GPIO_PIN_SET, 0},
    {CABIN_FLOOR4_PORT, CABIN_FLOOR4_PIN, ELEVATOR_BUTTON_ACTIVE_STATE, GPIO_PIN_SET, GPIO_PIN_SET, 0},
    {CABIN_FLOOR5_PORT, CABIN_FLOOR5_PIN, ELEVATOR_BUTTON_ACTIVE_STATE, GPIO_PIN_SET, GPIO_PIN_SET, 0},
    {CABIN_FLOOR6_PORT, CABIN_FLOOR6_PIN, ELEVATOR_BUTTON_ACTIVE_STATE, GPIO_PIN_SET, GPIO_PIN_SET, 0},
    {CABIN_FLOOR7_PORT, CABIN_FLOOR7_PIN, ELEVATOR_BUTTON_ACTIVE_STATE, GPIO_PIN_SET, GPIO_PIN_SET, 0},
    {CABIN_FLOOR8_PORT, CABIN_FLOOR8_PIN, ELEVATOR_BUTTON_ACTIVE_STATE, GPIO_PIN_SET, GPIO_PIN_SET, 0},
    {CABIN_FLOOR9_PORT, CABIN_FLOOR9_PIN, ELEVATOR_BUTTON_ACTIVE_STATE, GPIO_PIN_SET, GPIO_PIN_SET, 0},
    {CABIN_FLOOR10_PORT, CABIN_FLOOR10_PIN, ELEVATOR_BUTTON_ACTIVE_STATE, GPIO_PIN_SET, GPIO_PIN_SET, 0},
};

static DebouncedInput g_safety_inputs[] = {
    {EMERGENCY_STOP_PORT, EMERGENCY_STOP_PIN, ELEVATOR_SAFETY_ACTIVE_STATE, GPIO_PIN_SET, GPIO_PIN_SET, 0},
    {DOOR_OBSTRUCTION_PORT, DOOR_OBSTRUCTION_PIN, ELEVATOR_SAFETY_ACTIVE_STATE, GPIO_PIN_SET, GPIO_PIN_SET, 0},
    {UPPER_LIMIT_SWITCH_PORT, UPPER_LIMIT_SWITCH_PIN, ELEVATOR_SAFETY_ACTIVE_STATE, GPIO_PIN_SET, GPIO_PIN_SET, 0},
    {LOWER_LIMIT_SWITCH_PORT, LOWER_LIMIT_SWITCH_PIN, ELEVATOR_SAFETY_ACTIVE_STATE, GPIO_PIN_SET, GPIO_PIN_SET, 0},
};

static DebouncedInput g_hall_up_input = {
    HALL_UP_PORT, HALL_UP_PIN, ELEVATOR_BUTTON_ACTIVE_STATE, GPIO_PIN_SET, GPIO_PIN_SET, 0};

static DebouncedInput g_hall_down_input = {
    HALL_DOWN_PORT, HALL_DOWN_PIN, ELEVATOR_BUTTON_ACTIVE_STATE, GPIO_PIN_SET, GPIO_PIN_SET, 0};

#if ELEVATOR_ENABLE_FLOOR_SENSORS
static DebouncedInput g_floor_sensors[ELEVATOR_NUM_FLOORS] = {
    {FLOOR_SENSOR1_PORT, FLOOR_SENSOR1_PIN, ELEVATOR_FLOOR_SENSOR_ACTIVE_STATE, GPIO_PIN_SET, GPIO_PIN_SET, 0},
    {FLOOR_SENSOR2_PORT, FLOOR_SENSOR2_PIN, ELEVATOR_FLOOR_SENSOR_ACTIVE_STATE, GPIO_PIN_SET, GPIO_PIN_SET, 0},
    {FLOOR_SENSOR3_PORT, FLOOR_SENSOR3_PIN, ELEVATOR_FLOOR_SENSOR_ACTIVE_STATE, GPIO_PIN_SET, GPIO_PIN_SET, 0},
    {FLOOR_SENSOR4_PORT, FLOOR_SENSOR4_PIN, ELEVATOR_FLOOR_SENSOR_ACTIVE_STATE, GPIO_PIN_SET, GPIO_PIN_SET, 0},
    {FLOOR_SENSOR5_PORT, FLOOR_SENSOR5_PIN, ELEVATOR_FLOOR_SENSOR_ACTIVE_STATE, GPIO_PIN_SET, GPIO_PIN_SET, 0},
    {FLOOR_SENSOR6_PORT, FLOOR_SENSOR6_PIN, ELEVATOR_FLOOR_SENSOR_ACTIVE_STATE, GPIO_PIN_SET, GPIO_PIN_SET, 0},
    {FLOOR_SENSOR7_PORT, FLOOR_SENSOR7_PIN, ELEVATOR_FLOOR_SENSOR_ACTIVE_STATE, GPIO_PIN_SET, GPIO_PIN_SET, 0},
    {FLOOR_SENSOR8_PORT, FLOOR_SENSOR8_PIN, ELEVATOR_FLOOR_SENSOR_ACTIVE_STATE, GPIO_PIN_SET, GPIO_PIN_SET, 0},
    {FLOOR_SENSOR9_PORT, FLOOR_SENSOR9_PIN, ELEVATOR_FLOOR_SENSOR_ACTIVE_STATE, GPIO_PIN_SET, GPIO_PIN_SET, 0},
    {FLOOR_SENSOR10_PORT, FLOOR_SENSOR10_PIN, ELEVATOR_FLOOR_SENSOR_ACTIVE_STATE, GPIO_PIN_SET, GPIO_PIN_SET, 0},
};
#endif

static ElevatorOutputDirection g_active_direction = ELEVATOR_OUTPUT_STOPPED;
static ElevatorOutputDirection g_pending_direction = ELEVATOR_OUTPUT_STOPPED;
static uint32_t g_direction_change_deadline_ms = 0;

/* ============================================================================
 * PRIVATE HELPERS
 * ============================================================================ */

static void ConfigureInput(GPIO_TypeDef *port, uint16_t pin)
{
    GPIO_InitTypeDef init = {0};

    init.Pin = pin;
    init.Mode = GPIO_MODE_INPUT;
    init.Pull = GPIO_PULLUP;
    init.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(port, &init);
}

static void ConfigureOutput(GPIO_TypeDef *port, uint16_t pin)
{
    GPIO_InitTypeDef init = {0};

    init.Pin = pin;
    init.Mode = GPIO_MODE_OUTPUT_PP;
    init.Pull = GPIO_NOPULL;
    init.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(port, &init);
}

static bool IsInputActive(const DebouncedInput *input)
{
    return input->stable_state == input->active_state;
}

static void DebouncedInput_Init(DebouncedInput *input)
{
    GPIO_PinState raw_state = HAL_GPIO_ReadPin(input->port, input->pin);

    input->stable_state = raw_state;
    input->last_raw_state = raw_state;
    input->stable_ticks = 0;
}

static void DebouncedInput_Update(DebouncedInput *input)
{
    GPIO_PinState raw_state = HAL_GPIO_ReadPin(input->port, input->pin);

    if (raw_state == input->last_raw_state)
    {
        if (input->stable_ticks < ELEVATOR_DEBOUNCE_TICKS)
        {
            input->stable_ticks++;
        }
        else
        {
            input->stable_state = raw_state;
        }
    }
    else
    {
        input->last_raw_state = raw_state;
        input->stable_ticks = 0;
    }
}

static void DebounceArray(DebouncedInput *inputs, uint32_t count)
{
    for (uint32_t i = 0; i < count; i++)
    {
        DebouncedInput_Update(&inputs[i]);
    }
}

static void InitDebounceArray(DebouncedInput *inputs, uint32_t count)
{
    for (uint32_t i = 0; i < count; i++)
    {
        DebouncedInput_Init(&inputs[i]);
    }
}

static void ApplyMotorDirection(ElevatorOutputDirection direction)
{
    switch (direction)
    {
    case ELEVATOR_OUTPUT_MOVING_UP:
        HAL_GPIO_WritePin(MOTOR_DOWN_PORT, MOTOR_DOWN_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(MOTOR_UP_PORT, MOTOR_UP_PIN, GPIO_PIN_SET);
        break;

    case ELEVATOR_OUTPUT_MOVING_DOWN:
        HAL_GPIO_WritePin(MOTOR_UP_PORT, MOTOR_UP_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(MOTOR_DOWN_PORT, MOTOR_DOWN_PIN, GPIO_PIN_SET);
        break;

    case ELEVATOR_OUTPUT_STOPPED:
    default:
        HAL_GPIO_WritePin(MOTOR_UP_PORT, MOTOR_UP_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(MOTOR_DOWN_PORT, MOTOR_DOWN_PIN, GPIO_PIN_RESET);
        break;
    }

    g_active_direction = direction;
}

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

void Elevator_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();

#if ELEVATOR_ENABLE_FLOOR_SENSORS
    __HAL_RCC_GPIOF_CLK_ENABLE();
#endif

    for (uint32_t i = 0; i < ELEVATOR_NUM_FLOORS; i++)
    {
        ConfigureInput(g_cabin_buttons[i].port, g_cabin_buttons[i].pin);
    }

    for (uint32_t i = 0; i < (sizeof(g_safety_inputs) / sizeof(g_safety_inputs[0])); i++)
    {
        ConfigureInput(g_safety_inputs[i].port, g_safety_inputs[i].pin);
    }

    ConfigureInput(g_hall_up_input.port, g_hall_up_input.pin);
    ConfigureInput(g_hall_down_input.port, g_hall_down_input.pin);

#if ELEVATOR_ENABLE_FLOOR_SENSORS
    for (uint32_t i = 0; i < ELEVATOR_NUM_FLOORS; i++)
    {
        ConfigureInput(g_floor_sensors[i].port, g_floor_sensors[i].pin);
    }
#endif

    ConfigureOutput(MOTOR_UP_PORT, MOTOR_UP_PIN);
    ConfigureOutput(MOTOR_DOWN_PORT, MOTOR_DOWN_PIN);
    ConfigureOutput(DOOR_CONTROL_PORT, DOOR_CONTROL_PIN);

    Elevator_GPIO_AllOutputsOff();

    InitDebounceArray(g_cabin_buttons, ELEVATOR_NUM_FLOORS);
    InitDebounceArray(g_safety_inputs, sizeof(g_safety_inputs) / sizeof(g_safety_inputs[0]));
    DebouncedInput_Init(&g_hall_up_input);
    DebouncedInput_Init(&g_hall_down_input);

#if ELEVATOR_ENABLE_FLOOR_SENSORS
    InitDebounceArray(g_floor_sensors, ELEVATOR_NUM_FLOORS);
#endif
}

void Elevator_GPIO_DebounceTick(void)
{
    DebounceArray(g_cabin_buttons, ELEVATOR_NUM_FLOORS);
    DebounceArray(g_safety_inputs, sizeof(g_safety_inputs) / sizeof(g_safety_inputs[0]));
    DebouncedInput_Update(&g_hall_up_input);
    DebouncedInput_Update(&g_hall_down_input);

#if ELEVATOR_ENABLE_FLOOR_SENSORS
    DebounceArray(g_floor_sensors, ELEVATOR_NUM_FLOORS);
#endif
}

void Elevator_GPIO_AllOutputsOff(void)
{
    HAL_GPIO_WritePin(MOTOR_UP_PORT, MOTOR_UP_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR_DOWN_PORT, MOTOR_DOWN_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DOOR_CONTROL_PORT, DOOR_CONTROL_PIN, GPIO_PIN_RESET);

    g_active_direction = ELEVATOR_OUTPUT_STOPPED;
    g_pending_direction = ELEVATOR_OUTPUT_STOPPED;
    g_direction_change_deadline_ms = 0;
}

int Elevator_GPIO_GetCabinRequestFloor(void)
{
    for (uint32_t i = 0; i < ELEVATOR_NUM_FLOORS; i++)
    {
        if (IsInputActive(&g_cabin_buttons[i]))
        {
            return (int)(i + ELEVATOR_MIN_FLOOR);
        }
    }

    return 0;
}

int Elevator_GPIO_GetHallUpRequestFloor(void)
{
    return IsInputActive(&g_hall_up_input) ? (int)HALL_UP_FLOOR : 0;
}

int Elevator_GPIO_GetHallDownRequestFloor(void)
{
    return IsInputActive(&g_hall_down_input) ? (int)HALL_DOWN_FLOOR : 0;
}

int Elevator_GPIO_GetMeasuredFloor(void)
{
#if ELEVATOR_ENABLE_FLOOR_SENSORS
    int measured_floor = 0;

    for (uint32_t i = 0; i < ELEVATOR_NUM_FLOORS; i++)
    {
        if (IsInputActive(&g_floor_sensors[i]))
        {
            if (measured_floor != 0)
            {
                return 0;
            }

            measured_floor = (int)(i + ELEVATOR_MIN_FLOOR);
        }
    }

    return measured_floor;
#else
    return 0;
#endif
}

ElevatorSafetyInputs Elevator_GPIO_ReadSafetyInputs(void)
{
    ElevatorSafetyInputs inputs;

    inputs.emergency_stop = IsInputActive(&g_safety_inputs[0]);
    inputs.door_obstruction = IsInputActive(&g_safety_inputs[1]);
    inputs.upper_limit_switch = IsInputActive(&g_safety_inputs[2]);
    inputs.lower_limit_switch = IsInputActive(&g_safety_inputs[3]);

    return inputs;
}

bool Elevator_GPIO_SetMotorDirection(ElevatorOutputDirection direction)
{
    if (direction == g_active_direction)
    {
        return true;
    }

    if (direction == ELEVATOR_OUTPUT_STOPPED)
    {
        ApplyMotorDirection(ELEVATOR_OUTPUT_STOPPED);
        g_pending_direction = ELEVATOR_OUTPUT_STOPPED;
        g_direction_change_deadline_ms = HAL_GetTick() + MOTOR_DIRECTION_CHANGE_DEADTIME_MS;
        return true;
    }

    if (g_active_direction != ELEVATOR_OUTPUT_STOPPED)
    {
        ApplyMotorDirection(ELEVATOR_OUTPUT_STOPPED);
        g_pending_direction = direction;
        g_direction_change_deadline_ms = HAL_GetTick() + MOTOR_DIRECTION_CHANGE_DEADTIME_MS;
        return false;
    }

    if (g_direction_change_deadline_ms != 0 && HAL_GetTick() < g_direction_change_deadline_ms)
    {
        g_pending_direction = direction;
        return false;
    }

    ApplyMotorDirection(direction);
    g_pending_direction = ELEVATOR_OUTPUT_STOPPED;
    return true;
}

void Elevator_GPIO_SetDoorOpen(bool door_open)
{
    HAL_GPIO_WritePin(DOOR_CONTROL_PORT,
                      DOOR_CONTROL_PIN,
                      door_open ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void Elevator_GPIO_ServiceOutputs(void)
{
    if (g_pending_direction == ELEVATOR_OUTPUT_STOPPED)
    {
        return;
    }

    if (g_active_direction == ELEVATOR_OUTPUT_STOPPED && HAL_GetTick() >= g_direction_change_deadline_ms)
    {
        ApplyMotorDirection(g_pending_direction);
        g_pending_direction = ELEVATOR_OUTPUT_STOPPED;
    }
}
