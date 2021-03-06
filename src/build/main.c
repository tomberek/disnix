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
#include "build.h"

static void print_usage(const char *command)
{
    printf("Usage: %s [OPTION] DISTRIBUTED_DERIVATION\n\n", command);
    
    printf("The command `disnix-build' builds derivations on the given target machines\n");
    printf("specified in a distributed derivation XML file. When the building process is\n");
    printf("complete, the results are transfered back to the coordinator machine, so that\n");
    printf("they are kept for further use and do not have to be rebuilt again in case of a\n");
    printf("configuration change.\n\n");
    
    printf("In most cases this command should not be called directly. The command\n");
    printf("`disnix-env' automatically uses this command if the --build-on-targets is\n");
    printf("specified.\n\n");
    
    printf("Options:\n");
    printf("  -m, --max-concurrent-transfers=NUM  Maximum amount of concurrent closure\n");
    printf("                                      transfers. Defauls to: 2\n");
    printf("  -h, --help                          Shows the usage of this command to the user\n");
    printf("  -v, --version                       Shows the version of this command to the\n");
    printf("                                      user\n");
    
    printf("\nEnvironment:\n");
    printf("  DISNIX_TARGET_PROPERTY    Specifies which property in the infrastructure Nix\n");
    printf("                            expression specifies how to connect to the remote\n");
    printf("                            interface (defaults to: hostname)\n");
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
        {"max-concurrent-transfers", required_argument, 0, 'm'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };
    
    unsigned int max_concurrent_transfers = 2;
    
    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "m:hv", long_options, &option_index)) != -1)
    {
        switch(c)
        {
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

    if(optind >= argc)
    {
        fprintf(stderr, "ERROR: No distributed derivation file specified!\n");
        return 1;
    }
    else
        return build(argv[optind], max_concurrent_transfers); /* Perform distributed build operation */
}
