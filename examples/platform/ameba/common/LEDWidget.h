/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include <platform_stdlib.h>
#include <pwmout_api.h>
#include <PinNames.h>

class LEDWidget
{
public:
    //void Init(PinName pin);
    void Init(PinName pin);
    void Init(PinName redpin, PinName greenpin, PinName bluepin);
    void Init(PinName redpin, PinName greenpin, PinName bluepin, PinName cwhitepin, PinName wwhitepin);
    void deInit(void);
    void Set(bool state);
    void SetBrightness(uint8_t brightness);
    void SetColor(uint8_t Hue, uint8_t Saturation);
    void SetColorTemp(uint16_t colortemp);
    void HSB2rgb(uint16_t Hue, uint8_t Saturation, uint8_t brightness, uint8_t & red, uint8_t & green, uint8_t & blue);
    void simpleRGB2RGBW(uint8_t & red, uint8_t & green, uint8_t & blue, uint8_t & cwhite, uint8_t & wwhite);
    uint8_t mDefaultOnBrightness;
    uint16_t mHue;       // mHue [0, 360]
    uint8_t mSaturation; // mSaturation [0, 100]
    uint16_t mColorTemp;

private:
    pwmout_t *mPwm_obj = NULL;
    pwmout_t *mPwm_red = NULL;
    pwmout_t *mPwm_green = NULL;
    pwmout_t *mPwm_blue = NULL;
    pwmout_t *mPwm_cwhite = NULL;
    pwmout_t *mPwm_wwhite = NULL;
    bool mRgb = false;
    bool mRgbw = false;
    bool mState;
    void DoSet(bool state);
    uint16_t WhitePercentage[11][3] =
    {
        /*CT--coolwhite%--warmwhite%*/
        {2708, 0, 100},
        {2891, 10, 90},
        {3110, 20, 80},
        {3364, 30, 70},
        {3656, 40, 60},
        {3992, 50, 50},
        {4376, 60, 40},
        {4809, 70, 30},
        {5304, 80, 20},
        {5853, 90, 10},
        {6471, 100, 0}
    };

};
