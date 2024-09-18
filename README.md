<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
</head>
<body>

    <h1>ESP32 RS-232 Communication with LVGL Interface</h1>
    <p>This project demonstrates how to use an ESP32 microcontroller to interface with an LVGL (Light and Versatile Graphics Library) display and communicate with an RS-232 serial device. The LVGL display includes an on-screen keyboard that allows the user to input text, which is then sent over RS-232. Any received RS-232 data is displayed on the screen.</p>

    <h2>Features</h2>
    <ul>
        <li>Graphical interface using the <strong>LVGL library</strong>.</li>
        <li>RS-232 communication using the ESP32's UART (hardware serial).</li>
        <li>An on-screen keyboard to enter text and send it via RS-232.</li>
        <li>Received RS-232 data is displayed in real-time on the screen.</li>
        <li>Touchscreen calibration to ensure accurate touch input.</li>
    </ul>

    <h2>Hardware Requirements</h2>
    <ul>
        <li>ESP32 microcontroller</li>
        <li>TFT display with a touchscreen (320x240 resolution)</li>
        <li>RS-232 serial communication interface</li>
        <li>Buzzer and buttons for user interaction (optional)</li>
    </ul>

    <h2>Development Environment</h2>
    <ul>
        <li><strong>Platform:</strong> PlatformIO</li>
        <li><strong>Framework:</strong> Arduino (within PlatformIO)</li>
        <li><strong>Board:</strong> ESP32</li>
        <li><strong>Upload Protocol:</strong> Serial</li>
    </ul>

    <h2>Libraries Used</h2>
    <ul>
        <li><code>lvgl.h</code> - Light and Versatile Graphics Library for creating the GUI</li>
        <li><code>TFT_eSPI.h</code> - Library to control the TFT display</li>
        <li><code>SPI.h</code> - For SPI communication</li>
        <li><code>FS.h</code> - For managing the file system on ESP32 (used for storing calibration data)</li>
    </ul>

    <h2>Pin Definitions</h2>
    <pre>
#define TOUCH_CS 21          // Touch chip select pin
#define BUTTON_PIN_1 25      // GPIO pin for button 1
#define BUZZER_PIN 13        // GPIO pin for buzzer
#define RS232_RXD 26         // RS-232 receive pin
#define RS232_TXD 12         // RS-232 transmit pin
    </pre>
    <p>The RS-232 communication uses the ESP32's hardware serial (UART1) with RX on pin 26 and TX on pin 12. The TFT display and the touchscreen use SPI communication.</p>

    <h2>Key Components of the Code</h2>
    <h3>1. <code>touch_calibrate()</code></h3>
    <p>This function handles the touchscreen calibration. It checks if calibration data exists in the SPIFFS file system and uses it if available. If not, the user is prompted to manually calibrate the touchscreen by touching the corners of the display. The calibration data is then stored in the SPIFFS for future use.</p>

    <h3>2. <code>lvgl_port_tp_read()</code></h3>
    <p>This function reads the touchscreen input and feeds it into the LVGL library. It converts touch coordinates to the LVGL format and updates the input device state.</p>

    <h3>3. <code>my_disp_flush()</code></h3>
    <p>This function is responsible for drawing the LVGL interface on the TFT display. It flushes the graphical data from the buffer to the screen.</p>

    <h3>4. RS-232 Communication</h3>
    <p>The ESP32's UART1 is used to send and receive data over RS-232. The received data is stored in a string and displayed on the screen via LVGL. When the user enters text using the on-screen keyboard, it is sent via RS-232:</p>
    <pre>
const char *text = lv_textarea_get_text(textarea); // Get text from textarea
rs232.println(text);  // Send text via RS-232
    </pre>

    <h3>5. On-Screen Keyboard</h3>
    <p>When the user presses Button 2, an on-screen keyboard appears, allowing them to enter text. Once the text is entered, it is sent via RS-232, and the keyboard is removed from the display. The keyboard is created using the following code:</p>
    <pre>
textarea = lv_textarea_create(lv_scr_act());
keyboard = lv_keyboard_create(lv_scr_act());
lv_keyboard_set_textarea(keyboard, textarea);
    </pre>

    <h2>Display Calibration</h2>
    <p>The calibration process is crucial for ensuring that touch input is accurate. The calibration data is stored in the SPIFFS file system, and this is the process that checks if calibration is needed:</p>
    <pre>
if (SPIFFS.exists(CALIBRATION_FILE)) {
    // Use existing calibration data
    tft.setTouch(calData);
} else {
    // Perform manual calibration
    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);
    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    f.write((const unsigned char *)calData, 14);
}
    </pre>

    <h2>How It Works</h2>
    <p>When the ESP32 starts, it initializes the display, touchscreen, and RS-232 communication. The LVGL library manages the graphical interface, including buttons and an on-screen keyboard. The user can click a button to bring up the keyboard, enter text, and send it via RS-232. Any data received via RS-232 is displayed on the screen in real time.</p>

    <h3>Setup</h3>
    <p>In the <code>setup()</code> function, we initialize the following:</p>
    <ul>
        <li>Serial communication for debugging</li>
        <li>RS-232 communication on UART1</li>
        <li>TFT display and touch input</li>
        <li>LVGL for the graphical user interface</li>
    </ul>

    <h3>Loop</h3>
    <p>The main <code>loop()</code> function handles the LVGL tasks and continuously checks for incoming RS-232 data. If data is received, it is appended to a string and displayed on the screen.</p>

    <h2>Conclusion</h2>
    <p>This project provides a basic framework for interfacing an ESP32 with a touchscreen TFT display using the LVGL library and communicating with an RS-232 device. The on-screen keyboard allows for user input, and the received data is displayed in real-time.</p>

    <h3>Potential Enhancements</h3>
    <ul>
        <li>Add more sophisticated RS-232 message handling, such as parsing specific commands.</li>
        <li>Improve the user interface with more buttons and controls using LVGL.</li>
        <li>Implement error handling for RS-232 communication.</li>
    </ul>

</body>
</html>
