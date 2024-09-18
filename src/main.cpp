/*
 * Author: Varad Chaskar
 * 
 * Description:
 * This project interfaces an ESP32 with an LVGL (Light and Versatile Graphics Library) display 
 * and communicates with an RS-232 serial device. The LVGL display includes an on-screen keyboard 
 * to input text that is sent over RS-232. Received RS-232 data is displayed on the screen.
 * 
 * Libraries Used:
 *  - Arduino.h: Base library for Arduino framework.
 *  - FS.h: File system library used for managing SPIFFS.
 *  - SPI.h: For SPI communication.
 *  - lvgl.h: Light and Versatile Graphics Library for GUI display.
 *  - TFT_eSPI.h: TFT display library.
 */

// Include necessary libraries
#include <Arduino.h>
#include <FS.h>
#include <SPI.h>
#include <lvgl.h>
#include <TFT_eSPI.h>

// Pin Definitions
#define TOUCH_CS 21          // Touch chip select pin
#define BUTTON_PIN_1 25      // GPIO pin for button 1
#define BUZZER_PIN 13        // GPIO pin for buzzer

// RS-232 communication pins
#define RS232_RXD 26         // RS-232 receive pin
#define RS232_TXD 12         // RS-232 transmit pin

// File for storing touch calibration data
#define CALIBRATION_FILE "/TouchCalData3"
#define REPEAT_CAL true       // Set to true to force touch calibration
#define LVGL_REFRESH_TIME 5u  // Refresh time for LVGL in milliseconds

// Create TFT display instance
TFT_eSPI tft = TFT_eSPI();   /* TFT instance */
// Use hardware serial (UART1) for RS-232 communication
HardwareSerial rs232(1);     // RS-232 Serial (UART1)

// Screen resolution constants
static const uint32_t screenWidth = 320;
static const uint32_t screenHeight = 240;

// Buffers for LVGL display
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 10]; // Drawing buffer

// LVGL styles for button states
static lv_style_t style_default, style_pressed;

// LVGL objects
lv_obj_t *label;             // Label to show received RS-232 data
lv_obj_t *keyboard = NULL;   // On-screen keyboard object
lv_obj_t *textarea = NULL;   // Text area object for keyboard input

// Store received RS-232 data
String receivedData = "No data received yet.";  // Default message before receiving data

/* Touch calibration function */
void touch_calibrate() {
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // Begin SPIFFS file system, format if necessary
  if (!SPIFFS.begin()) {
    Serial.println("Formatting file system");
    SPIFFS.format();
    SPIFFS.begin();
  }

  // Check if calibration data exists
  if (SPIFFS.exists(CALIBRATION_FILE)) {
    if (!REPEAT_CAL) {
      File f = SPIFFS.open(CALIBRATION_FILE, "r");
      if (f) {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  // If calibration data is valid, set it
  if (calDataOK && !REPEAT_CAL) {
    tft.setTouch(calData);
  } else {
    // Perform manual calibration if data is invalid
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");
    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    // Save calibration data to SPIFFS
    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}

/* Touchscreen input reading for LVGL */
void lvgl_port_tp_read(lv_indev_drv_t *indev, lv_indev_data_t *data) {
  uint16_t touchX, touchY;
  bool touched = tft.getTouch(&touchX, &touchY); // Check if the screen is touched

  // Update LVGL input state based on touch
  if (!touched) {
    data->state = LV_INDEV_STATE_REL;  // No touch detected
  } else {
    data->state = LV_INDEV_STATE_PR;   // Touch detected
    data->point.x = touchX;            // X-coordinate of touch
    data->point.y = touchY;            // Y-coordinate of touch
  }
}

/* Display flushing function for LVGL */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);  // Width of the area
  uint32_t h = (area->y2 - area->y1 + 1);  // Height of the area

  // Write to the TFT screen
  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t *)&color_p->full, w * h, true);
  tft.endWrite();

  // Indicate to LVGL that flushing is done
  lv_disp_flush_ready(disp);
}

