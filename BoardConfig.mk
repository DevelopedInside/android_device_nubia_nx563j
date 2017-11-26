#
# Copyright (C) 2017 The LineageOS Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# Inherit from common msm8998-common
-include device/nubia/msm8998-common/BoardConfigCommon.mk

DEVICE_PATH := device/nubia/nx563j

# OTA Assert
TARGET_OTA_ASSERT_DEVICE := nx563j, NX563J

# CM Hardware
BOARD_USES_CYANOGEN_HARDWARE := true
BOARD_HARDWARE_CLASS += \
    hardware/cyanogen/cmhw \
    $(DEVICE_PATH)/cmhw
TARGET_TAP_TO_WAKE_NODE := "/data/tp/easy_wakeup_gesture"

# Kernel
TARGET_KERNEL_SOURCE := kernel/nubia/msm8998
TARGET_KERNEL_CONFIG := lineage_NX563J_defconfig

# Properties
TARGET_SYSTEM_PROP += $(DEVICE_PATH)/system.prop

# inherit from the proprietary version
-include vendor/nubia/nx563j/BoardConfigVendor.mk