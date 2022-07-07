/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

/**
 * @file DeviceCallbacks.cpp
 *
 * Implements all the callbacks to the application from the CHIP Stack
 *
 **/
#include "LightingManager.h"

#include <app/clusters/identify-server/identify-server.h>
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app/CommandHandler.h>
#include <app/server/Dnssd.h>
#include <app/util/af.h>
#include <app/util/basic-types.h>
#include <app/util/util.h>
#include <lib/dnssd/Advertiser.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>
#include <support/logging/Constants.h>

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::Inet;
using namespace ::chip::System;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Logging;

bool mEndpointOnOffState[1]; // only 1 LED implemented currently
uint32_t identifyTimerCount;
constexpr uint32_t kIdentifyTimerDelayMS     = 250;

Identify gIdentify0 = {
    chip::EndpointId{ 0 },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStart"); },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStop"); },
    EMBER_ZCL_IDENTIFY_IDENTIFY_TYPE_VISIBLE_LED,
};

Identify gIdentify1 = {
    chip::EndpointId{ 1 },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStart"); },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStop"); },
    EMBER_ZCL_IDENTIFY_IDENTIFY_TYPE_VISIBLE_LED,
};

void OnOnOffPostAttributeChangeCallback(EndpointId endpointId, AttributeId attributeId, uint8_t * value)
{
    VerifyOrExit(attributeId == ZCL_ON_OFF_ATTRIBUTE_ID,
                 ChipLogError(DeviceLayer, "Unhandled Attribute ID: '0x%04x", attributeId));
    VerifyOrExit(endpointId == 1 || endpointId == 2,
                 ChipLogError(DeviceLayer, "Unexpected EndPoint ID: `0x%02x'", endpointId));

    // At this point we can assume that value points to a bool value.
    mEndpointOnOffState[endpointId - 1] = *value;
    ChipLogProgress(DeviceLayer, "Calling Set with value: %d", *value);
    LightMgr().sLightLED.Set(*value);

exit:
    return;
}

void OnLevelControlAttributeChangeCallback(EndpointId endpointId, AttributeId attributeId, uint8_t * value)
{
    bool onOffState    = mEndpointOnOffState[endpointId - 1];
    uint8_t brightness = onOffState ? *value : 0;

    VerifyOrExit(attributeId == ZCL_CURRENT_LEVEL_ATTRIBUTE_ID,
            ChipLogError(DeviceLayer, "Unhandled Attribute ID: '0x%04x", attributeId));
    VerifyOrExit(endpointId == 1,
            ChipLogError(DeviceLayer, "Unexpected EndPoint ID: `0x%02x'", endpointId));

    // At this point we can assume that value points to a bool value.
    ChipLogProgress(DeviceLayer, "Calling SetBrightness with value: %d", brightness);
    LightMgr().sLightLED.SetBrightness(brightness);

exit:
    return;
}

