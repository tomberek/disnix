/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2015  Sander van der Burg
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

#include "edgestable.h"
#include <activationmapping.h>
#include <targets.h>

GHashTable *generate_edges_table(const GPtrArray *activation_array, GPtrArray *targets_array)
{    
    unsigned int i;
    
    /* Create empty hash table */
    GHashTable *edges_table = g_hash_table_new(g_str_hash, g_str_equal);

    for(i = 0; i < activation_array->len; i++)
    {
	/* Retrieve the current mapping from the array */
	ActivationMapping *mapping = g_ptr_array_index(activation_array, i);
	
	/* Retrieve the target property */
	Target *target = find_target(targets_array, mapping->target);
	gchar *target_key = find_target_key(target);
	
	/* Generate an edge table key, which consist of Nix store component:targetProperty */
	gchar *mapping_key = g_strconcat(mapping->key, ":", target_key, NULL);
	
	/* Retrieve the dependency array of the mapping from the edges table */
	GPtrArray *dependency_array = g_hash_table_lookup(edges_table, mapping_key);
	
	/* If the dependency array does not exists, create one and add it to the table */
	if(dependency_array == NULL)
	{
	    unsigned int j;
	    GPtrArray *depends_on = mapping->depends_on;
	    
	    /* Create new dependency array */
	    dependency_array = g_ptr_array_new();
	    
	    /* Create a list of mapping values for each dependency */
	    for(j = 0; j < depends_on->len; j++)
	    {
		ActivationMapping *actual_mapping;
		gchar *mapping_value, *target_key;
		Target *target;
		
		/* Retrieve current dependency from the array */
		ActivationMappingKey *dependency = g_ptr_array_index(depends_on, j);
		
		/* Find the activation mapping in the activation array */
		actual_mapping = find_activation_mapping(activation_array, dependency);
		
		/* Get the target interface */
		target = find_target(targets_array, actual_mapping->target);
		target_key = find_target_key(target);
		
		/* Generate mapping value from the service key and target property */
		mapping_value = g_strconcat(actual_mapping->key, ":", target_key, NULL);
		    
		/* Add mapping value to the dependency array */
	    	g_ptr_array_insert(dependency_array, -1, mapping_value);
	    }
	    
	    /* Associate the dependency array to the given mapping */
	    g_hash_table_insert(edges_table, mapping_key, dependency_array);
	}
    }

    /* Return the generated egdes table */
    return edges_table;
}

void destroy_edges_table(GHashTable *edges_table)
{
    GHashTableIter iter;
    gpointer *key;
    gpointer *value;

    g_hash_table_iter_init(&iter, edges_table);
    while(g_hash_table_iter_next(&iter, (gpointer*)&key, (gpointer*)&value)) 
    {
	gchar *mapping_key = (gchar*)key;
	GPtrArray *dependency_array = (GPtrArray*)value;
	unsigned int i;
	
	for(i = 0; i < dependency_array->len; i++)
	{
	    gchar *dep = g_ptr_array_index(dependency_array, i);
	    g_free(dep);
	}
	
	g_ptr_array_free(dependency_array, TRUE);
	g_free(mapping_key);
    }
        
    g_hash_table_destroy(edges_table);
}
