/*
   The build plugin: builds the project, shows what the compiler says, and goes to it.

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

/** \file build.c
 *  \brief Source: the build plugin
 */

#include <config.h>

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "lib/global.h"
#include "lib/keybind.h"
#include "lib/util.h"
#include "lib/widget.h"
#include "lib/tty/key.h"
#include "lib/tty/tty.h"
#include "lib/plugin-service.h"

#include "src/keymap.h"            // keymap_menu_shortcut ()
#include "src/editor/edit-impl.h"  // edit_widget_is_editor ()

#include "build-core.h"
#include "build-presets.h"
#include "build-profiles.h"
#include "build.h"

/*** global variables ****************************************************************************/

/*** file scope macro definitions ****************************************************************/

#define BUILD_SERVICE        "build"
#define BUILD_KEYMAP_SECTION "build"
#define BUILD_OUTPUT_MAX     (512 * 1024)
// the columns a tab goes to in the column the compiler names, as gcc counts them
#define BUILD_TAB_WIDTH 8

/*** file scope type declarations ****************************************************************/

enum
{
    BUILD_CMD_NONE = -1,
    BUILD_CMD_RUN,
    BUILD_CMD_NEXT,
    BUILD_CMD_PREV,
    BUILD_CMD_SHOW,
    BUILD_CMD_COUNT
};

enum
{
    BUILD_ACT_RUN,
    BUILD_ACT_NEXT,
    BUILD_ACT_PREV,
    BUILD_ACT_CONFIGURE,
    BUILD_ACT_PROFILES,
    BUILD_ACT_OUTPUT,
    BUILD_ACT_DEFAULTS
};

enum
{
    BUILD_MARK_ERROR,
    BUILD_MARK_WARNING,
    BUILD_MARK_COUNT
};

typedef struct
{
    mc_editor_host_t *host;
    char *root;     // of the build going on, or of the last one
    char *command;  // that it runs
    char *profile;  // the profile it builds, NULL when it has none
    char *dir;      // where the compiler runs: the names it gives are from there
    GPid pid;
    int output_fd;
    GString *output;
    char *title;       /* of the window of the output: "Build: ../coole-build, meson, debug" */
    GString *pending;  // a line not ended yet
    GString *partial;  // the bytes of a character a read has cut, for the next read
    gint64 window;
    // asked for by another plugin, before a start: the window goes when the build is fine
    gboolean quiet;
    GPtrArray *diagnostics;  // build_diagnostic_t
    GPtrArray *notes;        // GString, the notes of the compiler on each diagnostic
    // the window a list of the diagnostics once the build is over, the output under it
    gboolean listed;
    GString *list;  // its text
    GArray *rows;   // int, for each line of it: the diagnostic, or -1
    int current;
    // the note of the error gone to when the list was not in sight, after its line
    char *note_file;
    long note_line;
    long commands[BUILD_CMD_COUNT];
    int marks[BUILD_MARK_COUNT];
    gboolean service;
    char *presets_told;  // what was wrong in the presets of the user, said once
    guint project_signal;
    guint viewer_signal;
    gboolean configuring;             // the job sets the directory up, it does not build it
    gboolean with_tests;              // the build asked for runs the tests after it
    char *changes;                    // the options it gives the directory, "-Dx=1"
    struct build_options_view *view;  // the window of the options open, NULL when none
    char *defaults_root;              // the project of the defaults below
    char *defaults;                   // the options of its meson.build, as meson prints them
    gint64 defaults_time;             // build_meson_time () when they were read
} build_t;

/*** forward declarations (file scope functions) *************************************************/

static void build_list_cursor (build_t *build);
static void build_options_view_done (struct build_options_view *view, gboolean ok);
static void build_options_view_redraw (struct build_options_view *view);
static void build_options_view_wait (struct build_options_view *view);
static gboolean build_profile_window (build_t *build, const char *root, const char *name,
                                      gboolean is_new, char **saved_as);
static void build_note_clear (build_t *build);
static mc_ep_window_state_t build_output_state (void *data);

/*** file scope variables ************************************************************************/

static const mc_ep_command_t build_commands[BUILD_CMD_COUNT + 1] = {
    { "BuildRun", N_ ("Build the project"), "ctrl-f9; alt-shift-b" },
    { "BuildNextError", N_ ("Go to the next error of the build"), "alt-shift-j" },
    { "BuildPrevError", N_ ("Go to the previous error of the build"), "alt-shift-k" },
    { "BuildShowError", N_ ("Go to the error of the line of the list of the build"), "enter" },
    { NULL, NULL, NULL },
};

/* The marks of the gutter: their characters come from the skin, section [widget-editor]; those
   here, ASCII, when it has none or the terminal cannot show its own */
static const mc_ep_marker_kind_t build_mark_kinds[BUILD_MARK_COUNT] = {
    { "build.error", "build-error-char", "E", "E", "builderror", NULL, NULL, 6, "brightred" },
    { "build.warning", "build-warning-char", "!", "!", "buildwarning", NULL, NULL, 5, "yellow" },
};

/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

static void
build_note_free_cb (gpointer p)
{
    g_string_free ((GString *) p, TRUE);
}

/* --------------------------------------------------------------------------------------------- */

static void
build_diagnostic_free_cb (gpointer p)
{
    build_diagnostic_free ((build_diagnostic_t *) p);
}

/* --------------------------------------------------------------------------------------------- */

/* The project of the file in front, as the project plugin sees it, else its directory */
static char *
build_root_of (build_t *build, void *edit)
{
    char *file = edit != NULL ? build->host->get_current_file (build->host, edit) : NULL;
    GVariantDict args;
    GVariant *reply;
    char *root = NULL;

    g_variant_dict_init (&args, NULL);
    if (file != NULL)
        g_variant_dict_insert (&args, "file", "s", file);
    reply = build->host->service_call (build->host, "project", "root", g_variant_dict_end (&args),
                                       NULL);
    if (reply != NULL)
    {
        (void) g_variant_lookup (reply, "root", "s", &root);
        g_variant_unref (reply);
    }
    if (root == NULL)
        root = file != NULL ? g_path_get_dirname (file) : g_get_current_dir ();
    g_free (file);
    return root;
}

/* --------------------------------------------------------------------------------------------- */

static char *
build_config_path (const char *root)
{
    char *hash = g_compute_checksum_for_string (G_CHECKSUM_SHA256, root, -1);
    char *name = g_strconcat (hash, ".ini", (char *) NULL);
    char *path =
        g_build_filename (g_get_user_config_dir (), "bluerig", "build", name, (char *) NULL);

    g_free (name);
    g_free (hash);
    return path;
}

/* --------------------------------------------------------------------------------------------- */

/* The file of the project, .bluerig/project.ini */
static char *
build_project_path (const char *root)
{
    return g_build_filename (root, BUILD_PROJECT_DIR, BUILD_PROJECT_FILE, (char *) NULL);
}

/* --------------------------------------------------------------------------------------------- */

/* The command bluerig kept for the project before it had .bluerig/project.ini, in the configuration
   of the user; NULL when none */
static char *
build_legacy_command (const char *root)
{
    GKeyFile *keyfile = g_key_file_new ();
    char *path = build_config_path (root);
    char *command = NULL;

    if (g_key_file_load_from_file (keyfile, path, G_KEY_FILE_NONE, NULL))
        command = g_key_file_get_string (keyfile, "Build", "command", NULL);
    g_key_file_free (keyfile);
    g_free (path);
    if (command != NULL && *g_strstrip (command) == '\0')
        g_clear_pointer (&command, g_free);
    return command;
}

/* --------------------------------------------------------------------------------------------- */

/* The command the user gave the project, NULL when none: in the project; without the file of the
   project, where bluerig kept it before */
static char *
build_saved_command (const char *root)
{
    GKeyFile *keyfile = g_key_file_new ();
    char *path = build_project_path (root);
    char *command = NULL;

    if (!g_file_test (path, G_FILE_TEST_EXISTS))
        command = build_legacy_command (root);
    else if (g_key_file_load_from_file (keyfile, path, G_KEY_FILE_KEEP_COMMENTS, NULL))
        command = g_key_file_get_string (keyfile, BUILD_PROJECT_GROUP, "command", NULL);
    g_key_file_free (keyfile);
    g_free (path);
    if (command != NULL && *g_strstrip (command) == '\0')
        g_clear_pointer (&command, g_free);
    return command;
}

/* --------------------------------------------------------------------------------------------- */

/* A mistake said once: told again when it is another, or when it was put right before */
static void
build_tell_once (build_t *build, const char *text_value)
{
    if (g_strcmp0 (text_value, build->presets_told) == 0)
        return;
    g_free (build->presets_told);
    build->presets_told = g_strdup (text_value);
    build->host->message (build->host, D_ERROR, _ ("Build"), text_value);
}

/* --------------------------------------------------------------------------------------------- */

/* .bluerig/project.ini of @root into @keyfile, @there whether it is there; FALSE, the user told,
   when it is there but cannot be read: what the user wrote in it is not written over */
static gboolean
build_project_read (build_t *build, const char *root, GKeyFile *keyfile, gboolean *there)
{
    char *path = build_project_path (root);
    GError *error = NULL;
    gboolean ok = g_key_file_load_from_file (keyfile, path, G_KEY_FILE_KEEP_COMMENTS, &error);

    *there = ok;
    if (!ok && g_error_matches (error, G_FILE_ERROR, G_FILE_ERROR_NOENT))
        ok = TRUE;
    else if (!ok)
    {
        // as the presets tell it, once for both
        char *text_value = g_strdup_printf ("%s:\n%s", path, error->message);

        build_tell_once (build, text_value);
        g_free (text_value);
    }
    g_clear_error (&error);
    g_free (path);
    return ok;
}

/* --------------------------------------------------------------------------------------------- */

/* .bluerig/project.ini written as @keyfile has it; FALSE, the user told, when it cannot be */
static gboolean
build_project_write (build_t *build, const char *root, GKeyFile *keyfile)
{
    char *dir = g_build_filename (root, BUILD_PROJECT_DIR, (char *) NULL);
    char *path = build_project_path (root);
    char *contents = g_key_file_to_data (keyfile, NULL, NULL);
    GError *error = NULL;
    gboolean done;

    done =
        g_mkdir_with_parents (dir, 0755) == 0 && g_file_set_contents (path, contents, -1, &error);
    if (!done)
    {
        char *text_value =
            g_strdup_printf ("%s:\n%s", path, error != NULL ? error->message : g_strerror (errno));

        build->host->message (build->host, D_ERROR, _ ("Build"), text_value);
        g_free (text_value);
        g_clear_error (&error);
    }
    g_free (contents);
    g_free (path);
    g_free (dir);
    return done;
}

/* --------------------------------------------------------------------------------------------- */

/* The profiles of the project of @root, in the file of the project in the configuration of the
   user; a project without them has Default, the build found in it built by the command the
   user gave it before, until a profile is changed and they are kept */
static build_profiles_t *
build_profiles_of (const char *root)
{
    GKeyFile *keyfile = g_key_file_new ();
    char *path = build_config_path (root);
    build_profiles_t *profiles;

    if (!g_key_file_load_from_file (keyfile, path, G_KEY_FILE_KEEP_COMMENTS, NULL))
        g_clear_pointer (&keyfile, g_key_file_free);
    profiles = build_profiles_load (keyfile);
    if (keyfile != NULL)
        g_key_file_free (keyfile);
    g_free (path);
    if (profiles->list->len == 0)
    {
        build_info_t *found = build_detect (root);
        char *command = build_saved_command (root);

        build_profiles_free (profiles);
        profiles = build_profiles_default (root, found, command);
        g_free (command);
        build_info_free (found);
    }
    return profiles;
}

/* --------------------------------------------------------------------------------------------- */

/* The profiles of the project of @root kept, the rest of its file as it was */
static gboolean
build_profiles_save (build_t *build, const char *root, const build_profiles_t *profiles)
{
    GKeyFile *keyfile = g_key_file_new ();
    char *path = build_config_path (root);
    char *dir = g_path_get_dirname (path);
    GError *error = NULL;
    gboolean done;

    (void) g_key_file_load_from_file (keyfile, path, G_KEY_FILE_KEEP_COMMENTS, NULL);
    build_profiles_store (profiles, keyfile);
    (void) g_mkdir_with_parents (dir, 0700);
    done = g_key_file_save_to_file (keyfile, path, &error);
    if (!done)
    {
        build->host->message (build->host, D_ERROR, _ ("Build"), error->message);
        g_error_free (error);
    }
    g_key_file_free (keyfile);
    g_free (dir);
    g_free (path);
    return done;
}

/* --------------------------------------------------------------------------------------------- */

/* The profile Default made from the build found and the command= of before kept in the file of
   the user, when it has no profiles yet */
static void
build_profiles_keep_default (build_t *build, const char *root)
{
    GKeyFile *keyfile = g_key_file_new ();
    char *path = build_config_path (root);
    build_profiles_t *stored;
    gboolean none;

    if (!g_key_file_load_from_file (keyfile, path, G_KEY_FILE_KEEP_COMMENTS, NULL))
        g_clear_pointer (&keyfile, g_key_file_free);
    stored = build_profiles_load (keyfile);
    none = stored->list->len == 0;
    build_profiles_free (stored);
    if (keyfile != NULL)
        g_key_file_free (keyfile);
    g_free (path);
    if (none)
    {
        build_profiles_t *profiles = build_profiles_of (root);

        (void) build_profiles_save (build, root, profiles);
        build_profiles_free (profiles);
    }
}

/* --------------------------------------------------------------------------------------------- */

/* The command the user gives the profile @name of the project, NULL for the active one, kept */
static void
build_save_command (build_t *build, const char *root, const char *name, const char *command)
{
    build_profiles_t *profiles = build_profiles_of (root);
    build_profile_t *p =
        name != NULL ? build_profiles_find (profiles, name) : build_profiles_active (profiles);

    if (p != NULL)
    {
        g_free (p->build);
        p->build = command != NULL && *command != '\0' ? g_strdup (command) : NULL;
        (void) build_profiles_save (build, root, profiles);
    }
    build_profiles_free (profiles);
}

/* --------------------------------------------------------------------------------------------- */

/* What a file of bluerig of @name has: in the directory of its configuration, else in that of
   its data; NULL when there is none.  A build run by meson devenv, maybe not installed, has
   those of its source. */
static char *
build_bluerig_text (const char *name)
{
    const char *dirs[] = { g_getenv ("BLUERIG_MISC_DIR"), mc_global.sysconfig_dir,
                           mc_global.share_data_dir };
    char *text = NULL;
    guint i;

    for (i = 0; i < G_N_ELEMENTS (dirs) && text == NULL; i++)
        if (dirs[i] != NULL)
        {
            char *path = g_build_filename (dirs[i], name, (char *) NULL);

            if (!g_file_get_contents (path, &text, NULL, NULL))
                text = NULL;
            g_free (path);
        }
    return text;
}

/* --------------------------------------------------------------------------------------------- */

/* The copy of the project of @root, when it has one and @root is not NULL, then the presets of
   the user and of bluerig, read each time: a change to them is there at once.  A mistake in the
   copy or in the presets of the user is said once. */
static GPtrArray *
build_presets_load (build_t *build, const char *root)
{
    char *path =
        g_build_filename (g_get_user_config_dir (), "bluerig", BUILD_PRESETS_FILE, (char *) NULL);
    char *project_path = root != NULL ? build_project_path (root) : NULL;
    char *user = NULL, *project = NULL, *system = build_bluerig_text (BUILD_PRESETS_FILE);
    GError *project_error = NULL, *user_error = NULL;
    GPtrArray *presets;

    if (project_path != NULL && !g_file_get_contents (project_path, &project, NULL, NULL))
        project = NULL;
    if (!g_file_get_contents (path, &user, NULL, NULL))
        user = NULL;
    presets = build_presets_parse (project, user, system, &project_error, &user_error);
    if (project_error == NULL && user_error == NULL)
        g_clear_pointer (&build->presets_told, g_free);
    else
    {
        // a mistake of the copy first
        char *text_value =
            g_strdup_printf ("%s:\n%s", project_error != NULL ? project_path : path,
                             (project_error != NULL ? project_error : user_error)->message);

        build_tell_once (build, text_value);
        g_free (text_value);
        g_clear_error (&project_error);
        g_clear_error (&user_error);
    }
    g_free (user);
    g_free (project);
    g_free (system);
    g_free (project_path);
    g_free (path);
    return presets;
}

