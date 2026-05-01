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

    Elevator_Init(&controller);

    printf("Embedded Elevator Controller Simulation\n");
    printf("--------------------------------------\n");
    printf("Valid floors: 1 to 3\n");
    printf("Enter 0 as requested floor to keep the previous target.\n");
    printf("Enter emergency stop = 1 to trigger emergency stop.\n");
    printf("Press Ctrl+C to exit.\n\n");

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
        scanf("%d", (int *)&inputs.door_obstruction);

        printf("Emergency stop? 1=yes, 0=no: ");
        scanf("%d", (int *)&inputs.emergency_stop);

        printf("Upper limit switch active? 1=yes, 0=no: ");
        scanf("%d", (int *)&inputs.upper_limit_switch);

        printf("Lower limit switch active? 1=yes, 0=no: ");
        scanf("%d", (int *)&inputs.lower_limit_switch);

        Elevator_Update(&controller, inputs);

        printf("\nUpdated controller status:\n");
        Elevator_PrintStatus(&controller);
        printf("--------------------------------------\n\n");
    }

    return 0;
}