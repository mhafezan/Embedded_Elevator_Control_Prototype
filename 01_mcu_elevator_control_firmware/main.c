#include <stdio.h>
#include <stdbool.h>
#include "elevator_controller.h"

/*
 * This main file provides a simple terminal simulation for the elevator
 * controller. It allows users to enter floor requests and safety inputs.
 */

int main(void)
{
    ElevatorController controller;
    ElevatorInputs inputs;

    int door_obstruction_input;
    int emergency_stop_input;
    int upper_limit_input;
    int lower_limit_input;

    Elevator_Init(&controller);

    printf("--------------------------------------\n");
    printf("Embedded Elevator Controller Simulation Started\n");
    printf("--------------------------------------\n");
    printf("Valid floors: 1 to 3\n");
    printf("Enter 0 as requested floor to keep the previous target.\n");
    printf("Enter emergency stop = 1 to trigger emergency stop.\n");
    printf("Press Ctrl+C to exit from simulation.\n\n");

    while (true)
    {
        inputs.requested_floor = 0;
        inputs.door_obstruction = false;
        inputs.emergency_stop = false;
        inputs.upper_limit_switch = false;
        inputs.lower_limit_switch = false;

        Elevator_PrintStatus(&controller);

        printf("\nEnter requested floor 1-3, or 0 for no new request: ");
        scanf("%d", &inputs.requested_floor);

        printf("Door obstruction? 1=yes, 0=no: ");
        scanf("%d", &door_obstruction_input);

        printf("Emergency stop? 1=yes, 0=no: ");
        scanf("%d", &emergency_stop_input);

        printf("Upper limit switch active? 1=yes, 0=no: ");
        scanf("%d", &upper_limit_input);

        printf("Lower limit switch active? 1=yes, 0=no: ");
        scanf("%d", &lower_limit_input);

        inputs.door_obstruction = door_obstruction_input ? true : false;
        inputs.emergency_stop = emergency_stop_input ? true : false;
        inputs.upper_limit_switch = upper_limit_input ? true : false;
        inputs.lower_limit_switch = lower_limit_input ? true : false;

        Elevator_Update(&controller, inputs);

        printf("\nUpdated controller status:\n");
        Elevator_PrintStatus(&controller);
        printf("--------------------------------------\n\n");
    }

    return 0;
}