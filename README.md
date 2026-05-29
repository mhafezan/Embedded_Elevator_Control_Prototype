# Embedded Elevator Control Prototype

A C-based embedded elevator controller prototype that simulates the core logic of a multi-floor elevator system using a finite-state machine. The project includes both a terminal-based simulator and a graphical simulator built with raylib.

This project demonstrates embedded control concepts such as state-machine design, safety input handling, request queue management, motor control logic, and separation between controller firmware logic and user-interface visualization.

---

## Project Overview

The system models a simplified elevator controller for a 10-floor building. The controller processes cabin requests, hall requests, door obstruction signals, emergency stop input, and upper/lower limit switch conditions. Based on these inputs, it updates the elevator state, motor command, current floor, target floor, door status, and pending request queue.

The project is structured to resemble the type of control logic that could run periodically on a microcontroller after reading GPIO or sensor inputs.

---

## Key Features

- Finite-state machine-based elevator control logic
- Supports 10 floors by default
- Cabin floor requests
- Hall UP and Hall DOWN requests
- FIFO request queue
- Duplicate request prevention
- Emergency stop handling with highest priority
- Door obstruction handling
- Upper and lower limit switch protection
- Terminal-based simulation
- Professional GUI simulation using raylib
- Modular separation between controller logic and visualization

---

## Elevator States

The controller supports the following operating states:

| State | Description |
|---|---|
| `STATE_IDLE` | Elevator is stopped and waiting for a valid request |
| `STATE_MOVING_UP` | Elevator is moving upward toward the target floor |
| `STATE_MOVING_DOWN` | Elevator is moving downward toward the target floor |
| `STATE_DOOR_OPEN` | Elevator has arrived and the door is open |
| `STATE_EMERGENCY_STOP` | Emergency stop is active and motor movement is disabled |

---

## Motor Commands

The motor output is represented using three commands:

| Motor Command | Description |
|---|---|
| `MOTOR_STOP` | Motor is stopped |
| `MOTOR_UP` | Motor moves the elevator upward |
| `MOTOR_DOWN` | Motor moves the elevator downward |

---

## Safety Inputs

The controller supports the following safety-related inputs:

| Input | Description |
|---|---|
| `emergency_stop` | Immediately stops the elevator and enters emergency mode |
| `door_obstruction` | Keeps the door open while an obstruction is detected |
| `upper_limit_switch` | Prevents upward movement beyond the upper limit |
| `lower_limit_switch` | Prevents downward movement beyond the lower limit |

---

## Project Structure

```text
.
├── elevator_controller.c    # Core elevator controller logic
├── elevator_controller.h    # Controller states, data structures, and function declarations
├── main.c                   # Terminal-based elevator simulator
├── gui_simulator.c          # Graphical elevator simulator using raylib
├── stm32_main.c             # STM32 firmware entry-point scaffold
├── stm32_gpio_config.h      # STM32 pin mapping and GPIO configuration API
├── stm32_gpio_config.c      # STM32 GPIO debounce, input, and safe output layer
├── STM32_HARDWARE_PORTING.md # STM32 hardware integration checklist
└── README.md                # Project documentation
```

---

## Core Controller Design

The controller is implemented as a modular firmware-style component. The main API functions are:

```c
void Elevator_Init(ElevatorController *controller);
void Elevator_Update(ElevatorController *controller, ElevatorInputs inputs);
void Elevator_PrintStatus(const ElevatorController *controller);
```

### `Elevator_Init`

Initializes the elevator controller to a safe default condition:

- Current floor: 1
- Target floor: 1
- State: `STATE_IDLE`
- Motor: `MOTOR_STOP`
- Door: closed
- Request queue: empty

### `Elevator_Update`

Executes one controller update cycle. This function processes all input signals, updates the request queue, applies safety logic, and advances the finite-state machine.

In the simulation, each update cycle moves the elevator by one floor when the motor is active.

### `Elevator_PrintStatus`

Prints the current elevator status, including:

- Current floor
- Target floor
- Controller state
- Motor command
- Door status

---

## Request Queue Logic

The controller uses a FIFO request queue to store pending floor requests. Requests are accepted from:

- Cabin request buttons
- Hall UP request buttons
- Hall DOWN request buttons

The queue logic includes:

- Floor range validation
- Queue capacity checking
- Duplicate request prevention
- Automatic removal of served requests

---

## Terminal Simulation

The terminal simulator allows users to manually enter elevator requests and safety inputs.

### Compile Terminal Version

```bash
gcc -g main.c elevator_controller.c -o elevator_controller.exe
```

