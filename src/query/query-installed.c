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
#include "query-installed.h"
#include <infrastructure.h>
#include <client-interface.h>

int query_installed(gchar *interface, const gchar *target_property, gchar *infrastructure_expr, gchar *profile)
{
    /* Retrieve an array of all target machines from the infrastructure expression */
    GPtrArray *target_array = create_target_array(infrastructure_expr);

    if(target_array == NULL)
    {
        g_printerr("[coordinator]: Error retrieving targets from infrastructure model!\n");
        return 1;
    }
    else
    {
        int exit_status = 0;
        unsigned int i;
        
        /* For each target execute the query operation and display the results */
        for(i = 0; i < target_array->len; i++)
        {
            Target *target = g_ptr_array_index(target_array, i);
            gchar *client_interface = target->client_interface;
            gchar *target_key = find_target_key(target, target_property);
            int status;
            
            /* If no client interface is provided by the infrastructure model, use global one */
            if(client_interface == NULL)
                client_interface = interface;
            
            g_print("\nServices on: %s\n\n", target_key);
            
            status = wait_to_finish(exec_query_installed(client_interface, target_key, profile));
            
            if(status != 0)
            {
                g_printerr("[target: %s]: Failed executing the query operation!\n", target_key);
                exit_status = status;
            }
        }
        
        /* Delete the target array from memory */
        delete_target_array(target_array);
        
        /* Return the exit status, which is 0 if everything succeeds */
        return exit_status;
    }
}