/* Handle keyboard events */
static void kb_event_handler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);  // Get event type
  lv_obj_t *kb = lv_event_get_target(e);        // Get event target (keyboard)

  if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
    const char *text = lv_textarea_get_text(textarea); // Get text from textarea

    // Send data via RS-232
    rs232.println(text);
    Serial.println("Sent to RS-232: " + String(text));

    // Delete textarea and keyboard after input
    lv_obj_del(textarea);
    textarea = NULL;
    lv_obj_del(kb);
    keyboard = NULL;
  }
}

/* Event handler for Button 2 to show on-screen keyboard */
static void event_handler_btn2(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);

  if (code == LV_EVENT_CLICKED) {  // If button is clicked
    if (keyboard == NULL) {  // Check if keyboard is already present
      // Create a textarea for keyboard input
      textarea = lv_textarea_create(lv_scr_act());
      lv_obj_align(textarea, LV_ALIGN_TOP_MID, 0, 60); // Position it on screen
      lv_textarea_set_one_line(textarea, true);        // Single-line text input

      // Create keyboard and attach it to the textarea
      keyboard = lv_keyboard_create(lv_scr_act());
      lv_keyboard_set_textarea(keyboard, textarea);
      lv_obj_set_size(keyboard, screenWidth, screenHeight / 2);  // Resize keyboard
      lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_TEXT_LOWER);  // Set mode to lowercase text
      lv_obj_add_event_cb(keyboard, kb_event_handler, LV_EVENT_ALL, NULL); // Set event callback
    }
  }
}

/* Create buttons and set up their event handlers */
void lv_example_buttons(void) {
  // Create label to display received RS-232 data
  label = lv_label_create(lv_scr_act());
  lv_label_set_text(label, receivedData.c_str());  // Set default text
  lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -10);  // Position label

  // Define button dimensions
  const int button_width = 120;
  const int button_height = 60;

  // Create Button 2 for entering text
  lv_obj_t *btn2 = lv_btn_create(lv_scr_act());
  lv_obj_set_size(btn2, button_width, button_height);  // Set button size
  lv_obj_add_event_cb(btn2, event_handler_btn2, LV_EVENT_ALL, NULL);  // Set event callback
  lv_obj_align(btn2, LV_ALIGN_CENTER, 0, -40);  // Position Button 2

  // Add label to Button 2
  lv_obj_t *btn2_label = lv_label_create(btn2);
  lv_label_set_text(btn2_label, "Enter Text");  // Set button label
  lv_obj_center(btn2_label);  // Center the label
}

/* Main setup function */
void setup() {
  // Initialize serial communication and RS-232
  Serial.begin(115200);
  rs232.begin(9600, SERIAL_8N1, RS232_RXD, RS232_TXD);

  // Initialize TFT display and set orientation
  SPIFFS.begin();
  tft.begin();
  tft.setRotation(1);  // Landscape mode

  // Pre-set touch calibration data (hardcoded)
  uint16_t calData[5] = {275, 3574, 351, 3485, 7};
  tft.setTouch(calData);

  // Initialize LVGL and the display buffer
  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * 10);

  // Setup LVGL display driver
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  // Setup LVGL input driver for touch input
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = lvgl_port_tp_read;
  lv_indev_drv_register(&indev_drv);

  // Set up the GUI with buttons
  lv_example_buttons();
}

/* Main loop function */
void loop() {
  lv_timer_handler();  // Handle LVGL tasks
  delay(LVGL_REFRESH_TIME);

  // Update received RS-232 data and display it
  while (rs232.available()) {
    char c = rs232.read();  // Read incoming character
    receivedData += c;      // Append to receivedData string
    if (c == '\n') {        // If newline character is detected
      lv_label_set_text(label, receivedData.c_str());  // Update label
      receivedData = "";     // Clear receivedData for next message
    }
  }
}
