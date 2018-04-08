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
TARGET_OTA_ASSERT_DEVICE := NX563J,nx563j

# CM Hardware
BOARD_HARDWARE_CLASS += \
    $(DEVICE_PATH)/cmhw
TARGET_TAP_TO_WAKE_NODE := "/sys/class/tpnode/tpnode/synaptics/wake_gesture"

ifeq ($(TARGET_USE_PREBUILT_KERNEL),true)
BOARD_HARDWARE_CLASS += \
    $(DEVICE_PATH)/prebuilt/cmhw
endif

# Kernel
TARGET_KERNEL_CONFIG := lineage_NX563J_defconfig
DTS_NAME += \
    msm8998-v2.1-mtp-NX563J.dtb \
    msm8998-v2-mtp-NX563J.dtb \
    msm8998-mtp-NX563J.dtb
ZTEMT_DTS_NAME:=$(DTS_NAME)
export ZTEMT_DTS_NAME

ifeq ($(TARGET_USE_PREBUILT_KERNEL),true)
BOARD_CUSTOM_BOOTIMG_MK := $(DEVICE_PATH)/prebuilt/mkbootimg.mk
TARGET_PREBUILT_KERNEL := $(DEVICE_PATH)/prebuilt/zImage
endif

# Properties
TARGET_SYSTEM_PROP += $(DEVICE_PATH)/system.prop

# inherit from the proprietary version
-include vendor/nubia/nx563j/BoardConfigVendor.mk

ifeq ($(TARGET_USE_PREBUILT_KERNEL),true)
-include vendor/nubia/nx563j-prebuilt/BoardConfigVendor.mk
endif