void OnColorControlAttributeChangeCallback(EndpointId endpointId, AttributeId attributeId, uint8_t * value)
{
    VerifyOrExit(
            attributeId == ZCL_COLOR_CONTROL_CURRENT_HUE_ATTRIBUTE_ID ||
            attributeId == ZCL_COLOR_CONTROL_CURRENT_SATURATION_ATTRIBUTE_ID ||
            attributeId == ZCL_COLOR_CONTROL_COLOR_TEMPERATURE_ATTRIBUTE_ID,
            ChipLogError(DeviceLayer, "Unhandled AttributeId ID: '0x%04x", attributeId)
            );
    VerifyOrExit(
            endpointId == 1,
            ChipLogError(DeviceLayer, "Unexpected EndPoint ID: `0x%02x'", endpointId)
            );

    if (attributeId  == ZCL_COLOR_CONTROL_CURRENT_HUE_ATTRIBUTE_ID || attributeId == ZCL_COLOR_CONTROL_CURRENT_SATURATION_ATTRIBUTE_ID)
    {
        if (endpointId == 1)
        {
            uint8_t hue, saturation;
            if (attributeId == ZCL_COLOR_CONTROL_CURRENT_HUE_ATTRIBUTE_ID)
            {
                hue = *value;
                emberAfReadServerAttribute(endpointId, ZCL_COLOR_CONTROL_CLUSTER_ID, ZCL_COLOR_CONTROL_CURRENT_SATURATION_ATTRIBUTE_ID,
                                           &saturation, sizeof(uint8_t));
            }
            else
            {
                saturation = *value;
                emberAfReadServerAttribute(endpointId, ZCL_COLOR_CONTROL_CLUSTER_ID, ZCL_COLOR_CONTROL_CURRENT_HUE_ATTRIBUTE_ID, &hue,
                                           sizeof(uint8_t));
            }
            ChipLogProgress(DeviceLayer, "Calling SetColor with hue:%d, saturation:%d", hue, saturation);
            LightMgr().sLightLED.SetColor(hue, saturation);
        }
    }

    if (attributeId == ZCL_COLOR_CONTROL_COLOR_TEMPERATURE_ATTRIBUTE_ID)
    {
        if (endpointId == 1)
        {
            using Traits = NumericAttributeTraits<uint16_t>;
            Traits::StorageType temp;
            uint8_t * readable   = Traits::ToAttributeStoreRepresentation(temp);
            emberAfReadServerAttribute(endpointId, ZCL_COLOR_CONTROL_CLUSTER_ID, ZCL_COLOR_CONTROL_COLOR_TEMPERATURE_ATTRIBUTE_ID, readable, sizeof(temp));

            uint16_t colortemp;
            colortemp = Traits::StorageToWorking(temp);
            ChipLogProgress(DeviceLayer, "Calling SetColorTemp with colortemp:%d", colortemp);
            LightMgr().sLightLED.SetColorTemp(colortemp);
        }
    }
exit:
    return;
}

void IdentifyTimerHandler(Layer * systemLayer, void * appState, CHIP_ERROR error)
{
    if (identifyTimerCount)
    {
        identifyTimerCount--;
    }
}

void OnIdentifyPostAttributeChangeCallback(EndpointId endpointId, AttributeId attributeId, uint8_t * value)
{
    VerifyOrExit(attributeId == ZCL_IDENTIFY_TIME_ATTRIBUTE_ID,
                 ChipLogError(DeviceLayer, "Unhandled Attribute ID: '0x%04lu", attributeId));
    VerifyOrExit(endpointId == 1, ChipLogError(DeviceLayer, "Unexpected EndPoint ID: `0x%02x'", endpointId));

    // timerCount represents the number of callback executions before we stop the timer.
    // value is expressed in seconds and the timer is fired every 250ms, so just multiply value by 4.
    // Also, we want timerCount to be odd number, so the ligth state ends in the same state it starts.
    identifyTimerCount = (*value) * 4;
exit:
    return;
}

bool emberAfBasicClusterMfgSpecificPingCallback(chip::app::CommandHandler * commandObj)
{
    emberAfSendDefaultResponse(emberAfCurrentCommand(), EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & path, uint8_t type, uint16_t size, uint8_t * value)
{
    printf("\r\nMatterPostAttributeChangeCallback\r\n");
    switch (path.mClusterId)
    {
    case Clusters::OnOff::Id:
        printf("\r\n\r\nDebug1\r\n\r\n");
        OnOnOffPostAttributeChangeCallback(path.mEndpointId, path.mAttributeId, value);
        break;

    case Clusters::LevelControl::Id:
        printf("\r\n\r\nDebug2\r\n\r\n");
        OnLevelControlAttributeChangeCallback(path.mEndpointId, path.mAttributeId, value);
        break;

    case Clusters::ColorControl::Id:
        printf("\r\n\r\nDebug3\r\n\r\n");
        OnColorControlAttributeChangeCallback(path.mEndpointId, path.mAttributeId, value);
        break;

    case Clusters::Identify::Id:
        printf("\r\n\r\nDebug4\r\n\r\n");
        OnIdentifyPostAttributeChangeCallback(path.mEndpointId, path.mAttributeId, value);
        break;

    default:
        break;
    }
}
