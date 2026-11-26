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

/* The absolute @dir as it is named from the absolute @root, whatever the way: "../../opt/b" */
static char *
build_profiles_from_root (const char *root, const char *dir)
{
    char **r = g_strsplit (root, "/", -1);
    char **d = g_strsplit (dir, "/", -1);
    GString *out = g_string_new (NULL);
    guint i = 0, j;

    while (r[i] != NULL && d[i] != NULL && strcmp (r[i], d[i]) == 0)
        i++;
    for (j = i; r[j] != NULL; j++)
        if (*r[j] != '\0')
            g_string_append (out, out->len > 0 ? "/.." : "..");
    for (j = i; d[j] != NULL; j++)
        if (*d[j] != '\0')
        {
            if (out->len > 0)
                g_string_append_c (out, '/');
            g_string_append (out, d[j]);
        }
    if (out->len == 0)
        g_string_append_c (out, '.');
    g_strfreev (r);
    g_strfreev (d);
    return g_string_free (out, FALSE);
}

/* --------------------------------------------------------------------------------------------- */

/* Whether @p of @text follows -C or -B glued to it, the option a word of its own: "-C../build" */
static gboolean
build_profiles_glued (const char *text, const char *p)
{
    return p - text >= 2 && p[-2] == '-' && (p[-1] == 'C' || p[-1] == 'B')
        && (p - 2 == text || p[-3] == ' ');
}

/* --------------------------------------------------------------------------------------------- */

/* Whether a word at @p is @dir, or a path in it: "dir", "dir/x", "./dir" */
static gboolean
build_profiles_word_in (const char *p, const char *dir)
{
    const gsize len = dir != NULL ? strlen (dir) : 0;

    // "./build" is "build"
    if (len > 0 && dir[0] != '.' && dir[0] != '/' && p[0] == '.' && p[1] == '/')
        p += 2;

    return len > 0 && strncmp (p, dir, len) == 0
        && (p[len] == '\0' || strchr (" '\"/;&|)", p[len]) != NULL);
}

/* --------------------------------------------------------------------------------------------- */

/* Whether @text names the directory @old_rel or @old_abs as a word, or a path in it, "dir/x",
   but for the new one, @new_rel or @new_abs, which may be in it; @old_rel NULL: only the
   absolute one */
static gboolean
build_profiles_names_dir (const char *text, const char *old_rel, const char *old_abs,
                          const char *new_rel, const char *new_abs)
{
    const char *const olds[] = { old_abs, old_rel };
    const char *p;
    gsize i;

    for (p = text; text != NULL && *p != '\0'; p++)
    {
        if (p != text && strchr (" '\"=", p[-1]) == NULL && !build_profiles_glued (text, p))
            continue;
        if (build_profiles_word_in (p, new_abs) || build_profiles_word_in (p, new_rel))
            continue;
        for (i = 0; i < G_N_ELEMENTS (olds); i++)
        {
            const gsize len = olds[i] != NULL ? strlen (olds[i]) : 0;

            if (len > 0 && strncmp (p, olds[i], len) == 0
                && (p[len] == '\0' || strchr (" '\"/;&|)", p[len]) != NULL))
                return TRUE;
        }
    }
    return FALSE;
}

/* --------------------------------------------------------------------------------------------- */

/* Whether @command only builds the directory @dir, absolute, of @root as its system does:
   "meson compile -C DIR", "cmake --build DIR", "ninja -C DIR", the directory named in any way;
   a profile made from one with it builds its own directory as its system does, set up first */
static gboolean
build_profiles_plain_command (const char *command, const char *root, const char *dir)
{
    char **argv = NULL;
    const char *named = NULL;
    int argc = 0;
    gboolean plain = FALSE;

    if (command == NULL || !g_shell_parse_argv (command, &argc, &argv, NULL))
        return FALSE;
    if (argc == 4 && strcmp (argv[0], "meson") == 0 && strcmp (argv[1], "compile") == 0
        && strcmp (argv[2], "-C") == 0)
        named = argv[3];
    else if (argc == 3 && strcmp (argv[0], "cmake") == 0 && strcmp (argv[1], "--build") == 0)
        named = argv[2];
    else if (argc == 3 && strcmp (argv[0], "ninja") == 0 && strcmp (argv[1], "-C") == 0)
        named = argv[2];
    if (named != NULL)
    {
        char *a = g_canonicalize_filename (named, root);
        char *b = g_canonicalize_filename (dir, root);

        plain = strcmp (a, b) == 0;
        g_free (a);
        g_free (b);
    }
    g_strfreev (argv);
    return plain;
}

