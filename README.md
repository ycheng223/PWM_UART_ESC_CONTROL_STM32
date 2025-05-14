Baremetal implementation that bridges communication with VESC firmware to enable BLDC motor control using a computer through USB and STM32. Uses UART to communicate with the computer via USB and maps commands to PWM signal which are then transmitted to the ESC.

Requires PUTTY (for serial communication with USB port) and VESC to interface with the ESC. The ESC should have PPM input pins for two motors.

VESC should be configured to use PPM in App Settings with Control Type Duty Cycle

![image](https://github.com/user-attachments/assets/301c3701-9792-4ffe-94b8-05894c9145e6)

![image](https://github.com/user-attachments/assets/059bc42d-0e93-4d84-8005-6cdc22be5d67)

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