/* --------------------------------------------------------------------------------------------- */

/* The file in front when it is in the project of @root, by whose ending a project with no
   marker of a language has one; NULL for none */
static char *
build_front_file (build_t *build, const char *root)
{
    void *edit = build->host->window_top_file (build->host);
    char *file = edit != NULL ? build->host->get_current_file (build->host, edit) : NULL;
    const size_t len = root != NULL ? strlen (root) : 0;

    // a file of another project says nothing of this one; a root of "/" has a file of "/x"
    if (file != NULL
        && (len == 0 || strncmp (file, root, len) != 0
            || (root[len - 1] != '/' && file[len] != '/')))
        g_clear_pointer (&file, g_free);
    return file;
}

/* --------------------------------------------------------------------------------------------- */

/* How the project is built: by its profile @name, NULL for the active one, its command or that of
   its build system, else by that of its language, by its markers or the file in front; the
   directory the compiler runs in */
static gboolean
build_plan (build_t *build, const char *root, const char *name, char **command, char **dir)
{
    build_profiles_t *profiles = build_profiles_of (root);
    const build_profile_t *p =
        name != NULL ? build_profiles_find (profiles, name) : build_profiles_active (profiles);

    *dir = p != NULL ? build_profile_dir (p, root) : g_strdup (root);
    *command = NULL;
    if (p != NULL && build->with_tests)
    {
        // its tests after it whatever the profile says: their command, else that of its system
        build_profile_t tested = *p;
        char *test = p->test == NULL ? build_profile_test_command (p->system, *dir) : NULL;

        if (test != NULL)
            tested.test = test;
        tested.test_after_build = TRUE;
        *command = build_profile_command (&tested, root);
        g_free (test);
    }
    else if (p != NULL)
        *command = build_profile_command (p, root);
    build_profiles_free (profiles);
    if (*command == NULL)
    {
        GPtrArray *presets = build_presets_load (build, root);
        char *file = build_front_file (build, root);
        const build_preset_t *preset = build_presets_find (presets, root, file);

        if (preset != NULL)
            *command = g_strdup (preset->build);
        g_ptr_array_free (presets, TRUE);
        g_free (file);
    }
    return *command != NULL;
}

/* --------------------------------------------------------------------------------------------- */

static GVariant *
build_viewer (build_t *build, const char *method, GVariantDict *dict)
{
    return build->host->service_call (build->host, "viewer", method, g_variant_dict_end (dict),
                                      NULL);
}

/* --------------------------------------------------------------------------------------------- */

/* Where a diagnostic is, as the list shows it: the file from the root of the project, or its
   name alone outside it, and the line */
static char *
build_where (const build_t *build, const build_diagnostic_t *d)
{
    const size_t len = build->root != NULL ? strlen (build->root) : 0;
    const char *file = d->file;

    if (len > 0 && strncmp (file, build->root, len) == 0 && file[len] == PATH_SEP)
        file += len + 1;
    else
        file = x_basename (file);
    return g_strdup_printf ("%s %ld", file, d->line);
}

/* --------------------------------------------------------------------------------------------- */

/* What the build of @p is, in parts: its directory from the root of the project,
   "../coole-build-cov", and what it is built for, "debug, coverage", from its options, or "not
   set up"; to be freed */
static void
build_profile_describe (const build_profile_t *p, const char *root, char **where, char **what)
{
    GPtrArray *options;
    char *dir = build_profile_dir (p, root);
    char *summary = NULL;

    *where = build_profile_relative (root, dir);
    options = build_options_read (dir, p->system);
    if (options != NULL)
    {
        build_summary_t *s = build_summary_of (options, p->system);

        // built for coverage says whether the tests have left their data yet
        s->coverage_missing = s->coverage && !build_coverage_data (dir);
        summary = build_summary_text (s);
        build_summary_free (s);
        g_ptr_array_free (options, TRUE);
    }
    // a directory not set up yet says so
    else if (p->system != BUILD_NONE && build_system_of_dir (dir) != p->system)
        summary = g_strdup (_ ("not set up"));
    *what = summary != NULL ? summary : g_strdup ("");
    g_free (dir);
}

/* --------------------------------------------------------------------------------------------- */

/* What the build of @p is: "../coole-build-cov, meson, debug, coverage" */
static char *
build_profile_brief (const build_profile_t *p, const char *root)
{
    char *where, *what, *brief;

    build_profile_describe (p, root, &where, &what);
    brief = g_strdup_printf ("%s, %s%s%s", where, build_system_name (p->system),
                             *what != '\0' ? ", " : "", what);
    g_free (where);
    g_free (what);
    return brief;
}

/* --------------------------------------------------------------------------------------------- */

/* The title of the window of the output: the profile @name of @root, NULL for the active one,
   and what its build is, "Build Coverage: ../coole-build-cov, meson, debug, coverage", the
   name left out for a lone Default; "Build" when it has no build system.  On the frame it stays
   in sight however far the output goes. */
static char *
build_output_title (const char *root, const char *name)
{
    build_profiles_t *profiles = build_profiles_of (root);
    const build_profile_t *p =
        name != NULL ? build_profiles_find (profiles, name) : build_profiles_active (profiles);
    GString *head;
    char *brief;

    if (p == NULL || p->system == BUILD_NONE)
    {
        build_profiles_free (profiles);
        return g_strdup (_ ("Build"));
    }
    head = g_string_new (_ ("Build"));
    if (profiles->list->len > 1 || strcmp (p->name, BUILD_PROFILES_DEFAULT) != 0)
        g_string_append_printf (head, " %s", p->name);
    brief = build_profile_brief (p, root);
    g_string_append_printf (head, ": %s", brief);
    g_free (brief);
    build_profiles_free (profiles);
    return g_string_free (head, FALSE);
}

/* --------------------------------------------------------------------------------------------- */

static void
build_output_show (build_t *build)
{
    GVariantDict dict;
    GVariant *reply;
    long lines = 0;
    const char *text = build->listed ? build->list->str : build->output->str;
    const char *p;

    for (p = text; *p != '\0'; p++)
        if (*p == '\n')
            lines++;
    g_variant_dict_init (&dict, NULL);
    g_variant_dict_insert (&dict, "text", "s", text);
    g_variant_dict_insert (&dict, "cursorline", "b", build->listed);
    if (build->window != 0)
    {
        g_variant_dict_insert (&dict, "id", "x", build->window);
        reply = build_viewer (build, "set_text", &dict);
        if (reply == NULL)
            build->window = 0;
        else
        {
            g_variant_unref (reply);
            g_variant_dict_init (&dict, NULL);
            g_variant_dict_insert (&dict, "id", "x", build->window);
            g_variant_dict_insert (&dict, "title", "s",
                                   build->title != NULL ? build->title : _ ("Build"));
            reply = build_viewer (build, "set_title", &dict);
            if (reply != NULL)
                g_variant_unref (reply);
        }
    }
    if (build->window == 0)
    {
        g_variant_dict_init (&dict, NULL);
        g_variant_dict_insert (&dict, "text", "s", text);
        g_variant_dict_insert (&dict, "cursorline", "b", build->listed);
        g_variant_dict_insert (&dict, "title", "s",
                               build->title != NULL ? build->title : _ ("Build"));
        g_variant_dict_insert (&dict, "place", "s", "bottom");
        g_variant_dict_insert (&dict, "size", "i", 30);
        g_variant_dict_insert (&dict, "focus", "b", FALSE);
        reply = build_viewer (build, "open", &dict);
        if (reply != NULL)
        {
            (void) g_variant_lookup (reply, "id", "x", &build->window);
            g_variant_unref (reply);
        }
    }
    // the end of the output in sight; a list from its top
    if (build->window != 0)
    {
        g_variant_dict_init (&dict, NULL);
        g_variant_dict_insert (&dict, "id", "x", build->window);
        g_variant_dict_insert (&dict, "line", "x", build->listed ? 0 : (gint64) MAX (1, lines));
        reply = build_viewer (build, "scroll_to", &dict);
        if (reply != NULL)
            g_variant_unref (reply);
    }
    // the cursor of a list, and the view with it, on the error gone to
    build_list_cursor (build);
    // the window of the options stays on top of the output of its setup
    if (build->view != NULL)
        build_options_view_redraw (build->view);
}

/* --------------------------------------------------------------------------------------------- */

static void
build_marks_clear (build_t *build)
{
    int i;

    for (i = 0; i < BUILD_MARK_COUNT; i++)
        build->host->clear_markers (build->host, NULL, build->marks[i]);
}

/* --------------------------------------------------------------------------------------------- */

static void
build_mark (build_t *build, const build_diagnostic_t *d)
{
    build->host->set_marker (build->host, d->file, d->line,
                             build->marks[d->error ? BUILD_MARK_ERROR : BUILD_MARK_WARNING], TRUE);
}

/* --------------------------------------------------------------------------------------------- */

/* A whole line of the compiler */
static void
build_line (build_t *build, const char *line)
{
    build_diagnostic_t *d = build_parse_diagnostic (line, build->dir);

    // a note goes with the error before it
    if (d == NULL || strstr (line, ": note:") != NULL)
    {
        if (d != NULL && build->notes->len > 0)
        {
            GString *notes = g_ptr_array_index (build->notes, build->notes->len - 1);
            char *where = build_where (build, d);

            g_string_append_printf (notes, "%s%s: %s", notes->len > 0 ? "\n" : "", where,
                                    d->message);
            g_free (where);
        }
        build_diagnostic_free (d);
        return;
    }
    build_mark (build, d);
    g_ptr_array_add (build->diagnostics, d);
    g_ptr_array_add (build->notes, g_string_new (NULL));
}

/* --------------------------------------------------------------------------------------------- */

/* The line of the list of a diagnostic, from 0; -1 when there is no list or it is not in it */
static long
build_row_of (const build_t *build, int diagnostic)
{
    guint i;

    for (i = 0; build->listed && i < build->rows->len; i++)
        if (g_array_index (build->rows, int, i) == diagnostic)
            return (long) i;
    return -1;
}

/* --------------------------------------------------------------------------------------------- */

/* A line of the list, with what it is the line of: @diagnostic, or -1 */
static void build_list_add (build_t *build, int diagnostic, const char *format, ...)
    G_GNUC_PRINTF (3, 4);

static void
build_list_add (build_t *build, int diagnostic, const char *format, ...)
{
    va_list args;

    va_start (args, format);
    g_string_append_vprintf (build->list, format, args);
    va_end (args);
    g_string_append_c (build->list, '\n');
    g_array_append_val (build->rows, diagnostic);
}

/* --------------------------------------------------------------------------------------------- */

/* The list of the build over, as the Message window of Turbo C had it: a line of the result,
   then an error or a warning a line, its notes under it, and the output of the compiler last.
   The cursor of the list on the diagnostic gone to last, else the first */
static void
build_list (build_t *build, const char *result)
{
    guint i;

    build->listed = build->diagnostics->len > 0;
    g_string_truncate (build->list, 0);
    g_array_set_size (build->rows, 0);
    if (!build->listed)
        return;
    build_list_add (build, -1, "%s", result);
    for (i = 0; i < build->diagnostics->len; i++)
    {
        const build_diagnostic_t *d = g_ptr_array_index (build->diagnostics, i);
        const GString *notes = g_ptr_array_index (build->notes, i);
        char *where = build_where (build, d);

        // the colors of a terminal, red and yellow: the window shows those of the compiler
        build_list_add (build, (int) i, "%s%s\033[0m %s: %s", d->error ? "\033[1;31m" : "\033[33m",
                        d->error ? _ ("Error") : _ ("Warning"), where, d->message);
        g_free (where);
        if (notes->len > 0)
        {
            char **lines = g_strsplit (notes->str, "\n", -1);
            char **l;

            for (l = lines; *l != NULL; l++)
                build_list_add (build, (int) i, "        %s", *l);
            g_strfreev (lines);
        }
    }
    build_list_add (build, -1, "%s", "");
    build_list_add (build, -1, "%s", _ ("The output of the compiler:"));
    {
        char **lines = g_strsplit (build->output->str, "\n", -1);
        char **l;

        for (l = lines; *l != NULL; l++)
            if (l[1] != NULL || **l != '\0')
                build_list_add (build, -1, "%s", *l);
        g_strfreev (lines);
    }
}

/* --------------------------------------------------------------------------------------------- */

/* The cursor of the list on the diagnostic gone to, the view following it */
static void
build_list_cursor (build_t *build)
{
    const long row = build_row_of (build, build->current >= 0 ? build->current : 0);
    GVariantDict dict;
    GVariant *reply;

    if (row < 0 || build->window == 0)
        return;
    g_variant_dict_init (&dict, NULL);
    g_variant_dict_insert (&dict, "id", "x", build->window);
    g_variant_dict_insert (&dict, "line", "x", (gint64) row);
    reply = build_viewer (build, "move_to", &dict);
    if (reply != NULL)
        g_variant_unref (reply);
}

/* --------------------------------------------------------------------------------------------- */

/* The options the directory of the profile of the setup just done has: what sets it up again,
   kept in the profile; without them, the changes go into the options it had.  FALSE when the
   profile is no more or could not be written: the user is told by the writing */
static gboolean
build_profile_keep_setup (build_t *build)
{
    build_profiles_t *profiles = build_profiles_of (build->root);
    build_profile_t *p = build_profiles_find (profiles, build->profile);
    gboolean kept = FALSE;

    if (p != NULL)
    {
        const build_system_t system = build_system_of_dir (build->dir);
        char *now = system != BUILD_NONE ? build_profile_setup_of (build->dir, system) : NULL;
        /* what it had, what the directory has now over it, the changes over that: an option the
           directory does not tell, a toolchain file of cmake, is kept */
        char *both = build_profile_setup_merge (p->setup, now);
        char *setup = build_profile_setup_merge (both, build->changes);

        g_free (both);
        g_free (now);
        if (system != BUILD_NONE)
            p->system = system;
        g_free (p->setup);
        p->setup = setup;
        kept = build_profiles_save (build, build->root, profiles);
    }
    build_profiles_free (profiles);
    return kept;
}

/* --------------------------------------------------------------------------------------------- */

static void
build_finished (build_t *build, int status)
{
    const gboolean ok = WIFEXITED (status) && WEXITSTATUS (status) == 0;
    gboolean kept = TRUE;
    guint errors = 0, i;
    char *summary;

    for (i = 0; i < build->diagnostics->len; i++)
        if (((build_diagnostic_t *) g_ptr_array_index (build->diagnostics, i))->error)
            errors++;
    if (build->configuring)
        summary = g_strdup (ok ? _ ("\nSet up.\n") : _ ("\nThe setup failed.\n"));
    else if (ok)
        summary =
            g_strdup_printf (_ ("\nBuilt. Warnings: %u.\n"), build->diagnostics->len - errors);
    else
        summary = g_strdup_printf (_ ("\nThe build failed. Errors: %u, warnings: %u.\n"), errors,
                                   build->diagnostics->len - errors);
    // the options the directory has now set it up again: its profile keeps them
    if (build->configuring && ok && build->profile != NULL)
        kept = build_profile_keep_setup (build);
    g_string_append (build->output, summary);
    // a directory the build set up says now what it is built for
    g_free (build->title);
    build->title = build_output_title (build->root, build->profile);
    // the error gone to while the build ran stays the one of the list, its note in the list now
    build_list (build, g_strstrip (summary));
    if (build->listed)
        build_note_clear (build);
    g_free (summary);
    build_output_show (build);
    if (build->quiet && ok && build->window != 0)
    {
        GVariantDict dict;
        GVariant *reply;

        g_variant_dict_init (&dict, NULL);
        g_variant_dict_insert (&dict, "id", "x", build->window);
        reply = build_viewer (build, "close", &dict);
        if (reply != NULL)
            g_variant_unref (reply);
        build->window = 0;
    }
    if (build->service)
    {
        GVariantDict dict;

        g_variant_dict_init (&dict, NULL);
        g_variant_dict_insert (&dict, "root", "s", build->root);
        // which profile, in which directory: a plugin listens to the builds of one
        if (build->profile != NULL)
            g_variant_dict_insert (&dict, "profile", "s", build->profile);
        g_variant_dict_insert (&dict, "dir", "s", build->dir);
        // "configure": the directory set up; "build": built, its tests run when it says so
        g_variant_dict_insert (&dict, "step", "s", build->configuring ? "configure" : "build");
        g_variant_dict_insert (&dict, "ok", "b", ok);
        g_variant_dict_insert (&dict, "errors", "i", (gint32) errors);
        build->host->service_emit (build->host, BUILD_SERVICE, "finished",
                                   g_variant_dict_end (&dict));
    }
    widget_draw (WIDGET (build->host->host_data));
    if (build->view != NULL)
        build_options_view_done (build->view, ok && kept);
    tty_refresh ();
}

