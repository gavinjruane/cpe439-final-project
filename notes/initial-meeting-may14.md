# Initial Meeting

## Meeting Notes

- Trying to figure out vivado and move motors
    - Need a chassis because the rover is broken
- What do we actually want to happen in 2 and a half weeks?
    - Trying to create a remote controlled car
        - Use some of the existing code from rover
        - Josh has some sensors we could use
            - Bluetooth to UART
            - IMU
            - Ultrasonic sensor
            - Displays that can display data
        - Michael has code from his 316 project
    - We have one KD240 with one PMOD module
        - Motors should take 2 pins (with encoders, need 4)
        - Are we able to use some code but Crete some middle-line project that could suffice?
- We could use an ESP32 for wireless communication
    - We have just been blinking LEDs, sensors would be nice to use and experiment with
- Car could be good for FreeRTOS, many different tasks
    - Every sensor could be their their own module
    - Motor control task could be higher priority than others, for example
- How are we actually going to make the car?
    - We could buy a prebuilt chassis with motors already included, might even have an encoder on it
        - This would connect to a motor driver, and the MCU would connect to the motor driver
        - We could do a receiver/transmitter model with two boards
            - Bluetooth to COM port on PC
        - Could we just use the ESP32 for wireless? but we would have to figure out how to use FreeRTOS on the ESP32
            - platform.io vs Arduino IDE (probably easier)
        - Stretch goals include adding joystick with other MCU and adding object detection
    - Or we could CAD something and make our own chassis
        - We would have to ask someone to help us

## Implementation Steps

1. Create a repository
2. Diagram the tasks
    1. Second MCU a stretch goal
        1. First working on the laptop over Bluetooth with WASD
