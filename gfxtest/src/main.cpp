/*************************************
 * 
 * GEN4-IOD-24T Smart Home Hub
 * Student Project
 * 
 * Version: 0.3.0
 * 
 * Date: 6th August 2021
 * 
 * MIT License
 *
 * Copyright (c) 2021 GigaTortilla

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 *************************************/

#include "hub_res.h"
#include "hub_gfx.h"

#define OUT_TOPIC "/everything/smart/home\0"
#define RQ_ESP_STATUS_TOPIC "/ESP/status/rq\0"
#define STATUS_TOPIC "/IoD/status/out\0"
#define RQ_STATUS_TOPIC "/IoD/status/rq\0"
#define TEMP_TOPIC "/ESP/temp/out\0"

void mqttReconnect(const char *cID, const char *bUser, const char *bPass);
void mqttCallback(char *topic, byte *payload, unsigned int length);

// defining global variables for usability over security
os_timer_t loopTime;

// Read in from the sd card
char wifi_name[33];
char wpa2[64];
char brokerAddress[128];
char brokerUser[64];
char brokerPass[64];

float f_temp = 5.0f;

void setup()
{
  fn_serial_init(DEFAULT_BAUD);
  fn_gfx_init();
  delay(400);
  if(b_error_storage_init())
  {
    gfx.println("An image could not be loaded!");
    ESP.restart();
  }
  else
  {
    readDataSD(wifi_name, wpa2, brokerUser, brokerPass, brokerAddress);
    
    gfx.printf("Connecting to %s...\n", wifi_name);
    wifiInit(wifi_name, wpa2);

    timeClient.begin();
    timeClient.setTimeOffset(7200);       // UTC+2 (MEST)
    timeClient.setUpdateInterval(15000);
    timeClient.forceUpdate();
    
    mqttClient.setServer(brokerAddress, MQTT_PORT).setCallback(mqttCallback);
    gfx.println("Initialization complete!\nStarting graphical application...");
    delay(2000);
    
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
              ets_delay_us(10000);
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

      ets_delay_us(40000);
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
    // if (!mqttClient.connected())
    // {
    //   mqttReconnect(CLIENT_ID, brokerUser, brokerPass);
    // }
    // mqttClient.loop();

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

// Function to reconnect to the mqtt broker.
// Already has arguments for relocating to custom header file later on.
void mqttReconnect(const char* cID, const char* bUser, const char* bPass)
{
  Serial.print("\nConnecting to MQTT broker...");
  if (mqttClient.connect(cID, bUser, bPass))
  {
    Serial.println("\nConnected to MQTT broker!");
    mqttClient.subscribe(RQ_STATUS_TOPIC);
    mqttClient.subscribe(TEMP_TOPIC);
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