/* --------------------------------------------------------------------------------------------- */

static int
build_output_ready (int fd, void *data)
{
    build_t *build = (build_t *) data;
    char buf[4096];
    ssize_t n = read (fd, buf, sizeof (buf));

    if (n > 0)
    {
        char *newline;
        char *valid;
        const gchar *end;

        /* a character cut at the end of the read waits for its other bytes: made valid alone it
           would be a U+FFFD in the name of a file */
        g_string_append_len (build->partial, buf, n);
        if (!g_utf8_validate (build->partial->str, (gssize) build->partial->len, &end)
            && build->partial->str + build->partial->len - end < 4
            && g_utf8_get_char_validated (end, build->partial->str + build->partial->len - end)
                == (gunichar) -2)
        {
            valid = g_utf8_make_valid (build->partial->str, end - build->partial->str);
            g_string_erase (build->partial, 0, end - build->partial->str);
        }
        else
        {
            valid = g_utf8_make_valid (build->partial->str, (gssize) build->partial->len);
            g_string_truncate (build->partial, 0);
        }

        g_string_append (build->output, valid);
        g_string_append (build->pending, valid);
        g_free (valid);
        while ((newline = strchr (build->pending->str, '\n')) != NULL)
        {
            char *line = g_strndup (build->pending->str, (gsize) (newline - build->pending->str));

            g_string_erase (build->pending, 0, newline - build->pending->str + 1);
            build_line (build, line);
            g_free (line);
        }
        if (build->output->len > BUILD_OUTPUT_MAX)
        {
            gsize cut = build->output->len - BUILD_OUTPUT_MAX;

            // at the start of a character: the text stays UTF-8
            while (cut < build->output->len && (build->output->str[cut] & 0xC0) == 0x80)
                cut++;
            g_string_erase (build->output, 0, (gssize) cut);
        }
        build_output_show (build);
        tty_refresh ();
    }
    else if (n == 0 || (errno != EAGAIN && errno != EINTR))
    {
        int status = 0;

        delete_select_channel (fd);
        close (fd);
        build->output_fd = -1;
        if (build->pending->len > 0)
            build_line (build, build->pending->str);
        g_string_truncate (build->pending, 0);
        /* the output ends with the build, mostly: a command that sends it elsewhere, or a
           program it left running with it, ends it before; the build is then done when it is
           asked about */
        if (waitpid (build->pid, &status, WNOHANG) == build->pid)
        {
            g_spawn_close_pid (build->pid);
            build->pid = 0;
            build_finished (build, status);
            // the loop of the dialog runs then, not at the next key: a window waits for the end
            return 1;
        }
        // a window waiting for the end waits for the process
        if (build->view != NULL)
        {
            build_options_view_wait (build->view);
            return 1;
        }
    }
    return 0;
}

/* --------------------------------------------------------------------------------------------- */

static void
build_child_setup (gpointer data)
{
    (void) data;
    (void) setsid ();
}

/* --------------------------------------------------------------------------------------------- */

/* Whether the build still runs: one whose output has ended is done when its process is */
static gboolean
build_running (build_t *build)
{
    int status = 0;

    if (build->pid == 0)
        return FALSE;
    if (build->output_fd < 0 && waitpid (build->pid, &status, WNOHANG) == build->pid)
    {
        g_spawn_close_pid (build->pid);
        build->pid = 0;
        build_finished (build, status);
        return FALSE;
    }
    return TRUE;
}

/* --------------------------------------------------------------------------------------------- */

/* Whether a job runs, told the user: the build of a profile or its setup, one at a time */
static gboolean
build_busy (build_t *build)
{
    char *text;

    if (!build_running (build))
        return FALSE;
    if (build->profile == NULL)
        text = g_strdup (_ ("The build is running."));
    else if (build->configuring)
        text = g_strdup_printf (_ ("The setup of %s is running."), build->profile);
    else
        text = g_strdup_printf (_ ("The build of %s is running."), build->profile);
    build->host->message (build->host, D_NORMAL, _ ("Build"), text);
    g_free (text);
    return TRUE;
}

/* --------------------------------------------------------------------------------------------- */

/* Whether the job running is that of the profile @name of @root, told the user: the profile
   stays as it is meanwhile */
static gboolean
build_busy_with (build_t *build, const char *root, const char *name)
{
    if (!build_running (build) || g_strcmp0 (build->root, root) != 0
        || g_strcmp0 (build->profile, name) != 0)
        return FALSE;
    return build_busy (build);
}

/* --------------------------------------------------------------------------------------------- */

/* Run @command in @root, the job of the profile @name, NULL for the active one, whose compiler
   names the files from @dir; both taken; FALSE when it could not start */
static gboolean
build_spawn (build_t *build, const char *root, const char *name, char *command, char *dir)
{
    char *shell_command;
    char *argv[4];
    GError *error = NULL;
    int out;

    g_free (build->root);
    build->root = g_strdup (root);
    g_free (build->command);
    build->command = command;
    g_free (build->dir);
    build->dir = dir;
    g_ptr_array_set_size (build->diagnostics, 0);
    g_ptr_array_set_size (build->notes, 0);
    build_note_clear (build);
    build->listed = FALSE;
    build->current = -1;
    build_marks_clear (build);
    g_free (build->title);
    build->title = build_output_title (root, name);
    {
        build_profiles_t *profiles = build_profiles_of (root);
        const build_profile_t *p =
            name != NULL ? build_profiles_find (profiles, name) : build_profiles_active (profiles);

        g_free (build->profile);
        build->profile = p != NULL ? g_strdup (p->name) : NULL;
        build_profiles_free (profiles);
    }
    g_string_printf (build->output, "$ %s\n", command);
    g_string_truncate (build->pending, 0);

    shell_command = g_strconcat ("exec 2>&1; ", command, (char *) NULL);
    argv[0] = (char *) "/bin/sh";
    argv[1] = (char *) "-c";
    argv[2] = shell_command;
    argv[3] = NULL;
    // a session of its own: the build is stopped as a whole, make with its compilers
    if (!g_spawn_async_with_pipes (root, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD, build_child_setup,
                                   NULL, &build->pid, NULL, &out, NULL, &error))
    {
        g_string_append_printf (build->output, "%s\n", error->message);
        g_clear_error (&error);
        build_output_show (build);
        g_free (shell_command);
        build->pid = 0;
        return FALSE;
    }
    g_free (shell_command);
    build->output_fd = out;
    (void) fcntl (out, F_SETFL, fcntl (out, F_GETFL) | O_NONBLOCK);
    add_select_channel (out, build_output_ready, build);
    build_output_show (build);
    return TRUE;
}

/* --------------------------------------------------------------------------------------------- */

/* Build the project of @root by its profile @name, NULL for the active one; FALSE when it could
   not start */
static gboolean
build_start (build_t *build, const char *root, const char *name)
{
    char *command = NULL, *dir = NULL;

    if (build_busy (build))
        return FALSE;
    if (!build_plan (build, root, name, &command, &dir))
    {
        g_free (command);
        command = input_dialog (_ ("Build"), _ ("Command that builds the project:"),
                                "build-command", "make", INPUT_COMPLETE_COMMANDS);
        if (command == NULL || *command == '\0')
        {
            g_free (command);
            g_free (dir);
            return FALSE;
        }
        build_save_command (build, root, name, command);
    }
    if (build->host->save_modified_files != NULL
        && !build->host->save_modified_files (build->host, root))
    {
        g_free (command);
        g_free (dir);
        return FALSE;
    }
    build->configuring = FALSE;
    MC_PTR_FREE (build->changes);
    return build_spawn (build, root, name, command, dir);
}

/* --------------------------------------------------------------------------------------------- */

/* Give the build directory of the profile @name of @root the options @changes, "-Dx=1", NULL for
   none: set up when it is not yet; FALSE when it could not start, said why */
static gboolean
build_configure (build_t *build, const char *root, const char *name, const char *changes)
{
    build_profiles_t *profiles;
    const build_profile_t *p;
    char *command = NULL, *dir = NULL;

    if (build_busy (build))
        return FALSE;
    profiles = build_profiles_of (root);
    p = name != NULL ? build_profiles_find (profiles, name) : build_profiles_active (profiles);
    if (p != NULL)
    {
        command = build_profile_configure_command (p, root, changes);
        dir = build_profile_dir (p, root);
    }
    build_profiles_free (profiles);
    if (command == NULL)
    {
        build->host->message (build->host, D_ERROR, _ ("Build"),
                              _ ("Only a build of meson or cmake has options to set."));
        g_free (dir);
        return FALSE;
    }
    build->configuring = TRUE;
    g_free (build->changes);
    build->changes = g_strdup (changes);
    build->quiet = FALSE;
    return build_spawn (build, root, name, command, dir);
}

/* --------------------------------------------------------------------------------------------- */

/* Whether a window of the editor has the file @file */
static gboolean
build_file_loaded (build_t *build, const char *file)
{
    char *real = realpath (file, NULL);
    gboolean found = FALSE;
    GList *item;

    for (item = GROUP (build->host->host_data)->widgets; item != NULL && !found;
         item = g_list_next (item))
    {
        char *name;

        if (!edit_widget_is_editor (CONST_WIDGET (item->data)))
            continue;
        name = build->host->get_current_file (build->host, item->data);
        if (name != NULL && strcmp (name, file) == 0)
            found = TRUE;
        else if (name != NULL && real != NULL)
        {
            char *other = realpath (name, NULL);

            found = other != NULL && strcmp (other, real) == 0;
            free (other);
        }
        g_free (name);
    }
    free (real);
    return found;
}

/* --------------------------------------------------------------------------------------------- */

/* Take off the note of the error gone to last */
static void
build_note_clear (build_t *build)
{
    if (build->note_file != NULL)
        build->host->set_line_note (build->host, build->note_file, build->note_line, NULL);
    g_free (build->note_file);
    build->note_file = NULL;
}

/* --------------------------------------------------------------------------------------------- */

/* The line of a diagnostic in its file, the cursor on the column the compiler named.
   @load: the file is loaded when no window has it, else it is not shown.  The message goes
   after the line when the list is not in sight.  FALSE when the line is not shown */
static gboolean
build_show (build_t *build, const build_diagnostic_t *d, gboolean load)
{
    if (!load && !build_file_loaded (build, d->file))
        return FALSE;
    // the column of gcc counts the columns of the screen from 1, a tab to a stop of 8
    if (build->host->show_location_at != NULL
            ? !build->host->show_location_at (build->host, d->file, d->line, MAX (d->column - 1, 0),
                                              BUILD_TAB_WIDTH)
            : !build->host->show_location (build->host, d->file, d->line))
        return FALSE;
    build_note_clear (build);
    if (!build->listed || build_output_state (build) == MC_EP_WINDOW_CLOSED)
    {
        build->host->set_line_note (build->host, d->file, d->line, d->message);
        build->note_file = g_strdup (d->file);
        build->note_line = d->line;
    }
    return TRUE;
}

/* --------------------------------------------------------------------------------------------- */

static void
build_go (build_t *build, int step)
{
    const build_diagnostic_t *d;
    guint n = build->diagnostics->len;
    gboolean errors = FALSE;
    guint i;

    if (n == 0)
    {
        build->host->message (build->host, D_NORMAL, _ ("Build"),
                              _ ("The build has said nothing about the files."));
        return;
    }
    // the errors, one after another; the warnings only when there is no error
    for (i = 0; i < n && !errors; i++)
        errors = ((const build_diagnostic_t *) g_ptr_array_index (build->diagnostics, i))->error;
    for (i = 0; i < n; i++)
    {
        build->current = (build->current + step + (int) n) % (int) n;
        if (build->current < 0)
            build->current = step > 0 ? 0 : (int) n - 1;
        d = g_ptr_array_index (build->diagnostics, build->current);
        if (d->error || !errors)
            break;
    }
    d = g_ptr_array_index (build->diagnostics, build->current);
    (void) build_show (build, d, TRUE);
    build_list_cursor (build);
}

/* --------------------------------------------------------------------------------------------- */

/* The diagnostic of the line @row of the list in its file; FALSE when the line has none.
   @stay: the list keeps the focus, the file only shown */
static gboolean
build_show_row (build_t *build, long row, gboolean stay)
{
    const build_diagnostic_t *d;
    int i;

    if (!build->listed || row < 0 || row >= (long) build->rows->len)
        return FALSE;
    i = g_array_index (build->rows, int, row);
    if (i < 0 || i >= (int) build->diagnostics->len)
        return FALSE;
    build->current = i;
    d = g_ptr_array_index (build->diagnostics, i);
    // the cursor going down the list shows the files open, Enter opens one
    if (!build_show (build, d, !stay))
        return TRUE;
    if (stay && build->window != 0)
    {
        GVariantDict dict;
        GVariant *reply;

        g_variant_dict_init (&dict, NULL);
        g_variant_dict_insert (&dict, "id", "x", build->window);
        g_variant_dict_insert (&dict, "focus", "b", TRUE);
        reply = build_viewer (build, "show", &dict);
        if (reply != NULL)
            g_variant_unref (reply);
    }
    return TRUE;
}

/* --------------------------------------------------------------------------------------------- */

/* Enter in the list: the error of its line, in its file */
static gboolean
build_show_cursor (build_t *build)
{
    GVariantDict dict;
    GVariant *reply;
    gint64 row = -1;
    gboolean focused = FALSE, selected = FALSE;

    if (build->window == 0 || !build->listed)
        return FALSE;
    g_variant_dict_init (&dict, NULL);
    g_variant_dict_insert (&dict, "id", "x", build->window);
    reply = build_viewer (build, "info", &dict);
    if (reply == NULL)
        return FALSE;
    (void) g_variant_lookup (reply, "focused", "b", &focused);
    (void) g_variant_lookup (reply, "selected", "b", &selected);
    (void) g_variant_lookup (reply, "line", "x", &row);
    g_variant_unref (reply);
    // Enter on marked text copies it, as in any window of the viewer
    return focused && !selected && build_show_row (build, (long) row, FALSE);
}

/* --------------------------------------------------------------------------------------------- */

/* The list of the build: a double click on a line goes to its error, the cursor moved on it
   shows the error in its file, the list keeping the focus */
static void
build_viewer_signal (const char *name, const char *signal, GVariant *args, void *data)
{
    build_t *build = (build_t *) data;
    gint64 id = 0, line = -1;

    (void) name;
    if (build->window == 0 || !g_variant_lookup (args, "id", "x", &id) || id != build->window
        || !g_variant_lookup (args, "line", "x", &line))
        return;
    if (strcmp (signal, "activate") == 0)
        (void) build_show_row (build, (long) line, FALSE);
    else if (strcmp (signal, "cursor") == 0)
        (void) build_show_row (build, (long) line, TRUE);
}

/* --------------------------------------------------------------------------------------------- */

static gboolean
build_run_command (build_t *build, int cmd, void *edit)
{
    switch (cmd)
    {
    case BUILD_CMD_RUN:
    {
        build->quiet = FALSE;
        char *root =
            build_root_of (build, edit != NULL ? edit : build->host->window_top_file (build->host));

        (void) build_start (build, root, NULL);
        g_free (root);
        return TRUE;
    }
    case BUILD_CMD_NEXT:
        build_go (build, 1);
        return TRUE;
    case BUILD_CMD_PREV:
        build_go (build, -1);
        return TRUE;
    case BUILD_CMD_SHOW:
        // Enter of a file is that of the editor
        return edit == NULL && build_show_cursor (build);
    default:
        return FALSE;
    }
}

/* --------------------------------------------------------------------------------------------- */

static int
build_command (const build_t *build, long command)
{
    int i;

    for (i = 0; command != CK_IgnoreKey && i < BUILD_CMD_COUNT; i++)
        if (build->commands[i] == command)
            return i;
    return BUILD_CMD_NONE;
}

/* --------------------------------------------------------------------------------------------- */

