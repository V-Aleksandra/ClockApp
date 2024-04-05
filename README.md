# Clock with Alarm

This project implements a clock with an alarm feature. Upon starting the program, the user inputs the current (initial) time via UART:

- Enter current time: 18:02:42


The input format is HH:MM:SS, where HH represents hours, MM represents minutes, and SS represents seconds. After successful input, the program prints the current time to the PC terminal via UART every second:

- Current time: 18:02:43
- Current time: 18:02:44
- Current time: 18:02:45
- Current time: 18:02:46
- Current time: 18:02:47
- Current time: 18:02:48
- ...


The program also displays the current time on the LCD screen of the Sensor Kit and updates it every second:
At any time, the user can send a new time to the microcontroller via the PC terminal and UART, specifying the desired alarm activation time. When the current time (set at the beginning of the program) reaches the alarm time (specified via a subsequent UART message), the buzzer on the Sensor Kit is turned on and held on for 5 seconds.


## Technical Details

The development board used is the "Tiva TM4C123G LaunchPad Evaluation Board (EK-TM4C123GXL)" with Seeed Arduino Sensor Kit Base, which integrates sensors and actuators. The program is implemented using FreeRTOS.
