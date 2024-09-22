# Pico wh Game Controller Project

This project uses the **TinyUSB** stack and the **Raspberry Pi Pico** to construct a USB gaming controller. The game controller uses the USB Human Interface Device (HID) protocol to transmit data to a host, such as a PC, and reads input from buttons and an analogue joystick.

---

## Introduction

The **Pico wh Game Controller Project** demonstrates how to implement a USB HID game controller using a Raspberry Pi Pico microcontroller. It features the following:
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

- **Raspberry Pi Pico wh**: The microcontroller that interfaces with the buttons and joystick.
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
 
- **LED**: Connect the LED to GPIO 18

---

## Compilation and Building
To build the project from scratch, follow the steps given below, for this project I have used Windows Subsystem for Linus - Ubuntu in Windows 11:

**Note:** When you see a path like _/mnt/c/Users/Disha_ don't forget to replace it with the path to your own user folder on Windows. 

### Getting the SDK and the examples
```
cd /mnt/c/Users/Disha/
mkdir pico
cd pico

git clone -b master https://github.com/raspberrypi/pico-sdk.git
cd pico-sdk
git submodule update --init
cd ..
git clone -b master https://github.com/raspberrypi/pico-examples.git
```
### Installing the compilers etc
```
sudo apt update
sudo apt install gcc-arm-none-eabi libnewlib-arm-none-eabi build-essential libssl-dev tk tkinter python3-tk
```
### Build CMake from source


```
wget https://github.com/Kitware/CMake/releases/download/v3.19.4/cmake-3.19.4.tar.gz
tar -zxvf cmake-3.19.4.tar.gz
cd cmake-3.19.4/
./bootstrap
make
sudo make install
```

### Set the Pico-Sdk path
```
export PICO_SDK_PATH="/mnt/c/Users/Disha/pico/pico-sdk"
```

### Clone the Project
```
git clone -b master https://github.com/DishaMohindru1/Game_controller.git
```

### Build the Project
   Navigate to the project directory and use CMake to build the project:
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

## Upload the Firmware
   Once the build is complete, upload the `.uf2` file to the Raspberry Pi Pico:
   - Press and hold the **BOOTSEL** button while plugging the Pico into your computer.
   - a window will appear on your system
   - Drag and drop the `.uf2` file (created in the build file after running the make command) onto the Pico’s storage device that appears.
   - the window will close automatically, it indicates the raspberry pi is re-booted and has started working as a game controller.
   - the LED will start blinking indicating that it is mounted on the system.

---

## Usage

Once the firmware is uploaded, the Raspberry Pi Pico will act as a USB game controller. You can verify the functionality by:

1. connecting the Pico to a computer. It should be recognised as a USB gamepad by the operating system.
2. going to the control panel > devices and printers(open in new window) > right click on the tinyusb > game_controller settings > properties. Here, all the buttons and joystick movements can be tested.


---