static mc_ep_result_t
build_handle_key (void *data, int key, void *edit)
{
    build_t *build = (build_t *) data;
    const int cmd =
        build_command (build, build->host->command_lookup (build->host, BUILD_KEYMAP_SECTION, key));

    return build_run_command (build, cmd, edit) ? MC_EPR_OK : MC_EPR_NOT_SUPPORTED;
}

/* --------------------------------------------------------------------------------------------- */

static mc_ep_result_t
build_handle_action (void *data, long command, void *edit)
{
    build_t *build = (build_t *) data;

    return build_run_command (build, build_command (build, command), edit) ? MC_EPR_OK
                                                                           : MC_EPR_NOT_SUPPORTED;
}

/* --------------------------------------------------------------------------------------------- */

/* The marks of the build come back on a file opened again */
static mc_ep_result_t
build_file_open (void *data, void *edit)
{
    build_t *build = (build_t *) data;
    char *file = build->host->get_current_file (build->host, edit);
    guint i;

    for (i = 0; file != NULL && i < build->diagnostics->len; i++)
    {
        const build_diagnostic_t *d = g_ptr_array_index (build->diagnostics, i);

        if (strcmp (d->file, file) == 0)
            build_mark (build, d);
    }
    g_free (file);
    return MC_EPR_OK;
}

/* --------------------------------------------------------------------------------------------- */

static mc_ep_result_t
build_act_run (void *data, void *edit)
{
    return build_run_command (data, BUILD_CMD_RUN, edit) ? MC_EPR_OK : MC_EPR_FAILED;
}

static mc_ep_result_t
build_act_next (void *data, void *edit)
{
    return build_run_command (data, BUILD_CMD_NEXT, edit) ? MC_EPR_OK : MC_EPR_FAILED;
}

static mc_ep_result_t
build_act_prev (void *data, void *edit)
{
    return build_run_command (data, BUILD_CMD_PREV, edit) ? MC_EPR_OK : MC_EPR_FAILED;
}

/* --------------------------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------------------------- */
/* The window of the profiles */

/* @name as a part of the name of a directory: lower case, '-' for what is no letter or digit */
static char *
build_profile_dir_part (const char *name)
{
    char *part = g_ascii_strdown (name, -1);
    char *p;

    for (p = part; *p != '\0'; p++)
        if (!g_ascii_isalnum (*p))
            *p = '-';
    return part;
}

/* --------------------------------------------------------------------------------------------- */

/* The directory offered to the profile @name made from @from: next to that of @from, its name
   after it, "../coole-build" -> "../coole-build-coverage"; "build/coverage" in the root when
   @from builds in the root itself */
static char *
build_profile_offer_dir_first (const build_profile_t *from, const char *root, const char *name)
{
    char *part = build_profile_dir_part (name);
    char *dir = from != NULL ? build_profile_dir (from, root) : g_strdup (root);
    char *offer;

    if (from == NULL || from->dir == NULL || strcmp (dir, root) == 0)
        offer = g_build_filename ("build", part, (char *) NULL);
    else
    {
        char *given = g_strdup (from->dir);
        gsize len = strlen (given);

        while (len > 1 && given[len - 1] == '/')
            given[--len] = '\0';
        offer = g_strconcat (given, "-", part, (char *) NULL);
        g_free (given);
    }
    g_free (dir);
    g_free (part);
    return offer;
}

/* --------------------------------------------------------------------------------------------- */

/* @dir from @root as the system has it: its links followed when it is there */
static char *
build_profile_real_dir (const char *dir, const char *root)
{
    char *path = g_canonicalize_filename (dir, root);
    char *real = realpath (path, NULL);

    if (real == NULL)
        return path;
    g_free (path);
    path = g_strdup (real);
    free (real);
    return path;
}

/* --------------------------------------------------------------------------------------------- */

/* The profile of @profiles other than @self whose directory is @dir, absolute, a link to it as
   well; NULL for none */
static const build_profile_t *
build_profile_dir_owner (const build_profiles_t *profiles, const build_profile_t *self,
                         const char *root, const char *dir)
{
    char *want = build_profile_real_dir (dir, root);
    const build_profile_t *owner = NULL;
    guint i;

    for (i = 0; owner == NULL && i < profiles->list->len; i++)
    {
        const build_profile_t *q = (const build_profile_t *) g_ptr_array_index (profiles->list, i);
        char *abs = build_profile_dir (q, root);
        char *have = build_profile_real_dir (abs, root);

        if (q != self && strcmp (have, want) == 0)
            owner = q;
        g_free (have);
        g_free (abs);
    }
    g_free (want);
    return owner;
}

/* --------------------------------------------------------------------------------------------- */

/* The directory offered to the profile @name made from @from, one no profile of @profiles has
   and that is not there yet: "-2", "-3" after it when it is taken */
static char *
build_profile_offer_dir (const build_profiles_t *profiles, const build_profile_t *from,
                         const char *root, const char *name)
{
    char *first = build_profile_offer_dir_first (from, root, name);
    char *offer = g_strdup (first);
    int i;

    for (i = 2; i < 100; i++)
    {
        char *abs = g_canonicalize_filename (offer, root);
        const gboolean taken = build_profile_dir_owner (profiles, NULL, root, abs) != NULL
            || g_file_test (abs, G_FILE_TEST_EXISTS);

        g_free (abs);
        if (!taken)
            break;
        g_free (offer);
        offer = g_strdup_printf ("%s-%d", first, i);
    }
    g_free (first);
    return offer;
}

/* --------------------------------------------------------------------------------------------- */

/* A new profile made from @from, its name asked; its window after it; FALSE when none is made.
   @profiles are those read before: the file has the profiles now */
static gboolean
build_profile_new (build_t *build, const char *root, build_profiles_t *profiles,
                   const build_profile_t *from)
{
    gboolean made;
    char *name, *dir, *from_dir;
    build_profile_t *p;

    name = input_dialog (_ ("Build"), _ ("Name of the new profile:"), "build-profile-name", "",
                         INPUT_COMPLETE_NONE);
    if (name == NULL || *g_strstrip (name) == '\0')
    {
        g_free (name);
        return FALSE;
    }
    dir = build_profile_offer_dir (profiles, from, root, name);
    p = build_profiles_add (profiles, root, name, from, dir);
    g_free (dir);
    if (p == NULL)
    {
        build->host->message (build->host, D_ERROR, _ ("Build"),
                              _ ("There is a profile of that name, or the name has [ or ]."));
        g_free (name);
        return FALSE;
    }
    // what set up the directory of the other one sets up this one
    from_dir = from != NULL ? build_profile_dir (from, root) : NULL;
    if (p->setup == NULL && from_dir != NULL)
        p->setup = build_profile_setup_of (from_dir, from->system);
    if (p->test == NULL)
    {
        char *abs = build_profile_dir (p, root);

        p->test = build_profile_test_command (p->system, abs);
        g_free (abs);
    }
    g_free (from_dir);
    g_free (name);
    // kept for the window to read it; it takes it away again when it is closed unsaved
    if (!build_profiles_save (build, root, profiles))
        return FALSE;
    name = g_strdup (p->name);
    made = build_profile_window (build, root, name, TRUE, NULL);
    g_free (name);
    return made;
}

/* --------------------------------------------------------------------------------------------- */
/* The window of the options of a profile */

typedef enum
{
    BUILD_VIEW_SHOWN,    // as the directory has them
    BUILD_VIEW_CHANGED,  // changes not applied
    BUILD_VIEW_APPLYING,
    BUILD_VIEW_FAILED,
    BUILD_VIEW_APPLIED
} build_view_state_t;

/* Which options the table shows; F3 goes to the next one */
typedef enum
{
    BUILD_VIEW_MAIN,    // those set by the user, of the project, and what a build is for
    BUILD_VIEW_DIFFER,  // those off their default, or changed
    BUILD_VIEW_ALL,
    BUILD_VIEW_MODES
} build_view_mode_t;

#define BUILD_VIEW_COLS 4

struct build_options_view
{
    build_t *build;
    char *root;
    char *name;   // of the profile
    char *dir;    // absolute
    char *where;  // as the user names it, "../coole-build"
    build_system_t system;
    GPtrArray *options;   // build_option_t; NULL when none are known
    GHashTable *changes;  // name -> value, not applied yet
    GArray *rows;         // guint: the option of each row shown
    build_view_mode_t mode;
    build_view_state_t state;
    gboolean close_after;  // closed once the changes are applied
    gboolean closing;      // they are: the loop of the window closes it
    int introspect_fd;     // meson introspect telling the defaults, -1 when it is not running
    GString *introspect;
    WDialog *dlg;
    WInput *in_name;
    WInput *in_dir;
    WInput *in_build;
    WInput *in_test;
    WCheck *after;
    WHLine *mode_line;
    WTable *table;
    WLabel *about;
    WLabel *choices;
    gboolean is_new;  // made for the window: gone when it is closed unsaved
    gboolean saved;
    // the fields as the profile has them
    char *old_dir;
    char *old_build;
    char *old_test;
    gboolean old_after;
};

typedef struct build_options_view build_options_view_t;

/* --------------------------------------------------------------------------------------------- */

/* Whether @o is shown: changed or all of them; those off their default; or in the first sight,
   what the user set, the options of the project and the few that say what a build is for */
static gboolean
build_option_main (const build_options_view_t *view, const build_option_t *o)
{
    static const char *const meson[] = {
        "buildtype",  "optimization", "debug",    "werror", "b_coverage",
        "b_sanitize", "b_lundef",     "b_ndebug", NULL,
    };
    static const char *const cmake[] = {
        "CMAKE_BUILD_TYPE", "CMAKE_C_FLAGS", "CMAKE_CXX_FLAGS", "CMAKE_INSTALL_PREFIX", NULL,
    };

    if (g_hash_table_contains (view->changes, o->name) || view->mode == BUILD_VIEW_ALL)
        return TRUE;
    if (view->mode == BUILD_VIEW_DIFFER)
        return o->default_value != NULL ? strcmp (o->value, o->default_value) != 0 : o->set_by_user;
    if (view->system == BUILD_MESON)
        return o->set_by_user || g_strcmp0 (o->section, "user") == 0
            || g_strv_contains (meson, o->name);
    return !o->advanced
        && (!g_str_has_prefix (o->name, "CMAKE_") || g_strv_contains (cmake, o->name));
}

/* --------------------------------------------------------------------------------------------- */

static const build_option_t *
build_view_option (const build_options_view_t *view, int row)
{
    if (view->options == NULL || row < 0 || (guint) row >= view->rows->len)
        return NULL;
    return (const build_option_t *) g_ptr_array_index (view->options,
                                                       g_array_index (view->rows, guint, row));
}

/* --------------------------------------------------------------------------------------------- */

/* The value of @o as it will be: changed, else as it is */
static const char *
build_view_value (const build_options_view_t *view, const build_option_t *o)
{
    const char *changed = (const char *) g_hash_table_lookup (view->changes, o->name);

    return changed != NULL ? changed : o->value;
}

/* --------------------------------------------------------------------------------------------- */

static int
build_view_rows (const void *data)
{
    return (int) ((const build_options_view_t *) data)->rows->len;
}

static const char *
build_view_cell (const void *data, int row, int col)
{
    const build_options_view_t *view = (const build_options_view_t *) data;
    const build_option_t *o = build_view_option (view, row);

    if (o == NULL)
        return "";
    switch (col)
    {
    case 0:
        // a change not applied
        return g_hash_table_contains (view->changes, o->name) ? "*" : "";
    case 1:
        return o->name;
    case 2:
        return build_view_value (view, o);
    default:
        return o->default_value != NULL ? o->default_value : "";
    }
}

/* An option that differs from its default stands out */
static int
build_view_cell_color (void *data, int row, int col)
{
    const build_options_view_t *view = (const build_options_view_t *) data;
    const build_option_t *o = build_view_option (view, row);

    (void) col;
    if (o == NULL || o->default_value == NULL
        || strcmp (build_view_value (view, o), o->default_value) == 0)
        return -1;
    return dialog_colors[DLG_COLOR_HOT_NORMAL];
}

/* --------------------------------------------------------------------------------------------- */

/* The rows shown: the main options or all of them, by name; the cursor on the option @name, NULL
   for the one it is on */
static int
build_view_compare (gconstpointer a, gconstpointer b, gpointer data)
{
    const GPtrArray *options = (const GPtrArray *) data;
    const build_option_t *x =
        (const build_option_t *) g_ptr_array_index (options, *(const guint *) a);
    const build_option_t *y =
        (const build_option_t *) g_ptr_array_index (options, *(const guint *) b);

    return strcmp (x->name, y->name);
}

static void
build_view_fill (build_options_view_t *view, const char *name)
{
    const build_option_t *current = name == NULL && view->table != NULL
        ? build_view_option (view, table_get_current (view->table))
        : NULL;
    char *keep = g_strdup (name != NULL ? name : current != NULL ? current->name : NULL);
    int at = 0;
    guint i;

    g_array_set_size (view->rows, 0);
    for (i = 0; view->options != NULL && i < view->options->len; i++)
        if (build_option_main (view, (const build_option_t *) g_ptr_array_index (view->options, i)))
            g_array_append_val (view->rows, i);
    if (view->options != NULL)
        g_array_sort_with_data (view->rows, build_view_compare, view->options);
    for (i = 0; keep != NULL && i < view->rows->len; i++)
        if (strcmp (build_view_option (view, (int) i)->name, keep) == 0)
            at = (int) i;
    g_free (keep);
    if (view->table != NULL)
    {
        table_quick_search_stop (view->table);
        table_set_current (view->table, at);
    }
}

/* --------------------------------------------------------------------------------------------- */

/* The options as the directory has them now, else as the profile will set it up */
static void
build_view_load (build_options_view_t *view)
{
    build_t *build = view->build;
    const gboolean set_up = build_system_of_dir (view->dir) == view->system;
    const char *defaults =
        g_strcmp0 (build->defaults_root, view->root) == 0 ? build->defaults : NULL;
    // the rows are indexes into the options there: the cursor is kept by the name of its option
    const build_option_t *current =
        view->table != NULL ? build_view_option (view, table_get_current (view->table)) : NULL;
    char *keep = current != NULL ? g_strdup (current->name) : NULL;

    g_array_set_size (view->rows, 0);
    if (view->options != NULL)
        g_ptr_array_free (view->options, TRUE);
    view->options = NULL;
    if (set_up)
    {
        view->options = build_options_read (view->dir, view->system);
        if (view->options != NULL && defaults != NULL)
            (void) build_options_set_defaults (view->options, defaults);
    }
    else if (view->system == BUILD_MESON && defaults != NULL)
    {
        build_profiles_t *profiles = build_profiles_of (view->root);
        const build_profile_t *p = build_profiles_find (profiles, view->name);

        view->options = build_options_of_source (defaults);
        if (p != NULL)
            build_profile_setup_apply (view->options, p->setup);
        build_profiles_free (profiles);
    }
    build_view_fill (view, keep);
    g_free (keep);
}

/* --------------------------------------------------------------------------------------------- */

/* What the field with the focus is, on the two lines under the table; FALSE when no field has
   the focus */
static gboolean
build_view_field_help (build_options_view_t *view, int width)
{
    Widget *current =
        GROUP (view->dlg)->current != NULL ? WIDGET (GROUP (view->dlg)->current->data) : NULL;
    char *first = NULL, *second = NULL;

    if (current == WIDGET (view->in_name))
    {
        first = g_strdup (_ ("The name of the profile, without [ and ]."));
        second = g_strdup ("");
    }
    else if (current == WIDGET (view->in_dir))
    {
        first = g_strdup (_ ("The build directory, from the root of the project or absolute."));
        second = g_strdup (_ ("One not set up yet is set up by F2 or by the first build."));
    }
    else if (current == WIDGET (view->in_build))
    {
        build_profiles_t *profiles = build_profiles_of (view->root);
        const build_profile_t *p = build_profiles_find (profiles, view->name);
        char *usual = NULL;

        if (p != NULL)
        {
            build_profile_t plain = *p;

            plain.build = NULL;
            usual = build_profile_build_command (&plain, view->root);
            // the directory as the profile names it, short enough for the line
            if (usual != NULL)
            {
                char *quoted = g_shell_quote (view->dir);
                char **parts = g_strsplit (usual, quoted, -1);

                g_free (usual);
                usual = g_strjoinv (view->where, parts);
                g_strfreev (parts);
                g_free (quoted);
            }
        }
        first = g_strdup (_ ("The command that builds the profile, run in the root of the "
                             "project."));
        second = g_strdup_printf (_ ("Empty: that of the build system, %s"),
                                  usual != NULL ? usual : _ ("none"));
        g_free (usual);
        build_profiles_free (profiles);
    }
    else if (current == WIDGET (view->in_test))
    {
        first = g_strdup (_ ("The command that runs the tests, run in the root of the project."));
        second = g_strdup (_ ("Empty: no tests. Ctrl-F9 runs them after the build when the box "
                              "under it is checked."));
    }
    else if (current == WIDGET (view->after))
    {
        first = g_strdup (_ ("Run the test command after each build of the profile."));
        second = g_strdup (_ ("The coverage of the tests comes from a build with coverage and its "
                              "tests run."));
    }
    if (first == NULL)
        return FALSE;
    label_set_text (view->about, str_trunc (first, width));
    label_set_text (view->choices, str_trunc (second, width));
    g_free (first);
    g_free (second);
    return TRUE;
}

