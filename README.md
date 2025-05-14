Baremetal implementation that bridges communication with VESC firmware to enable BLDC motor control using a computer through USB and a STM32F411CE Nucleo Board (https://www.st.com/en/evaluation-tools/nucleo-f411re.html). Uses UART2 on the NUCLEO to receive keystroke input from a computer via USB and maps it to two PWM signals used by VESC to control the RPM of two BLDC motors. The ratio of these RPMs then determines the direction of the robot.

PWM waveforms are generated from morpho pins PC6 and PB6 on the STM32F411CE so these should be wired to the PPM/PWM pins on the ESC.

Pins PA2 (TX) and PA3 (RX) are also used for USART2.

Keyboard inputs are mapped as:

W = Forward  
S = Reverse  
A = Left  
D = Right  
Q = Forward Left  
E = Forward Right  
Z = Reverse Left  
C = Reverse Right  
[ = Speed Up  
] = Slow Down  
' ' = Standby  

For ease of use, recommend mapping the keyboard inputs to a wireless gamepad or joystick using a keyboard emulator like Joy2Key.

Requires PUTTY (for serial communication with USB port) and VESC to interface with the ESC. The ESC should have PPM input pins for two motors.

VESC should be configured to use PPM in App Settings with Control Type Duty Cycle

![image](https://github.com/user-attachments/assets/301c3701-9792-4ffe-94b8-05894c9145e6)

![image](https://github.com/user-attachments/assets/059bc42d-0e93-4d84-8005-6cdc22be5d67)
