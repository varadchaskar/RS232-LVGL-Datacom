<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>RS232 and LVGL Display Interface - Documentation</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 20px;
            line-height: 1.6;
        }

        h1, h2, h3 {
            color: #333;
        }

        code {
            background-color: #f4f4f4;
            padding: 2px 5px;
            border-radius: 4px;
        }

        pre {
            background-color: #f4f4f4;
            padding: 10px;
            border-left: 4px solid #ccc;
            overflow-x: auto;
        }

        .code-section {
            background-color: #f9f9f9;
            padding: 15px;
            border: 1px solid #ddd;
            margin-top: 10px;
        }

        .important {
            color: #d9534f;
            font-weight: bold;
        }
    </style>
</head>

<body>

    <h1>RS232 and LVGL Display Interface with PlatformIO</h1>

    <p>
        This project demonstrates how to interface an RS232 serial communication with an LVGL-based GUI using an ESP32. The system allows for sending data through an on-screen keyboard and displaying the received data via RS232 on the screen. This project is set up using <strong>PlatformIO</strong> in the VSCode IDE.
    </p>

    <h2>Project Overview</h2>
    <p>
        The ESP32 communicates with an RS232 interface to send and receive data. It also uses the LVGL (Lightweight Graphics Library) for the GUI, where users can input text using an on-screen keyboard and see the received data displayed on the screen. A TFT display is used as the output device.
    </p>

    <h2>Key Features</h2>
    <ul>
        <li>RS232 data transmission and reception using UART1 on ESP32.</li>
        <li>Graphical interface built using the LVGL library.</li>
        <li>On-screen keyboard for text input.</li>
        <li>Display of RS232 received data on a TFT screen.</li>
        <li>Touchscreen calibration using SPIFFS to store calibration data.</li>
    </ul>

    <h2>Required Libraries</h2>
    <p>Ensure the following libraries are included in your <code>platformio.ini</code>:</p>
    <pre>
[env:esp32]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps = 
    lvgl/lvgl@^8.3.0
    Bodmer/TFT_eSPI@^2.5.0
    esp32/FS@^1.0.0
    SPI
    ArduinoJSON
    </pre>

    <h2>Explanation of the Code</h2>

    <div class="code-section">
        <h3>1. Include Necessary Libraries</h3>
        <pre>
#include &lt;Arduino.h&gt;
#include &lt;FS.h&gt;
#include &lt;SPI.h&gt;
#include &lt;lvgl.h&gt;
#include &lt;TFT_eSPI.h&gt;
        </pre>
        <p>
            These libraries are crucial for handling file system operations, SPI communication, the LVGL graphics library, and interfacing with the TFT display.
        </p>
    </div>

    <div class="code-section">
        <h3>2. Pin Configuration</h3>
        <pre>
#define TOUCH_CS 21
#define BUTTON_PIN_1 25
#define BUZZER_PIN 13
#define RS232_RXD 26
#define RS232_TXD 12
        </pre>
        <p>
            These are the pin configurations for the touch controller, buttons, buzzer, and RS232 communication. Modify these values if you’re using different GPIOs.
        </p>
    </div>

    <div class="code-section">
        <h3>3. TFT and RS232 Initialization</h3>
        <pre>
TFT_eSPI tft = TFT_eSPI();  // TFT instance
HardwareSerial rs232(1);    // RS232 communication on UART1
        </pre>
        <p>
            This initializes the TFT display object using the <code>TFT_eSPI</code> library and the RS232 communication on UART1 of the ESP32.
        </p>
    </div>

    <div class="code-section">
        <h3>4. Touch Calibration</h3>
        <pre>
void touch_calibrate() {
    uint16_t calData[5];
    ...
    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);
    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f) {
        f.write((const unsigned char*)calData, 14);
        f.close();
    }
}
        </pre>
        <p>
            The <code>touch_calibrate()</code> function is responsible for calibrating the touchscreen. It stores the calibration data in SPIFFS for reuse.
        </p>
    </div>

    <div class="code-section">
        <h3>5. RS232 Data Reception and Display</h3>
        <pre>
if (rs232.available()) {
    receivedData = "";
    while (rs232.available()) {
        receivedData += (char)rs232.read();  // Read RS232 data
    }
    lv_label_set_text(label, receivedData.c_str());
    Serial.println("Received from RS-232: " + receivedData);
}
        </pre>
        <p>
            This section checks if data is available from the RS232 interface and reads it. The received data is displayed on the TFT screen and also printed to the serial monitor.
        </p>
    </div>

    <div class="code-section">
        <h3>6. Keyboard Input and Sending Data via RS232</h3>
        <pre>
void kb_event_handler(lv_event_t *e) {
    if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        const char *text = lv_textarea_get_text(textarea);  // Get text from textarea
        rs232.println(text);  // Send data via RS232
        Serial.println("Sent to RS-232: " + String(text));
    }
}
        </pre>
        <p>
            This function handles events triggered by the on-screen keyboard. When the user inputs text, the text is sent via RS232 and displayed on the screen.
        </p>
    </div>

    <h2>PlatformIO Setup</h2>
    <p>
        Make sure to have the correct setup in your <code>platformio.ini</code> file. This ensures that all dependencies and configurations are correctly set up.
    </p>
    <pre>
[env:esp32]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps = 
    lvgl/lvgl@^8.3.0
    Bodmer/TFT_eSPI@^2.5.0
    esp32/FS@^1.0.0
    SPI
    ArduinoJSON
    </pre>

    <h2>Running the Code</h2>
    <p>
        1. Open VSCode and ensure PlatformIO is installed.<br>
        2. Create a new project and replace the code with the provided source code.<br>
        3. Adjust the pins and configuration if necessary.<br>
        4. Upload the code to your ESP32 and open the serial monitor to observe RS232 communication.
    </p>

</body>

</html>