/* --------------------------------------------------------------------------------------------- */

/* What the lines under the table say: the option of the cursor, the state, the keys */
static void
build_view_show (build_options_view_t *view)
{
    const int width = WIDGET (view->dlg)->rect.cols - 4;
    const build_option_t *o = build_view_option (view, table_get_current (view->table));
    const gboolean set_up = build_system_of_dir (view->dir) == view->system;
    const char *state, *footer;
    char *text, *title, *padded;
    WFrame *frame;

    // a field with the focus: what it is, under the table where the option is said else
    if (build_view_field_help (view, width))
        goto frame;
    if (o != NULL)
        text = g_strdup_printf ("%s: %s", o->name, o->description != NULL ? o->description : "");
    else if (view->system != BUILD_MESON && view->system != BUILD_CMAKE)
        text = g_strdup (_ ("Only a build of meson or cmake has options to set."));
    else if (view->introspect_fd >= 0)
        text = g_strdup (_ ("Reading the options of meson.build..."));
    else if (!set_up)
        text = g_strdup (_ ("The directory is not set up yet: F2 sets it up."));
    else
        text = g_strdup ("");
    label_set_text (view->about, str_trunc (text, width));
    g_free (text);
    if (o != NULL && o->choices != NULL)
    {
        char *joined = g_strjoinv (", ", o->choices);

        text = g_strdup_printf (_ ("One of: %s"), joined);
        g_free (joined);
    }
    else if (o != NULL && o->type == BUILD_OPTION_FEATURE)
        text = g_strdup (_ ("One of: enabled, disabled, auto"));
    else if (o != NULL && o->type == BUILD_OPTION_ARRAY)
        text = g_strdup (_ ("A list, its items joined by ','"));
    else
        text = g_strdup ("");
    label_set_text (view->choices, str_trunc (text, width));
    g_free (text);
frame:
    // where it is and how it goes on the frame, as the Build window says what it builds
    switch (view->state)
    {
    case BUILD_VIEW_CHANGED:
        state = _ ("changed, not applied");
        break;
    case BUILD_VIEW_APPLYING:
        state = _ ("applying, see Build output");
        break;
    case BUILD_VIEW_FAILED:
        state = _ ("failed, see Build output");
        break;
    case BUILD_VIEW_APPLIED:
        state = _ ("applied");
        break;
    default:
        state = NULL;
        break;
    }
    text = g_strdup_printf (_ ("Build profile: %s (%s, %s%s)%s%s"), view->name, view->where,
                            build_system_name (view->system), set_up ? "" : _ (", not set up"),
                            state != NULL ? ": " : "", state != NULL ? state : "");
    // the keys of the window, the same whatever has the focus; the help says what they do
    footer = "Del F2 F3 F4";
    {
        static const char *const modes[BUILD_VIEW_MODES] = {
            N_ ("Main options"),
            N_ ("Options off their default"),
            N_ ("All options"),
        };

        char *line = g_strdup_printf (" %s ", _ (modes[view->mode]));

        hline_set_text (view->mode_line, line);
        g_free (line);
    }
    /* the frame drawn again paints over the whole window: set when the text is another one, and
       the window drawn again then */
    frame = FRAME (view->dlg->bg);
    title = g_strdup_printf (" %s ", text);
    padded = g_strdup_printf (" %s ", footer);
    if (g_strcmp0 (frame->title, title) != 0 || g_strcmp0 (frame->footer, padded) != 0)
    {
        frame_set_title (frame, text);
        frame_set_footer (frame, footer);
        widget_draw (WIDGET (view->dlg));
    }
    g_free (padded);
    g_free (title);
    g_free (text);
}

/* --------------------------------------------------------------------------------------------- */

/* The process of the setup has not ended with its output: the loop of the window waits for it */
static void
build_options_view_wait (build_options_view_t *view)
{
    widget_idle (WIDGET (view->dlg), TRUE);
}

/* --------------------------------------------------------------------------------------------- */

static void
build_options_view_redraw (build_options_view_t *view)
{
    widget_draw (WIDGET (view->dlg));
    tty_refresh ();
}

/* --------------------------------------------------------------------------------------------- */

/* The setup has ended: the options the directory has now, or the changes kept to try again */
static void
build_options_view_done (build_options_view_t *view, gboolean ok)
{
    if (view->state != BUILD_VIEW_APPLYING)
        return;
    if (ok)
    {
        g_hash_table_remove_all (view->changes);
        view->state = BUILD_VIEW_APPLIED;
        build_view_load (view);
        if (view->close_after)
        {
            /* closed by the loop of the window, not from inside the reading of the output: the
               list under it is drawn again then */
            view->closing = TRUE;
            widget_idle (WIDGET (view->dlg), TRUE);
            return;
        }
    }
    else
    {
        view->state = BUILD_VIEW_FAILED;
        view->close_after = FALSE;
    }
    build_view_show (view);
    build_options_view_redraw (view);
}

/* --------------------------------------------------------------------------------------------- */

/* The time of the newest of the files that give a meson project its options, meson.build,
   meson_options.txt and meson.options; 0 without meson.build */
static gint64
build_meson_time (const char *root)
{
    static const char *const names[] = { "meson.build", "meson_options.txt", "meson.options" };
    gint64 newest = 0;
    gsize i;

    for (i = 0; i < G_N_ELEMENTS (names); i++)
    {
        char *file = g_build_filename (root, names[i], (char *) NULL);
        struct stat st;

        if (stat (file, &st) == 0)
            newest = MAX (newest, (gint64) st.st_mtime);
        else if (i == 0)
        {
            g_free (file);
            return 0;
        }
        g_free (file);
    }
    return newest;
}

/* --------------------------------------------------------------------------------------------- */

/* The defaults meson tells: read when it has told them all */
static int
build_view_introspect_ready (int fd, void *data)
{
    build_options_view_t *view = (build_options_view_t *) data;
    char buf[4096];
    const ssize_t n = read (fd, buf, sizeof (buf));

    if (n > 0)
    {
        g_string_append_len (view->introspect, buf, n);
        return 0;
    }
    if (n < 0 && (errno == EAGAIN || errno == EINTR))
        return 0;
    delete_select_channel (fd);
    close (fd);
    view->introspect_fd = -1;
    if (view->introspect->str[0] == '[')
    {
        build_t *build = view->build;

        g_free (build->defaults_root);
        build->defaults_root = g_strdup (view->root);
        g_free (build->defaults);
        build->defaults = g_string_free (view->introspect, FALSE);
        view->introspect = g_string_new (NULL);
        build->defaults_time = build_meson_time (view->root);
        build_view_load (view);
    }
    build_view_show (view);
    build_options_view_redraw (view);
    return 1;
}

/* --------------------------------------------------------------------------------------------- */

/* Ask meson for the defaults of the options of the project, once for its meson.build as it is */
static void
build_view_introspect (build_options_view_t *view)
{
    build_t *build = view->build;
    char *argv[] = { (char *) "meson", (char *) "introspect", (char *) "--buildoptions",
                     (char *) "meson.build", NULL };
    const gint64 time = build_meson_time (view->root);
    int out;

    if (view->system != BUILD_MESON || time == 0
        || (g_strcmp0 (build->defaults_root, view->root) == 0 && build->defaults_time == time))
        return;
    if (g_spawn_async_with_pipes (view->root, argv, NULL,
                                  G_SPAWN_SEARCH_PATH | G_SPAWN_STDERR_TO_DEV_NULL, NULL, NULL,
                                  NULL, NULL, &out, NULL, NULL))
    {
        view->introspect_fd = out;
        (void) fcntl (out, F_SETFL, fcntl (out, F_GETFL) | O_NONBLOCK);
        add_select_channel (out, build_view_introspect_ready, view);
    }
}

/* --------------------------------------------------------------------------------------------- */

/* A new value for the option of the cursor: a boolean the other one, one of the choices from a
   list, else as typed */
static void
build_view_change (build_options_view_t *view)
{
    const build_option_t *o = build_view_option (view, table_get_current (view->table));
    static const char *const feature[] = { "enabled", "disabled", "auto", NULL };
    char *name, *now;
    char *value = NULL;

    if (o == NULL || view->state == BUILD_VIEW_APPLYING)
        return;
    /* the options may be read again while a dialog below is up, when meson tells the defaults:
       what the dialogs need is copied, the option looked up again after them */
    name = g_strdup (o->name);
    now = g_strdup (build_view_value (view, o));
    if (o->type == BUILD_OPTION_BOOLEAN)
        value = g_strdup (build_option_toggled (now));
    else if ((o->choices != NULL && o->choices[0] != NULL) || o->type == BUILD_OPTION_FEATURE)
    {
        char **choices = g_strdupv (
            o->choices != NULL && o->choices[0] != NULL ? o->choices : (char **) feature);
        const int count = (int) g_strv_length (choices);
        Listbox *list = listbox_window_new (MIN (count, 16), MIN (COLS - 8, 40), name, NULL);
        int i, at = 0;
        const char *chosen;

        for (i = 0; i < count; i++)
        {
            LISTBOX_APPEND_TEXT (list, 0, choices[i], (void *) choices[i], FALSE);
            if (strcmp (choices[i], now) == 0)
                at = i;
        }
        listbox_set_current (list->list, at);
        chosen = (const char *) listbox_run_with_data (list, NULL);
        value = g_strdup (chosen);
        g_strfreev (choices);
    }
    else
    {
        char *prompt = g_strdup_printf ("%s:", name);

        value = input_dialog (_ ("Build"), prompt, "build-option", now, INPUT_COMPLETE_NONE);
        g_free (prompt);
    }
    g_free (now);
    o = build_options_find (view->options, name);
    g_free (name);
    if (value == NULL || o == NULL)
    {
        g_free (value);
        return;
    }
    // back to what the directory has: no change
    if (strcmp (value, o->value) == 0)
    {
        g_free (value);
        (void) g_hash_table_remove (view->changes, o->name);
    }
    else
        g_hash_table_insert (view->changes, g_strdup (o->name), value);
    view->state = g_hash_table_size (view->changes) > 0 ? BUILD_VIEW_CHANGED : BUILD_VIEW_SHOWN;
}

/* --------------------------------------------------------------------------------------------- */

/* Set the directory up with the changes, "-Da=1 -Db=2" in the order of the names */
static void
build_view_apply (build_options_view_t *view)
{
    GList *names, *l;
    char *changes = NULL;

    // make and the others have no options: their fields alone are saved
    if (view->state == BUILD_VIEW_APPLYING
        || (view->system != BUILD_MESON && view->system != BUILD_CMAKE))
    {
        view->close_after = FALSE;
        return;
    }
    names = g_list_sort (g_hash_table_get_keys (view->changes), (GCompareFunc) strcmp);
    for (l = names; l != NULL; l = g_list_next (l))
    {
        const char *value = (const char *) g_hash_table_lookup (view->changes, l->data);
        const build_option_t *o = build_options_find (view->options, (const char *) l->data);
        char *arg = NULL, *more;

        // a list of meson typed "a,b" goes as a list, its items apart: "['a', 'b']"
        if (view->system == BUILD_MESON && o != NULL && o->type == BUILD_OPTION_ARRAY
            && *value != '[')
        {
            char **items = *value != '\0' ? g_strsplit (value, ",", -1) : g_new0 (char *, 1);

            arg = build_option_meson_list ((const char *const *) items);
            g_strfreev (items);
        }
        more = build_profile_setup_add (changes, (const char *) l->data, arg != NULL ? arg : value);
        g_free (arg);
        g_free (changes);
        changes = more;
    }
    g_list_free (names);
    // nothing to change in a directory set up: nothing to do
    if (changes == NULL && build_system_of_dir (view->dir) == view->system)
    {
        view->close_after = FALSE;
        return;
    }
    if (build_configure (view->build, view->root, view->name, changes))
        view->state = BUILD_VIEW_APPLYING;
    else
        view->close_after = FALSE;
    g_free (changes);
}

/* --------------------------------------------------------------------------------------------- */

/* Whether the fields differ from what the profile has */
static gboolean
build_view_fields_changed (const build_options_view_t *view)
{
    return strcmp (input_get_ctext (view->in_name), view->name) != 0
        || g_strcmp0 (input_get_ctext (view->in_dir), view->old_dir) != 0
        || g_strcmp0 (input_get_ctext (view->in_build), view->old_build) != 0
        || g_strcmp0 (input_get_ctext (view->in_test), view->old_test) != 0
        || view->after->state != view->old_after;
}

/* --------------------------------------------------------------------------------------------- */

/* The fields into the profile, kept: the profiles that name it name its new name; FALSE when
   they are wrong, said why */
static gboolean
build_view_save_fields (build_options_view_t *view)
{
    build_t *build = view->build;
    char *name = g_strstrip (g_strdup (input_get_ctext (view->in_name)));
    char *dir = g_strstrip (g_strdup (input_get_ctext (view->in_dir)));
    char *command = g_strstrip (g_strdup (input_get_ctext (view->in_build)));
    char *test = g_strstrip (g_strdup (input_get_ctext (view->in_test)));
    build_profiles_t *profiles = build_profiles_of (view->root);
    build_profile_t *p = build_profiles_find (profiles, view->name);
    const char *problem = NULL;
    gboolean moved;

    // its name and directory stay while it is built or set up
    if (p != NULL && (strcmp (name, p->name) != 0 || g_strcmp0 (dir, p->dir) != 0)
        && build_busy_with (build, view->root, p->name))
    {
        g_free (name);
        g_free (dir);
        g_free (command);
        g_free (test);
        build_profiles_free (profiles);
        return FALSE;
    }
    if (p == NULL)
        problem = _ ("The profile is no more.");
    else if (*name == '\0' || strchr (name, '[') != NULL || strchr (name, ']') != NULL)
        problem = _ ("A profile needs a name, without [ and ].");
    else if (strcmp (name, p->name) != 0 && build_profiles_find (profiles, name) != NULL)
        problem = _ ("There is a profile of that name.");
    else if (*dir == '\0')
        problem = _ ("A profile needs a build directory.");
    else if ((p->system == BUILD_MESON || p->system == BUILD_CMAKE)
             && build_profile_dir_owner (profiles, p, view->root, dir) != NULL)
        problem = _ ("Another profile builds in that directory.");
    if (problem != NULL)
    {
        build->host->message (build->host, D_ERROR, _ ("Build"), problem);
        g_free (name);
        g_free (dir);
        g_free (command);
        g_free (test);
        build_profiles_free (profiles);
        return FALSE;
    }
    // a new name: the profiles that name it name the new one
    if (strcmp (name, p->name) != 0)
    {
        guint i;

        if (g_strcmp0 (profiles->active, p->name) == 0)
        {
            g_free (profiles->active);
            profiles->active = g_strdup (name);
        }
        if (g_strcmp0 (profiles->coverage, p->name) == 0)
        {
            g_free (profiles->coverage);
            profiles->coverage = g_strdup (name);
        }
        for (i = 0; i < profiles->list->len; i++)
        {
            build_profile_t *o = (build_profile_t *) g_ptr_array_index (profiles->list, i);

            if (g_strcmp0 (o->from, p->name) == 0)
            {
                g_free (o->from);
                o->from = g_strdup (name);
            }
        }
        g_free (p->name);
        p->name = g_strdup (name);
    }
    moved = g_strcmp0 (p->dir, dir) != 0;
    // a command of its own follows the directory where it surely names it
    if (moved && *command != '\0' && p->dir != NULL)
    {
        char *follows = build_profile_follow_dir (command, view->root, p->dir, dir);

        g_free (command);
        command = follows;
    }
    // the test command of its system follows the directory, one of the user's own stays
    if (moved && *test != '\0')
    {
        char *old_abs = build_profile_dir (p, view->root);
        char *new_abs = g_canonicalize_filename (dir, view->root);
        char *follows = build_profile_test_follow (test, p->system, view->root, old_abs, new_abs);

        if (follows != NULL)
        {
            g_free (test);
            test = follows;
        }
        g_free (new_abs);
        g_free (old_abs);
    }
    g_free (p->dir);
    p->dir = dir;
    {
        // the system of the directory when it is set up, else the one it had
        char *abs = build_profile_dir (p, view->root);
        const build_system_t system = build_system_of_dir (abs);

        if (system != BUILD_NONE)
            p->system = system;
        g_free (abs);
    }
    g_free (p->build);
    p->build = *command != '\0' ? command : (g_free (command), NULL);
    g_free (p->test);
    p->test = *test != '\0' ? test : (g_free (test), NULL);
    p->test_after_build = view->after->state;
    // not written: not saved, the window stays with what is in it (the user was told why)
    if (!build_profiles_save (build, view->root, profiles))
    {
        g_free (name);
        build_profiles_free (profiles);
        return FALSE;
    }

    g_free (view->name);
    view->name = name;
    g_free (view->old_dir);
    view->old_dir = g_strdup (p->dir);
    g_free (view->old_build);
    view->old_build = g_strdup (p->build != NULL ? p->build : "");
    g_free (view->old_test);
    view->old_test = g_strdup (p->test != NULL ? p->test : "");
    view->old_after = p->test_after_build;
    view->system = p->system;
    view->saved = TRUE;
    // the fields show what is saved: a test command that followed the directory, the spaces cut
    input_assign_text (view->in_name, view->name);
    input_assign_text (view->in_dir, view->old_dir);
    input_assign_text (view->in_build, view->old_build);
    input_assign_text (view->in_test, view->old_test);
    if (moved)
    {
        // another directory: its options
        g_free (view->dir);
        view->dir = build_profile_dir (p, view->root);
        g_free (view->where);
        view->where = build_profile_relative (view->root, view->dir);
        build_view_load (view);
    }
    build_profiles_free (profiles);
    return TRUE;
}

