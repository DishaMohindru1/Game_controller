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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tusb.h"
#include <pico/stdlib.h>
#include "pico_hid.h"

#ifndef JUST_STDIO
#include "bsp/board.h"
#include "usb_descriptors.h"
#endif

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

/* Digital Systems 
 * Digital systems rely on input/output interactions to communicate with the physical world.
 * The blink pattern defined here represents the system's various states and is part of how 
 * the digital system provides feedback to the user. For example, blinking LEDs can show 
 * if a USB device is connected or suspended.
 *
 * This part defines different blink intervals for the LED to indicate system status.
 * - BLINK_NOT_MOUNTED: Device is not connected to a USB host (no communication)
 * - BLINK_MOUNTED: Device is successfully mounted (connected) and ready to use
 * - BLINK_SUSPENDED: Device is in low-power suspended mode
 */
enum  {
  BLINK_NOT_MOUNTED = 250,  // 250ms interval for unmounted state
  BLINK_MOUNTED = 1000,     // 1000ms interval for mounted state
  BLINK_SUSPENDED = 2500,   // 2500ms interval for suspended state
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;  // Start with not mounted state

// Define GPIO for external LED - Hardware Components (Output Devices)
// This GPIO pin will control an external LED to visually represent the state of the device.
#define LED_GPIO 18  // External LED connected to GPIO 18

void led_blinking_task(void);  // Task to control the blinking LED
void hid_task(void);           // Task to handle USB HID (Human Interface Device)

/*------------- MAIN -------------*/
/* Digital Systems Architecture & Components of Digital Systems
 * The main function is the entry point of the digital system. It initializes hardware components
 * (GPIO pins, USB subsystem) and enters an infinite loop where it continuously performs tasks 
 * such as handling USB communication (via `tusb_init` and `tud_task`), reading input from 
 * buttons, and controlling output devices (LED).
 */
int main(void)
{
  #ifndef JUST_STDIO
  board_init();   // Initialize the board-specific hardware, such as setting up clocks and peripherals
  tusb_init();    // Initialize TinyUSB stack to handle USB communication
  #else
  stdio_init_all();  // Initialize the standard input/output (used in environments without TinyUSB)
  printf("Starting up");
  #endif

  // Setting up Input Devices (buttons and joystick)
  setup_controller_buttons();  // Configure the buttons and joystick as input devices

  // Initialize GPIO 18 for external LED as an output device - Output Device Interaction
  gpio_init(LED_GPIO);         // Initialize GPIO pin 18
  gpio_set_dir(LED_GPIO, GPIO_OUT);  // Set GPIO 18 as an output pin to control an external LED

  // Infinite loop to continuously process USB and gamepad tasks
  while (1)
  {
    #ifndef JUST_STDIO
    tud_task(); // TinyUSB device task (handles USB requests from the host) - Networks/USB Communication
    led_blinking_task();  // Task to control the LED blinking pattern (device feedback via output device)
    hid_task();  // Task to handle Human Interface Device (HID) report generation and transmission
    #else

    /* Input Devices
     * HID gamepad report: This is a structured data format representing the state of the gamepad.
     * It includes joystick positions, button presses, and D-pad (HAT) state.
     * 
     * Here we create an empty report that will later be populated based on user input (button presses or
     * joystick movement). The system will eventually send this report to the USB host.
     */
    hid_gamepad_report_t report =
    {
        .x   = 0, // left analog stick X-axis
        .y   = 0, // left analog stick Y-axis
        .z   = 0, // right analog stick X-axis
        .rz  = 0, // right analog stick Y-axis
        .rx  = 0, // left analog trigger
        .ry  = 0, // right analog trigger
        .buttons = 0 // Buttons (32-bit mask, can represent up to 32 buttons)
    };
    
    // Update the HID report with the latest input (buttons, joysticks)
    update_hid_report_controller(&report);

    // Print the current state of the D-pad (hat switch) and buttons for debugging purposes
    printf("hat: %d buttons: %d\n", report.hat, report.buttons);  // Output to console (output device)
    #endif
  }

  return 0;  // This return is never reached since the while loop runs infinitely
}

//--------------------------------------------------------------------+
// USB Device Callbacks - USB Communication
//--------------------------------------------------------------------+

/* USB Callback: Invoked when the USB device is mounted (connected to a host)
 * Networks and the Internet (USB Protocol)
 * When the device is successfully connected to a host (e.g., a PC), the system enters the "mounted" state.
 * This function is called, and the LED blink pattern is updated to indicate the device is mounted.
 */
void tud_mount_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;  // Change blink pattern to mounted state (1000ms interval)
}

/* USB Callback: Invoked when the USB device is unmounted (disconnected from the host)
 * USB Protocol
 * When the USB connection is lost or the device is disconnected, this callback is triggered.
 * The system will return to the "not mounted" state, and the LED blink pattern will reflect this.
 */
void tud_umount_cb(void)
{
  blink_interval_ms = BLINK_NOT_MOUNTED;  // Set blink pattern to unmounted state (250ms interval)
}

