# Speedometer — ATmega328P Embedded System
 
An embedded systems project built on the Arduino (ATmega328P) that measures the speed of a moving object using an ultrasonic rangefinder and displays the results on an LCD screen.
 
## How It Works
 
Two distance measurements are taken to an object using an ultrasonic sensor. By tracking the time between the two measurements, the device calculates how fast the object is moving and displays the speed in cm/sec on the LCD.
 
- Press **Start** to take the first range measurement
- Press **Stop** to take the second range measurement
- The device calculates and displays the object's speed, along with both distances and the elapsed time between measurements
## Features
 
- **Ultrasonic rangefinder** — measures distances up to 400 cm with 0.1 cm precision
- **LCD display** — shows both range measurements, elapsed time, calculated speed, and speed threshold
- **Rotary encoder** — adjusts a speed threshold (1–99 cm/sec) stored in non-volatile EEPROM memory so the setting persists across power cycles
- **RGB LED** — lights red if speed exceeds the threshold, green if at or below, and blue if no valid measurement has been made
- **Servo motor dial** — visually counts down the 10-second window between the first and second measurement
- **Serial link** — transmits the local speed to a remote device and receives/displays the remote device's speed
- **Buzzer** — plays an ascending or descending three-tone sequence depending on whether the received remote speed is above or below the local threshold
## Hardware
 
| Component | Interface |
|---|---|
| Ultrasonic range sensor | Digital I/O + Pin Change Interrupt |
| RGB LED (common anode) | 3x Digital I/O |
| Rotary encoder | 2x Digital I/O + interrupt |
| Servo motor | PWM via Timer/Counter2 |
| Serial link (74HCT125 tri-state buffer) | USART0 |
| Buzzer | PWM via Timer/Counter0 |
| Speed threshold | EEPROM (ATmega328P internal) |
 
## Implementation Notes
 
- All timing and distance calculations use **fixed-point integer arithmetic** — no floating point
- **TIMER1** is used for both pulse-width measurement (rangefinder echo) and stopwatch timing between measurements
- **TIMER0** drives the buzzer tone generation via ISR
- **TIMER2** generates the servo PWM signal
- Serial communication uses a simple `@[speed]$` ASCII protocol at 9600 baud with interrupt-driven receiving and error recovery
- Code is organized across multiple `.c` source files by subsystem (LCD, encoder, timers, serial)
 
