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

#include "distribute.h"
#include <distributionmapping.h>
#include <client-interface.h>

int distribute(gchar *interface, gchar *manifest_file)
{
    int exit_status = 0;
    
    /* Generate a distribution array from the manifest file */
    GArray *distribution_array = generate_distribution_array(manifest_file);
	
    if(distribution_array != NULL)
    {
	unsigned int i;
	
        /* Iterate over the distribution array and distribute the profiles to the target machines */
        for(i = 0; i < distribution_array->len; i++)
        {
    	    DistributionItem *item = g_array_index(distribution_array, DistributionItem*, i);
	    int status;
	    
	    /* Invoke copy closure operation */
	    g_printerr("Distributing intra-dependency closure of profile: %s to target: %s\n", item->profile, item->target);		
	    status = wait_to_finish(exec_copy_closure_to(interface, item->target, item->profile));
		
	    /* On error stop the distribute process */
	    if(status != 0)
	    {
		g_printerr("Distribution operation failed!\n");
	        exit_status = status;
	        break;
	    }
	}
	
	/* Delete distribution array from memory */
	delete_distribution_array(distribution_array);
    }
	
    /* Return the exit status, which is 0 if everything succeeds */
    return exit_status;
}
