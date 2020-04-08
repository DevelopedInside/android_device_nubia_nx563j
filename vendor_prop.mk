#
# Copyright (C) 2019 The LineageOS Project
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

# Cabl
PRODUCT_PROPERTY_OVERRIDES += \
    ro.qualcomm.cabl=2 \
    ro.vendor.display.cabl=2

# Camera
PRODUCT_PROPERTY_OVERRIDES += \
    ro.total.camera.number=3

# Charger
PRODUCT_PROPERTY_OVERRIDES += \
    persist.sys.standard_charger=USB_HVDCP_3

# Thermal
PRODUCT_PROPERTY_OVERRIDES += \
    persist.sys.lcd.backlight.max=4095 \
    persist.vendor.product.perf=1 \
    ro.vendor.thermal.product=0
