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

#include "package-management.h"
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <pwd.h>
#include <errno.h>

#define BUFFER_SIZE 1024
#define NIX_STORE_CMD "nix-store"
#define NIX_COLLECT_GARBAGE_CMD "nix-collect-garbage"
#define NIX_ENV_CMD "nix-env"

#define RESOLVED_PATH_MAX_SIZE 4096

pid_t pkgmgmt_import_closure(int closure_fd, int stdout, int stderr)
{
    if(closure_fd == -1)
        return -1;
    else
    {
        pid_t pid = fork();
        
        if(pid == 0)
        {
            char *args[] = {NIX_STORE_CMD, "--import", NULL};
            
            dup2(closure_fd, 0);
            dup2(stdout, 1);
            dup2(stderr, 2);
            execvp(NIX_STORE_CMD, args);
            _exit(1);
        }
        
        return pid;
    }
}

gchar **pkgmgmt_export_closure(gchar *tmpdir, gchar **derivation, int stderr)
{
    gchar *tempfilename = g_strconcat(tmpdir, "/disnix.XXXXXX", NULL);
    int closure_fd = mkstemp(tempfilename);
    
    if(closure_fd == -1)
    {
        g_free(tempfilename);
        return NULL;
    }
    else
    {
        pid_t pid = fork();
        
        if(pid == -1)
        {
            close(closure_fd);
            g_free(tempfilename);
            return NULL;
        }
        else if(pid == 0)
        {
            unsigned int i, derivation_length = g_strv_length(derivation);
            gchar **args = (char**)g_malloc((3 + derivation_length) * sizeof(gchar*));

            args[0] = NIX_STORE_CMD;
            args[1] = "--export";

            for(i = 0; i < derivation_length; i++)
                args[i + 2] = derivation[i];

            args[i + 2] = NULL;

            dup2(closure_fd, 1);
            dup2(stderr, 2);
            execvp(NIX_STORE_CMD, args);
            _exit(1);
        }
        else
        {
            gchar **tempfilepaths;
            
            wait(&pid);

            if(WIFEXITED(pid) && WEXITSTATUS(pid) == 0)
            {
                tempfilepaths = (gchar**)g_malloc(2 * sizeof(gchar*));
                tempfilepaths[0] = tempfilename;
                tempfilepaths[1] = NULL;
                
                if(fchmod(closure_fd, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) == 1)
                    dprintf(stderr, "Cannot change permissions on exported closure: %s\n", tempfilename);
            }
            else
            {
                tempfilepaths = NULL;
                g_free(tempfilename);
            }
            
            close(closure_fd);
            
            return tempfilepaths;
        }
    }
}

pid_t pkgmgmt_print_invalid_packages(gchar **derivation, int pipefd[2], int stderr)
{
    if(pipe(pipefd) == 0)
    {
        pid_t pid = fork();

        if(pid == 0)
        {
            unsigned int i, derivation_length = g_strv_length(derivation);
            gchar **args = (char**)g_malloc((4 + derivation_length) * sizeof(gchar*));

            close(pipefd[0]); /* Close read-end of the pipe */

            args[0] = NIX_STORE_CMD;
            args[1] = "--check-validity";
            args[2] = "--print-invalid";

            for(i = 0; i < derivation_length; i++)
                args[i + 3] = derivation[i];

            args[i + 3] = NULL;
            
            dup2(pipefd[1], 1); /* Attach write-end to stdout */
            dup2(stderr, 2); /* Attach logger to stderr */
            execvp(NIX_STORE_CMD, args);
            _exit(1);
        }
        
        return pid;
    }
    else
        return -1;
}

pid_t pkgmgmt_realise(gchar **derivation, int pipefd[2], int stderr)
{
    if(pipe(pipefd) == 0)
    {
        pid_t pid = fork();

        if(pid == 0)
        {
            unsigned int i, derivation_size = g_strv_length(derivation);
            gchar **args = (gchar**)g_malloc((3 + derivation_size) * sizeof(gchar*));

            close(pipefd[0]); /* Close read-end of pipe */

            args[0] = NIX_STORE_CMD;
            args[1] = "-r";

            for(i = 0; i < derivation_size; i++)
                args[i + 2] = derivation[i];

            args[i + 2] = NULL;

            dup2(pipefd[1], 1);
            dup2(stderr, 2);
            execvp(NIX_STORE_CMD, args);
            _exit(1);
        }
        
        return pid;
    }
    else
        return -1;
}

