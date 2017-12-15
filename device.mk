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

# Overlays
DEVICE_PACKAGE_OVERLAYS += $(LOCAL_PATH)/overlay

# Inherit from msm8998-common
$(call inherit-product, device/nubia/msm8998-common/msm8998.mk)

# Individual audio configs
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/audio/speaker.ftcfg:system/etc/speaker.ftcfg

# Bootanimation
TARGET_BOOTANIMATION_HALF_RES := true
TARGET_SCREEN_HEIGHT := 1920
TARGET_SCREEN_WIDTH := 1080

# Device uses high-density artwork where available
PRODUCT_AAPT_CONFIG := normal
PRODUCT_AAPT_PREF_CONFIG := xxhdpi

# Display configs
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/display/calib.cfg:system/etc/calib.cfg \
    $(LOCAL_PATH)/display/qdcm_calib_data_jdi_r63452_1080p_5p5_mipi_cmd_panel.xml:system/etc/qdcm_calib_data_jdi_r63452_1080p_5p5_mipi_cmd_panel.xml

# Init scripts
PRODUCT_PACKAGES += \
    init.target.rc

# Input
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/idc/input_proxy.idc:system/usr/idc/input_proxy.idc

# Keylayout
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/keylayout/nubia_synaptics_dsx.kl:system/usr/keylayout/nubia_synaptics_dsx.kl

$(call inherit-product, vendor/nubia/nx563j/nx563j-vendor.mk)