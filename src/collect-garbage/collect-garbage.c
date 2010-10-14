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

#include "collect-garbage.h"

#include <sys/types.h>
#include <sys/wait.h>

#include <infrastructure.h>
#include <client-interface.h> 

int collect_garbage(gchar *interface, gchar *target_property, gchar *infrastructure_expr, gboolean delete_old)
{
    int exit_status = 0;
    
    /* Retrieve an array of all target machines from the infrastructure expression */
    GArray *target_array = create_target_array(infrastructure_expr, target_property);
	
    if(target_array != NULL)
    {
        unsigned int i, running_processes = 0;	    
        int status;
	
        /* Spawn garbage collection processes */
        for(i = 0; i < target_array->len; i++)
        {    
    	    gchar *target = g_array_index(target_array, gchar*, i);
				
	    g_printerr("Collecting garbage on: %s\n", target);
	    status = exec_collect_garbage(interface, target, delete_old);		
	
	    /* If an operation failed, change the exit status */
	    if(status == -1)
	    {
	        g_printerr("Error executing garbage collection operation!\n");
	        exit_status = status;
	    }
	    else
	        running_processes++;
	}
	    	    
	/* Check statusses of the running processes */	    
	for(i = 0; i < running_processes; i++)
	{
	    /* Wait until a garbage collector processes is finished */
	    wait(&status);
	    
	    /* If one of the processes fail, change the exit status */
	    if(WEXITSTATUS(status) != 0)
		exit_status = WEXITSTATUS(status);
	}
	    	
	/* Delete the target array from memory */
	delete_target_array(target_array);
    }
	
    /* Return the exit status, which is 0 if everything succeeds */
    return exit_status;
}
