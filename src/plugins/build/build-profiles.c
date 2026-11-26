/*
   The build profiles of a project: a build directory and how it is built, each.

   Copyright (C) 2026
   Ilia Maslakov <il.smind@gmail.com>

   Written by:
   Ilia Maslakov <il.smind@gmail.com>, 2026

   This file is part of bluerig.

   bluerig is free software: you can redistribute it
   and/or modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of the License,
   or (at your option) any later version.

   bluerig is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/** \file build-profiles.c
 *  \brief Source: the build profiles of a project, a build directory and how it is built each
 */

#include <config.h>

#include <string.h>

#include "lib/global.h"

#include "build-profiles.h"

/*** global variables ****************************************************************************/

/*** file scope macro definitions ****************************************************************/

/*** file scope type declarations ****************************************************************/

/*** forward declarations (file scope functions) *************************************************/

/*** file scope variables ************************************************************************/

static const struct
{
    build_system_t system;
    const char *name;
} build_system_names[] = {
    { BUILD_NONE, "none" },
    { BUILD_MESON, "meson" },
    { BUILD_CMAKE, "cmake" },
    { BUILD_MAKE, "make" },
};

/* --------------------------------------------------------------------------------------------- */
/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

static void
build_profile_free (gpointer p)
{
    build_profile_t *profile = (build_profile_t *) p;

    g_free (profile->name);
    g_free (profile->dir);
    g_free (profile->from);
    g_free (profile->setup);
    g_free (profile->build);
    g_free (profile->test);
    g_free (profile);
}

/* --------------------------------------------------------------------------------------------- */

static build_profiles_t *
build_profiles_new (void)
{
    build_profiles_t *profiles = g_new0 (build_profiles_t, 1);

    profiles->list = g_ptr_array_new_with_free_func (build_profile_free);
    return profiles;
}

/* --------------------------------------------------------------------------------------------- */

/* A string of @group, NULL for none or an empty one */
static char *
build_profiles_string (GKeyFile *keyfile, const char *group, const char *key)
{
    char *value = g_key_file_get_string (keyfile, group, key, NULL);

    if (value != NULL && *g_strstrip (value) == '\0')
        g_clear_pointer (&value, g_free);
    return value;
}

/* --------------------------------------------------------------------------------------------- */

static void
build_profiles_set (GKeyFile *keyfile, const char *group, const char *key, const char *value)
{
    if (value != NULL)
        g_key_file_set_string (keyfile, group, key, value);
    else
        (void) g_key_file_remove_key (keyfile, group, key, NULL);
}

/* --------------------------------------------------------------------------------------------- */

/* @text with each of the names @olds, NULL ended, made the one of @news at its place: a whole
   name only, between the start, a space, a quote or '=' and the end, a space, a quote, '/' or
   one of ;&|), the name "build" not in "--build"; the longest name first where two fit */
static char *
build_profiles_replace (const char *text, const char *const *olds, const char *const *news)
{
    GString *out;
    const char *p;

    if (text == NULL)
        return NULL;
    out = g_string_new (NULL);
    for (p = text; *p != '\0';)
    {
        gsize best = 0;
        int which = -1, i;

        if (p == text || strchr (" '\"=", p[-1]) != NULL)
            for (i = 0; olds[i] != NULL; i++)
            {
                const gsize len = strlen (olds[i]);

                if (len > best && strncmp (p, olds[i], len) == 0
                    && (p[len] == '\0' || strchr (" '\"/;&|)", p[len]) != NULL))
                {
                    best = len;
                    which = i;
                }
            }
        if (which >= 0)
        {
            g_string_append (out, news[which]);
            p += best;
        }
        else
            g_string_append_c (out, *p++);
    }
    return g_string_free (out, FALSE);
}

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

const char *
build_system_name (build_system_t system)
{
    size_t i;

    for (i = 0; i < G_N_ELEMENTS (build_system_names); i++)
        if (build_system_names[i].system == system)
            return build_system_names[i].name;
    return "none";
}

/* --------------------------------------------------------------------------------------------- */

build_system_t
build_system_of_name (const char *name)
{
    size_t i;

    for (i = 0; name != NULL && i < G_N_ELEMENTS (build_system_names); i++)
        if (strcmp (build_system_names[i].name, name) == 0)
            return build_system_names[i].system;
    return BUILD_NONE;
}

/* --------------------------------------------------------------------------------------------- */

