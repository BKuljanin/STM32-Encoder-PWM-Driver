# STM32-Encoder-PWM-Driver

# AS5600 Magnetic Encoder PWM Capture with Angular Speed Calculation on STM32 NucleoF446RE

**Note:** This project is created in STM32CubeIDE for convenience of project management, compilation, and debugging. However, **timer input capture (TIM3)** is configured manually in **bare-metal code** using register-level manipulation. The I2C peripheral uses the HAL driver for AS5600 configuration. This combination provides direct hardware control over the time-critical PWM capture path while using HAL for one-time sensor setup.

---

## Overview

This project implements a **PWM-based angular position and speed measurement system** using the **AS5600 magnetic rotary encoder**. The AS5600 is configured over I2C to output its angular position as a **920 Hz PWM signal**, where the duty cycle is proportional to the magnet angle (0 to 360 degrees). The STM32's **TIM3** is configured in **PWM input capture mode** on PA6 to measure both the pulse width and period of the incoming signal. From these measurements, the angle is computed as a duty cycle ratio, and angular speed is derived by differentiating consecutive angle samples. A **first-order IIR low-pass filter** is applied to the speed signal to suppress noise amplified by the differentiation.

---

## Key Features

* **Bare-Metal Timer Input Capture**
  TIM3 configured in PWM input capture mode using direct register access. CH1 captures the rising edge, CH2 captures the falling edge on the same input (TI1).

* **I2C Sensor Configuration**
  AS5600 CONF register written over I2C to switch the output stage from default analog to digital PWM at 920 Hz.

* **Angle from Duty Cycle**
  Angular position computed from the ratio of pulse width to period, mapped to 0-360 degrees.

* **Angular Speed with Low-Pass Filtering**
  Speed derived from consecutive angle differences, filtered with a configurable first-order IIR filter (150 Hz cutoff) to attenuate noise amplified by differentiation.

---

## Core Files

* **`main.c`**
  Program entry point, system initialization, and main loop that processes angle and speed calculations when a new PWM measurement is ready.

* **`timer.c`**
  Bare-metal configuration of **TIM3 CH1/CH2** in PWM input capture mode on PA6. Contains the `TIM3_IRQHandler` which captures pulse width and period from CCR1/CCR2 registers.

* **`i2c.c`**
  HAL-based I2C1 initialization and AS5600 configuration. Writes the CONF register (0x08) to enable PWM output mode at 920 Hz.

* **`as5600.c`**
  Angle computation from duty cycle ratio and angular speed calculation with first-order IIR low-pass filter.

---

## Hardware Connections

| Component         | STM32 Pin | Function                        |
| ----------------- | --------- | ------------------------------- |
| AS5600 VCC        | 3.3 V     | Power Supply                    |
| AS5600 GND        | GND       | Common Ground                   |
| AS5600 SCL        | PB6       | I2C1 Clock                      |
| AS5600 SDA        | PB7       | I2C1 Data                       |
| AS5600 OUT        | PA6       | PWM Output to TIM3 CH1 Input    |
| AS5600 DIR        | GND       | Rotation Direction (CW)         |

> **Important:** I2C requires pull-up resistors on SDA and SCL. Use **4.7 kOhm or 10 kOhm** external pull-ups to 3.3 V.

---

## Clock Configuration

* **System Clock:** 84 MHz (HSI 16 MHz, PLL: PLLM=16, PLLN=336, PLLP=4)
* **APB1 Timer Clock:** 84 MHz (APB1 prescaler = /2, timer clock = 2x APB1)
* **TIM3 Prescaler:** 83 (84 MHz / 84 = **1 MHz** timer tick = 1 us resolution)
* **I2C1 Clock:** 100 kHz (standard mode)

---

## PWM Input Capture

TIM3 is configured to capture the AS5600 PWM signal using two channels on the same input pin (TI1):

* **CH1 (CCR1):** Captures counter value on the **rising edge**
* **CH2 (CCR2):** Captures counter value on the **falling edge**

On each rising edge interrupt, the pulse width and period are computed:

$$PulseWidth = CCR2_{falling} - CCR1_{previous\ rising}$$

$$Period = CCR1_{current\ rising} - CCR1_{previous\ rising}$$

---

## Angle Calculation

The AS5600 encodes angular position as PWM duty cycle. The angle is recovered from:

$$Angle = \frac{PulseWidth}{Period} \times 360°$$

This yields a 12-bit equivalent resolution (4096 steps over 360 degrees, approximately 0.088 degrees per step).

---

## Speed Calculation and Filtering

Angular speed is derived by differentiating consecutive angle measurements:

$$\omega_{raw} = \frac{Angle_{current} - Angle_{previous}}{Period\ [seconds]}$$

A first-order IIR low-pass filter is applied to suppress noise amplified by differentiation:

$$\alpha = \frac{2\pi \cdot \Delta t \cdot f_c}{1 + 2\pi \cdot \Delta t \cdot f_c}$$

$$\omega_{filtered}[n] = \alpha \cdot \omega_{raw}[n] + (1 - \alpha) \cdot \omega_{filtered}[n-1]$$

Where $f_c$ = 150 Hz cutoff frequency and $\Delta t$ = period in seconds.

---

## AS5600 Register Configuration

| Register | Address | Value  | Description                          |
| -------- | ------- | ------ | ------------------------------------ |
| CONF (L) | 0x08    | 0xE0   | OUTS = 10 (PWM), PWMF = 11 (920 Hz) |

The CONF register is a 14-bit register split across addresses 0x07 (high byte) and 0x08 (low byte). The output stage and PWM frequency settings are in the low byte at 0x08.

---

## Reference Materials

All register settings, timer configurations, and GPIO modes in this code are implemented based on:

- **Reference Manual:** RM0390 Rev 7
- **Datasheet:** DS10693 Rev 10
- **User Manual:** UM1724 Rev 17
- **Cortex M4 Generic User Guide:** DUI 0553A
- **Sensor Documentation:** AS5600 Magnetic Rotary Position Sensor Datasheet
