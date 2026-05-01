# MCU Elevator Control Firmware Simulation

## Overview

This project implements a simplified embedded C state-machine controller for a residential elevator/lift system. The controller simulates floor requests, door operation, motor direction, emergency stop behavior, obstruction detection, and limit-switch protection.
The objective of this project is to demonstrate hands-on familiarity with embedded control logic, safety-oriented state transitions, firmware organization, and hardware/software interface thinking.

## Notes

This is a software simulation assuming inputs come from user terminal. In a real embedded system, the inputs would be read from GPIO pins, sensors, buttons, and safety circuits, while outputs would control relays, motor drivers, contactors, or a VFD interface

## Features

- Floor request handling for a 3-floor elevator model
- State-machine based control logic
- Motor direction commands: up, down, stop
- Door open/close behavior
- Emergency stop handling
- Door obstruction detection
- Upper and lower limit-switch protection
- Modular C implementation with separate header/source files

## Controller States

The controller supports the following states:

- `STATE_IDLE`
- `STATE_MOVING_UP`
- `STATE_MOVING_DOWN`
- `STATE_DOOR_OPEN`
- `STATE_EMERGENCY_STOP`

## Simulated Inputs

The simulation accepts the following user inputs:

- Requested floor
- Door obstruction sensor
- Emergency stop signal
- Upper limit switch
- Lower limit switch

## Simulated Outputs

The controller produces the following outputs:

- Current floor
- Target floor
- Controller state
- Motor command
- Door status

## Build Instructions

Compile using GCC:

```bash
gcc main.c elevator_controller.c -o elevator_controller