/*
 * Disnix - A distributed application layer for Nix
 * Copyright (C) 2008-2010  Sander van der Burg
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "defaultoptions.h"
#include <stdlib.h>

/** Default interface value */
static char *default_interface = "disnix-ssh-client";

/** Default target property value */ 
static char *default_target_property = "hostname";

/** Default profile value */
static char *default_profile = "default";

char *check_interface_option(char *interface)
{
    if(interface == NULL)
    {
	char *interface_env = getenv("DISNIX_CLIENT_INTERFACE");
	
	if(interface_env == NULL)
	    interface = default_interface;
	else
	    interface = interface_env;
    }
    
    return interface;
}

char *check_target_property_option(char *target_property)
{
    char *target_property_env = getenv("DISNIX_TARGET_PROPERTY");
	
    if(target_property_env == NULL)
        target_property = default_target_property;
    else
        target_property = target_property_env;

    return target_property;
}

char *check_profile_option(char *profile)
{
    if(profile == NULL)
        profile = default_profile;

    return profile;
}