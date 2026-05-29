# STM32 Hardware Porting Checklist

The STM32 firmware in this folder is now structured as a hardware porting layer plus the reusable elevator controller logic. It is still intended to be imported into a STM32CubeIDE project that is generated for the exact microcontroller and board you are using.

## Files to Add to STM32CubeIDE

Add these files to the CubeIDE project source tree:

- `elevator_controller.c`
- `elevator_controller.h`
- `stm32_main.c`
- `stm32_gpio_config.c`
- `stm32_gpio_config.h`

If your generated CubeIDE project already has a `main.c`, either replace its user-code sections with the logic from `stm32_main.c` or rename `stm32_main.c` to `main.c` inside the CubeIDE project.

## Required Board-Specific Steps

1. Generate a STM32CubeIDE project for your exact STM32 part number and board.
2. Replace the default `SystemClock_Config()` in `stm32_main.c` with the CubeMX-generated clock configuration for your board.
3. Set `ELEVATOR_STM32_HAL_HEADER` if your project does not use `stm32f4xx_hal.h`.
4. Verify every GPIO assignment in `stm32_gpio_config.h` against your schematic and package pinout.
5. Confirm input polarity. The default assumes pull-up inputs where button/switch activation pulls the pin low.
6. Use motor drivers, relays, or contactors between STM32 GPIO pins and the motor/door hardware. Do not drive a motor directly from GPIO.
7. Put emergency stop and final limit switches in the hardware power/interlock path. The software emergency and limit handling is a secondary protection layer.
8. Enable `ELEVATOR_ENABLE_FLOOR_SENSORS` only after wiring and validating real floor sensors.
9. Enable `DEBUG_UART_ENABLED` only after retargeting `printf()` to a UART.
10. Enable `ELEVATOR_ENABLE_IWDG` only after validating watchdog timing on the target board.

## Default GPIO Profile

| Signal | Default Pin(s) | Direction | Active State |
|---|---:|---|---|
| Cabin floor 1-8 buttons | PA0-PA7 | Input pull-up | Low |
| Cabin floor 9-10 buttons | PB0-PB1 | Input pull-up | Low |
| Emergency stop | PC0 | Input pull-up | Low |
| Door obstruction | PC1 | Input pull-up | Low |
| Upper limit switch | PC2 | Input pull-up | Low |
| Lower limit switch | PC3 | Input pull-up | Low |
| Motor up command | PD0 | Output push-pull | High |
| Motor down command | PD1 | Output push-pull | High |
| Door open command | PD2 | Output push-pull | High |
| Hall up shortcut | PE0 | Input pull-up | Low |
| Hall down shortcut | PE1 | Input pull-up | Low |
| Optional floor sensors | PF0-PF9 | Input pull-up | Low |

## Firmware Safety Improvements Included

- Project-specific GPIO wrapper names are used instead of redefining STM32 HAL functions.
- Button, hall request, safety, and optional floor-sensor inputs are debounced every millisecond.
- Active-low input polarity matches the pull-up switch configuration.
- Motor direction outputs are interlocked so up and down commands are not intentionally driven at the same time.
- A dead-time is inserted before energizing the opposite direction after a direction change.
- The main elevator update runs in the main loop instead of doing all work inside `SysTick_Handler()`.
- Optional independent watchdog support is available behind `ELEVATOR_ENABLE_IWDG`.
- Real floor-position feedback can be enabled with `ELEVATOR_ENABLE_FLOOR_SENSORS`.

## Remaining Hardware Validation

Before using this with real moving hardware, validate the following on a bench setup:

- GPIO voltage levels and current limits.
- Relay or motor-driver input requirements.
- Emergency stop hardware interruption path.
- Limit switch behavior at both travel extremes.
- Floor sensor noise, alignment, and ambiguity when between floors.
- Direction reversal dead-time.
- Watchdog reset behavior.
- Safe power-up and brownout behavior.