build_profiles_t *
build_profiles_load (GKeyFile *keyfile)
{
    build_profiles_t *profiles = build_profiles_new ();
    char **groups, **g;

    if (keyfile == NULL)
        return profiles;
    groups = g_key_file_get_groups (keyfile, NULL);
    for (g = groups; *g != NULL; g++)
    {
        build_profile_t *profile;
        char *system;

        if (!g_str_has_prefix (*g, BUILD_PROFILES_GROUP_PREFIX)
            || (*g)[strlen (BUILD_PROFILES_GROUP_PREFIX)] == '\0'
            || build_profiles_find (profiles, *g + strlen (BUILD_PROFILES_GROUP_PREFIX)) != NULL)
            continue;
        profile = g_new0 (build_profile_t, 1);
        profile->name = g_strdup (*g + strlen (BUILD_PROFILES_GROUP_PREFIX));
        profile->dir = build_profiles_string (keyfile, *g, "dir");
        system = build_profiles_string (keyfile, *g, "system");
        profile->system = build_system_of_name (system);
        g_free (system);
        profile->from = build_profiles_string (keyfile, *g, "from");
        profile->setup = build_profiles_string (keyfile, *g, "setup");
        profile->build = build_profiles_string (keyfile, *g, "build");
        profile->test = build_profiles_string (keyfile, *g, "test");
        profile->test_after_build = g_key_file_get_boolean (keyfile, *g, "test_after_build", NULL);
        g_ptr_array_add (profiles->list, profile);
    }
    g_strfreev (groups);
    profiles->active = build_profiles_string (keyfile, BUILD_PROFILES_GROUP, "profile");
    profiles->coverage = build_profiles_string (keyfile, BUILD_PROFILES_GROUP, "coverage_profile");
    // a name of no profile names none
    if (profiles->active != NULL && build_profiles_find (profiles, profiles->active) == NULL)
        g_clear_pointer (&profiles->active, g_free);
    if (profiles->coverage != NULL && build_profiles_find (profiles, profiles->coverage) == NULL)
        g_clear_pointer (&profiles->coverage, g_free);
    return profiles;
}

/* --------------------------------------------------------------------------------------------- */

void
build_profiles_store (const build_profiles_t *profiles, GKeyFile *keyfile)
{
    char **groups, **g;
    guint i;

    groups = g_key_file_get_groups (keyfile, NULL);
    for (g = groups; *g != NULL; g++)
        if (g_str_has_prefix (*g, BUILD_PROFILES_GROUP_PREFIX))
            (void) g_key_file_remove_group (keyfile, *g, NULL);
    g_strfreev (groups);

    build_profiles_set (keyfile, BUILD_PROFILES_GROUP, "profile", profiles->active);
    build_profiles_set (keyfile, BUILD_PROFILES_GROUP, "coverage_profile", profiles->coverage);
    for (i = 0; i < profiles->list->len; i++)
    {
        const build_profile_t *p = (const build_profile_t *) g_ptr_array_index (profiles->list, i);
        char *group = g_strconcat (BUILD_PROFILES_GROUP_PREFIX, p->name, (char *) NULL);

        build_profiles_set (keyfile, group, "dir", p->dir);
        g_key_file_set_string (keyfile, group, "system", build_system_name (p->system));
        build_profiles_set (keyfile, group, "from", p->from);
        build_profiles_set (keyfile, group, "setup", p->setup);
        build_profiles_set (keyfile, group, "build", p->build);
        build_profiles_set (keyfile, group, "test", p->test);
        if (p->test_after_build)
            g_key_file_set_boolean (keyfile, group, "test_after_build", TRUE);
        g_free (group);
    }
}

/* --------------------------------------------------------------------------------------------- */

void
build_profiles_free (build_profiles_t *profiles)
{
    if (profiles == NULL)
        return;
    g_ptr_array_free (profiles->list, TRUE);
    g_free (profiles->active);
    g_free (profiles->coverage);
    g_free (profiles);
}

/* --------------------------------------------------------------------------------------------- */

build_profiles_t *
build_profiles_default (const char *root, const build_info_t *found, const char *command)
{
    build_profiles_t *profiles = build_profiles_new ();
    build_profile_t *profile = g_new0 (build_profile_t, 1);

    profile->name = g_strdup (BUILD_PROFILES_DEFAULT);
    profile->dir = g_strdup (found != NULL && found->build_dir != NULL ? found->build_dir : root);
    profile->system = found != NULL ? found->system : BUILD_NONE;
    profile->build = command != NULL && *command != '\0' ? g_strdup (command) : NULL;
    g_ptr_array_add (profiles->list, profile);
    return profiles;
}

/* --------------------------------------------------------------------------------------------- */

