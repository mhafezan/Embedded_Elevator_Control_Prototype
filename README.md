# Embedded Elevator Controller Simulation in C

## Overview

This repository contains a simplified **embedded elevator controller simulation** written in C. The project demonstrates how a basic elevator control algorithm can be implemented using a finite-state machine, motor command logic, floor-request handling, and safety-related input signals.

The program is designed as a small embedded-systems-style prototype. It simulates the type of control logic that could run periodically on a microcontroller after reading digital inputs such as push buttons, limit switches, emergency-stop signals, and door-obstruction sensors.

## Project Objectives

The main objectives of this project are to demonstrate:

- Finite-state-machine-based elevator control logic
- Basic embedded firmware structure in C
- Separation between application logic and controller logic
- Safe motor-control decisions based on system state and input signals
- Handling of floor requests using a FIFO request queue
- Emergency-stop and limit-switch protection logic
- Terminal-based simulation for testing controller behavior

## Features

The elevator controller supports the following features:

- Three-floor elevator operation
- Floor request input from the terminal
- FIFO-based request queue
- Duplicate request filtering
- Motor control commands:
  - Stop
  - Move up
  - Move down
- Elevator states:
  - Idle
  - Moving up
  - Moving down
  - Door open
  - Emergency stop
- Door-obstruction handling
- Emergency-stop handling with highest priority
- Upper and lower limit-switch protection
- Status printing for debugging and simulation

## Repository Structure

```text
.
├── main.c
├── elevator_controller.c
├── elevator_controller.h
└── README.md
```

### File Descriptions

| File | Description |
|---|---|
| `main.c` | Provides the terminal-based simulation interface. It collects user inputs, calls the elevator controller update function, and prints the updated controller status. |
| `elevator_controller.c` | Implements the elevator controller logic, including initialization, state transitions, request queue management, motor control, and safety handling. |
| `elevator_controller.h` | Defines constants, data structures, enumerations, and public function prototypes used by the elevator controller. |
| `README.md` | Project documentation. |

## System Architecture

The project separates the elevator simulation into two main layers:

1. **Application Layer**  
   Implemented in `main.c`. This layer handles user interaction through the terminal and passes input signals to the controller.

2. **Controller Logic Layer**  
   Implemented in `elevator_controller.c` and declared in `elevator_controller.h`. This layer contains the elevator finite-state machine, request queue, safety logic, and motor-control decisions.

This structure is similar to a simple embedded firmware design, where the main loop repeatedly reads inputs, updates the controller, and writes outputs.

## Elevator States

The elevator controller uses the following states:

| State | Description |
|---|---|
| `STATE_IDLE` | Elevator is stopped and waiting for a floor request. |
| `STATE_MOVING_UP` | Elevator is moving upward toward the target floor. |
| `STATE_MOVING_DOWN` | Elevator is moving downward toward the target floor. |
| `STATE_DOOR_OPEN` | Elevator has reached the requested floor and the door is open. |
| `STATE_EMERGENCY_STOP` | Elevator movement is disabled due to an emergency-stop condition. |

## Motor Commands

The controller generates one of the following motor commands:

| Command | Description |
|---|---|
| `MOTOR_STOP` | Stop elevator movement. |
| `MOTOR_UP` | Move elevator upward. |
| `MOTOR_DOWN` | Move elevator downward. |

## Input Signals

The simulation accepts the following input signals:

| Input | Description |
|---|---|
| `requested_floor` | Requested destination floor. Valid values are 1 to 3. A value of 0 means no new request. |
| `door_obstruction` | Indicates whether the door is obstructed. |
| `emergency_stop` | Immediately stops elevator movement and places the controller in emergency-stop state. |
| `upper_limit_switch` | Prevents upward movement when the elevator reaches the upper physical limit. |
| `lower_limit_switch` | Prevents downward movement when the elevator reaches the lower physical limit. |

## Control Logic Summary

The main controller function is:

```c
void Elevator_Update(ElevatorController *controller, ElevatorInputs inputs);
```

This function performs the following operations:

1. Checks emergency-stop input.
2. Adds valid floor requests to the FIFO queue.
3. Selects the next target floor.
4. Checks upper and lower limit-switch protection.
5. Updates the elevator state using a finite-state machine.
6. Updates motor commands and door status.