pid_t pkgmgmt_set_profile(gchar *profile, gchar *derivation, int stdout, int stderr)
{
    gchar *profile_path;
    ssize_t resolved_path_size;
    char resolved_path[BUFFER_SIZE];
    pid_t pid;
    
    mkdir(LOCALSTATEDIR "/nix/profiles/disnix", 0755);
    profile_path = g_strconcat(LOCALSTATEDIR "/nix/profiles/disnix/", profile, NULL);
    
    /* Resolve the manifest file to which the disnix profile points */
    resolved_path_size = readlink(profile_path, resolved_path, BUFFER_SIZE);
    
    if(resolved_path_size != -1 && (strlen(profile_path) != resolved_path_size || strncmp(resolved_path, profile_path, resolved_path_size) != 0)) /* If the symlink resolves not to itself, we get a generation symlink that we must resolve again */
    {
        gchar *generation_path;

        resolved_path[resolved_path_size] = '\0';
        
        generation_path = g_strconcat(LOCALSTATEDIR "/nix/profiles/disnix/", resolved_path, NULL);
        resolved_path_size = readlink(generation_path, resolved_path, BUFFER_SIZE);
        
        g_free(generation_path);
    }
    
    pid = fork();

    if(pid == 0)
    {
        if(resolved_path_size == -1 || (strlen(derivation) == resolved_path_size && strncmp(resolved_path, derivation, resolved_path_size) != 0)) /* Only configure the configurator profile if the given manifest is not identical to the previous manifest */
        {
            gchar *args[] = {NIX_ENV_CMD, "-p", profile_path, "--set", derivation, NULL};
            dup2(stdout, 1);
            dup2(stderr, 2);
            execvp(NIX_ENV_CMD, args);
            dprintf(stderr, "Error with executing nix-env\n");
            _exit(1);
        }
        else
            _exit(0);
    }
    
    g_free(profile_path);
    return pid;
}

pid_t pkgmgmt_query_requisites(gchar **derivation, int pipefd[2], int stderr)
{
    if(pipe(pipefd) == 0)
    {
        pid_t pid = fork();

        if(pid == 0)
        {
            unsigned int i, derivation_size = g_strv_length(derivation);
            char **args = (char**)g_malloc((3 + derivation_size) * sizeof(char*));

            close(pipefd[0]); /* Close read-end of pipe */

            args[0] = NIX_STORE_CMD;
            args[1] = "-qR";
            
            for(i = 0; i < derivation_size; i++)
                args[i + 2] = derivation[i];

            args[i + 2] = NULL;

            dup2(pipefd[1], 1);
            dup2(stderr, 2);
            execvp(NIX_STORE_CMD, args);
            _exit(1);
        }
        
        return pid;
    }
    else
        return -1;
}

pid_t pkgmgmt_collect_garbage(int delete_old, int stdout, int stderr)
{
    pid_t pid = fork();
    
    if(pid == 0)
    {
        dup2(stdout, 1);
        dup2(stderr, 2);
        
        if(delete_old)
        {
            char *args[] = {NIX_COLLECT_GARBAGE_CMD, "-d", NULL};
            execvp(NIX_COLLECT_GARBAGE_CMD, args);
        }
        else
        {
            char *args[] = {NIX_COLLECT_GARBAGE_CMD, NULL};
            execvp(NIX_COLLECT_GARBAGE_CMD, args);
        }
        
        dprintf(stderr, "Error with executing garbage collect process\n");
        _exit(1);
    }
    
    return pid;
}

pid_t pkgmgmt_instantiate(gchar *infrastructure_expr, int pipefd[2])
{
    if(pipe(pipefd) == 0)
    {
        pid_t pid = fork();
    
        if(pid == 0)
        {
            char *cmd = "nix-instantiate";
            char *const args[] = {cmd, "--eval-only", "--strict", "--xml", infrastructure_expr, NULL};
            close(pipefd[0]); /* Close read-end of pipe */
            dup2(pipefd[1], 1); /* Attach write-end to stdout */
            execvp(cmd, args);
            _exit(1);
        }
    
        return pid;
    }
    else
        return -1;
}

