$(call inherit-product, device/nubia/nx563j/full_nx563j.mk)

# Inherit some common Lineage stuff.
$(call inherit-product, vendor/lineage/config/common_full_phone.mk)

PRODUCT_NAME := lineage_nx563j
PRODUCT_DEVICE := nx563j
PRODUCT_BRAND := nubia
PRODUCT_MODEL := NX563J
PRODUCT_MANUFACTURER := nubia

PRODUCT_GMS_CLIENTID_BASE := android-nubia

PRODUCT_BUILD_PROP_OVERRIDES += \
    PRIVATE_BUILD_DESC="NX563J-user 8.1.0 OPM1.171019.011 nubia02011149 release-keys"

BUILD_FINGERPRINT := nubia/NX563J/NX563J:8.1.0/OPM1.171019.011/nubia02011149:user/release-keys
