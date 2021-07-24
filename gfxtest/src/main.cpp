/*************************************

// MIT License

// Copyright (c) 2021 Martin Appel

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

 **************************************
 * 
 * GEN4-IOD-24T Smart Home Hub
 * 
 * Version: 0.1.1
 * 
 * Creator: Martin Appel | 182048/AUT
 * 
 *************************************/

#include <core_esp8266_features.h>
#include "ESP8266WiFi.h"
#include "WiFiUdp.h"
#include "NTPClient.h"
#include "PubSubClient.h"
#include "GFX4d.h"
#include "belegGFXConst.h"

#define DEFAULT_BAUD 115200
#define MQTT_PORT 1883
#define MQTT_PORT_SSL 8883    // later possibility for ssl encrypted communication
#define CLIENT_ID "IoD_24T"
#define OUT_TOPIC "/everything/smart/home\0"
#define RQ_ESP_STATUS_TOPIC "/ESP/status/rq\0"
#define STATUS_TOPIC "/IoD/status/out\0"
#define RQ_STATUS_TOPIC "/IoD/status/rq\0"
#define TEMP_TOPIC "/ESP/temp/out\0"

void fn_serial_init(uint32_t i_baudrate);
void fn_gfx_init(void);
void rebuildScreen(void);
void buildMainMenu(void);
void buildTempScreen(uint8_t temp);
bool b_error_storage_init(void);
void fn_WiFi_graphical_init(void);
void mqttReconnect(const char* cID, const char* bUser, const char* bPass);
void mqttCallback(char* topic, byte* payload, unsigned int length);

// defining global variables for usability over security
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
WiFiClient espClient;
PubSubClient mqttClient(espClient);
GFX4d gfx = GFX4d();

// Can be read from the SD card in the general wifi setup at a later state
uint8_t broker[4] = {192, 168, 1, 3};
const char* brokerUser = "mqtt-user";
const char* brokerPass = "REDACTED";

float f_temp = 5;

void setup()
{
  fn_serial_init(DEFAULT_BAUD);
  fn_gfx_init();
  ets_delay_us(400000);
  if(b_error_storage_init())
  {
    gfx.println("An image could not be loaded!");
    ESP.restart();
  }
  else
  {
    fn_WiFi_graphical_init();

    timeClient.begin();
    timeClient.setTimeOffset(7200);
    timeClient.setUpdateInterval(15000);
    timeClient.forceUpdate();
    
    mqttClient.setServer(broker, MQTT_PORT).setCallback(mqttCallback);
    gfx.println("Initialization complete!\nStarting graphical application...");
    delay(3000);
    
    // Background
    rebuildScreen();
    buildMainMenu();
    gfx.TextWindow(8, 8, 140, 32, BLACK, YELLOW_TOP);
    // Displaying the time at display startup
    gfx.MoveTo(16, 16);
    gfx.print(timeClient.getFormattedTime());
    // general touch enable
    gfx.touch_Set(TOUCH_ENABLE);
  }
}

