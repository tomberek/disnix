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

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <defaultoptions.h>
#include "snapshot.h"

static void print_usage(const char *command)
{
    printf("Usage: %s [OPTION] [MANIFEST]\n\n", command);
    
    printf("Snapshots the state of components deployed in a network of machines.\n\n");
    
    printf("By default, this command only snapshots the state of components that are recently\n");
    printf("deactivated, i.e. the ones that are not defined in the current deployment\n");
    printf("manifest, but in the previous one.\n\n");
    
    printf("A full snapshot of the state of the entire system can be done by providing the\n");
    printf("--no-upgrade parameter.\n\n");
    
    printf("If no manifest has been provided, the last deployed one is used. Moreover, it\n");
    printf("also snapshots the entire system if no manifest is provided. As a result, simply\n");
    printf("running `disnix-snapshot' makes a backup of the state of the entire system.\n\n");
    
    printf("Options:\n");
    printf("  -c, --container=CONTAINER            Name of the container in which the\n");
    printf("                                       mutable component is deployed\n");
    printf("  -C, --component=COMPONENT            Name of the mutable component to take\n");
    printf("                                       snapshots from\n");
    printf("  -o, --old-manifest=MANIFEST          Nix profile path where the manifest\n");
    printf("                                       should be stored, so that Disnix knows\n");
    printf("                                       the current configuration of a\n");
    printf("                                       distributed system. By default it is\n");
    printf("                                       stored in the profile directory of the\n");
    printf("                                       user.\n");
    printf("      --no-upgrade                     Indicates that no upgrade should be\n");
    printf("                                       performed and the state of all components\n");
    printf("                                       should be restored.\n");
    printf("      --transfer-only                  Transfers the snapshot from the target\n");
    printf("                                       machines, but does not actually restore\n");
    printf("                                       them\n");
    printf("      --all                            Transfers all snapshot generations of the\n");
    printf("                                       target machines, not the latest\n");
    printf("  -p, --profile=PROFILE                Name of the profile in which the services\n");
    printf("                                       are registered. Defaults to: default\n");
    printf("      --coordinator-profile-path=PATH  Path to the manifest of the previous\n");
    printf("                                       configuration. By default this tool will\n");
    printf("                                       use the manifest stored in the disnix\n");
    printf("                                       coordinator profile instead of the\n");
    printf("                                       specified one, which is usually sufficient\n");
    printf("                                       in most cases.\n");
    printf("  -m, --max-concurrent-transfers=NUM   Maximum amount of concurrent closure\n");
    printf("                                       transfers. Defauls to: 2\n");
    printf("  -h, --help                           Shows the usage of this command to the\n");
    printf("                                       user\n");

    printf("\nEnvironment:\n");
    printf("  DISNIX_PROFILE    Sets the name of the profile that stores the manifest on the\n");
    printf("                    coordinator machine and the deployed services per machine on\n");
    printf("                    each target (Defaults to: default)\n");
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
        {"container", required_argument, 0, 'c'},
        {"component", required_argument, 0, 'C'},
        {"coordinator-profile-path", required_argument, 0, 'P'},
        {"profile", required_argument, 0, 'p'},
        {"old-manifest", required_argument, 0, 'o'},
        {"no-upgrade", no_argument, 0, 'u'},
        {"transfer-only", no_argument, 0, 't'},
        {"all", no_argument, 0, 'a'},
        {"max-concurrent-transfers", required_argument, 0, 'm'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };
    
    unsigned int max_concurrent_transfers = 2;
    int transfer_only = FALSE;
    int all = FALSE;
    int no_upgrade = FALSE;
    char *manifest_file;
    char *old_manifest = NULL;
    char *profile = NULL;
    char *coordinator_profile_path = NULL;
    char *container = NULL;
    char *component = NULL;
    
    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "c:C:m:o:p:hv", long_options, &option_index)) != -1)
    {
        switch(c)
        {
            case 'c':
                container = optarg;
                break;
            case 'C':
                component = optarg;
                break;
            case 'p':
                profile = optarg;
                break;
            case 'P':
                coordinator_profile_path = optarg;
                break;
            case 'o':
                old_manifest = optarg;
                break;
            case 'u':
                no_upgrade = TRUE;
                break;
            case 'a':
                all = TRUE;
                break;
            case 't':
                transfer_only = TRUE;
                break;
            case 'm':
                max_concurrent_transfers = atoi(optarg);
                break;
            case 'h':
            case '?':
                print_usage(argv[0]);
                return 0;
            case 'v':
                print_version(argv[0]);
                return 0;
        }
    }

    /* Validate options */
    
    profile = check_profile_option(profile);
    
    if(optind >= argc)
        manifest_file = NULL;
    else
        manifest_file = argv[optind];
    
    return snapshot(manifest_file, max_concurrent_transfers, transfer_only, all, old_manifest, coordinator_profile_path, profile, no_upgrade, container, component); /* Execute snapshot operation */
}
