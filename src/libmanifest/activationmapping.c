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

#include "activationmapping.h"
#include <stdlib.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <xmlutil.h>
#define min(a,b) ((a) < (b) ? (a) : (b))

static gint compare_activation_mapping_keys(const ActivationMappingKey **l, const ActivationMappingKey **r)
{
    const ActivationMappingKey *left = *l;
    const ActivationMappingKey *right = *r;
    
    /* Compare the service keys */
    gint status = g_strcmp0(left->key, right->key);
    
    if(status == 0)
    {
        status = g_strcmp0(left->target, right->target); /* If services are equal then compare the targets */
        
        if(status == 0)
            return g_strcmp0(left->container, right->container); /* If targets are equal then compare the containers */
        else
            return status;
    }
    else
        return status;
}

static gint compare_activation_mapping(const ActivationMapping **l, const ActivationMapping **r)
{
    return compare_activation_mapping_keys((const ActivationMappingKey **)l, (const ActivationMappingKey **)r);
}

GPtrArray *create_activation_array(const gchar *manifest_file)
{
    xmlDocPtr doc;
    xmlNodePtr node_root;
    xmlXPathObjectPtr result;
    GPtrArray *activation_array;
    
    /* Parse the XML document */
    
    if((doc = xmlParseFile(manifest_file)) == NULL)
    {
	g_printerr("Error with parsing the manifest XML file!\n");
	xmlCleanupParser();
	return NULL;
    }

    /* Retrieve root element */
    node_root = xmlDocGetRootElement(doc);
    
    if(node_root == NULL)
    {
        g_printerr("The distribution manifest XML file is empty!\n");
	xmlFreeDoc(doc);
	xmlCleanupParser();
	return NULL;
    }

    /* Query the distribution elements */
    result = executeXPathQuery(doc, "/manifest/activation/mapping");
    
    /* Initialize activation array */
    activation_array = g_ptr_array_new();
    
    /* Iterate over all the distribution elements and add them to the array */
    
    if(result)
    {
	xmlNodeSetPtr nodeset = result->nodesetval;
	unsigned int i;
	
	/* Iterate over all the mapping elements */
	for(i = 0; i < nodeset->nodeNr; i++)
        {
	    xmlNodePtr mapping_children = nodeset->nodeTab[i]->children;
	    gchar *key = NULL;
	    gchar *target = NULL;
	    gchar *container = NULL;
	    gchar *service = NULL;
	    gchar *name = NULL;
	    gchar *type = NULL;
	    GPtrArray *depends_on = NULL;
	    ActivationMappingStatus status = ACTIVATIONMAPPING_DEACTIVATED;
	    ActivationMapping *mapping = (ActivationMapping*)g_malloc(sizeof(ActivationMapping));
	    
	    /* Iterate over all the mapping item children (service,target,targetProperty,type,dependsOn elements) */
	    
	    while(mapping_children != NULL)
	    {
		if(xmlStrcmp(mapping_children->name, (xmlChar*) "key") == 0)
		    key = g_strdup((gchar*)mapping_children->children->content);
		else if(xmlStrcmp(mapping_children->name, (xmlChar*) "service") == 0)
		    service = g_strdup((gchar*)mapping_children->children->content);
		else if(xmlStrcmp(mapping_children->name, (xmlChar*) "name") == 0)
		    name = g_strdup((gchar*)mapping_children->children->content);
		else if(xmlStrcmp(mapping_children->name, (xmlChar*) "type") == 0)
		    type = g_strdup((gchar*)mapping_children->children->content);
		else if(xmlStrcmp(mapping_children->name, (xmlChar*) "target") == 0)
		    target = g_strdup((gchar*)mapping_children->children->content);
		else if(xmlStrcmp(mapping_children->name, (xmlChar*) "container") == 0)
		    container = g_strdup((gchar*)mapping_children->children->content);
		else if(xmlStrcmp(mapping_children->name, (xmlChar*) "dependsOn") == 0)
		{
		    xmlNodePtr depends_on_children = mapping_children->children;
		    depends_on = g_ptr_array_new();
		    
		    /* Iterate over all services in dependsOn (dependency element) */
		    while(depends_on_children != NULL)
		    {
			xmlNodePtr dependency_children = depends_on_children->children;
			gchar *key = NULL;
			gchar *target = NULL;
			gchar *container = NULL;
			ActivationMappingKey *dependency = (ActivationMappingKey*)g_malloc(sizeof(ActivationMappingKey));
			
			if(xmlStrcmp(depends_on_children->name, (xmlChar*) "dependency") == 0) /* Only iterate over dependency nodes */
			{
			    /* Iterate over all dependency properties */
			    while(dependency_children != NULL)
			    {
				if(xmlStrcmp(dependency_children->name, (xmlChar*) "key") == 0)
				    key = g_strdup((gchar*)dependency_children->children->content);
				else if(xmlStrcmp(dependency_children->name, (xmlChar*) "target") == 0)
				    target = g_strdup((gchar*)dependency_children->children->content);
				else if(xmlStrcmp(dependency_children->name, (xmlChar*) "container") == 0)
				    container = g_strdup((gchar*)dependency_children->children->content);
				
				dependency_children = dependency_children->next;
			    }
			
			    dependency->key = key;
			    dependency->target = target;
			    dependency->container = container;
			    g_ptr_array_add(depends_on, dependency);
			}
			
			depends_on_children = depends_on_children->next;
		    }
		    
		    /* Sort the dependency array */
		    g_ptr_array_sort(depends_on, (GCompareFunc)compare_activation_mapping_keys);
		}
		
		mapping_children = mapping_children->next;
	    }
	    
	    mapping->key = key;
	    mapping->target = target;
	    mapping->container = container;
	    mapping->service = service;
	    mapping->name = name;
	    mapping->type = type;
	    mapping->depends_on = depends_on;
	    mapping->status = status;
	    
	    /* Add the mapping to the array */
	    g_ptr_array_add(activation_array, mapping);
	}
    }

    /* Cleanup */
    xmlXPathFreeObject(result);
    xmlFreeDoc(doc);
    xmlCleanupParser();

    /* Sort the activation array */
    g_ptr_array_sort(activation_array, (GCompareFunc)compare_activation_mapping);
    
    /* Return the activation array */
    return activation_array;
}