Emergency stop has the highest priority. If the emergency-stop input is active, the motor is immediately stopped and the elevator enters the `STATE_EMERGENCY_STOP` state.

## Request Queue

The controller uses a simple FIFO queue to store floor requests. The request queue:

- Accepts only valid floor numbers
- Rejects duplicate requests
- Rejects new requests when the queue is full
- Processes requests in the order they were received

The queue size is defined as:

```c
#define REQUEST_QUEUE_SIZE MAX_FLOOR
```

For the current implementation, the system supports three floors, so the queue can hold up to three requests.

## Build Instructions

### Requirements

To build and run this project, you need:

- GCC compiler
- A terminal or command prompt
- Optional: Visual Studio Code with C/C++ extension

### Compile on Windows, Linux, or macOS

Run the following command from the project directory:

```bash
gcc -Wall -Wextra -g main.c elevator_controller.c -o elevator_controller
```

On Windows, you may prefer to generate an executable with `.exe` extension:

```bash
gcc -Wall -Wextra -g main.c elevator_controller.c -o elevator_controller.exe
```

## Run Instructions

### Linux/macOS

```bash
./elevator_controller
```

### Windows

```bash
elevator_controller.exe
```

or, from PowerShell:

```powershell
.\elevator_controller.exe
```

## Example Program Output

```text
--------------------------------------
Embedded Elevator Controller Simulation Started
--------------------------------------
Valid floors: 1 to 3
Enter 0 as requested floor for no new request.
Enter emergency stop = 1 to trigger emergency stop.
Press Ctrl+C to exit from simulation.

Current Floor: 1 | Target Floor: 1 | State: IDLE | Motor: STOP | Door: CLOSED

Enter requested floor 1 to 3, or 0 for no new request: 3
Door obstruction? 1=yes, 0=no: 0
Emergency stop? 1=yes, 0=no: 0
Upper limit switch active? 1=yes, 0=no: 0
Lower limit switch active? 1=yes, 0=no: 0

Updated controller status:
Current Floor: 1 | Target Floor: 3 | State: MOVING_UP | Motor: UP | Door: CLOSED
--------------------------------------
```

## Example Test Scenarios

You can manually test the controller using the terminal prompts.

### Scenario 1: Move from Floor 1 to Floor 3

1. Start the program.
2. Enter requested floor `3`.
3. Set all safety inputs to `0`.
4. Continue entering `0` for no new request.
5. Observe the elevator moving upward until it reaches floor 3.

### Scenario 2: Emergency Stop

1. Enter any valid floor request.
2. Set emergency stop to `1`.
3. The controller should immediately stop the motor and enter `STATE_EMERGENCY_STOP`.

### Scenario 3: Door Obstruction

1. Request a floor.
2. When the elevator reaches the target floor and opens the door, set door obstruction to `1`.
3. The door should remain open while the obstruction is active.

### Scenario 4: Limit Switch Protection

1. Simulate upward movement.
2. Activate the upper limit switch.
3. The controller should stop upward movement.
4. Simulate downward movement.
5. Activate the lower limit switch.
6. The controller should stop downward movement.

## Embedded-Systems Relevance

Although this project runs as a terminal simulation, the controller structure is similar to embedded firmware. In a real microcontroller-based implementation:

- Floor buttons would be read from GPIO inputs.
- Door sensors and limit switches would be digital inputs.
- Motor commands would be sent to a motor driver, relay circuit, or variable frequency drive interface.
- `Elevator_Update()` would be called periodically inside a timed loop or real-time scheduler.
- The terminal input section in `main.c` would be replaced by hardware input-reading functions.

## Possible Future Improvements

Potential extensions to this project include:

- Add unit tests for each controller state.
- Add timer-based door-open delay.
- Add support for more floors.
- Add priority scheduling instead of simple FIFO scheduling.
- Add separate cabin and hall call requests.
- Add overload sensor input.
- Add maintenance mode.
- Add hardware abstraction layer for GPIO-based implementation.
- Add graphical simulation or serial-monitor interface.
- Add integration with an RTOS task loop.

## Author

**Mohammad Hafezan**

GitHub Portfolio Project: Embedded Elevator Controller Simulation in C
