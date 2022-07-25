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

# Device was launched with N-MR1
$(call inherit-product, $(SRC_TARGET_DIR)/product/product_launched_with_n_mr1.mk)

# Overlays
DEVICE_PACKAGE_OVERLAYS += \
    $(LOCAL_PATH)/overlay \
    $(LOCAL_PATH)/overlay-lineage

PRODUCT_ENFORCE_RRO_EXCLUDED_OVERLAYS += \
    $(LOCAL_PATH)/overlay-lineage/packages/apps/Snap

# Inherit from msm8998-common
$(call inherit-product, device/nubia/msm8998-common/msm8998.mk)

# Individual audio configs
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/audio/audio_platform_info.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_platform_info.xml \
    $(LOCAL_PATH)/audio/audio_policy_volumes.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_volumes.xml \
    $(LOCAL_PATH)/audio/mixer_paths_tasha.xml:$(TARGET_COPY_OUT_VENDOR)/etc/mixer_paths_tasha.xml \
    $(LOCAL_PATH)/audio/speaker.ftcfg:$(TARGET_COPY_OUT_VENDOR)/etc/speaker.ftcfg

# Bootanimation
TARGET_SCREEN_HEIGHT := 1920
TARGET_SCREEN_WIDTH := 1080

# Camera
PRODUCT_PACKAGES += \
    libNubiaImageAlgorithm_shim \
    libfui \
    libui_shim.vendor:32

# Consumerir
PRODUCT_PACKAGES += \
    android.hardware.ir@1.0-impl \
    android.hardware.ir@1.0-service

# Consumerir
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.consumerir.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.consumerir.xml

# Display configs
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/display/qdcm_calib_data_jdi_r63452_1080p_5p5_mipi_cmd_panel.xml:$(TARGET_COPY_OUT_VENDOR)/etc/qdcm_calib_data_jdi_r63452_1080p_5p5_mipi_cmd_panel.xml

# Keylayout
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/keylayout/nubia_synaptics_dsx.kl:$(TARGET_COPY_OUT_VENDOR)/usr/keylayout/nubia_synaptics_dsx.kl

# Lights
PRODUCT_PACKAGES += \
    android.hardware.light@2.0-service.nx563j

# Lineage hardware
PRODUCT_PACKAGES += \
    vendor.lineage.touch@1.0-service.nx563j

# NFC
PRODUCT_PACKAGES += \
    libnfc \
    com.android.nfc_extras \
    NfcNci \
    Tag

PRODUCT_PACKAGES += \
    android.hardware.nfc@1.0-impl-bcm \
    android.hardware.nfc@1.0-service

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/nfc/libnfc-brcm.conf:$(TARGET_COPY_OUT_VENDOR)/etc/libnfc-brcm.conf \
    $(LOCAL_PATH)/configs/nfc/libnfc-nci.conf:$(TARGET_COPY_OUT_PRODUCT)/etc/libnfc-nci.conf \
    $(LOCAL_PATH)/configs/nfc/nfcee_access.xml:$(TARGET_COPY_OUT_SYSTEM)/etc/nfcee_access.xml

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.nfc.hce.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.nfc.hce.xml \
    frameworks/native/data/etc/android.hardware.nfc.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.nfc.xml \
    frameworks/native/data/etc/com.android.nfc_extras.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/com.android.nfc_extras.xml \
    frameworks/native/data/etc/com.nxp.mifare.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/com.nxp.mifare.xml

# Properties
-include $(LOCAL_PATH)/vendor_prop.mk

# Ramdisk scripts
PRODUCT_PACKAGES += \
    fstab.qcom

PRODUCT_PACKAGES += \
    init.nubia.device.rc \
    init.nubia.touch.lcd.rc

# VNDK
# Update this list with what each blob is actually for
# libgui_vendor: libnubia_effect.so, camera.msm8998.so
# libstdc++: camera.msm8998
PRODUCT_PACKAGES += \
    libgui_vendor \
    libstdc++.vendor

# Wifi
PRODUCT_PACKAGES += \
    WifiOverlay_nx563j

# Get non-open-source specific aspects
$(call inherit-product, vendor/nubia/nx563j/nx563j-vendor.mk)