build_profile_t *
build_profiles_find (const build_profiles_t *profiles, const char *name)
{
    guint i;

    if (profiles == NULL || name == NULL)
        return NULL;
    for (i = 0; i < profiles->list->len; i++)
    {
        build_profile_t *p = (build_profile_t *) g_ptr_array_index (profiles->list, i);

        if (strcmp (p->name, name) == 0)
            return p;
    }
    return NULL;
}

/* --------------------------------------------------------------------------------------------- */

build_profile_t *
build_profiles_active (const build_profiles_t *profiles)
{
    build_profile_t *p;

    if (profiles == NULL || profiles->list->len == 0)
        return NULL;
    p = build_profiles_find (profiles, profiles->active);
    return p != NULL ? p : (build_profile_t *) g_ptr_array_index (profiles->list, 0);
}

/* --------------------------------------------------------------------------------------------- */

build_profile_t *
build_profiles_add (build_profiles_t *profiles, const char *root, const char *name,
                    const build_profile_t *from, const char *dir)
{
    build_profile_t *p;

    if (name == NULL || *name == '\0' || strchr (name, ']') != NULL || strchr (name, '[') != NULL
        || build_profiles_find (profiles, name) != NULL)
        return NULL;
    p = g_new0 (build_profile_t, 1);
    p->name = g_strdup (name);
    p->dir = g_strdup (dir != NULL ? dir : from != NULL ? from->dir : NULL);
    if (from != NULL)
    {
        // its directory, as it is given and absolute, is the new one in the commands
        char *from_abs = build_profile_dir (from, root);
        char *new_abs = build_profile_dir (p, root);
        const char *olds[] = { from->dir != NULL ? from->dir : from_abs, from_abs, NULL };
        const char *news[] = { p->dir != NULL ? p->dir : new_abs, new_abs, NULL };

        p->system = from->system;
        p->from = g_strdup (from->name);
        p->setup = g_strdup (from->setup);
        p->build = build_profiles_replace (from->build, olds, news);
        p->test = build_profiles_replace (from->test, olds, news);
        g_free (new_abs);
        g_free (from_abs);
        p->test_after_build = from->test_after_build;
    }
    g_ptr_array_add (profiles->list, p);
    return p;
}

/* --------------------------------------------------------------------------------------------- */

gboolean
build_profiles_remove (build_profiles_t *profiles, const char *name)
{
    build_profile_t *p = build_profiles_find (profiles, name);

    if (p == NULL)
        return FALSE;
    if (g_strcmp0 (profiles->active, name) == 0)
        g_clear_pointer (&profiles->active, g_free);
    if (g_strcmp0 (profiles->coverage, name) == 0)
        g_clear_pointer (&profiles->coverage, g_free);
    return g_ptr_array_remove (profiles->list, p);
}

/* --------------------------------------------------------------------------------------------- */

char *
build_profile_dir (const build_profile_t *profile, const char *root)
{
    if (profile->dir == NULL)
        return g_strdup (root);
    return g_canonicalize_filename (profile->dir, root);
}

/* --------------------------------------------------------------------------------------------- */

char *
build_profile_command (const build_profile_t *profile, const char *root)
{
    char *build = NULL, *command;

    if (profile->build != NULL)
        build = g_strdup (profile->build);
    else
    {
        char *dir = build_profile_dir (profile, root);
        char *quoted = g_shell_quote (dir);

        switch (profile->system)
        {
        case BUILD_MESON:
            build = g_strdup_printf ("meson compile -C %s", quoted);
            break;
        case BUILD_CMAKE:
            build = g_strdup_printf ("cmake --build %s", quoted);
            break;
        case BUILD_MAKE:
            build = g_strdup ("make");
            break;
        default:
            break;
        }
        g_free (quoted);
        g_free (dir);
    }
    if (build == NULL || !profile->test_after_build || profile->test == NULL)
        return build;
    command = g_strconcat (build, " && ", profile->test, (char *) NULL);
    g_free (build);
    return command;
}

/* --------------------------------------------------------------------------------------------- */

char *
build_profile_test_command (build_system_t system, const char *dir)
{
    char *quoted = g_shell_quote (dir);
    char *command;

    switch (system)
    {
    case BUILD_MESON:
        command = g_strdup_printf ("meson test -C %s", quoted);
        break;
    case BUILD_CMAKE:
        command = g_strdup_printf ("ctest --test-dir %s", quoted);
        break;
    case BUILD_MAKE:
        command = g_strdup ("make check");
        break;
    default:
        command = NULL;
        break;
    }
    g_free (quoted);
    return command;
}

/* --------------------------------------------------------------------------------------------- */
