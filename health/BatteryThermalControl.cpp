/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "pixelhealth/BatteryThermalControl.h"

using aidl::android::hardware::health::BatteryStatus;
using aidl::android::hardware::health::HealthInfo;

namespace hardware {
namespace google {
namespace pixel {
namespace health {

using android::base::GetIntProperty;

BatteryThermalControl::BatteryThermalControl(const std::string &path) : mThermalSocMode(path) {
    mStatus = true;
}

void BatteryThermalControl::setThermalMode(bool isEnable, bool isWeakCharger) {
    std::string action = (isEnable) ? "enabled" : "disabled";

    if (mStatus != isEnable) {
        if (!isEnable && isWeakCharger)
            return;
        if (android::base::WriteStringToFile(action, mThermalSocMode)) {
            mStatus = isEnable;
        } else {
            LOG(ERROR) << "Error Write: \"" << action << "\" to " << mThermalSocMode
                       << " error:" << strerror(errno);
        }
    }
}

void BatteryThermalControl::updateThermalState(const struct android::BatteryProperties *props) {
    int bcl_disable = GetIntProperty("persist.vendor.disable.bcl.control", 0);

    if (bcl_disable)
        setThermalMode(false, true);
    else
        setThermalMode(props->batteryStatus != android::BATTERY_STATUS_CHARGING &&
                               props->batteryStatus != android::BATTERY_STATUS_FULL,
                       props->maxChargingCurrent * props->maxChargingVoltage < 37500000);
}

void BatteryThermalControl::updateThermalState(const HealthInfo &health_info) {
    int bcl_disable = GetIntProperty("persist.vendor.disable.bcl.control", 0);

    if (bcl_disable)
        setThermalMode(false, true);
    else
        setThermalMode(
                health_info.batteryStatus != BatteryStatus::CHARGING &&
                        health_info.batteryStatus != BatteryStatus::FULL,
                health_info.maxChargingCurrentMicroamps * health_info.maxChargingVoltageMicrovolts <
                        37500000);
}

}  // namespace health
}  // namespace pixel
}  // namespace google
}  // namespace hardware
