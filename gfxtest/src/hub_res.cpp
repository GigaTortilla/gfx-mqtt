/*************************************
 * 
 * GEN4-IOD-24T Smart Home Hub Ressources
 * Student Project
 * 
 * Version: 0.3
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
#include "SD.h"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Starts the communication for troubleshooting using the serial monitor during run-time.
void fn_serial_init(uint32_t i_baudrate)
{
  Serial.begin(i_baudrate);
  delay(10);
  Serial.println("Serial communication established!");
  Serial.println("Commencing addditional setup of the display...");
}

int readDataSD(char *ssid, char *wpa2psk)
{
  String line;
  File sd_content = SD.open(SD_FILE, FILE_READ);

  while (sd_content.available())
  {
    line = sd_content.readStringUntil('\n');
    Serial.println(line);
    switch (line.charAt(0))
    {
    case 'w':
      line.toCharArray(ssid, 33, 2);
      ssid[line.length() - 3] = '\0';
      break;
    case 'p':
      line.toCharArray(wpa2psk, 64, 2);
      wpa2psk[line.length() - 3] = '\0';
      break;
    default:
      break;
    }
    line.clear();
  }
  sd_content.close();
  
  return 0;
}

int readDataSD(char *ssid, char *wpa2psk, char *mqttUser, char *mqttPwd, char* mqttBroker)
{
  String line;
  File sd_content = SD.open(SD_FILE, FILE_READ);

  while (sd_content.available())
  {
    line = sd_content.readStringUntil('\n');
    switch (line.charAt(0))
    {
    case 'u':
      line.toCharArray(mqttUser, 64, 2);
      mqttUser[line.length() - 3] = '\0';
      break;
    case 'b':
      line.toCharArray(mqttPwd, 64, 2);
      mqttPwd[line.length() - 3] = '\0';
      break;
    case 'w':
      line.toCharArray(ssid, 33, 2);
      ssid[line.length() - 3] = '\0';
      break;
    case 'p':
      line.toCharArray(wpa2psk, 64, 2);
      wpa2psk[line.length() - 3] = '\0';
      break;
    case 'a':
      line.toCharArray(mqttBroker, 128, 2);
      mqttBroker[line.length() - 3] = '\0';
      break;
    default:
      break;
    }
    line.clear();
  }
  sd_content.close();
  
  return 0;
}

/* 
 * Initializes the wifi connection to the local wifi router.
 * Requires the GFX4d library for troubleshooting purposes. 
 */
int wifiInit(char *ssid, char *wpa2psk)
{
  unsigned long timeoutCount = 0UL;

  // Wifi-connection
  WiFi.begin(ssid, wpa2psk);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(WiFi.waitForConnectResult());
    timeoutCount++; // WIP
  }
  Serial.print("\nConnected, IP address: ");
  Serial.println(WiFi.localIP());

  return 0;
}