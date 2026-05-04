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
    printf("Valid floors: %d to %d\n", MIN_FLOOR, MAX_FLOOR);
    printf("Enter 0 when there is no cabin or hall request.\n");
    printf("Cabin request: button pressed inside the elevator.\n");
    printf("Hall UP/DOWN request: button pressed outside the elevator.\n");
    printf("Enter emergency stop = 1 to trigger emergency stop.\n");
    printf("Press Ctrl+C to exit from simulation.\n\n");

    while (true)
    {
        inputs.cabin_request_floor = 0;
        inputs.hall_up_request_floor = 0;
        inputs.hall_down_request_floor = 0;

        inputs.door_obstruction = false;
        inputs.emergency_stop = false;
        inputs.upper_limit_switch = false;
        inputs.lower_limit_switch = false;

        Elevator_PrintStatus(&controller);

        printf("\nCabin request floor %d to %d, or 0 for no cabin request: ", MIN_FLOOR, MAX_FLOOR);
        scanf("%d", &inputs.cabin_request_floor);
        printf("Hall UP request floor %d to %d, or 0 for no hall UP request: ", MIN_FLOOR, MAX_FLOOR);
        scanf("%d", &inputs.hall_up_request_floor);
        printf("Hall DOWN request floor %d to %d, or 0 for no hall DOWN request: ", MIN_FLOOR, MAX_FLOOR);
        scanf("%d", &inputs.hall_down_request_floor);

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