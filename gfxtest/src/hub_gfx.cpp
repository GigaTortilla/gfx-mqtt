/*************************************
 * 
 * GEN4-IOD-24T Smart Home Hub Graphics
 * Student Project
 * 
 * Version: 0.2
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

#include "hub_gfx.h"

GFX4d gfx;

bool b_error_storage_init(void)
{
  gfx.println("Looking for SD card...");
  delay(400);
  if (gfx.CheckSD())
  {
    gfx.println("SD Card found!");
    gfx.println("Opening File...");
    delay(300);
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
