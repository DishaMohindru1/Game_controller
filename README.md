# Pico Game Controller Project

This project uses the **TinyUSB** stack and the **Raspberry Pi Pico** to construct a USB gaming controller. The game controller uses the USB Human Interface Device (HID) protocol to transmit data to a host, such as a PC, and reads input from buttons and an analogue joystick.

---

## Introduction

The **Pico Game Controller Project** demonstrates how to implement a USB HID game controller using a Raspberry Pi Pico microcontroller. It features the following:
- **Analog joystick input** (read via the ADC on the Pico).
- **Digital button inputs** (mapped to GPIO pins).
- **USB communication** using the HID protocol to send the gamepad state to a host machine.

This project is great for learning how to utilise the USB HID protocol to connect with a host machine and how to interface devices, such as joysticks and buttons, with a microcontroller.

---

## Project Structure

The project consists of the following key files:

- **main.c**: includes setup and job scheduling for the gaming controller's main loop..
- **pico_hid.c**: creates the HID report, manages button and joystick input reading, and updates it in accordance with the controller state.
- **pico_hid.h**: header file that defines the structures and methods for processing HID jobs and reading inputs.
- **tusb_config.h**: TinyUSB configuration file that specifies different settings for the USB stack and HID class.
- **usb_descriptors.c**: defines the HID report descriptors, configuration descriptors, and device descriptors for USB.
- **usb_descriptors.h**: Header file for the USB descriptors.

---

## Hardware Components

- **Raspberry Pi Pico**: The microcontroller that interfaces with the buttons and joystick.
- **Buttons**: Digital buttons connected to GPIO pins, used for gamepad actions.
- **Analog Joystick**: An analog joystick connected to ADC pins, used for directional input.
- **GPIO 18**: An external LED (optional) connected to GPIO 18, used for status indication.

---

## Software Components

- **TinyUSB**: A lightweight USB host/device stack that implements USB communication on the Raspberry Pi Pico.
- **HID Class**: The USB Human Interface Device (HID) class is used to send gamepad data to the host.

### Code Overview

1. **main.c**
   - Initializes the system (USB stack, GPIO, ADC).
   - Schedules the tasks for reading inputs and sending HID reports.

2. **pico_hid.c**
   - Handles reading input from buttons and joystick.
   - Updates the gamepad state (HID report) with the current inputs.
   - Sends the HID report to the host.

3. **tusb_config.h**
   - Defines USB configuration parameters, such as buffer sizes and endpoint configuration.

4. **usb_descriptors.c**
   - Defines the USB descriptors, such as device descriptor, HID report descriptor, and configuration descriptor.

---

## Setup

### Prerequisites

- **Raspberry Pi Pico**.
- **Pico SDK**: Ensure you have the Raspberry Pi Pico SDK installed.
- **TinyUSB Library**: The project uses TinyUSB for USB communication.
- **Button and Joystick**: Connected to GPIO pins as defined in the project.

### Wiring

- **Buttons**: Connect buttons to the GPIO pins specified in `pico_hid.c`. Example:
  - Button 1 (South): GPIO 7
  - Button 2 (East): GPIO 8
  - Button 3 (North): GPIO 5
  - Button 4 (West): GPIO 6
  - Mode Button: GPIO 9
  - Select Button: GPIO 20
  - Start Button: GPIO 21

- **Joystick**: Connect the X-axis and Y-axis of the joystick to the ADC pins:
  - X-axis: GPIO 26
  - Y-axis: GPIO 27

---

## Compilation and Building

1. **Clone the Project**:
   Clone the project and ensure that the **Pico SDK** is installed and initialized.

2. **Build the Project**:
   Navigate to the project directory and use CMake to build the project:
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

3. **Upload the Firmware**:
   Once the build is complete, you can upload the `.uf2` file to the Raspberry Pi Pico:
   - Press and hold the **BOOTSEL** button while plugging the Pico into your computer.
   - Drag and drop the `.uf2` file onto the Pico’s storage device that appears.

---

## Usage

Once the firmware is uploaded, the Raspberry Pi Pico will act as a USB game controller. You can verify the functionality by:

1. connecting the Pico to a computer. It should be recognised as a USB gamepad by the operating system.
2. The host's gamepad should display the buttons pressed and the joystick movements.


---