/* --------------------------------------------------------------------------------------------- */

/* Whether the word at @p of @text follows an option that takes a directory: -C, -B, --build,
   --test-dir, or cd, given as a word of its own, or "=" */
static gboolean
build_profiles_after_dir_option (const char *text, const char *p)
{
    static const char *const options[] = { "-C", "-B", "--build", "--test-dir", "cd", NULL };
    const char *end = p;
    int i;

    if (end > text && (end[-1] == '\'' || end[-1] == '"'))
        end--;
    if (end > text && end[-1] == '=')
        return TRUE;
    // glued to its option: -C../build
    if (build_profiles_glued (text, end))
        return TRUE;
    if (end == text || end[-1] != ' ')
        return FALSE;
    while (end > text && end[-1] == ' ')
        end--;
    for (i = 0; options[i] != NULL; i++)
    {
        const gsize len = strlen (options[i]);

        if ((gsize) (end - text) >= len && strncmp (end - len, options[i], len) == 0
            && (end - len == text || end[-len - 1] == ' '))
            return TRUE;
    }
    return FALSE;
}

/* --------------------------------------------------------------------------------------------- */

/* @text with the directory @old_abs, absolute, made @new_abs where it is a whole word or the start
   of a path, and @old_rel, as the profile names it, made @new_rel only where it is surely the
   directory: after an option that takes one or at the start of a path, "dir/x"; so the word
   "build" of "make build" stays.  @old_rel NULL: only the absolute one. */
