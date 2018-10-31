/*
 * Copyright (C) 2016 The CyanogenMod Project
 *           (C) 2017-2018 The LineageOS Project
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

package org.lineageos.settings.device;

import org.lineageos.internal.util.FileUtils;

import java.util.HashMap;
import java.util.Map;

public class Constants {

    // Preference keys
    public static final String BUTTON_SWAP_KEY = "button_swap";

    // Nodes
    public static final String TOUCHPANEL_BUTTON_SWAP_NODE = "/sys/class/input/input1/reversed_keys";

    // Holds <preference_key> -> <proc_node> mapping
    public static final Map<String, String> sBooleanNodePreferenceMap = new HashMap<>();

    // Holds <preference_key> -> <default_values> mapping
    public static final Map<String, Object> sNodeDefaultMap = new HashMap<>();

    public static final String[] sButtonPrefKeys = {
        BUTTON_SWAP_KEY
    };

    static {
        sBooleanNodePreferenceMap.put(BUTTON_SWAP_KEY, TOUCHPANEL_BUTTON_SWAP_NODE);
        sNodeDefaultMap.put(BUTTON_SWAP_KEY, false);
    }
}
