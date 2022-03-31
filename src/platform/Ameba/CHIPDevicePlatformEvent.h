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

/**
 *    @file
 *          Defines platform-specific event types and data for the chip
 *          Device Layer on Ameba platforms.
 */

#pragma once

#include <platform/CHIPDeviceEvent.h>
#include <wifi_constants.h>

namespace chip {
namespace DeviceLayer {
namespace DeviceEventType {

/**
 * Enumerates platform-specific event types that are visible to the application.
 */
enum
{
    kAmebaSystemEvents = kRange_PublicPlatformSpecific
};

/**
 * Enumerates platform-specific event types that are internal to the chip Device Layer.
 */
#if 1
enum InternalPlatformSpecificEventTypes
{
    kQorvoBLEConnected = kRange_InternalPlatformSpecific,
    kQorvoBLEDisconnected,
    kCHIPoBLECCCWriteEvent,
    kCHIPoBLERXCharWriteEvent,
    kCHIPoBLETXCharWriteEvent,
    kRtkWiFiStationConnectedEvent,
    kRtkWiFiScanCompletedEvent,
};
#endif

} // namespace DeviceEventType

/**
 * Represents platform-specific event information.
 */
struct ChipDevicePlatformEvent final
{
    union
    {
        WIFI_EVENT_INDICATE AmebaWiFiEventID;
        uint8_t AmebaOtherEventID;
    } AmebaSystemEvents;
};

} // namespace DeviceLayer
} // namespace chip