static char *
build_profiles_replace (const char *text, const char *old_rel, const char *old_abs,
                        const char *new_rel, const char *new_abs)
{
    GString *out;
    const char *p;
    const gsize rel_len = old_rel != NULL ? strlen (old_rel) : 0;
    const gsize abs_len = strlen (old_abs);

    if (text == NULL)
        return NULL;
    out = g_string_new (NULL);
    for (p = text; *p != '\0';)
    {
        const gboolean starts =
            p == text || strchr (" '\"=", p[-1]) != NULL || build_profiles_glued (text, p);
        // "./build" is "build"
        const gsize dot =
            starts && rel_len > 0 && old_rel[0] != '.' && p[0] == '.' && p[1] == '/' ? 2 : 0;
        const char *with = NULL;
        gsize len = 0;

        if (starts && strncmp (p, old_abs, abs_len) == 0
            && (p[abs_len] == '\0' || strchr (" '\"/;&|)", p[abs_len]) != NULL))
        {
            with = new_abs;
            len = abs_len;
        }
        else if (starts && rel_len > 0 && strncmp (p + dot, old_rel, rel_len) == 0
                 && (p[dot + rel_len] == '/'
                     || ((p[dot + rel_len] == '\0' || strchr (" '\";&|)", p[dot + rel_len]) != NULL)
                         && build_profiles_after_dir_option (text, p))))
        {
            with = new_rel;
            len = dot + rel_len;
        }
        if (with == NULL)
            g_string_append_c (out, *p++);
        else
        {
            // a word of its own, not in quotes, is quoted when the new directory needs it
            const gboolean quoted = p != text && (p[-1] == '\'' || p[-1] == '"');

            if (!quoted && with[strcspn (with, " \t'\"\\$`&|;()<>*?[]#~")] != '\0')
            {
                char *q = g_shell_quote (with);

                g_string_append (out, q);
                g_free (q);
            }
            else
                g_string_append (out, with);
            p += len;
        }
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

char *
build_profile_relative (const char *root, const char *dir)
{
    char *parent;
    char *rel;

    if (strcmp (dir, root) == 0)
        return g_strdup (".");
    if (g_str_has_prefix (dir, root) && dir[strlen (root)] == '/')
        return g_strdup (dir + strlen (root) + 1);
    parent = g_path_get_dirname (root);
    if (strcmp (parent, "/") != 0 && g_str_has_prefix (dir, parent) && dir[strlen (parent)] == '/')
        rel = g_strconcat ("..", dir + strlen (parent), (char *) NULL);
    else
        rel = g_strdup (dir);
    g_free (parent);
    return rel;
}

/* --------------------------------------------------------------------------------------------- */

build_profiles_t *
build_profiles_default (const char *root, const build_info_t *found, const char *command)
{
    build_profiles_t *profiles = build_profiles_new ();
    build_profile_t *profile = g_new0 (build_profile_t, 1);

    profile->name = g_strdup (BUILD_PROFILES_DEFAULT);
    profile->dir = build_profile_relative (
        root, found != NULL && found->build_dir != NULL ? found->build_dir : root);
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
        /* its directory, as it is given and absolute, is the new one in the commands; none of
           a profile that builds in the root, whose commands name no build directory */
        char *from_abs = build_profile_dir (from, root);
        char *new_abs = build_profile_dir (p, root);
        const gboolean in_root = strcmp (from_abs, root) == 0;
        // an absolute directory may be named from the root too, "../../opt/b/proj"
        char *old_rel = from->dir != NULL && !g_path_is_absolute (from->dir)
            ? g_strdup (from->dir)
            : build_profiles_from_root (root, from_abs);
        const char *new_rel = p->dir != NULL ? p->dir : new_abs;

        build_profile_t plain = *from;
        char *usual;

        p->system = from->system;
        p->from = g_strdup (from->name);
        p->setup = g_strdup (from->setup);
        // the command its system has for it is none: that of the new one sets its directory up
        plain.build = NULL;
        usual = build_profile_build_command (&plain, root);
        if (from->build != NULL && g_strcmp0 (from->build, usual) != 0
            && !build_profiles_plain_command (from->build, root, from_abs))
            p->build = in_root
                ? g_strdup (from->build)
                : build_profiles_replace (from->build, old_rel, from_abs, new_rel, new_abs);
        p->test = in_root || from->test == NULL
            ? g_strdup (from->test)
            : build_profiles_replace (from->test, old_rel, from_abs, new_rel, new_abs);
        /* a command that names the directory of @from still, where it could not be told for
           sure that it is the directory: none of it, the new profile builds its own directory
           as its system does, its tests as their command of the system */
        if (!in_root && build_profiles_names_dir (p->build, old_rel, from_abs, new_rel, new_abs))
            g_clear_pointer (&p->build, g_free);
        if (!in_root && build_profiles_names_dir (p->test, old_rel, from_abs, new_rel, new_abs))
        {
            g_free (p->test);
            p->test = build_profile_test_command (p->system, new_abs);
        }
        g_free (usual);
        g_free (old_rel);
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

/* The command that sets the directory @dir of @profile up with its options, "meson setup DIR
   -Dx=1", "cmake -S ROOT -B DIR -Dx=1"; NULL for a system without one */
static char *
build_profile_setup_command (const build_profile_t *profile, const char *root, const char *dir)
{
    char *quoted = g_shell_quote (dir);
    const char *setup = profile->setup != NULL ? profile->setup : "";
    const char *space = profile->setup != NULL ? " " : "";
    char *command = NULL;

    if (profile->system == BUILD_MESON)
        command = g_strdup_printf ("meson setup %s%s%s", quoted, space, setup);
    else if (profile->system == BUILD_CMAKE)
    {
        char *source = g_shell_quote (root);

        command = g_strdup_printf ("cmake -S %s -B %s%s%s", source, quoted, space, setup);
        g_free (source);
    }
    g_free (quoted);
    return command;
}

/* --------------------------------------------------------------------------------------------- */

char *
build_profile_build_command (const build_profile_t *profile, const char *root)
{
    char *dir = build_profile_dir (profile, root);
    const gboolean set_up = build_system_of_dir (dir) == profile->system;
    char *setup = set_up ? NULL : build_profile_setup_command (profile, root, dir);
    char *build = NULL;

    if (profile->build != NULL)
    {
        /* a command of its own that builds the directory, "meson compile -C DIR -j 4", the
           directory not set up yet: set up first, as the command of its system would */
        char *rel = profile->dir != NULL && !g_path_is_absolute (profile->dir)
            ? g_strdup (profile->dir)
            : build_profiles_from_root (root, dir);
        // names it as a word or a path in it, not as a part of another word
        const gboolean names = build_profiles_names_dir (profile->build, rel, dir, NULL, NULL);

        g_free (rel);
        if (setup != NULL && names && strstr (profile->build, "meson setup") == NULL
            && strstr (profile->build, "cmake -S") == NULL
            && strstr (profile->build, "cmake -B") == NULL)
            build = g_strdup_printf ("%s && %s", setup, profile->build);
        else
            build = g_strdup (profile->build);
    }
    else
    {
        char *quoted = g_shell_quote (dir);

        // a directory not set up yet, that of a new profile: set up first
        switch (profile->system)
        {
        case BUILD_MESON:
            build = setup == NULL ? g_strdup_printf ("meson compile -C %s", quoted)
                                  : g_strdup_printf ("%s && meson compile -C %s", setup, quoted);
            break;
        case BUILD_CMAKE:
            build = setup == NULL ? g_strdup_printf ("cmake --build %s", quoted)
                                  : g_strdup_printf ("%s && cmake --build %s", setup, quoted);
            break;
        case BUILD_MAKE:
            build = g_strdup ("make");
            break;
        default:
            break;
        }
        g_free (quoted);
    }
    g_free (setup);
    g_free (dir);
    return build;
}

/* --------------------------------------------------------------------------------------------- */

char *
build_profile_command (const build_profile_t *profile, const char *root)
{
    char *build = build_profile_build_command (profile, root);
    char *command;

    if (build == NULL || !profile->test_after_build || profile->test == NULL)
        return build;
    /* a command of the user's own in a shell of its own, which ends on a new line: its cd does not
       move the tests, a comment at its end does not take them in */
    if (profile->build != NULL)
        command = g_strconcat ("(", build, "\n) && ", profile->test, (char *) NULL);
    else
        command = g_strconcat (build, " && ", profile->test, (char *) NULL);
    g_free (build);
    return command;
}

/* --------------------------------------------------------------------------------------------- */

build_system_t
build_system_of_dir (const char *dir)
{
    char *path;
    build_system_t system = BUILD_NONE;

    if (dir == NULL)
        return BUILD_NONE;
    path = g_build_filename (dir, "meson-info", "meson-info.json", (char *) NULL);
    if (g_file_test (path, G_FILE_TEST_IS_REGULAR))
        system = BUILD_MESON;
    g_free (path);
    if (system == BUILD_NONE)
    {
        path = g_build_filename (dir, "CMakeCache.txt", (char *) NULL);
        if (g_file_test (path, G_FILE_TEST_IS_REGULAR))
            system = BUILD_CMAKE;
        g_free (path);
    }
    return system;
}

/* --------------------------------------------------------------------------------------------- */

char *
build_profile_setup_of (const char *dir, build_system_t system)
{
    GPtrArray *options = build_options_read (dir, system);
    GString *setup;
    guint i;

    if (options == NULL)
        return NULL;
    setup = g_string_new (NULL);
    for (i = 0; i < options->len; i++)
    {
        const build_option_t *o = (const build_option_t *) g_ptr_array_index (options, i);
        char *arg, *quoted;

        if (!o->set_by_user)
            continue;
        arg = g_strdup_printf ("-D%s=%s", o->name, o->value);
        quoted = g_shell_quote (arg);
        // a word that needs no quotes goes as it is
        if (strspn (arg, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_=.,/+:")
            == strlen (arg))
            g_string_append_printf (setup, "%s%s", setup->len > 0 ? " " : "", arg);
        else
            g_string_append_printf (setup, "%s%s", setup->len > 0 ? " " : "", quoted);
        g_free (quoted);
        g_free (arg);
    }
    g_ptr_array_free (options, TRUE);
    if (setup->len == 0)
    {
        g_string_free (setup, TRUE);
        return NULL;
    }
    return g_string_free (setup, FALSE);
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
