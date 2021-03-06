/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2016  Sander van der Burg
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

#ifndef __DISNIX_PROFILES_H
#define __DISNIX_PROFILES_H
#include <glib.h>

/**
 * Sets the Disnix profiles on the target machines and the Disnix coordinator
 * profile on the coordinator machine so that the current configuration is known
 * and the installed components are no longer considered garbage.
 *
 * @param manifest_file Path to the manifest file representing the deployment state
 * @param coordinator_profile_path Path where the current deployment configuration must be stored
 * @param profile Name of the distributed profile
 * @param no_coordinator_profile Do not create a coordinator profile
 * @param no_target_profiles Do not create Disnix profiles on the target machines
 * @return 0 if everything succeeds, else a non-zero exit status
 */
int set_profiles(const gchar *manifest_file, const gchar *coordinator_profile_path, char *profile, const int no_coordinator_profile, const int no_target_profiles);

#endif