/* --------------------------------------------------------------------------------------------- */

/* Save the fields, apply the changes of the options, and close the window when it is done:
   now when there is nothing to apply, else once the setup ends well */
static void
build_view_save (build_options_view_t *view)
{
    if (view->state == BUILD_VIEW_APPLYING || !build_view_save_fields (view))
        return;
    if (g_hash_table_size (view->changes) == 0)
    {
        view->dlg->ret_value = B_ENTER;
        dlg_close (view->dlg);
        return;
    }
    view->close_after = TRUE;
    build_view_apply (view);
}

/* --------------------------------------------------------------------------------------------- */

static cb_ret_t
build_view_callback (Widget *w, Widget *sender, widget_msg_t msg, int parm, void *data)
{
    WDialog *dlg = DIALOG (w);
    build_options_view_t *view = (build_options_view_t *) dlg->data.p;
    gboolean in_table;

    if (view == NULL)
        return dlg_default_callback (w, sender, msg, parm, data);
    in_table = widget_get_state (WIDGET (view->table), WST_FOCUSED);
    switch (msg)
    {
    case MSG_IDLE:
        /* the output of the setup has ended before its process: it is waited for here, a little
           at a time, nothing else polling it while the window is up */
        if (view->state == BUILD_VIEW_APPLYING && build_running (view->build))
        {
            g_usleep (10 * 1000);
            return MSG_HANDLED;
        }
        widget_idle (w, FALSE);
        // the setup the window waited for is done well
        if (view->closing)
        {
            dlg->ret_value = B_ENTER;
            dlg_close (dlg);
        }
        return MSG_HANDLED;
    case MSG_CHANGED_FOCUS:
        // the lines under the table say what has the focus
        build_view_show (view);
        break;
    case MSG_NOTIFY:
        if (sender != WIDGET (view->table))
            break;
        // Enter saves, as in the fields: F4 changes the option
        if (parm == CK_Enter)
        {
            build_view_save (view);
            build_view_show (view);
            build_options_view_redraw (view);
            return MSG_HANDLED;
        }
        build_view_show (view);
        return MSG_HANDLED;
    case MSG_UNHANDLED_KEY:
        switch (parm)
        {
        case KEY_F (4):
            if (!in_table)
                return MSG_NOT_HANDLED;
            build_view_change (view);
            break;
        case KEY_DC:
        {
            const build_option_t *o = build_view_option (view, table_get_current (view->table));

            if (!in_table)
                return MSG_NOT_HANDLED;
            if (o == NULL || view->state == BUILD_VIEW_APPLYING)
                return MSG_HANDLED;
            (void) g_hash_table_remove (view->changes, o->name);
            if (g_hash_table_size (view->changes) == 0)
                view->state = BUILD_VIEW_SHOWN;
            break;
        }
        case KEY_F (3):
            if (!in_table)
                return MSG_NOT_HANDLED;
            view->mode = (view->mode + 1) % BUILD_VIEW_MODES;
            build_view_fill (view, NULL);
            break;
        case KEY_F (2):
            // the fields first: the options go to the directory they name
            if (view->state != BUILD_VIEW_APPLYING && build_view_save_fields (view))
                build_view_apply (view);
            break;
        default:
            return MSG_NOT_HANDLED;
        }
        build_view_show (view);
        build_options_view_redraw (view);
        return MSG_HANDLED;
    case MSG_ACTION:
        // Enter in the fields saves them, and applies the options
        if (parm == CK_Ok)
        {
            build_view_save (view);
            build_view_show (view);
            build_options_view_redraw (view);
            return MSG_HANDLED;
        }
        // what is not saved is not lost unasked; a setup going on goes on without the window
        if (parm == CK_Cancel && view->state != BUILD_VIEW_APPLYING
            && (g_hash_table_size (view->changes) > 0 || build_view_fields_changed (view)))
        {
            switch (query_dialog (_ ("Build"), _ ("The profile is changed, not saved."), D_NORMAL,
                                  3, _ ("&Save"), _ ("&Discard"), _ ("&Cancel")))
            {
            case 0:
                build_view_save (view);
                build_view_show (view);
                build_options_view_redraw (view);
                return MSG_HANDLED;
            case 1:
                break;
            default:
                return MSG_HANDLED;
            }
        }
        break;
    default:
        break;
    }
    return dlg_default_callback (w, sender, msg, parm, data);
}

/* --------------------------------------------------------------------------------------------- */

/* The window of the profile @name: its name, directory, commands, and under them the options of
   its build directory, changed and applied while it stays: the setup goes into Build output, the
   options read again from the directory after it.  A profile @is_new is gone when the window is
   closed without saving it.  TRUE when it was saved; its name then in @saved_as, NULL for none,
   which it may have changed. */
static gboolean
build_profile_window (build_t *build, const char *root, const char *name, gboolean is_new,
                      char **saved_as)
{
    build_profiles_t *profiles = build_profiles_of (root);
    const build_profile_t *p = build_profiles_find (profiles, name);
    static const table_column_def_t defs[BUILD_VIEW_COLS] = {
        { 1, J_CENTER, TABLE_COL_TEXT },
        { 0, J_LEFT, TABLE_COL_TEXT },
        { 0, J_LEFT, TABLE_COL_TEXT },
        { 0, J_LEFT, TABLE_COL_TEXT },
    };
    const char *titles[BUILD_VIEW_COLS + 1] = { "", _ ("Name"), _ ("Value"), _ ("Default"), NULL };
    const char *l_name = _ ("Name:"), *l_dir = _ ("Directory:"), *l_build = _ ("Build command:");
    const char *l_test = _ ("Test command:");
    build_options_view_t view = { 0 };
    table_datasource_t ds = { 0 };
    WGroup *g;
    WHLine *search;
    int dlg_w, dlg_h, x, lw;
    gboolean saved;

    if (p == NULL)
    {
        build_profiles_free (profiles);
        return FALSE;
    }
    // one window of a profile at a time: the setup it starts tells that one its end
    if (build->view != NULL)
    {
        build->host->message (build->host, D_ERROR, _ ("Build"),
                              _ ("The window of a build profile is open already."));
        // one made for the window is not made
        if (is_new && build_profiles_remove (profiles, name))
            (void) build_profiles_save (build, root, profiles);
        build_profiles_free (profiles);
        return FALSE;
    }
    view.build = build;
    view.root = g_strdup (root);
    view.name = g_strdup (p->name);
    view.dir = build_profile_dir (p, root);
    view.system = p->system;
    view.changes = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
    view.rows = g_array_new (FALSE, FALSE, sizeof (guint));
    view.introspect_fd = -1;
    view.introspect = g_string_new (NULL);
    view.where = build_profile_relative (root, view.dir);
    view.is_new = is_new;
    view.old_dir = g_strdup (p->dir != NULL ? p->dir : "");
    view.old_build = g_strdup (p->build != NULL ? p->build : "");
    view.old_test = g_strdup (p->test != NULL ? p->test : "");
    view.old_after = p->test_after_build;

    dlg_w = MIN (COLS - 4, 100);
    dlg_h = MIN (LINES - 2, 34);
    view.dlg = dlg_create (TRUE, 0, 0, dlg_h, dlg_w, WPOS_CENTER, TRUE, dialog_colors,
                           build_view_callback, NULL, "[Build profile]", _ ("Build profile"));
    view.dlg->help_file = "build.md";
    view.dlg->data.p = &view;
    g = GROUP (view.dlg);
    // the fields: the labels of a width, the name and the directory on one line
    lw = MAX (str_term_width1 (l_build), str_term_width1 (l_test)) + 1;
    group_add_widget (g, label_new (1, 2, l_name));
    view.in_name =
        input_new (1, 2 + lw, input_colors, 20, p->name, "build-profile-name", INPUT_COMPLETE_NONE);
    group_add_widget (g, view.in_name);
    x = 2 + lw + 20 + 2;
    group_add_widget (g, label_new (1, x, l_dir));
    x += str_term_width1 (l_dir) + 1;
    view.in_dir = input_new (1, x, input_colors, MAX (dlg_w - x - 2, 10), view.old_dir,
                             "build-profile-dir", INPUT_COMPLETE_FILENAMES | INPUT_COMPLETE_CD);
    group_add_widget (g, view.in_dir);
    group_add_widget (g, label_new (2, 2, l_build));
    view.in_build = input_new (2, 2 + lw, input_colors, dlg_w - lw - 4, view.old_build,
                               "build-command", INPUT_COMPLETE_COMMANDS);
    group_add_widget (g, view.in_build);
    group_add_widget (g, label_new (3, 2, l_test));
    view.in_test = input_new (3, 2 + lw, input_colors, dlg_w - lw - 4, view.old_test,
                              "build-profile-test", INPUT_COMPLETE_COMMANDS);
    group_add_widget (g, view.in_test);
    view.after = check_new (4, 2, view.old_after, _ ("&Run the tests after the build"));
    group_add_widget (g, view.after);
    build_profiles_free (profiles);

    // the options: which of them on the line over them, the rows, the search, the option
    view.mode_line = hline_new (5, -1, -1);
    group_add_widget (g, view.mode_line);
    view.table = table_new (6, 1, MAX (dlg_h - 10, 2), dlg_w - 2, BUILD_VIEW_COLS, defs);
    table_set_column_sizing (view.table, 0, 1, FALSE);
    table_set_column_sizing (view.table, 1, 20, TRUE);
    table_set_column_sizing (view.table, 2, 12, TRUE);
    table_set_column_sizing (view.table, 3, 8, TRUE);
    ds.get_nrows = build_view_rows;
    ds.get_text = build_view_cell;
    ds.data = &view;
    table_set_datasource (view.table, ds);
    table_set_header (view.table, titles);
    table_set_cell_color (view.table, build_view_cell_color);
    view.table->scrollbar = TRUE;
    // letters narrow the list to the names that have them in their order
    table_set_quick_search (view.table, 1, TRUE, FALSE);
    table_set_quick_filter_fuzzy (view.table, TRUE, NULL);
    search = hline_new (dlg_h - 4, -1, -1);
    search->text_align = J_LEFT;
    table_set_quick_search_label (view.table, search);
    group_add_widget (g, view.table);
    group_add_widget (g, search);
    view.about = label_new (dlg_h - 3, 2, "");
    view.choices = label_new (dlg_h - 2, 2, "");
    group_add_widget (g, view.about);
    group_add_widget (g, view.choices);

    build->view = &view;
    build_view_introspect (&view);
    build_view_load (&view);
    table_layout (view.table);
    // a profile just made is named first, one there has its options looked at
    widget_select (is_new ? WIDGET (view.in_name) : WIDGET (view.table));
    build_view_show (&view);
    (void) dlg_run (view.dlg);
    build->view = NULL;
    if (view.introspect_fd >= 0)
    {
        delete_select_channel (view.introspect_fd);
        close (view.introspect_fd);
    }
    widget_destroy (WIDGET (view.dlg));
    saved = view.saved;
    if (saved_as != NULL)
        *saved_as = saved ? g_strdup (view.name) : NULL;
    if (is_new && !saved)
    {
        // closed unsaved: not made
        profiles = build_profiles_of (root);
        if (build_profiles_remove (profiles, view.name))
            (void) build_profiles_save (build, root, profiles);
        build_profiles_free (profiles);
    }
    if (view.options != NULL)
        g_ptr_array_free (view.options, TRUE);
    g_hash_table_destroy (view.changes);
    g_array_free (view.rows, TRUE);
    g_string_free (view.introspect, TRUE);
    g_free (view.root);
    g_free (view.name);
    g_free (view.dir);
    g_free (view.where);
    g_free (view.old_dir);
    g_free (view.old_build);
    g_free (view.old_test);
    return saved;
}

/* --------------------------------------------------------------------------------------------- */

/* The cells of the table of the profiles, a row of BUILD_PROFILE_COLS for each */
#define BUILD_PROFILE_COLS 5

static int
build_profiles_rows (const void *data)
{
    return (int) (((const GPtrArray *) data)->len / BUILD_PROFILE_COLS);
}

static const char *
build_profiles_cell (const void *data, int row, int col)
{
    const GPtrArray *cells = (const GPtrArray *) data;
    const guint at = (guint) (row * BUILD_PROFILE_COLS + col);

    return at < cells->len ? (const char *) g_ptr_array_index (cells, at) : "";
}

/* What the keys of the window of the profiles do to the one of the cursor, as in Far */
enum
{
    BUILD_B_NEW = B_USER,
    BUILD_B_EDIT,
    BUILD_B_DELETE,
    BUILD_B_BUILD,
    BUILD_B_ACTIVATE
};

/* Space makes the profile of the cursor the active one, Enter too and closes the window, Ins
   makes a new one from it, F4 edits it and its options, Del deletes it, F5 builds it */
static cb_ret_t
build_profiles_dialog_callback (Widget *w, Widget *sender, widget_msg_t msg, int parm, void *data)
{
    int ret = B_CANCEL;

    if (msg == MSG_NOTIFY && parm == CK_Enter)
        ret = B_ENTER;
    else if (msg == MSG_UNHANDLED_KEY)
        switch (parm)
        {
        case KEY_IC:
            ret = BUILD_B_NEW;
            break;
        case KEY_F (4):
            ret = BUILD_B_EDIT;
            break;
        case KEY_DC:
        case KEY_F (8):
            ret = BUILD_B_DELETE;
            break;
        case KEY_F (5):
            ret = BUILD_B_BUILD;
            break;
        case ' ':
            ret = BUILD_B_ACTIVATE;
            break;
        default:
            break;
        }
    if (ret == B_CANCEL)
        return dlg_default_callback (w, sender, msg, parm, data);
    DIALOG (w)->ret_value = ret;
    dlg_close (DIALOG (w));
    return MSG_HANDLED;
}

/* --------------------------------------------------------------------------------------------- */

/* The profiles of the project in one dialog, by keys: the active one is marked; Enter makes the
   one of the cursor it, Ins makes one from it, F4 and Del change it, F5 builds it
   and leaves the active one.  The dialog comes back after all but Enter and F5 */
