# Fingerprint
PRODUCT_PACKAGES += \
    fingerprintd

# fingerprint pay
PRODUCT_BOOT_JARS += \
    ifaa_fingerprint

# Permissions
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.fingerprint.xml:system/etc/permissions/android.hardware.fingerprint.xml