### Introduction
"space-invaders-freeRTOS-STM32" is a project that implements the classic game of Space Invaders on an STM32 microcontroller using FreeRTOS.

The project includes all the necessary source code, libraries, and configuration files to build and run the game on an STM32 development board. The project is built using the Keil MDK-ARM toolchain, but it can be adapted to other toolchains as well.

The game's graphics are rendered using an SSD1306 OLED display, and the player's movements and shots are controlled by a joystick and button connected to the STM32 board. The game logic is implemented using FreeRTOS tasks and queues, which allows for efficient and synchronized multitasking.

The repository also includes detailed documentation on the project's architecture, configuration, and usage. The documentation covers the hardware setup, software installation, and step-by-step instructions on how to build and run the game.

Overall, the "space-invaders-freeRTOS-STM32" repository is an excellent resource for anyone looking to learn how to implement real-time applications using FreeRTOS on an STM32 microcontroller. It provides a practical example of how to use multitasking, synchronization, and I/O operations to create a fun and engaging game.

### step-by-step guid
#### Hardware Setup:

1.Connect an STM32 development board to your computer using a USB cable.
2.Connect an SSD1306 OLED display to the STM32 board using the I2C interface.
3.Connect a joystick and button to the STM32 board using the GPIO pins.

#### Software Installation:

1.Install the Keil MDK-ARM toolchain on your computer.
2.Install the STMicroelectronics CubeMX software on your computer.
3.Clone the "space-invaders-freeRTOS-STM32" repository to your local machine using Git.

#### Building the Project:

1.Open CubeMX and import the project's configuration file from the "Config" folder of the repository.
2.Generate the project code using CubeMX.
3.Open Keil MDK-ARM and open the project file (.uvprojx) located in the "MDK-ARM" folder of the repository.
4.Build the project by clicking the "Build" button in Keil MDK-ARM.
#### Uploading the Game:

1.Connect the STM32 board to your computer using a USB cable.
2.Use a JTAG/SWD debugger to upload the compiled binary file (.bin) to the STM32 board.
3.Disconnect the JTAG/SWD debugger and reset the STM32 board.
The game should now be running on the STM32 board and display on the OLED screen.

#### Playing the Game:

1.Use the joystick to move the player left or right.
2.Use the button to fire shots at the incoming alien invaders.
3.Try to survive as long as possible and beat your high score!
That's it! You should now have the "space-invaders-freeRTOS-STM32" game up and running on your STM32 board. If you encounter any issues or have questions, please inform me via email.

for more details about free-RTOS and its API you can refer to link below :
https://www.keil.com/pack/doc/CMSIS/RTOS2/html/modules.html