### Run Terminal Version

```bash
./elevator_controller.exe
```

On Windows Command Prompt or PowerShell, you can also run:

```bash
elevator_controller.exe
```

### Terminal Inputs

During execution, the program asks for:

- Cabin request floor
- Hall UP request floor
- Hall DOWN request floor
- Door obstruction status
- Emergency stop status
- Upper limit switch status
- Lower limit switch status

Enter `0` when there is no floor request.

---

## GUI Simulation

The GUI simulator provides a visual dashboard for the elevator system. It displays the elevator shaft, current floor, target floor, controller state, motor command, door status, and safety inputs.

The GUI is implemented using raylib and keeps the embedded controller logic separated from the visualization layer.

### GUI Features

- Visual elevator shaft
- Current floor indicator
- Target floor highlight
- Cabin movement visualization
- Floor request buttons
- Emergency stop button
- Door obstruction toggle
- Upper limit switch toggle
- Lower limit switch toggle
- Live display of state, motor, and door status

---

## Requirements

### For Terminal Simulation

- GCC compiler
- C standard library

### For GUI Simulation

- GCC compiler
- raylib
- Windows/MSYS2 environment recommended for the provided build command

---

## Compile GUI Version on Windows/MSYS2

After installing raylib, compile the GUI simulator using:

```bash
gcc gui_simulator.c elevator_controller.c -o gui_simulator.exe -lraylib -lopengl32 -lgdi32 -lwinmm
```

### Run GUI Version

```bash
./gui_simulator.exe
```

Or on Windows:

```bash
gui_simulator.exe
```

---

## Example Usage

### Terminal Simulation Example

```text
Embedded Elevator Controller Simulation Started
Valid floors: 1 to 10

Cabin request floor 1 to 10, or 0 for no cabin request: 5
Hall UP request floor 1 to 10, or 0 for no hall UP request: 0
Hall DOWN request floor 1 to 10, or 0 for no hall DOWN request: 0
Door obstruction? 1=yes, 0=no: 0
Emergency stop? 1=yes, 0=no: 0
Upper limit switch active? 1=yes, 0=no: 0
Lower limit switch active? 1=yes, 0=no: 0
```

The controller then updates its state and moves the elevator toward the requested floor.

---

## Design Highlights

### 1. Modular Firmware-Style Architecture

The controller logic is implemented independently from the terminal and GUI interfaces. This makes the project easier to test, extend, and port to embedded hardware.

### 2. Finite-State Machine Control

The elevator behavior is modeled using explicit operating states. This improves readability and makes the controller behavior predictable.

### 3. Safety-First Logic

Emergency stop has the highest priority and immediately disables motor movement. Limit switches prevent movement beyond valid floor boundaries, and door obstruction keeps the door open.

### 4. GUI and Logic Separation

The GUI simulator only sends user input events to the controller. It does not directly modify the control behavior, which preserves the separation between application logic and visualization.

---


## STM32 Hardware Firmware

The repository includes an STM32-oriented firmware scaffold that connects the shared elevator controller to GPIO inputs and outputs. The STM32 layer now separates board-specific GPIO configuration into `stm32_gpio_config.h` and `stm32_gpio_config.c`, debounces inputs, uses active-low pull-up button logic, supports optional floor-position sensors, and applies safe motor-output interlocking before driving motor direction pins.

This firmware must still be imported into a STM32CubeIDE project generated for the exact target MCU and board. Before flashing real hardware, replace the example clock configuration with the CubeMX-generated `SystemClock_Config`, verify every pin against the schematic, use external motor/door driver hardware, and wire emergency stop and final limit switches into a hardware safety path. See `01_mcu_elevator_control_firmware/STM32_HARDWARE_PORTING.md` for the full checklist.

---

## Possible Future Improvements

- Add direction-aware request scheduling
- Add separate cabin and hall request indicators
- Add door opening and closing timers
- Add acceleration and deceleration profiles
- Add support for multiple elevators
- Add unit tests for controller states and edge cases
- Add hardware abstraction layer for GPIO-based deployment
- Port the controller logic to an embedded board such as STM32, Arduino, or Raspberry Pi Pico

---

## Technologies Used

- C programming language
- GCC compiler
- raylib graphics library
- Finite-state machine design
- Embedded control logic concepts

---

## Learning Objectives

This project is useful for practicing:

- Embedded C programming
- State-machine implementation
- Real-time control logic
- Safety-critical input handling
- Modular software design
- GUI-based simulation of embedded systems
- Separating firmware logic from visualization logic

---

## Author

Mohammad Hafezan

---