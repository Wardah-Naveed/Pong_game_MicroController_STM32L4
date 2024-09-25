# Pong Game on STM32 Microcontroller

## Technologies needed:
-> STM32Cube IDE 1.14 and above
-> Windows 11

## Project Overview
This project implements a retro-inspired "Pong" game using the STM32L475 microcontroller, SSD1306 OLED display, and FreeRTOS. It showcases how embedded systems with limited resources can be programmed to perform interactive tasks in real-time, including game logic, display management, and user input handling.

## Table of Contents
1. [Introduction](#introduction)
2. [Motivation](#motivation)
3. [Hardware Components](#hardware-components)
4. [Software Components](#software-components)
5. [Working](#working)
6. [Development Environment](#development-environment)
7. [Conclusion](#conclusion)

## Introduction
The project demonstrates the integration of limited hardware resources with real-time software to create an interactive gaming experience. The aim is to use the STM32L475's capabilities to implement a classic game, "Pong," while highlighting its power management features, making it suitable for real-time applications.

## Motivation
The motivation behind the project is to explore how microcontroller-based embedded systems can handle interactive applications under constrained resources. Pong, as one of the foundational video games, is selected for its simplicity, allowing us to showcase efficient multitasking, resource management, and FreeRTOS capabilities.

## Hardware Components
- **Microcontroller: STM32L475**
  - Cortex-M4 core running at 80 MHz
  - 1 MB Flash memory and 128 KB RAM
  - Ideal for real-time embedded applications with efficient power management.
  
- **Display: SSD1306 OLED**
  - Resolution: 128x64 pixels (monochrome)
  - Interface: I2C/SPI for efficient communication with the microcontroller.
  
- **Input Devices**
  - Three buttons: LEFT, RIGHT, and WAKEUP, for paddle control and game reset.
  
- **Audio Feedback**
  - Buzzer provides sound feedback for events like paddle hits or game over.

## Software Components
- **Operating System: FreeRTOS**
  - Efficient task management and multitasking features.
  - Critical task prioritization for game logic and display updates.
  
- **Libraries and Headers**
  - Utilized libraries include:
    - `ssd1306.h` (for display control)
    - `ssd1306_fonts.h` (for font management on OLED)
  
- **Pin Definitions**
  - Buttons and buzzer are connected to specific GPIO pins.
  
- **Game Logic**
  - Variables like ball position, velocity, and paddle control are used to handle game states and updates.

## Working
1. **Task Definitions**
    - **GameTask**: Manages ball movement, collision detection, score updates, and game-over conditions.
    - **PaddleTask**: Reads input from LEFT and RIGHT buttons to move the paddle.
    - **DisplayTask**: Updates the OLED screen with the current game visuals, including ball, paddle, and score.
    - **GameOverTask**: Handles the game-over state and resets the game when the WAKEUP button is pressed.

2. **Game States & Variables**
   - Ball position (`ball_x`, `ball_y`, etc.), paddle position, scores, and game state variables are used to maintain and update the game in real-time.

## Development Environment
- **IDE**: STM32 Cube IDE
  - The project is developed and compiled using STM32 Cube IDE, which provides an integrated environment for writing, compiling, and debugging STM32 microcontroller-based applications.
  
- **Programming Language**: C
  - All code is written in C, and the project utilizes STM32 HAL (Hardware Abstraction Layer) for hardware interfacing.
  
- **Build and Flashing**
  - The code is compiled in STM32 Cube IDE and flashed onto the STM32L475 microcontroller using ST-LINK.

## Conclusion
The project successfully demonstrates the capabilities of the STM32L475 microcontroller and FreeRTOS in managing interactive applications with constrained resources. It provides a fun and interactive way to understand multitasking, power management, and real-time performance in embedded systems.

## How to Run
1. Clone the repository to your local machine.
2. Open the project in **STM32 Cube IDE**.
3. Ensure that FreeRTOS and necessary libraries are included in your build environment.
4. Follow the pin configuration as outlined in the hardware section.
5. Compile and upload the code to the STM32L475 microcontroller using ST-LINK.

## License
This project is licensed under the MIT License.