/* USB Callback: Invoked when the USB bus is suspended
 * Networks and the Internet (USB Protocol)
 * This occurs when the USB host (e.g., a PC) requests the device to enter a low-power suspended state.
 * In this state, the system reduces power consumption and the LED blink pattern is slowed down.
 */
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;  // Set blink pattern to suspended state (2500ms interval)
}

/* USB Callback: Invoked when the USB bus is resumed (communication is re-established)
 * Networks and the Internet (USB Protocol)
 * When the USB host resumes communication after suspension, the device returns to the mounted state.
 */
void tud_resume_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;  // Change blink pattern back to mounted state (1000ms interval)
}

//--------------------------------------------------------------------+
// USB HID (Human Interface Device) - USB Communication
//--------------------------------------------------------------------+

#ifndef JUST_STDIO

/* Input Devices and USB Communication
 * This function prepares and sends a HID report to the host (e.g., a PC) to communicate the 
 * current state of the gamepad (buttons, joystick positions). HID reports are the standard 
 * method of transmitting input device data over USB. The system constantly updates and sends 
 * this report to keep the host in sync with the current state of the input devices.
 */
static void send_hid_report(uint8_t report_id, uint32_t btn)
{
  if ( !tud_hid_ready() ) return;  // If the HID system is not ready, exit early

  switch(report_id)
  {
    case REPORT_ID_GAMEPAD:  // Handle gamepad reports
    {
      // Ensure we avoid sending multiple consecutive zero reports
      static bool has_gamepad_key = false;

      // Create an empty HID report for the gamepad
      hid_gamepad_report_t report =
      {
        .x   = 0, // left analog X-axis
        .y   = 0, // left analog Y-axis
        .z   = 0, // right analog X-axis
        .rz  = 0, // right analog Y-axis
        .rx  = 0, // left trigger
        .ry  = 0, // right trigger
        .hat = 0, // D-pad (HAT switch)
        .buttons = 0 // 32-bit mask for buttons
      };

      // Update the HID report with current button and joystick states
      update_hid_report_controller(&report);

      // Send the report if there is any input
      if ( !is_empty(&report) )
      {
        tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));  // Send the report via USB
        has_gamepad_key = true;  // Mark that we have active input
      }
      else if (has_gamepad_key)
      {
        // If previously active but no input now, send a zeroed report to "release" buttons
        tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
        has_gamepad_key = false;  // No longer has active input
      }
    }
    break;

    default: break;  // Handle other report types (if any)
  }
}

/* USB Communication
 * This function is called every 10ms to send a HID report to the host. The regular polling interval
 * ensures that the host receives timely updates on the state of the gamepad, even if the user
 * doesn't interact with it frequently. The system continuously monitors input devices and updates
 * the HID report accordingly.
 */
void hid_task(void)
{
  const uint32_t interval_ms = 10;  // Polling interval for HID reports (every 10ms)
  static uint32_t start_ms = 0;

  if ( board_millis() - start_ms < interval_ms ) return;  // Ensure enough time has passed before the next report
  start_ms += interval_ms;

  uint32_t const btn = board_button_read();  // Read the state of the buttons (Input Device interaction)

  // Handle remote wakeup if the device is suspended
  if ( tud_suspended() && btn )
  {
    tud_remote_wakeup();  // Wake up the host if remote wakeup is enabled
  }
  else
  {
    send_hid_report(REPORT_ID_GAMEPAD, btn);  // Send the gamepad report to the host
  }
}

// Callback invoked after a report is successfully sent
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len)
{
  (void) instance;
  (void) len;

  uint8_t next_report_id = report[0] + 1;

  // Continue sending reports if necessary
  if (next_report_id < REPORT_ID_COUNT)
  {
    send_hid_report(next_report_id, board_button_read());
  }
}

// Handle GET_REPORT requests from the host (USB communication request)
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  // This is where the device would send a report to the host when requested (not implemented here)
  return 0;
}

// Handle SET_REPORT requests from the host (USB communication request)
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
}

//--------------------------------------------------------------------+
// BLINKING TASK - Output Devices (LED)
//--------------------------------------------------------------------+

/* Components of Digital Systems (Output Devices)
 * The LED blinking task controls the external LED (connected to GPIO 18) based on the device's state.
 * It uses the `gpio_put` function to control the LED and toggles its state at specific intervals.
 * The intervals represent the device's state: mounted, unmounted, or suspended.
 */
void led_blinking_task(void)
{
  static uint32_t start_ms = 0;  // Keep track of the last time the LED was toggled
  static bool led_state = false;  // The current state of the LED (on or off)

  // If blinking is disabled (no interval set), return early
  if (!blink_interval_ms) return;

  // Check if enough time has passed to toggle the LED
  if ( board_millis() - start_ms < blink_interval_ms ) return;
  start_ms += blink_interval_ms;

  // Control the external LED connected to GPIO 18
  gpio_put(LED_GPIO, led_state);  // Turn the LED on or off
  led_state = !led_state;  // Toggle the state for the next cycle
}

#endif