static mc_ep_result_t
build_act_profiles (void *data, void *edit)
{
    build_t *build = (build_t *) data;
    char *root = build_root_of (build, edit);
    guint cursor = 0;

    while (TRUE)
    {
        build_profiles_t *profiles = build_profiles_of (root);
        const build_profile_t *active = build_profiles_active (profiles);
        // room for a dozen profiles, more as many as the screen holds
        const int rows_h = MIN (MAX ((int) profiles->list->len, 12), MAX (LINES - 8, 3));
        const int dlg_w = MIN (COLS - 4, 78);
        // the frame, the header and the rows, the frame
        const int dlg_h = rows_h + 3;
        static const table_column_def_t defs[BUILD_PROFILE_COLS] = {
            { 1, J_CENTER, TABLE_COL_TEXT }, { 0, J_LEFT, TABLE_COL_TEXT },
            { 0, J_LEFT, TABLE_COL_TEXT },   { 0, J_LEFT, TABLE_COL_TEXT },
            { 0, J_LEFT, TABLE_COL_TEXT },
        };
        const char *titles[BUILD_PROFILE_COLS + 1] = {
            "", _ ("Name"), _ ("Directory"), _ ("System"), _ ("Built for"), NULL,
        };
        GPtrArray *cells = g_ptr_array_new_with_free_func (g_free);
        table_datasource_t ds = { 0 };
        WDialog *dlg;
        WTable *table;
        build_profile_t *p = NULL;
        int ret, row;
        guint i;

        for (i = 0; i < profiles->list->len; i++)
        {
            const build_profile_t *q =
                (const build_profile_t *) g_ptr_array_index (profiles->list, i);
            char *where, *what;

            build_profile_describe (q, root, &where, &what);
            g_ptr_array_add (cells, g_strdup (q == active ? "*" : ""));
            g_ptr_array_add (cells, g_strdup (q->name));
            g_ptr_array_add (cells, where);
            g_ptr_array_add (cells, g_strdup (build_system_name (q->system)));
            g_ptr_array_add (cells, what);
        }

        dlg = dlg_create (TRUE, 0, 0, dlg_h, dlg_w, WPOS_CENTER, TRUE, dialog_colors,
                          build_profiles_dialog_callback, NULL, "[Build profiles]",
                          _ ("Build profiles"));
        dlg->help_file = "build.md";
        frame_set_footer (FRAME (dlg->bg), "F4 F5 Ins Del Space");
        table = table_new (1, 1, rows_h + 1, dlg_w - 2, BUILD_PROFILE_COLS, defs);
        table_set_column_sizing (table, 0, 1, FALSE);
        table_set_column_sizing (table, 1, 8, FALSE);
        table_set_column_sizing (table, 2, 12, TRUE);
        table_set_column_sizing (table, 3, 6, FALSE);
        table_set_column_sizing (table, 4, 10, TRUE);
        ds.get_nrows = build_profiles_rows;
        ds.get_text = build_profiles_cell;
        ds.data = cells;
        table_set_datasource (table, ds);
        table_set_header (table, titles);
        table->scrollbar = TRUE;
        table_set_current (table, (int) MIN (cursor, profiles->list->len - 1));
        group_add_widget (GROUP (dlg), table);
        widget_select (WIDGET (table));
        ret = dlg_run (dlg);
        row = table_get_current (table);
        if (row >= 0 && (guint) row < profiles->list->len)
        {
            cursor = (guint) row;
            p = (build_profile_t *) g_ptr_array_index (profiles->list, cursor);
        }
        widget_destroy (WIDGET (dlg));
        g_ptr_array_free (cells, TRUE);

        switch (ret)
        {
        case B_ENTER:
        case BUILD_B_ACTIVATE:
            if (p != NULL)
            {
                g_free (profiles->active);
                profiles->active = g_strdup (p->name);
                (void) build_profiles_save (build, root, profiles);
            }
            // Space stays in the window, the mark on the profile made active
            if (ret == BUILD_B_ACTIVATE)
            {
                build_profiles_free (profiles);
                continue;
            }
            break;
        case BUILD_B_NEW:
            if (build_profile_new (build, root, profiles, p))
                cursor = profiles->list->len - 1;
            build_profiles_free (profiles);
            continue;
        case BUILD_B_EDIT:
            if (p != NULL)
            {
                char *name = g_strdup (p->name);

                build_profiles_free (profiles);
                (void) build_profile_window (build, root, name, FALSE, NULL);
                g_free (name);
            }
            else
                build_profiles_free (profiles);
            continue;
        case BUILD_B_DELETE:
            if (p != NULL && !build_busy_with (build, root, p->name)
                && query_dialog (_ ("Build"), _ ("Delete the profile? Its build directory stays."),
                                 D_NORMAL, 2, _ ("&Delete"), _ ("&Cancel"))
                    == 0)
            {
                char *gone = g_strdup (p->name);

                (void) build_profiles_remove (profiles, gone);
                (void) build_profiles_save (build, root, profiles);
                g_free (gone);
                if (cursor > 0)
                    cursor--;
            }
            build_profiles_free (profiles);
            continue;
        case BUILD_B_BUILD:
            if (p != NULL)
            {
                char *name = g_strdup (p->name);

                build->quiet = FALSE;
                (void) build_start (build, root, name);
                g_free (name);
            }
            break;
        default:
            break;
        }
        build_profiles_free (profiles);
        break;
    }
    g_free (root);
    return MC_EPR_OK;
}

/* --------------------------------------------------------------------------------------------- */

static mc_ep_result_t
build_act_configure (void *data, void *edit)
{
    build_t *build = (build_t *) data;
    char *root = build_root_of (build, edit);
    build_profiles_t *profiles = build_profiles_of (root);
    const build_profile_t *p = build_profiles_active (profiles);
    char *name = p != NULL ? g_strdup (p->name) : NULL;

    // the window of the active profile: its command, its tests, its options
    build_profiles_free (profiles);
    if (name != NULL)
    {
        // a Default made from what was found is kept first, the window reads it from the file
        build_profiles_keep_default (build, root);
        (void) build_profile_window (build, root, name, FALSE, NULL);
    }
    g_free (name);
    g_free (root);
    return MC_EPR_OK;
}

/* The project of @root takes a copy of the preset of its language, .bluerig/project.ini, when it
   has none, or again when @again; the command the user gave it stays but @again, that bluerig kept
   before in the configuration of the user too.  FALSE when no preset fits it, or when the file is
   there but cannot be read: it is left as it is. */
static gboolean
build_project_adopt (build_t *build, const char *root, gboolean again)
{
    GKeyFile *keyfile = g_key_file_new ();
    GPtrArray *presets = NULL;
    const build_preset_t *preset = NULL;
    gboolean has = FALSE;
    gboolean readable, done = FALSE;

    /* the command= of before, from which Default is made while the user has no profiles, goes
       with the copy made again: Default is kept first, the command with it */
    if (again)
        build_profiles_keep_default (build, root);
    readable = build_project_read (build, root, keyfile, &has);

    if (readable && !again && has
        && g_key_file_has_key (keyfile, BUILD_PROJECT_GROUP, "language", NULL))
        done = TRUE;
    else if (readable)
    {
        // the presets alone, not the copy there is; by the markers, else by the file in front
        char *file = build_front_file (build, root);

        presets = build_presets_load (build, NULL);
        preset = build_presets_find (presets, root, file);
        g_free (file);
    }
    if (preset != NULL)
    {
        build_info_t *info = build_detect (root);
        // a build system of its own builds the project, not the command of the language
        char *text = build_presets_project_text (preset, info == NULL);
        GKeyFile *copy = g_key_file_new ();
        char **groups = g_key_file_get_groups (keyfile, NULL);
        guint i, j;

        (void) g_key_file_load_from_data (copy, text, (gsize) -1, G_KEY_FILE_KEEP_COMMENTS, NULL);
        // the command the user gave it, and what else the file has, stay
        for (i = 0; groups[i] != NULL; i++)
        {
            char **keys = g_key_file_get_keys (keyfile, groups[i], NULL, NULL);
            const gboolean own = strcmp (groups[i], BUILD_PROJECT_GROUP) == 0;

            for (j = 0; keys != NULL && keys[j] != NULL; j++)
                if (!own || (!again && strcmp (keys[j], "command") == 0))
                {
                    char *value = g_key_file_get_value (keyfile, groups[i], keys[j], NULL);

                    g_key_file_set_value (copy, groups[i], keys[j], value);
                    g_free (value);
                }
            g_strfreev (keys);
        }
        g_strfreev (groups);
        if (!again && !g_key_file_has_key (copy, BUILD_PROJECT_GROUP, "command", NULL))
        {
            char *command = build_legacy_command (root);

            if (command != NULL)
                g_key_file_set_string (copy, BUILD_PROJECT_GROUP, "command", command);
            g_free (command);
        }
        done = build_project_write (build, root, copy);
        g_key_file_free (copy);
        g_free (text);
        build_info_free (info);
    }
    if (presets != NULL)
        g_ptr_array_free (presets, TRUE);
    g_key_file_free (keyfile);
    return done;
}

/* --------------------------------------------------------------------------------------------- */

/* The project of the file in front back to the preset of its language */
static mc_ep_result_t
build_act_defaults (void *data, void *edit)
{
    build_t *build = (build_t *) data;
    char *root = build_root_of (build, edit);
    GKeyFile *keyfile = g_key_file_new ();
    gboolean there;
    const gboolean readable = build_project_read (build, root, keyfile, &there);
    char *text_value;

    g_key_file_free (keyfile);
    // a file that cannot be read is put right by the user, not written over
    if (!readable)
    {
        g_free (root);
        return MC_EPR_FAILED;
    }
    if (query_dialog (_ ("Build"),
                      _ ("Make .bluerig/project.ini of the project the preset of its language\n"
                         "again, as presets.ini has it?  The build command given to the project\n"
                         "goes; its debug configurations stay, until they are made anew."),
                      D_NORMAL, 2, _ ("&Yes"), _ ("&No"))
        != 0)
    {
        g_free (root);
        return MC_EPR_FAILED;
    }
    if (build_project_adopt (build, root, TRUE))
        text_value = g_strdup_printf (_ ("%s/%s/%s\nhas the preset of the language again."), root,
                                      BUILD_PROJECT_DIR, BUILD_PROJECT_FILE);
    else
        text_value = g_strdup_printf (_ ("No preset of presets.ini fits\n%s"), root);
    build->host->message (build->host, D_NORMAL, _ ("Build"), text_value);
    g_free (text_value);
    g_free (root);
    return MC_EPR_OK;
}

/* --------------------------------------------------------------------------------------------- */

/* A project opened as one, not that of a file: it takes the preset of its language */
static void
build_project_changed (const char *name, const char *signal, GVariant *args, void *user_data)
{
    build_t *build = (build_t *) user_data;
    const char *root = NULL;
    gboolean chosen = FALSE;

    (void) name;
    if (strcmp (signal, "changed") != 0 || args == NULL)
        return;
    (void) g_variant_lookup (args, "root", "&s", &root);
    (void) g_variant_lookup (args, "chosen", "b", &chosen);
    // an empty root: the project closed, nothing to adopt
    if (root != NULL && *root != '\0' && chosen)
        (void) build_project_adopt (build, root, FALSE);
}

static void
build_project_startup (void *data)
{
    build_t *build = (build_t *) data;

    (void) build_project_adopt (build, build->host->startup_option (build->host, "debug"), FALSE);
}

/* --------------------------------------------------------------------------------------------- */
/* The service */
/* --------------------------------------------------------------------------------------------- */