static pid_t execute_set_coordinator_profile(gchar *profile_path, gchar *manifest_file_path)
{
    pid_t pid = fork();
    
    if(pid == 0)
    {
        char *const args[] = {NIX_ENV_CMD, "-p", profile_path, "--set", manifest_file_path, NULL};
        execvp(NIX_ENV_CMD, args);
        _exit(1);
    }
    
    return pid;
}

static gchar *compose_coordinator_profile_basedir(const gchar *coordinator_profile_path)
{
    /* Get current username */
    char *username = (getpwuid(geteuid()))->pw_name;
    
    if(coordinator_profile_path == NULL)
        return g_strconcat(LOCALSTATEDIR, "/nix/profiles/per-user/", username, "/disnix-coordinator", NULL);
    else
        return g_strdup(coordinator_profile_path);
}

/*
 * If the manifest file is an absolute path or a relative path starting
 * with ./ then the path is OK
 */
static gchar *normalize_manifest_path(const gchar *manifest_file)
{
    if((strlen(manifest_file) >= 1 && manifest_file[0] == '/') ||
       (strlen(manifest_file) >= 2 && (manifest_file[0] == '.' || manifest_file[1] == '/')))
        return g_strdup(manifest_file); /* If path is absolute or prefixed with ./ just use it */
    else
        return g_strconcat("./", manifest_file, NULL); /* Otherwise add ./ in front of the path */
}

static int compare_profile_paths(const gchar *profile_path, char *resolved_path, ssize_t resolved_path_size)
{
    return (strlen(profile_path) == resolved_path_size && strncmp(resolved_path, profile_path, resolved_path_size) == 0);
}

static ssize_t resolve_profile_symlink(gchar *profile_path, gchar *profile_base_dir, char *resolved_path)
{
    ssize_t resolved_path_size = readlink(profile_path, resolved_path, RESOLVED_PATH_MAX_SIZE);
    
    if(resolved_path_size == -1)
        return -1;
    else
    {
        if(!compare_profile_paths(profile_path, resolved_path, resolved_path_size)) /* If the symlink resolves not to itself, we get a generation symlink that we must resolve again */
        {
            gchar *generation_path;
            
            resolved_path[resolved_path_size] = '\0';
            generation_path = g_strconcat(profile_base_dir, "/", resolved_path, NULL);
            resolved_path_size = readlink(generation_path, resolved_path, RESOLVED_PATH_MAX_SIZE);
            
            g_free(generation_path);
        }
        
        return resolved_path_size;
    }
}

int pkgmgmt_set_coordinator_profile(const gchar *coordinator_profile_path, const gchar *manifest_file, const gchar *profile)
{
    gchar *profile_base_dir, *profile_path;
    char resolved_path[RESOLVED_PATH_MAX_SIZE];
    ssize_t resolved_path_size;
    int status;
    
    /* Determine which profile path to use, if a coordinator profile path is given use this value otherwise the default */
    profile_base_dir = compose_coordinator_profile_basedir(coordinator_profile_path);
    
    /* Create the profile directory */
    if(mkdir(profile_base_dir, 0755) == -1 && errno != EEXIST)
    {
        g_printerr("[coordinator]: Cannot create profile directory: %s\n", profile_base_dir);
        g_free(profile_base_dir);
        return 1;
    }
    
    /* Determine the path to the profile */
    profile_path = g_strconcat(profile_base_dir, "/", profile, NULL);
    
    /* Resolve the manifest file to which the coordinator profile points */
    resolved_path_size = resolve_profile_symlink(profile_path, profile_base_dir, resolved_path);

    if(resolved_path_size == -1 || !compare_profile_paths(manifest_file, resolved_path, resolved_path_size)) /* Only configure the configurator profile if the given manifest is not identical to the previous manifest */
    {
        /*
         * Execute nix-env --set operation to change the coordinator profile so
         * that the new configuration is known
         */
        
        gchar *manifest_file_path = normalize_manifest_path(manifest_file); /* Normalize manifest file path */
        pid_t pid = execute_set_coordinator_profile(profile_path, manifest_file_path);
        
        /* If the process suceeds the the operation succeeded */
        if(pid == -1)
            status = -1;
        else
        {
            wait(&pid);
    
            if(WIFEXITED(pid))
                status = WEXITSTATUS(pid);
            else
                status = 1;
        }
        
        /* Cleanup */
        g_free(manifest_file_path);
    }
    else
        status = 0;
    
    /* Cleanup */
    g_free(profile_base_dir);
    g_free(profile_path);
    
    /* Return status */
    return status;
}
