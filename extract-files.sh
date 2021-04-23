#!/bin/bash
#
# Copyright (C) 2016 The CyanogenMod Project
# Copyright (C) 2017 The LineageOS Project
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

function blob_fixup() {
    case "${1}" in
    # Patch blobs for VNDK
    vendor/lib/hw/camera.msm8998.so)
        sed -i "s|libgui.so|libfui.so|g" "${2}"
        "${PATCHELF}" --remove-needed "libandroid.so" "${2}"
        ;;
    vendor/lib/libnubia_effect.so | vendor/lib64/libnubia_effect.so)
        sed -i "s|libgui.so|libfui.so|g" "${2}"
        ;;
    vendor/lib/libNubiaImageAlgorithm.so)
        "${PATCHELF}" --remove-needed  "libjnigraphics.so" "${2}"
        "${PATCHELF}" --remove-needed  "libnativehelper.so" "${2}"
        "${PATCHELF}" --add-needed "libui_shim.so" "${2}"
        "${PATCHELF}" --add-needed "libNubiaImageAlgorithm_shim.so" "${2}"
        ;;
    vendor/lib/libarcsoft_picauto.so)
        "${PATCHELF}" --remove-needed "libandroid.so" "${2}"
        ;;
    esac
}

# If we're being sourced by the common script that we called,
# stop right here. No need to go down the rabbit hole.
if [ "${BASH_SOURCE[0]}" != "${0}" ]; then
    return
fi

set -e

# Required!
export DEVICE=nx563j
export DEVICE_COMMON=msm8998-common
export VENDOR=nubia

export DEVICE_BRINGUP_YEAR=2017

"./../../${VENDOR}/${DEVICE_COMMON}/extract-files.sh" "$@"
