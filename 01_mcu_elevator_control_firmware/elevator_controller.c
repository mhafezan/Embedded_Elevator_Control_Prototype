#include <stdio.h>
#include "elevator_controller.h"

/*
 * Initialize the elevator controller to a safe default state.
 */
void Elevator_Init(ElevatorController *controller)
{
    controller->current_floor = 1;
    controller->target_floor = 1;
    controller->state = STATE_IDLE;
    controller->motor = MOTOR_STOP;
    controller->door_open = false;
}

/*
 * Convert controller state to readable text.
 */
const char *Elevator_StateToString(ElevatorState state)
{
    switch (state)
    {
    case STATE_IDLE:
        return "IDLE";
    case STATE_MOVING_UP:
        return "MOVING_UP";
    case STATE_MOVING_DOWN:
        return "MOVING_DOWN";
    case STATE_DOOR_OPEN:
        return "DOOR_OPEN";
    case STATE_EMERGENCY_STOP:
        return "EMERGENCY_STOP";
    default:
        return "UNKNOWN";
    }
}

/*
 * Convert motor command to readable text.
 */
const char *Elevator_MotorToString(MotorCommand motor)
{
    switch (motor)
    {
    case MOTOR_STOP:
        return "STOP";
    case MOTOR_UP:
        return "UP";
    case MOTOR_DOWN:
        return "DOWN";
    default:
        return "UNKNOWN";
    }
}

/*
 * Main elevator control logic.
 *
 * This function represents the type of state-machine logic that could run
 * periodically on a microcontroller. In real firmware, this function would
 * be called in a timed loop after reading GPIO inputs.
 */
void Elevator_Update(ElevatorController *controller, ElevatorInputs inputs)
{
    /*
     * Emergency stop has the highest priority.
     * Any emergency stop immediately disables motor movement.
     */
    if (inputs.emergency_stop)
    {
        controller->state = STATE_EMERGENCY_STOP;
        controller->motor = MOTOR_STOP;
        controller->door_open = false;
        return;
    }

    /*
     * Ignore invalid floor requests.
     */
    if (inputs.requested_floor >= MIN_FLOOR &&
        inputs.requested_floor <= MAX_FLOOR)
    {
        controller->target_floor = inputs.requested_floor;
    }

    /*
     * Prevent upward movement if upper limit switch is active.
     */
    if (inputs.upper_limit_switch && controller->motor == MOTOR_UP)
    {
        controller->motor = MOTOR_STOP;
        controller->state = STATE_IDLE;
        return;
    }

    /*
     * Prevent downward movement if lower limit switch is active.
     */
    if (inputs.lower_limit_switch && controller->motor == MOTOR_DOWN)
    {
        controller->motor = MOTOR_STOP;
        controller->state = STATE_IDLE;
        return;
    }

    /*
     * State-machine behavior.
     */
    switch (controller->state)
    {
    case STATE_IDLE:
        controller->door_open = false;
        controller->motor = MOTOR_STOP;

        if (controller->target_floor > controller->current_floor)
        {
            controller->state = STATE_MOVING_UP;
            controller->motor = MOTOR_UP;
        }
        else if (controller->target_floor < controller->current_floor)
        {
            controller->state = STATE_MOVING_DOWN;
            controller->motor = MOTOR_DOWN;
        }
        else
        {
            controller->state = STATE_DOOR_OPEN;
            controller->door_open = true;
        }
        break;

    case STATE_MOVING_UP:
        controller->door_open = false;
        controller->motor = MOTOR_UP;

        /*
         * In this simplified simulation, each update cycle moves one floor.
         */
        if (controller->current_floor < MAX_FLOOR)
        {
            controller->current_floor++;
        }

        if (controller->current_floor >= controller->target_floor)
        {
            controller->motor = MOTOR_STOP;
            controller->state = STATE_DOOR_OPEN;
            controller->door_open = true;
        }
        break;

    case STATE_MOVING_DOWN:
        controller->door_open = false;
        controller->motor = MOTOR_DOWN;

        /*
         * In this simplified simulation, each update cycle moves one floor.
         */
        if (controller->current_floor > MIN_FLOOR)
        {
            controller->current_floor--;
        }

        if (controller->current_floor <= controller->target_floor)
        {
            controller->motor = MOTOR_STOP;
            controller->state = STATE_DOOR_OPEN;
            controller->door_open = true;
        }
        break;

    case STATE_DOOR_OPEN:
        controller->motor = MOTOR_STOP;
        controller->door_open = true;

        /*
         * If the door is obstructed, keep the door open.
         */
        if (inputs.door_obstruction)
        {
            controller->door_open = true;
        }
        else
        {
            /*
             * After one update cycle without obstruction, close the door
             * and return to IDLE.
             */
            controller->door_open = false;
            controller->state = STATE_IDLE;
        }
        break;

    case STATE_EMERGENCY_STOP:
        controller->motor = MOTOR_STOP;
        controller->door_open = false;

        /*
         * In this simple model, emergency stop can be cleared when the
         * emergency input becomes false.
         */
        if (!inputs.emergency_stop)
        {
            controller->state = STATE_IDLE;
        }
        break;

    default:
        /*
         * Fail-safe behavior for undefined states.
         */
        controller->state = STATE_EMERGENCY_STOP;
        controller->motor = MOTOR_STOP;
        controller->door_open = false;
        break;
    }
}

/*
 * Print the current controller status.
 */
void Elevator_PrintStatus(const ElevatorController *controller)
{
    printf("Current Floor: %d | Target Floor: %d | State: %s | Motor: %s | Door: %s\n",
           controller->current_floor,
           controller->target_floor,
           Elevator_StateToString(controller->state),
           Elevator_MotorToString(controller->motor),
           controller->door_open ? "OPEN" : "CLOSED");
}