void delete_activation_array(GPtrArray *activation_array)
{
    if(activation_array != NULL)
    {
        unsigned int i;
    
        for(i = 0; i < activation_array->len; i++)
        {
            ActivationMapping *mapping = g_ptr_array_index(activation_array, i);
            unsigned int j;
            
            g_free(mapping->key);
            g_free(mapping->target);
            g_free(mapping->container);
            g_free(mapping->service);
            g_free(mapping->name);
            g_free(mapping->type);

            if(mapping->depends_on != NULL)
            {
                for(j = 0; j < mapping->depends_on->len; j++)
                {
                    ActivationMappingKey *dependency = g_ptr_array_index(mapping->depends_on, j);
                    g_free(dependency->key);
                    g_free(dependency->target);
                    g_free(dependency->container);
                    g_free(dependency);
                }
            }
            
            g_ptr_array_free(mapping->depends_on, TRUE);
            g_free(mapping);
        }
    
        g_ptr_array_free(activation_array, TRUE);
    }
}

ActivationMapping *find_activation_mapping(const GPtrArray *activation_array, const ActivationMappingKey *key)
{
    ActivationMapping **ret = bsearch(&key, activation_array->pdata, activation_array->len, sizeof(gpointer), (int (*)(const void*, const void*)) compare_activation_mapping);
    
    if(ret == NULL)
        return NULL;
    else
        return *ret;
}

ActivationMappingKey *find_dependency(const GPtrArray *depends_on, const ActivationMappingKey *key)
{
    ActivationMappingKey **ret = bsearch(&key, depends_on->pdata, depends_on->len, sizeof(gpointer), (int (*)(const void*, const void*)) compare_activation_mapping_keys);
    
    if(ret == NULL)
        return NULL;
    else
        return *ret;
}

