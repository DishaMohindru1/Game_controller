/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "pico/stdlib.h"  // Standard I/O for Pico SDK (to control GPIOs, etc.)
#include "tusb.h"         // TinyUSB library for USB communication
#include "pico_hid.h"     // Custom header for gamepad HID reports
#include "hardware/adc.h" // Library to interact with the Analog-to-Digital Converter (ADC)

//----------------------- Components of Digital Systems -----------------------//
// Enum for joystick directions
// This enumeration helps to differentiate between the X and Y axes of the joystick. 
// In digital systems, input devices (like joysticks) need to be abstracted into software so that 
// the system can differentiate between their different inputs (e.g., which axis is being used).
typedef enum
{
  ADC_LEFT_JOY_X = 0,  // Left joystick X-axis
  ADC_LEFT_JOY_Y,      // Left joystick Y-axis
} joy_direction;

// Enum for source types (buttons or ADC channels)
// The system handles two types of inputs: buttons (digital) and joystick axes (analog). This
// enumeration represents the type of input source (digital button or analog joystick).
typedef enum
{
  SRC_BUTTON,  // Digital button input (GPIO pin)
  SRC_ADC      // Analog input (ADC for joystick)
} button_source_kind;

//----------------------- Components of Digital Systems (Input Devices) -----------------------//
// Struct to define a button source
// The system uses buttons as input devices. Each button has an action associated with it 
// (e.g., "jump", "shoot"), and the input is detected through a specific GPIO pin on the hardware.
typedef struct
{
  uint32_t action;   // Action associated with the button (e.g., GAMEPAD_BUTTON_SOUTH)
  uint8_t gpio_pin;  // The GPIO pin number where the button is connected
} button_source;

// Struct to define joystick values from ADC
// The joystick is an analog input device. Its position is measured using the Analog-to-Digital Converter (ADC).
// Here, we store the joystick's value (from the ADC) for the X and Y axes.
typedef struct
{
  uint16_t value;  // Holds the ADC value for the joystick axis (12-bit ADC value)
} joystick_adc_map_t;

static joystick_adc_map_t joy_map[2]; // Define joy_map for 2 axes (X and Y)

//----------------------- Components of Digital Systems (Digital Systems Architecture) -----------------------//

// Struct for ADC source (joystick analog input)
// Each joystick axis is connected to a specific ADC channel (X-axis or Y-axis). This struct defines
// the source of the joystick input, specifying the ADC channel used.
typedef struct
{
  joy_direction direction;  // Joystick direction (X or Y)
  uint8_t adc_channel;      // The ADC channel corresponding to this joystick axis
} adc_source;

// Union for input sources (buttons or joystick ADC)
// This union allows the system to handle multiple types of inputs (buttons or ADC for joystick)
// by encapsulating them into one data structure. Depending on the input type, it can store
// either button information or ADC data for the joystick.
typedef union
{
  adc_source adc_src;         // ADC input source (for joystick)
  button_source button_src;   // Button input source (for buttons)
} input_source;

// Struct to combine input source kind with input data
// This struct represents an input device in the system. It could either be a button (SRC_BUTTON)
// or an ADC channel for the joystick (SRC_ADC). The source field specifies which kind of input it is,
// and the data field contains the actual input data (button GPIO pin or ADC channel).
typedef struct
{
  button_source_kind source;  // Type of input (button or ADC)
  input_source data;          // Data associated with the input (button GPIO pin or ADC channel)
} button_data;

//----------------------- Components of Digital Systems (Input Devices) -----------------------//
// Button configuration
// This array defines the mapping of the gamepad buttons to specific GPIO pins.
// The system uses this configuration to know which GPIO pin corresponds to which button action.
// For example, the "South" button (ACTION = GAMEPAD_BUTTON_SOUTH) is connected to GPIO pin 7.
const button_data _button_config[] = {
    {SRC_BUTTON, {.button_src = {GAMEPAD_BUTTON_SOUTH, 7}}},   // South button on GPIO 7
    {SRC_BUTTON, {.button_src = {GAMEPAD_BUTTON_EAST, 8}}},    // East button on GPIO 8
    {SRC_BUTTON, {.button_src = {GAMEPAD_BUTTON_NORTH, 5}}},   // North button on GPIO 5
    {SRC_BUTTON, {.button_src = {GAMEPAD_BUTTON_WEST, 6}}},    // West button on GPIO 6
    {SRC_BUTTON, {.button_src = {GAMEPAD_BUTTON_MODE, 9}}},    // Mode button on GPIO 9
    {SRC_BUTTON, {.button_src = {GAMEPAD_BUTTON_SELECT, 20}}}, // Select button on GPIO 20
    {SRC_BUTTON, {.button_src = {GAMEPAD_BUTTON_START, 21}}}}; // Start button on GPIO 21

