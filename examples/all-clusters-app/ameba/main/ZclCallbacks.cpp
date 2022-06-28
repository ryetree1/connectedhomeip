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
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app/CommandHandler.h>
#include <app/server/Dnssd.h>
#include <app/util/af.h>
#include <app/util/basic-types.h>
#include <app/util/util.h>
#include <common/DeviceCallbacks.h>
#include <common/CHIPDeviceManager.h>
#include <common/LEDWidget.h>
#include <lib/dnssd/Advertiser.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>
#include <support/logging/Constants.h>
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
#include <ota/OTAInitializer.h>
#endif

static const char * TAG = "app-devicecallbacks";

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceManager;
using namespace ::chip::Logging;

uint32_t identifyTimerCount;
constexpr uint32_t kIdentifyTimerDelayMS     = 250;

#if 0
void DeviceCallbacks::PostAttributeChangeCallback(EndpointId endpointId, ClusterId clusterId, AttributeId attributeId, uint8_t type,
                                                  uint16_t size, uint8_t * value)
{
    switch (clusterId)
    {
    case ZCL_ON_OFF_CLUSTER_ID:
        OnOnOffPostAttributeChangeCallback(endpointId, attributeId, value);
        break;

    case ZCL_IDENTIFY_CLUSTER_ID:
        OnIdentifyPostAttributeChangeCallback(endpointId, attributeId, value);
        break;

    default:
        break;
    }
}
#endif

void OnOnOffPostAttributeChangeCallback(EndpointId endpointId, AttributeId attributeId, uint8_t * value)
{
    VerifyOrExit(attributeId == ZCL_ON_OFF_ATTRIBUTE_ID,
                 ChipLogError(DeviceLayer, TAG, "Unhandled Attribute ID: '0x%04x", attributeId));
    VerifyOrExit(endpointId == 1 || endpointId == 2,
                 ChipLogError(DeviceLayer, TAG, "Unexpected EndPoint ID: `0x%02x'", endpointId));

    // At this point we can assume that value points to a bool value.
    // TODO: Move this to LightingManager
    //onoffLED.Set(*value);

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
                 ChipLogError(DeviceLayer, "[%s] Unhandled Attribute ID: '0x%04lu", TAG, attributeId));
    VerifyOrExit(endpointId == 1, ChipLogError(DeviceLayer, "[%s] Unexpected EndPoint ID: `0x%02x'", TAG, endpointId));

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
#if 0
    chip::DeviceManager::CHIPDeviceManagerCallbacks * cb =
        chip::DeviceManager::CHIPDeviceManager::GetInstance().GetCHIPDeviceManagerCallbacks();
    if (cb != nullptr)
    {
        cb->PostAttributeChangeCallback(path.mEndpointId, path.mClusterId, path.mAttributeId, type, size, value);
    }
#endif

    switch (path.mClusterId)
    {
    case ZCL_ON_OFF_CLUSTER_ID:
        OnOnOffPostAttributeChangeCallback(path.mEndpointId, path.mAttributeId, value);
        break;

    case ZCL_IDENTIFY_CLUSTER_ID:
        OnIdentifyPostAttributeChangeCallback(path.mEndpointId, path.mAttributeId, value);
        break;

    default:
        break;
    }
}

