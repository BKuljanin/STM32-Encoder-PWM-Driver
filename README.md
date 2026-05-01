# AS5600 Absolute Magnetic Encoder PWM Driver on STM32F446RE

PWM based angular position and speed measurement using the AS5600 magnetic rotary encoder on an STM32 Nucleo F446RE. The AS5600 is configured over I2C to output a 920 Hz PWM signal where the duty cycle represents the magnet angle. TIM3 captures the pulse width and period using hardware input capture, and the firmware computes angle and angular speed from those measurements.

## Core Files

`timer.c` configures TIM3 in PWM input capture mode on PA6 using direct register access. CH1 captures the rising edge, CH2 captures the falling edge on the same input (TI1). The ISR computes pulse width and period from CCR1/CCR2 on each rising edge interrupt.

`i2c.c` initializes I2C1 with HAL and writes the AS5600 CONF register (0x08) to switch the output from default analog to PWM at 920 Hz.

`as5600.c` computes angle from the duty cycle ratio with dead zone compensation, and derives angular speed by differentiating filtered angle samples. Two stage LP filtering is applied: first on angle (before differentiation), then on speed.

`main.c` initializes everything and runs the main loop, processing angle and speed on each new PWM measurement.

## How the Angle is Computed

The AS5600 does not use the full 0% to 100% duty cycle for angle. Each PWM cycle is 4351 ticks total: 128 ticks always LOW, 4095 ticks representing 0 to 360 degrees, and 128 ticks always HIGH. This guarantees the output is never a flat line so the MCU can always detect edges.

The raw duty cycle is converted to angle by removing this bias:

$$raw = duty \times 4351 - 128$$

Clamped to 0 to 4095, then:

$$angle = \frac{raw}{4095} \times 360°$$

## Speed Calculation

Angular speed is the derivative of consecutive filtered angle samples:

$$\omega_{raw} = \frac{\Delta angle}{period\ [seconds]}$$

The angle is unwrapped before filtering to prevent the LP filter from seeing a false 360 degree jump at the 0/360 boundary. After filtering, the angle is wrapped back to 0 to 360.

A first order IIR low pass filter is used on both angle and speed:

$$\alpha = \frac{2\pi \cdot \Delta t \cdot f_c}{1 + 2\pi \cdot \Delta t \cdot f_c}$$

$$y[n] = \alpha \cdot x[n] + (1 - \alpha) \cdot y[n-1]$$

Angle cutoff: 30 Hz. Speed cutoff: 20 Hz.

## PWM Input Capture

TIM3 runs at 1 MHz (84 MHz / 84 prescaler, 1 us resolution). Both channels listen on the same pin (TI1):

| Channel | Edge    | Register | Measures          |
|---------|---------|----------|-------------------|
| CH1     | Rising  | CCR1     | Period            |
| CH2     | Falling | CCR2     | Pulse width       |

On each rising edge interrupt:

$$PulseWidth = CCR2_{falling} - CCR1_{previous\ rising}$$

$$Period = CCR1_{current\ rising} - CCR1_{previous\ rising}$$

Input capture filter is enabled on both channels to reject noise from edge ringing.

## Hardware Connections

| Component         | STM32 Pin | Function                        |
|-------------------|-----------|----------------------------------|
| AS5600 VCC        | 3.3 V     | Power Supply                    |
| AS5600 GND        | GND       | Common Ground                   |
| AS5600 SCL        | PB6       | I2C1 Clock                      |
| AS5600 SDA        | PB7       | I2C1 Data                       |
| AS5600 OUT        | PA6       | PWM Output to TIM3 CH1 Input    |
| AS5600 DIR        | GND       | Rotation Direction (CW)         |
| AS5600 PGO        | 3.3 V     | HIGH enables output pin         |

I2C requires pullup resistors on SDA and SCL. This project uses 5 kOhm external pullups to 3.3 V.

## Clock Configuration

| Parameter          | Value                                              |
|--------------------|----------------------------------------------------|
| System Clock       | 84 MHz (HSI 16 MHz, PLL: PLLM=16, PLLN=336, PLLP=4) |
| APB1 Timer Clock   | 84 MHz (APB1 prescaler /2, timer clock 2x APB1)   |
| TIM3 Prescaler     | 83 (84 MHz / 84 = 1 MHz, 1 us resolution)         |

## AS5600 Register Configuration

| Register | Address | Value | Description                          |
|----------|---------|-------|--------------------------------------|
| CONF (L) | 0x08    | 0xE0  | OUTS = 10 (PWM), PWMF = 11 (920 Hz) |

The CONF register is split across 0x07 (high byte) and 0x08 (low byte). Output stage and PWM frequency settings are in the low byte.

## Reference Materials

* RM0390 Rev 7 (STM32F446 Reference Manual)
* DS10693 Rev 10 (STM32F446 Datasheet)
* UM1724 Rev 17 (Nucleo 64 User Manual)
* DUI 0553A (Cortex M4 Generic User Guide)
* AS5600 Datasheet (ams AG)