GPtrArray *intersect_activation_array(const GPtrArray *left, const GPtrArray *right)
{
    unsigned int i;
    GPtrArray *return_array = g_ptr_array_new();
    
    if(left->len < right->len)
    {
	for(i = 0; i < left->len; i++)
	{
	    ActivationMapping *left_mapping = g_ptr_array_index(left, i);
	    
	    if(find_activation_mapping(right, (ActivationMappingKey*)left_mapping) != NULL)
		g_ptr_array_add(return_array, left_mapping);
	}
    }
    else
    {
	for(i = 0; i < right->len; i++)
	{
	    ActivationMapping *right_mapping = g_ptr_array_index(right, i);

	    if(find_activation_mapping(left, (ActivationMappingKey*)right_mapping) != NULL)
		g_ptr_array_add(return_array, right_mapping);
	}
    }
    
    return return_array;
}

GPtrArray *union_activation_array(GPtrArray *left, GPtrArray *right, const GPtrArray *intersect)
{
    unsigned int i;
    GPtrArray *return_array = g_ptr_array_new();
    
    /* Create a clone of the left array and mark mappings as activated */
    
    for(i = 0; i < left->len; i++)
    {
        ActivationMapping *mapping = g_ptr_array_index(left, i);
        mapping->status = ACTIVATIONMAPPING_ACTIVATED;
        g_ptr_array_add(return_array, mapping);
    }
    
    /* Append all mappings from the right array which are not in the intersection and mark them as deactivated */
    
    for(i = 0; i < right->len; i++)
    {
        ActivationMapping *mapping = g_ptr_array_index(right, i);
        mapping->status = ACTIVATIONMAPPING_DEACTIVATED;
        
        if(find_activation_mapping(intersect, (ActivationMappingKey*)mapping) == NULL)
            g_ptr_array_add(return_array, mapping);
    }
    
    /* Sort the activation array */
    g_ptr_array_sort(return_array, (GCompareFunc)compare_activation_mapping);
    
    /* Return the activation array */
    return return_array;
}

GPtrArray *substract_activation_array(const GPtrArray *left, const GPtrArray *right)
{
    unsigned int i;
    GPtrArray *return_array = g_ptr_array_new();
    
    /* Add all elements of the left array that are not in the right array */
    for(i = 0; i < left->len; i++)
    {
	ActivationMapping *mapping = g_ptr_array_index(left, i);
	
	if(find_activation_mapping(right, (ActivationMappingKey*)mapping) == NULL)
	    g_ptr_array_add(return_array, mapping);
    }
    
    /* Return the activation array */
    return return_array;
}

GPtrArray *find_interdependent_mappings(const GPtrArray *activation_array, const ActivationMapping *mapping)
{
    GPtrArray *return_array = g_ptr_array_new();
    unsigned int i;
    
    /* For each activation mapping, check whether there is a inter-dependency on the requested mapping */
    for(i = 0; i < activation_array->len; i++)
    {
	ActivationMapping *current_mapping = g_ptr_array_index(activation_array, i);
	ActivationMappingKey *found_dependency = find_dependency(current_mapping->depends_on, (ActivationMappingKey*)mapping);
	
	if(found_dependency != NULL)
	    g_ptr_array_add(return_array, current_mapping);
    }
    
    return return_array;
}

void print_activation_array(const GPtrArray *activation_array)
{
    unsigned int i;
    
    for(i = 0; i < activation_array->len; i++)
    {
	ActivationMapping *mapping = g_ptr_array_index(activation_array, i);
	unsigned int j;
	
	g_print("key: %s\n", mapping->key);
	g_print("target: %s\n", mapping->target);
	g_print("container: %s\n", mapping->container);
	g_print("service: %s\n", mapping->service);
	g_print("type: %s\n", mapping->type);
	g_print("dependsOn:\n");
	
	for(j = 0; j < mapping->depends_on->len; j++)
	{
	    ActivationMappingKey *dependency = g_ptr_array_index(mapping->depends_on, j);
	    
	    g_print("  key: %s\n", dependency->key);
	    g_print("  target: %s\n", dependency->target);
	    g_print("  container: %s\n", dependency->container);
	}
	
	g_print("\n");
    }
}