void loop()
{
  static bool b_standbyFlag = false;
  static uint8_t i_button, i_last_button = 0;
  static uint8_t i_currentScreen = 0;
  static unsigned long l_currentTime;
  static unsigned long l_lastTime1;
  static unsigned long l_lastTime2;
  static unsigned int i_wakeTime = 10000;
  l_lastTime1 = 0;
  l_lastTime2 = 0;

  while (1)
  {
    // Checking for inputs from the resistive touch display
    gfx.touch_Update();
    if (gfx.touch_GetPen() != NOTOUCH)
    {
      i_button = 0;
      i_button = gfx.CheckButtons();

      // If the display was in standby at the time at which the update occured
      // it turns the display on and requires another input to react to
      gfx.BacklightOn(true);
      l_lastTime1 = l_currentTime;
      if((i_last_button == 0) && (i_button > 0))
      {
        i_last_button = 1;  // button pressed
        if (gfx.touch_GetPen() == TOUCH_PRESSED)
        {  
          switch(gfx.imageTouched())
          { 
            case iWinbutton1:
              mqttClient.publish(OUT_TOPIC, "Restarting in a safe manner...");
              mqttClient.publish(STATUS_TOPIC, "101");
              mqttClient.disconnect();
              timeClient.end();
              espClient.stopAll();
              delay(10);
              ESP.restart();
              break;

            case iWinbutton2:
              rebuildScreen();
              buildMainMenu();
              i_currentScreen = 0;
              break;

            case iWinbutton9:
              mqttClient.publish(RQ_ESP_STATUS_TOPIC, NULL);
              rebuildScreen();
              buildTempScreen(20);
              i_currentScreen = 1;
              break;
          }
        }
      }
      else
        i_last_button = 0;

      delay(40);
      b_standbyFlag = false;
      l_lastTime1 = millis();
    }

    // Darkening the display after a fixed interval
    l_currentTime = millis();
    if ((l_currentTime - l_lastTime1 > i_wakeTime) && !b_standbyFlag)
    {
      gfx.BacklightOn(false);
      b_standbyFlag = true;
      l_lastTime1 = millis();
    }

    // Checking for updates from the NTP server
    if ((l_currentTime - l_lastTime2) >= 1000)
    {
      // update the time displayed at the top of the screen
      timeClient.update();
      gfx.MoveTo(16, 16);
      gfx.print(timeClient.getFormattedTime());
      l_lastTime2 = millis();
    }
    
    // Checking whether or not the MQTT connection is still working and reconnecting if it's not
    if (!mqttClient.connected())
    {
      mqttReconnect(CLIENT_ID, brokerUser, brokerPass);
    }
    mqttClient.loop();

    // Updates the temperature screen if required
    if (i_currentScreen == 1)
    {
      gfx.UserImages(iThermometer1, (uint16_t)f_temp);
      gfx.MoveTo(180, 132);
      gfx.printf("%.2f°C", f_temp);
    }

    // Required for ESP8266
    yield(); 
  }  
}

// Starts the communication for troubleshooting using the serial monitor during run-time.
void fn_serial_init(uint32_t i_baudrate)
{
  Serial.begin(i_baudrate);
  ets_delay_us(10000);
  Serial.println("Serial communication established!");
  Serial.println("Commencing addditional setup of the display...");
}

// Starts the iod display oriented in LANDSCAPE and displays the SDK version.
void fn_gfx_init(void)
{
  gfx.begin();
  gfx.Cls();
  gfx.ScrollEnable(false);
  gfx.BacklightOn(true);
  gfx.Orientation(LANDSCAPE);
  gfx.SmoothScrollSpeed(5);
  gfx.TextColor(WHITE, BLACK); gfx.Font(2);  gfx.TextSize(1);
  gfx.println("Serial port initialized!");
  gfx.print("SDK version ");
  gfx.println(ESP.getSdkVersion());
}

void rebuildScreen(void)
{
  gfx.Cls(GREY_BG);
  gfx.RectangleFilled(0, 0, 319, 47, YELLOW_TOP);
  gfx.imageTouchEnable(iWinbutton2, false);
  gfx.imageTouchEnable(iWinbutton9, false);
  gfx.imageTouchEnable(iWinbutton10, false);
  gfx.imageTouchEnable(iWinbutton11, false);
  gfx.imageTouchEnable(iWinbutton12, false);
  gfx.imageTouchEnable(iWinbutton13, false);
  gfx.imageTouchEnable(iWinbutton14, false);
  gfx.UserImages(iWinbutton1, 0);
  gfx.imageTouchEnable(iWinbutton1, true);
}

void buildMainMenu(void)
{
  gfx.UserImages(iWinbutton9, 0);
  gfx.imageTouchEnable(iWinbutton9, true);
  gfx.UserImages(iWinbutton10, 0);
  gfx.imageTouchEnable(iWinbutton10, true);
  gfx.UserImages(iWinbutton11, 0);
  gfx.imageTouchEnable(iWinbutton11, true);
  gfx.UserImages(iWinbutton12, 0);
  gfx.imageTouchEnable(iWinbutton12, true);
  gfx.UserImages(iWinbutton13, 0);
  gfx.imageTouchEnable(iWinbutton13, true);
  gfx.UserImages(iWinbutton14, 0);
  gfx.imageTouchEnable(iWinbutton14, true);
}

