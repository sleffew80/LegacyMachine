/*
* LegacyMachine - A libRetro implementation for creating simple lo-fi
* frontends intended to simulate the look and feel of the classic
* video gaming consoles, computers, and arcade machines being emulated.
*
* Copyright (C) 2022-2024 Steven Leffew
* All rights reserved
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
* */

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>

#include <file/file_path.h>

#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFArray.h>
#include <Foundation/NSPathUtilities.h>

#include "../PlatformDriver.h"
#include "../../MainEngine.h"

 /* Get environment and set directories. */
static void MacOS_GetEnvironment(void)
{
    SettingsManager* settings = GetSettingsManagerContext();

    char base_path[PATH_MAX] = {0};
    const char *home_path = getenv("HOME");

    CFURLRef bundle_url;
    CFStringRef bundle_path;
    char bundle_path_buffer[PATH_MAX] = {0};
    char documents_path_buffer[PATH_MAX] = {0};
    char application_data_path[PATH_MAX] = {0};
    
    CFBundleRef bundle = CFBundleGetMainBundle();

    if(!bundle)
        return;

    bundle_url = CFBundleCopyBundleURL(bundle);
    bundle_path = CFURLCopyFileSystemPath(bundle_url, kCFURLPOSIXPathStyle);
    CFStringGetCString(bundle_path, bundle_path_buffer, sizeof(bundle_path_buffer), kCFStringEncodingUTF8);
    CFRelease(bundle_path);
    CFRelease(bundle_url);
    path_resolve_realpath(bundle_path_buffer, sizeof(bundle_path_buffer), true);

    if(home_path)
    {
        strlcpy(base_path, home_path, PATH_MAX);
        strlcat(base_path, "/Library/Application Support/", sizeof(base_path));
        fill_pathname_join(application_data_path, base_path,
            settings->program_name, sizeof(application_data_path));
    }

    strlcpy(settings->main_directory, bundle_path_buffer, sizeof(settings->main_directory));
    strlcat(settings->main_directory, "/Contents/MacOS", sizeof(settings->main_directory));
    strlcpy(settings->asset_directory, bundle_path_buffer, sizeof(settings->main_directory));
    strlcat(settings->asset_directory, "/Contents/Resources", sizeof(settings->main_directory));
    fill_pathname_join(settings->setting_directory, application_data_path, 
        "Settings", sizeof(settings->setting_directory));
    fill_pathname_join(settings->core_directory, application_data_path, 
        "Cores", sizeof(settings->core_directory));
    fill_pathname_join(settings->content_directory, application_data_path,
        "Content", sizeof(settings->content_directory));
    fill_pathname_join(settings->system_directory, application_data_path,
        "System", sizeof(settings->system_directory));
    fill_pathname_join(settings->save_directory, application_data_path, 
        "Saves", sizeof(settings->save_directory));
    fill_pathname_join(settings->state_directory, application_data_path,
        "States", sizeof(settings->state_directory));
}

/**************************************************************************************************
 * MacOS Platform Driver
 *************************************************************************************************/

PlatformDriver macos_platform_driver = {
	MacOS_GetEnvironment,
	false
};
