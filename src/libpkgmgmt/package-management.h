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

#ifndef __DISNIX_PACKAGE_MANAGEMENT_H
#define __DISNIX_PACKAGE_MANAGEMENT_H

#include <glib.h>
#include <unistd.h>

pid_t pkgmgmt_import_closure(int closure_fd, int stdout, int stderr);

gchar **pkgmgmt_export_closure(gchar *tmpdir, gchar **derivation, int stderr);

pid_t pkgmgmt_print_invalid_packages(gchar **derivation, int pipefd[2], int stderr);

pid_t pkgmgmt_realise(gchar **derivation, int pipefd[2], int stderr);

pid_t pkgmgmt_set_profile(gchar *profile, gchar *derivation, int stdout, int stderr);

pid_t pkgmgmt_query_requisites(gchar **derivation, int pipefd[2], int stderr);

pid_t pkgmgmt_collect_garbage(int delete_old, int stdout, int stderr);

pid_t pkgmgmt_instantiate(gchar *infrastructure_expr, int pipefd[2]);

pid_t pkgmgmt_set_coordinator_profile(gchar *profile_path, gchar *manifest_file_path);

#endif
