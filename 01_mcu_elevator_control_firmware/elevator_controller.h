#ifndef ELEVATOR_CONTROLLER_H
#define ELEVATOR_CONTROLLER_H

#include <stdbool.h>

/*
 * Simplified embedded elevator controller simulation.
 * This file defines the controller states, motor commands,
 * input signals, and public controller functions.
 */

#define MIN_FLOOR 1
#define MAX_FLOOR 10
#define REQUEST_QUEUE_SIZE MAX_FLOOR

typedef enum
{
    STATE_IDLE,
    STATE_MOVING_UP,
    STATE_MOVING_DOWN,
    STATE_DOOR_OPEN,
    STATE_EMERGENCY_STOP
} ElevatorState;

typedef enum
{
    MOTOR_STOP,
    MOTOR_UP,
    MOTOR_DOWN
} MotorCommand;

typedef struct
{
    int cabin_request_floor;
    int hall_up_request_floor;
    int hall_down_request_floor;

    bool door_obstruction;
    bool emergency_stop;
    bool upper_limit_switch;
    bool lower_limit_switch;

    /*
     * Optional hardware floor feedback. Set to 0 when no real floor sensor is
     * available; simulation code will then advance one floor per update cycle.
     */
    int measured_floor;
} ElevatorInputs;

typedef struct
{
    int current_floor;
    int target_floor;
    int request_queue[REQUEST_QUEUE_SIZE];
    int request_count;
    ElevatorState state;
    MotorCommand motor;
    bool door_open;
} ElevatorController;

void Elevator_Init(ElevatorController *controller);
void Elevator_Update(ElevatorController *controller, ElevatorInputs inputs);
void Elevator_PrintStatus(const ElevatorController *controller);

const char *Elevator_StateToString(ElevatorState state);
const char *Elevator_MotorToString(MotorCommand motor);

#endif