/*
 * Disnix - A distributed application layer for Nix
 * Copyright (C) 2008-2013  Sander van der Burg
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

#ifndef __DISNIX_ACTIVATIONMAPPING_H
#define __DISNIX_ACTIVATIONMAPPING_H
#include <glib.h>

/**
 * Contains properties of an inter-dependency
 */
typedef struct
{
    /** Hash code that uniquely defines a service */
    gchar *key;
    /** Target property referring to the target machine to which the service is deployed */
    gchar *target;
}
ActivationMappingKey;

/**
 * Contains all properties to activate a specific service on
 * a specific machine. This struct maps (serivce,target) ->
 * (targetProperty,type,depends_on,activated)
 */
typedef struct
{
    /** Hash code that uniquely defines a service */
    gchar *key;
    /** Target property referring to the target machine to which the service is deployed */
    gchar *target;
    /** Nix store path to the service */
    gchar *service;
    /* Name of the service */
    gchar *name;
    /** Property used to connect to the disnix service */
    gchar *targetProperty;
    /** Activation type */
    gchar *type;
    /** Array of ActivationMappingKey items representing the inter-dependencies */
    GArray *depends_on;
    /** Indicated whether the service is activated or not */
    gboolean activated;
}
ActivationMapping;

/**
 * Creates an array with activation mappings from a manifest XML file.
 *
 * @param manifest_file Path to the manifest XML file
 * @return GArray containing activation mappings
 */
GArray *create_activation_array(const gchar *manifest_file);

/**
 * Deletes an array with activation mappings including its contents.
 *
 * @param activation_array Activation array to delete
 */
void delete_activation_array(GArray *activation_array);

/**
 * Returns the index of the activation mapping with the given keys
 * in the activation array.
 *
 * @param activation_array Activation array
 * @param key Hash code that uniquely defines a service
 * @param target Target property referring to the target machine to which the service is deployed
 * @return Index of the activation mapping or -1 if not found
 */
gint activation_mapping_index(const GArray *activation_array, gchar *key, gchar *target);

/**
 * Returns the activation mapping with the given keys in the activation array.
 *
 * @param activation_array Activation array
 * @param key Hash code that uniquely defines a service
 * @param target Target property referring to the target machine to which the service is deployed
 * @return The activation mapping with the specified keys, or NULL if it cannot be found
 */
ActivationMapping *get_activation_mapping(const GArray *activation_array, gchar *key, gchar *target);

/**
 * Returns the index of a dependency with the given keys in the dependsOn array.
 *
 * @param depends_on dependsOn array
 * @param key Hash code that uniquely defines a service
 * @param target Target property referring to the target machine to which the service is deployed
 * @return Index of the dependency or -1 if not found
 */
gint dependency_index(const GArray *depends_on, gchar *key, gchar *target);

/**
 * Returns the dependency with the given keys in the dependsOn array.
 *
 * @param depends_on dependsOn array
 * @param key Hash code that uniquely defines a service
 * @param target Target property referring to the target machine to which the service is deployed
 * @return The dependency mapping with the specified keys, or NULL if it cannot be found
 */
ActivationMappingKey *get_dependency(const GArray *depends_on, gchar *key, gchar *target);

/**
 * Returns the intersection of the two given arrays.
 * The array that is returned contains pointers to elements in
 * both left and right, so it should be free with g_array_free().
 *
 * @param left Array with activation mappings
 * @param right Array with activation mappings
 * @return Array with activation mappings both in left and right
 */
GArray *intersect_activation_array(GArray *left, GArray *right);

/**
 * Returns a new array in which the activation mappings in
 * right and substracted from left.
 * The array that is returned contains pointers to elements in
 * left, so it should be free with g_array_free().
 *
 * @param left Array with activation mappings
 * @param right Array with activation mappings
 * @return Array with right substracted from left.
 */
GArray *substract_activation_array(GArray *left, GArray *right);

/**
 * Returns the union of left and right using the intersection,
 * and marks all the activation mappings in left as inactive
 * and activation mappings in right as active.
 *
 * @param left Array with activation mappings
 * @param right Array with activation mappings 
 * @param intersect Intersection of left and right
 * @return Array with activation mappings in both left and right,
 *         marked as active and inactive
 */
GArray *union_activation_array(GArray *left, GArray *right, GArray *intersect);

/**
 * Searches for all the mappings in an array that have an inter-dependency
 * on the given mapping.
 *
 * @param activation_array Array of activation mappings
 * @param mapping Activation mapping from which to derive the
 *                interdependent mapping
 * @return Array with interdependent activation mappings
 */
GArray *find_interdependent_mappings(GArray *activation_array, const ActivationMapping *mapping);

/**
 * Prints the given activation array.
 *
 * @param activation_array Activation array to print
 */
void print_activation_array(const GArray *activation_array);

#endif
