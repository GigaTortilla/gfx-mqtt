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

#ifndef HUB_GFX_H
#define HUB_GFX_H

#include "GFX4d.h"
#include "belegGFXConst.h"

bool b_error_storage_init(void);
void fn_gfx_init(void);
void rebuildScreen(void);
void buildMainMenu(void);
void buildTempScreen(uint8_t temp);

extern GFX4d gfx;

#endif