static GVariant *
build_call (void *data, const char *method, GVariant *args, GError **error)
{
    build_t *build = (build_t *) data;
    GVariantDict reply;
    const char *root = NULL, *path = NULL, *project = NULL, *profile = NULL;

    if (args != NULL)
    {
        (void) g_variant_lookup (args, "root", "&s", &root);
        (void) g_variant_lookup (args, "path", "&s", &path);
        (void) g_variant_lookup (args, "project", "&s", &project);
        // a profile of the project by its name, the active one when not given
        (void) g_variant_lookup (args, "profile", "&s", &profile);
    }
    // a profile named that is not there is an error, not another profile quietly
    if (profile != NULL && root != NULL
        && (strcmp (method, "info") == 0 || strcmp (method, "run") == 0
            || strcmp (method, "configure") == 0 || strcmp (method, "window") == 0))
    {
        build_profiles_t *profiles = build_profiles_of (root);
        const gboolean there = build_profiles_find (profiles, profile) != NULL;

        build_profiles_free (profiles);
        if (!there)
        {
            g_set_error (error, MC_SERVICE_ERROR, MC_SERVICE_ERROR_ARGS,
                         "build: no profile %s in the project", profile);
            return NULL;
        }
    }
    g_variant_dict_init (&reply, NULL);

    if (strcmp (method, "info") == 0 && root != NULL)
    {
        // how the project is built, by its profile, and the programs it has built
        build_profiles_t *profiles = build_profiles_of (root);
        const build_profile_t *p = profile != NULL ? build_profiles_find (profiles, profile)
                                                   : build_profiles_active (profiles);
        const build_system_t system = p != NULL ? p->system : BUILD_NONE;
        char *command = NULL, *dir = NULL;
        GPtrArray *programs;

        (void) build_plan (build, root, p != NULL ? p->name : NULL, &command, &dir);
        if (command != NULL)
            g_variant_dict_insert (&reply, "command", "s", command);
        g_variant_dict_insert (&reply, "dir", "s", dir);
        g_variant_dict_insert (&reply, "system", "s", build_system_name (system));
        if (p != NULL)
            g_variant_dict_insert (&reply, "profile", "s", p->name);
        // what the build directory is built for: buildtype, coverage, sanitize
        if (system != BUILD_NONE)
        {
            GPtrArray *options = build_options_read (dir, system);

            if (options != NULL)
            {
                build_summary_t *summary = build_summary_of (options, system);

                if (summary->buildtype != NULL)
                    g_variant_dict_insert (&reply, "buildtype", "s", summary->buildtype);
                g_variant_dict_insert (&reply, "coverage", "b", summary->coverage);
                if (summary->sanitize != NULL)
                    g_variant_dict_insert (&reply, "sanitize", "s", summary->sanitize);
                build_summary_free (summary);
                g_ptr_array_free (options, TRUE);
            }
        }
        programs = build_find_programs (dir);
        g_variant_dict_insert_value (
            &reply, "programs",
            g_variant_new_strv ((const gchar *const *) programs->pdata, (gssize) programs->len));
        g_ptr_array_free (programs, TRUE);
        build_profiles_free (profiles);
        g_free (command);
        g_free (dir);
    }
    else if (strcmp (method, "preset") == 0 && (root != NULL || path != NULL))
    {
        // the language of the project of root, else of the file path, as the copy of the project
        // has it, else presets.ini; project: the project whose copy comes first, root not given
        GPtrArray *presets = build_presets_load (build, root != NULL ? root : project);
        const build_preset_t *preset = build_presets_find (presets, root, path);

        if (preset != NULL)
        {
            const char *const strings[][2] = {
                { "build", preset->build },     { "adapter", preset->adapter },
                { "address", preset->address }, { "launch", preset->launch },
                { "entry", preset->entry },
            };
            guint i;

            g_variant_dict_insert (&reply, "id", "s", preset->id);
            for (i = 0; i < G_N_ELEMENTS (strings); i++)
                if (strings[i][1] != NULL)
                    g_variant_dict_insert (&reply, strings[i][0], "s", strings[i][1]);
            // the first gdb there, else the first named, which the start says is not there
            for (i = 0; preset->gdb != NULL && preset->gdb[i] != NULL; i++)
            {
                char *found = g_find_program_in_path (preset->gdb[i]);
                const gboolean there = found != NULL;

                g_free (found);
                if (there)
                    break;
            }
            if (preset->gdb != NULL)
                g_variant_dict_insert (&reply, "gdb", "s",
                                       preset->gdb[preset->gdb[i] != NULL ? i : 0]);
            if (preset->program != NULL && root != NULL)
            {
                char *program = build_presets_expand (preset->program, root);

                if (program != NULL)
                {
                    char *absolute = g_canonicalize_filename (program, root);

                    g_variant_dict_insert (&reply, "program", "s", absolute);
                    g_free (absolute);
                }
                g_free (program);
            }
            if (preset->hide != NULL)
                g_variant_dict_insert_value (
                    &reply, "hide", g_variant_new_strv ((const gchar *const *) preset->hide, -1));
            if (preset->extensions != NULL)
                g_variant_dict_insert_value (
                    &reply, "extensions",
                    g_variant_new_strv ((const gchar *const *) preset->extensions, -1));
        }
        g_ptr_array_free (presets, TRUE);
    }
    else if (strcmp (method, "elf") == 0 && path != NULL)
    {
        build_elf_t elf;

        if (!build_elf_read (path, &elf))
            memset (&elf, 0, sizeof (elf));
        g_variant_dict_insert (&reply, "executable", "b", elf.executable);
        g_variant_dict_insert (&reply, "debug_info", "b", elf.debug_info);
        g_variant_dict_insert (&reply, "optimized", "b", elf.optimized);
    }
    else if (strcmp (method, "run") == 0 && root != NULL)
    {
        gboolean started;

        // "tests": the tests run after the build, whatever the profile says
        (void) g_variant_lookup (args, "tests", "b", &build->with_tests);
        started = build_start (build, root, profile);
        build->with_tests = FALSE;
        build->quiet = started;
        g_variant_dict_insert (&reply, "started", "b", started);
        // the profile it builds: "finished" names it, the active one may be another by then
        if (started && build->profile != NULL)
            g_variant_dict_insert (&reply, "profile", "s", build->profile);
    }
    else if (strcmp (method, "configure") == 0 && root != NULL)
    {
        /* the directory of the profile set up, the active one when not given, with "setup" the
           options it changes, "-Db_coverage=true"; set up as the profile says when it is not
           yet.  "finished" tells the end, its "step" "configure". */
        const char *setup = NULL;
        gboolean started;

        (void) g_variant_lookup (args, "setup", "&s", &setup);
        started =
            build_configure (build, root, profile, setup != NULL && *setup != '\0' ? setup : NULL);
        g_variant_dict_insert (&reply, "started", "b", started);
    }
    else if (strcmp (method, "profiles") == 0 && root != NULL)
    {
        // the names of the profiles, the active one, that of coverage
        build_profiles_t *profiles = build_profiles_of (root);
        const build_profile_t *active = build_profiles_active (profiles);
        GPtrArray *names = g_ptr_array_new ();
        guint i;

        for (i = 0; i < profiles->list->len; i++)
            g_ptr_array_add (names,
                             ((build_profile_t *) g_ptr_array_index (profiles->list, i))->name);
        g_variant_dict_insert_value (
            &reply, "names",
            g_variant_new_strv ((const gchar *const *) names->pdata, (gssize) names->len));
        if (active != NULL)
            g_variant_dict_insert (&reply, "active", "s", active->name);
        if (profiles->coverage != NULL)
            g_variant_dict_insert (&reply, "coverage", "s", profiles->coverage);
        g_ptr_array_free (names, TRUE);
        build_profiles_free (profiles);
    }
    else if (strcmp (method, "create") == 0 && root != NULL && profile != NULL)
    {
        /* a profile @profile made from "from", the active one when not given, in "dir", else
           next to that of the other one; "setup" the options it adds to those of the other one,
           "-Db_coverage=true"; "coverage" makes it the profile of coverage.  Its directory is set
           up by its first build. */
        build_profiles_t *profiles = build_profiles_of (root);
        const char *from_name = NULL, *dir = NULL, *setup = NULL;
        gboolean coverage = FALSE;
        const build_profile_t *from;
        build_profile_t *p;
        char *offer = NULL;

        (void) g_variant_lookup (args, "from", "&s", &from_name);
        (void) g_variant_lookup (args, "dir", "&s", &dir);
        (void) g_variant_lookup (args, "setup", "&s", &setup);
        (void) g_variant_lookup (args, "coverage", "b", &coverage);
        from = from_name != NULL ? build_profiles_find (profiles, from_name)
                                 : build_profiles_active (profiles);
        // a profile to make it from that is not there: an error, not a profile of nothing
        if (from_name != NULL && from == NULL)
        {
            build_profiles_free (profiles);
            g_variant_dict_clear (&reply);
            g_set_error (error, MC_SERVICE_ERROR, MC_SERVICE_ERROR_ARGS,
                         "build: no profile %s in the project", from_name);
            return NULL;
        }
        if (dir == NULL)
            dir = offer = build_profile_offer_dir (profiles, from, root, profile);
        // a directory of another profile is not shared: its setup would not be done again
        else if (build_profile_dir_owner (profiles, NULL, root, dir) != NULL)
        {
            g_variant_dict_insert (&reply, "ok", "b", FALSE);
            g_variant_dict_insert (&reply, "error", "s",
                                   _ ("Another profile builds in that directory."));
            build_profiles_free (profiles);
            return g_variant_dict_end (&reply);
        }
        p = build_profiles_add (profiles, root, profile, from, dir);
        if (p != NULL)
        {
            char *from_dir = from != NULL ? build_profile_dir (from, root) : NULL;
            char *abs = build_profile_dir (p, root);

            if (p->setup == NULL && from_dir != NULL)
                p->setup = build_profile_setup_of (from_dir, from->system);
            if (setup != NULL && *setup != '\0')
            {
                char *both = build_profile_setup_merge (p->setup, setup);

                g_free (p->setup);
                p->setup = both;
            }
            if (p->test == NULL)
                p->test = build_profile_test_command (p->system, abs);
            if (coverage)
            {
                g_free (profiles->coverage);
                profiles->coverage = g_strdup (p->name);
            }
            g_variant_dict_insert (&reply, "ok", "b", build_profiles_save (build, root, profiles));
            g_variant_dict_insert (&reply, "dir", "s", abs);
            g_free (abs);
            g_free (from_dir);
        }
        else
            g_variant_dict_insert (&reply, "ok", "b", FALSE);
        g_free (offer);
        build_profiles_free (profiles);
    }
    else if (strcmp (method, "running") == 0)
        g_variant_dict_insert (&reply, "running", "b", build_running (build));
    else
    {
        g_variant_dict_clear (&reply);
        g_set_error (error, MC_SERVICE_ERROR, MC_SERVICE_ERROR_METHOD,
                     "build: no method %s, or not its arguments", method);
        return NULL;
    }
    return g_variant_dict_end (&reply);
}

/* --------------------------------------------------------------------------------------------- */

static const mc_ep_window_kind_t build_output_kind;

static void *
build_open (mc_editor_host_t *host, void *editor_dialog)
{
    build_t *build = g_new0 (build_t, 1);
    int i;

    (void) editor_dialog;
    build->host = host;
    build->output_fd = -1;
    build->output = g_string_new (NULL);
    build->pending = g_string_new (NULL);
    build->partial = g_string_new (NULL);
    build->diagnostics = g_ptr_array_new_with_free_func (build_diagnostic_free_cb);
    build->notes = g_ptr_array_new_with_free_func (build_note_free_cb);
    build->list = g_string_new (NULL);
    build->rows = g_array_new (FALSE, FALSE, sizeof (int));
    build->current = -1;
    host->commands_register (host, BUILD_KEYMAP_SECTION, N_ ("&Build"), build_commands);
    // Ctrl held, the bar says what Ctrl-F9 does
    if (host->command_label != NULL)
        host->command_label (host, build_commands[BUILD_CMD_RUN].name, N_ ("ButtonBar|Build"));
    if (host->window_kind != NULL)
        host->window_kind (host, &build_output_kind, build);
    for (i = 0; i < BUILD_CMD_COUNT; i++)
        build->commands[i] = host->command_id (host, build_commands[i].name);
    for (i = 0; i < BUILD_MARK_COUNT; i++)
        build->marks[i] = host->marker_kind (host, &build_mark_kinds[i]);
    build->service = host->service_register (host, BUILD_SERVICE, build_call, build, NULL);
    if (host->service_connect != NULL)
    {
        build->project_signal =
            host->service_connect (host, "project", build_project_changed, build);
        build->viewer_signal = host->service_connect (host, "viewer", build_viewer_signal, build);
    }
    // bluerig -D: the project is there before this plugin hears of it; once the editor runs, which
    // can tell what is wrong
    if (host->startup_option (host, "debug") != NULL)
        host->call_later (host, build_project_startup, build);
    return build;
}

/* --------------------------------------------------------------------------------------------- */

static void
build_close (void *data)
{
    build_t *build = (build_t *) data;

    // the key is gone with the plugin: the bar shows the editor's there again
    if (build->host->command_label != NULL)
        build->host->command_label (build->host, build_commands[BUILD_CMD_RUN].name, NULL);
    if (build->output_fd >= 0)
    {
        delete_select_channel (build->output_fd);
        close (build->output_fd);
    }
    if (build->pid != 0)
    {
        int i;

        // all of the build, the shell and what it started; killed if it does not end
        (void) kill (-build->pid, SIGTERM);
        for (i = 0; i < 50 && waitpid (build->pid, NULL, WNOHANG) == 0; i++)
            g_usleep (10000);
        if (i == 50)
        {
            (void) kill (-build->pid, SIGKILL);
            (void) waitpid (build->pid, NULL, 0);
        }
        g_spawn_close_pid (build->pid);
    }
    if (build->service)
        build->host->service_unregister (build->host, BUILD_SERVICE);
    if (build->project_signal != 0)
        build->host->service_disconnect (build->host, build->project_signal);
    if (build->viewer_signal != 0)
        build->host->service_disconnect (build->host, build->viewer_signal);
    g_ptr_array_free (build->diagnostics, TRUE);
    g_ptr_array_free (build->notes, TRUE);
    g_free (build->note_file);
    g_string_free (build->list, TRUE);
    g_array_free (build->rows, TRUE);
    g_string_free (build->output, TRUE);
    g_free (build->title);
    g_free (build->profile);
    g_string_free (build->pending, TRUE);
    g_string_free (build->partial, TRUE);
    g_free (build->root);
    g_free (build->command);
    g_free (build->dir);
    g_free (build->presets_told);
    g_free (build->changes);
    g_free (build->defaults_root);
    g_free (build->defaults);
    g_free (build);
}

/* --------------------------------------------------------------------------------------------- */

static gboolean
build_ok_to_quit (void *data)
{
    build_t *build = (build_t *) data;

    return !build_running (build)
        || query_dialog (_ ("Build"), _ ("Stop the build and quit?"), D_NORMAL, 2, _ ("&Stop"),
                         _ ("&Cancel"))
        == 0;
}

/* --------------------------------------------------------------------------------------------- */

/* The window of the output of the build, again */
static mc_ep_result_t
build_act_output (void *data, void *edit)
{
    build_t *build = (build_t *) data;

    /* the options may have changed since the last build: the title found again, of the profile
       the output is of; before any build, of the active one of the project in front */
    {
        char *root = build->root != NULL ? g_strdup (build->root) : build_root_of (build, edit);

        g_free (build->title);
        build->title = build_output_title (root, build->root != NULL ? build->profile : NULL);
        g_free (root);
    }
    if (build->output->len == 0)
        g_string_assign (build->output, _ ("No build yet: Ctrl-F9 builds the project.\n"));
    build->quiet = FALSE;
    build_output_show (build);
    return MC_EPR_OK;
}

/* The output in the menu Window */
static mc_ep_window_state_t
build_output_state (void *data)
{
    build_t *build = (build_t *) data;
    GVariantDict dict;
    GVariant *reply;
    gboolean visible = FALSE, focused = FALSE;

    if (build->window == 0)
        return MC_EP_WINDOW_CLOSED;
    g_variant_dict_init (&dict, NULL);
    g_variant_dict_insert (&dict, "id", "x", build->window);
    reply = build_viewer (build, "info", &dict);
    if (reply == NULL)
    {
        // closed by the user
        build->window = 0;
        return MC_EP_WINDOW_CLOSED;
    }
    (void) g_variant_lookup (reply, "visible", "b", &visible);
    (void) g_variant_lookup (reply, "focused", "b", &focused);
    g_variant_unref (reply);
    return !visible ? MC_EP_WINDOW_CLOSED : focused ? MC_EP_WINDOW_FOCUSED : MC_EP_WINDOW_OPEN;
}

static void
build_output_raise (void *data)
{
    build_t *build = (build_t *) data;
    GVariantDict dict;
    GVariant *reply;

    (void) build_act_output (data, NULL);
    if (build->window == 0)
        return;
    g_variant_dict_init (&dict, NULL);
    g_variant_dict_insert (&dict, "id", "x", build->window);
    g_variant_dict_insert (&dict, "focus", "b", TRUE);
    reply = build_viewer (build, "show", &dict);
    if (reply != NULL)
        g_variant_unref (reply);
}

static void
build_output_close (void *data)
{
    build_t *build = (build_t *) data;
    GVariantDict dict;
    GVariant *reply;

    if (build->window == 0)
        return;
    g_variant_dict_init (&dict, NULL);
    g_variant_dict_insert (&dict, "id", "x", build->window);
    reply = build_viewer (build, "close", &dict);
    if (reply != NULL)
        g_variant_unref (reply);
    build->window = 0;
}

static void *
build_output_window (void *data)
{
    build_t *build = (build_t *) data;
    GVariantDict dict;
    GVariant *reply;
    guint64 window = 0;

    if (build_output_state (data) == MC_EP_WINDOW_CLOSED)
        return NULL;
    g_variant_dict_init (&dict, NULL);
    g_variant_dict_insert (&dict, "id", "x", build->window);
    reply = build_viewer (build, "info", &dict);
    if (reply != NULL)
    {
        (void) g_variant_lookup (reply, "window", "t", &window);
        g_variant_unref (reply);
    }
    return (void *) (gsize) window;
}

static const mc_ep_window_kind_t build_output_kind = {
    .name = "build.output",
    .window = build_output_window,
    .label = N_ ("&Build output"),
    .state = build_output_state,
    .show = build_output_raise,
    .close = build_output_close,
};

static const mc_ep_action_t build_actions[] = {
    { "Build", build_act_run },
    { "Next error", build_act_next },
    { "Previous error", build_act_prev },
    { "Configure", build_act_configure },
    { "Profiles", build_act_profiles },
    { "Output", build_act_output },
    { "Language defaults", build_act_defaults },
};

/* The key of a menu entry, that of the command of its action */
static char *
build_menu_shortcut (int action_index)
{
    int cmd;

    switch (action_index)
    {
    case BUILD_ACT_RUN:
        cmd = BUILD_CMD_RUN;
        break;
    case BUILD_ACT_NEXT:
        cmd = BUILD_CMD_NEXT;
        break;
    case BUILD_ACT_PREV:
        cmd = BUILD_CMD_PREV;
        break;
    default:
        return NULL;
    }
    return keymap_menu_shortcut (BUILD_KEYMAP_SECTION, build_commands[cmd].name);
}

static const mc_ep_cmd_menu_entry_t build_menu[] = {
    { MC_EP_MENU_PROJECT, NULL, 0, NULL },
    { MC_EP_MENU_PROJECT, N_ ("B&uild project"), BUILD_ACT_RUN, NULL },
    { MC_EP_MENU_PROJECT, N_ ("Build profile&s..."), BUILD_ACT_PROFILES, NULL },
    { MC_EP_MENU_PROJECT, N_ ("Bu&ild output"), BUILD_ACT_OUTPUT, NULL },
    { MC_EP_MENU_PROJECT, N_ ("&Next build error"), BUILD_ACT_NEXT, NULL },
    { MC_EP_MENU_PROJECT, N_ ("&Previous build error"), BUILD_ACT_PREV, NULL },
    { MC_EP_MENU_PROJECT, N_ ("&Language defaults of the project..."), BUILD_ACT_DEFAULTS, NULL },
};

static const mc_editor_plugin_t build_plugin = {
    .api_version = MC_EDITOR_PLUGIN_API_VERSION,
    .name = "build",
    .display_name = N_ ("Build"),
    .flags = MC_EPF_NONE,
    .open = build_open,
    .close = build_close,
    .handle_key = build_handle_key,
    .handle_action = build_handle_action,
    .on_file_open = build_file_open,
    .ok_to_quit = build_ok_to_quit,
    .actions = build_actions,
    .action_count = G_N_ELEMENTS (build_actions),
    .cmd_menu_entries = build_menu,
    .cmd_menu_entry_count = G_N_ELEMENTS (build_menu),
    .get_menu_shortcut = build_menu_shortcut,
    .help_file = "build.md",
    .help_node = "build",
    .summary = N_ ("Builds the project and goes to the errors and warnings of the compiler"),
};

/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

const mc_editor_plugin_t *
build_get_plugin (void)
{
    return &build_plugin;
}

/* --------------------------------------------------------------------------------------------- */