void buildTempScreen(uint8_t temp)
{
  gfx.UserImages(iThermometer1, temp);
  gfx.UserImages(iWinbutton2, 0);
  gfx.imageTouchEnable(iWinbutton2, true);
  gfx.UserImage(iStatictext1);
}

bool b_error_storage_init(void)
{
  gfx.println("Looking for SD card...");
  ets_delay_us(400000);
  if (gfx.CheckSD())
  {
    gfx.println("SD Card found!");
    gfx.println("Opening File...");
    ets_delay_us(300000);
    // Opens DAT and GCI files for read using filename without extension. Note! Workshop generates files with Short filenames
    gfx.Open4dGFX("belegGFX");
    gfx.println("GFX file opened!");
    return 0;
  }
  else
  {
    gfx.println("[ERROR]: No SD card was found!");
    return 1;
  }
}

/* 
 * Initializes the wifi connection to the local wifi router.
 * Requires the GFX4d library for troubleshooting purposes. 
 */
void fn_WiFi_graphical_init(void)
{
  // Open the password file which includes the ssid with a total length of not more than 32 characters
  // and the wpa2 access code which has a maximum length of 63. Including the 3 required escape characters 
  // this leads to a total buffer length of 98 characters.
  uint8_t i_buflen = 98;
  uint8_t i_pivot, i;
  i_pivot = 0;
  char c_buf[i_buflen];
  char c_ssidbuf[33];
  char c_pwbuf[64];

  // Initializing by writing zeroes into the buffer arrays
  for (i = 0; i < i_buflen; i++)
  {
    c_buf[i] = 0;
  }
  for (i = 0; i < 33; i++)
  {
    c_ssidbuf[i] = 0;
  }
  for (i = 0; i < 64; i++)
  {
    c_pwbuf[i] = 0;
  }

  // File opens 
  File wpa2_access = SD.open("wifi.txt", FILE_READ);

  if (wpa2_access)
  {
    // Reading the contents of the wifi.txt file and storing them in a buffer array
    i = 0;
    while (wpa2_access.available())
    {
      c_buf[i++] = wpa2_access.read();
    }
    // appending the null character to limit the character string within the array
    c_buf[i] = '\0';
  }
  // closing the file to restrict unwanted access
  wpa2_access.close();

  for (i = 0; i < 33; i++)
  {
    // If the loop hits the first LF or CR character it breaks execution 
    // and skips the 2 escape characters for the next loop
    if ((c_buf[i] == '\n') || (c_buf[i] == '\r'))
    {
      i_pivot = i + 2;
      break;
    }
    else
      c_ssidbuf[i] = c_buf[i];
  }
  // appending the null character to limit the character string within the array
  c_ssidbuf[i] = '\0';

  for (i = i_pivot; i < i_buflen; i++)
  {
    c_pwbuf[i - i_pivot] = c_buf[i];
  }
  // appending the null character to limit the character string within the array
  c_pwbuf[i - i_pivot] = '\0';

  // Wifi-connection
  WiFi.begin(c_ssidbuf, c_pwbuf);  
  gfx.printf("Connecting to %s...\n", c_ssidbuf);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    delay(500);
    gfx.print(WiFi.waitForConnectResult());
  }
  Serial.print("\nConnected, IP address: ");
  Serial.println(WiFi.localIP());
}

// Function to reconnect to the mqtt broker.
// Already has arguments for relocating to custom header file later on.
void mqttReconnect(const char* cID, const char* bUser, const char* bPass)
{
  while (!mqttClient.connected())
  {
    Serial.print("\nConnecting to MQTT broker...");
    if (!mqttClient.connect(cID, bUser, bPass))
      delay(5000);
    else
    {
      Serial.println("\nConnected to MQTT broker!");
      mqttClient.subscribe(RQ_STATUS_TOPIC);
      mqttClient.subscribe(TEMP_TOPIC);
    }
  }
}

// MQTT callback funtion to react to data received via MQTT
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Status requested via MQTT
  if (strcmp(topic, RQ_STATUS_TOPIC) == 0)
    Serial.println("Status requested!");
  // Temperature update from external esp
  if (strcmp(topic, TEMP_TOPIC) == 0)
  {
    Serial.println("Temperature update received!");
    f_temp = strtof((char*)payload, NULL);;
  }
}