const int _button_config_count = 6;  // The total number of buttons configured

//----------------------- Components of Digital Systems -----------------------//
// Setup GPIO for buttons
// This function initializes the GPIO pins used by the buttons so that the system can detect button presses.
// For each button, it initializes the corresponding GPIO pin as an input and enables the pull-up resistor.
void setup_controller_buttons(void)
{
  for (int i = 0; i < _button_config_count; i++)  // Loop through each button configuration
  {
    // Initialize GPIO pin for the button
    gpio_init(_button_config[i].data.button_src.gpio_pin);
    gpio_set_dir(_button_config[i].data.button_src.gpio_pin, GPIO_IN);  // Set pin as input
    gpio_pull_up(_button_config[i].data.button_src.gpio_pin);  // Enable pull-up resistor
  }

  // Initialize ADC for joystick - Analog Input Devices
  // The joystick is an analog device, so we configure the ADC channels for the X and Y axes.
  adc_init();  // Initialize the ADC hardware
  adc_gpio_init(26);  // GPIO 26 connected to the joystick X-axis (ADC input)
  adc_gpio_init(27);  // GPIO 27 connected to the joystick Y-axis (ADC input)
}

//----------------------- Data and Storage (Binary Representation) -----------------------//
// Check if HID report is empty
// This function checks whether the gamepad HID report contains any input data (e.g., buttons pressed).
// It compares all the fields of the HID report to zero and returns true if no buttons are pressed and
// the joystick is in the neutral position. This shows how data is stored and processed in binary form.
bool is_empty(const hid_gamepad_report_t *report)
{
  // Return true if all fields (buttons, hat, joystick axes) are zero
  return 0 == report->buttons + report->hat + report->x + report->y + report->z + report->rx + report->ry + report->rz;
}

//----------------------- Input Devices -----------------------//
// Update button values in HID report
// This function reads the state of each button (via its GPIO pin) and updates the HID report to reflect
// whether the button is pressed. It interacts directly with the hardware (GPIO) and converts this 
// input into data that can be sent via USB to the host.
void update_button(hid_gamepad_report_t *report, const button_source *data)
{
  if (!gpio_get(data->gpio_pin))  // If button is pressed (GPIO pin is pulled low)
  {
    report->buttons |= data->action;  // Set the corresponding button action in the report
  }
}

//----------------------- Networks and the Internet (USB Communication) -----------------------//
// Update the HID report for the controller
// This function collects input from all buttons and the joystick and updates the gamepad HID report.
// It reads the button states (digital input) and the joystick positions (analog input via ADC) and
// prepares a structured HID report to be sent to the host via USB.
void update_hid_report_controller(hid_gamepad_report_t *report)
{
  // Update the button states
  for (int i = 0; i < _button_config_count; i++)
  {
    update_button(report, &_button_config[i].data.button_src);  // Update each button in the HID report
  }

   //----------------------- Input Devices (Joystick) -----------------------//
  // Read joystick ADC values
  // The joystick is an analog input device. We use the ADC (Analog-to-Digital Converter) to read its position.
  adc_select_input(0);           // Select ADC input 0 (X-axis, connected to GPIO 26)
  joy_map[0].value = adc_read(); // Read the X-axis value (12-bit value between 0 and 4095)

  adc_select_input(1);           // Select ADC input 1 (Y-axis, connected to GPIO 27)
  joy_map[1].value = adc_read(); // Read the Y-axis value (12-bit value between 0 and 4095)

  //----------------------- Data and Storage (Binary Representation) -----------------------//
  // Scale 12-bit ADC values to 8-bit range
  // The ADC produces a 12-bit value (0-4095). This value needs to be scaled down to 8 bits (0-255)
  // to fit into the HID report format, which uses 8-bit fields for joystick positions.
  report->x = joy_map[0].value / 16;  // Scale X-axis to 8-bit range
  report->y = joy_map[1].value / 16;  // Scale Y-axis to 8-bit range
}