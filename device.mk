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

# Get non-open-source specific aspects
$(call inherit-product, vendor/nubia/nx563j/nx563j-vendor.mk)

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

# Consumerir
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.consumerir.xml:system/etc/permissions/android.hardware.consumerir.xml

# Init scripts
PRODUCT_PACKAGES += \
    init.target.rc

# Input
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/idc/input_proxy.idc:system/usr/idc/input_proxy.idc

# Keylayout
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/keylayout/nubia_synaptics_dsx.kl:system/usr/keylayout/nubia_synaptics_dsx.kl

# Temp files
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/prebuilt/lib/hw/vibrator.default.so:system/lib/hw/vibrator.default.so \
    $(LOCAL_PATH)/prebuilt/lib/hw64/vibrator.default.so:system/lib/hw64/vibrator.default.so \
    $(LOCAL_PATH)/prebuilt/lib/modules/qca_cld3/qca_cld3_wlan.ko:system/lib/modules/qca_cld3/qca_cld3_wlan.ko \
    $(LOCAL_PATH)/prebuilt/lib/modules/br_netfilter.ko:system/lib/modules/br_netfilter.ko \
    $(LOCAL_PATH)/prebuilt/lib/modules/mpq-adapter.ko:system/lib/modules/mpq-adapter.ko \
    $(LOCAL_PATH)/prebuilt/lib/modules/mpq-dmx-hw-plugin.ko:system/lib/modules/mpq-dmx-hw-plugin.ko \
    $(LOCAL_PATH)/prebuilt/lib/modules/multi_channel_hook.ko:system/lib/modules/multi_channel_hook.ko \
    $(LOCAL_PATH)/prebuilt/lib/modules/rdbg.ko:system/lib/modules/rdbg.ko \
    $(LOCAL_PATH)/prebuilt/lib/modules/test-iosched.ko:system/lib/modules/test-iosched.ko \
    $(LOCAL_PATH)/prebuilt/lib/modules/texfat.ko:system/lib/modules/texfat.ko \
    $(LOCAL_PATH)/prebuilt/lib/modules/tspp.ko:system/lib/modules/tspp.ko \
    $(LOCAL_PATH)/prebuilt/lib/modules/ufs_test.ko:system/lib/modules/ufs_test.ko \
    $(LOCAL_PATH)/prebuilt/lib/modules/wil6210.ko:system/lib/modules/wil6210.ko \
    $(LOCAL_PATH)/prebuilt/lib/modules/wlan.ko:system/lib/modules/wlan.ko
