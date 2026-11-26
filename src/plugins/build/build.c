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
} build_t;

/*** forward declarations (file scope functions) *************************************************/

static void build_list_cursor (build_t *build);
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

/* The command the user gives the project: that of its active profile, kept */
static void
build_save_command (build_t *build, const char *root, const char *command)
{
    build_profiles_t *profiles = build_profiles_of (root);
    build_profile_t *p = build_profiles_active (profiles);

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

    *command = p != NULL ? build_profile_command (p, root) : NULL;
    *dir = p != NULL ? build_profile_dir (p, root) : g_strdup (root);
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

static void
build_finished (build_t *build, int status)
{
    guint errors = 0, i;
    char *summary;

    for (i = 0; i < build->diagnostics->len; i++)
        if (((build_diagnostic_t *) g_ptr_array_index (build->diagnostics, i))->error)
            errors++;
    if (WIFEXITED (status) && WEXITSTATUS (status) == 0)
        summary =
            g_strdup_printf (_ ("\nBuilt. Warnings: %u.\n"), build->diagnostics->len - errors);
    else
        summary = g_strdup_printf (_ ("\nThe build failed. Errors: %u, warnings: %u.\n"), errors,
                                   build->diagnostics->len - errors);
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
    if (build->quiet && WIFEXITED (status) && WEXITSTATUS (status) == 0 && build->window != 0)
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
        g_variant_dict_insert (&dict, "ok", "b", WIFEXITED (status) && WEXITSTATUS (status) == 0);
        g_variant_dict_insert (&dict, "errors", "i", (gint32) errors);
        build->host->service_emit (build->host, BUILD_SERVICE, "finished",
                                   g_variant_dict_end (&dict));
    }
    widget_draw (WIDGET (build->host->host_data));
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

/* Build the project of @root by its profile @name, NULL for the active one; FALSE when it could
   not start */
static gboolean
build_start (build_t *build, const char *root, const char *name)
{
    char *command = NULL, *dir = NULL, *shell_command;
    char *argv[4];
    GError *error = NULL;
    int out;

    if (build_running (build))
    {
        build->host->message (build->host, D_NORMAL, _ ("Build"), _ ("The build is running."));
        return FALSE;
    }
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
        build_save_command (build, root, command);
    }
    if (build->host->save_modified_files != NULL
        && !build->host->save_modified_files (build->host, root))
    {
        g_free (command);
        g_free (dir);
        return FALSE;
    }

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

/* The form of the profile @p of @profiles: its name, directory, commands; changed on OK, FALSE
   on Cancel */
static gboolean
build_profile_form (build_t *build, const char *root, build_profiles_t *profiles,
                    build_profile_t *p)
{
    char *title = g_strdup_printf (_ ("Build profile: %s"), p->name);
    gboolean done = FALSE;

    while (!done)
    {
        char *name = NULL, *dir = NULL, *command = NULL, *test = NULL;
        gboolean after = p->test_after_build;
        const char *problem = NULL;
        int ret;
        quick_widget_t widgets[] = {
            QUICK_LABELED_INPUT (_ ("Name:"), input_label_above, p->name, "build-profile-name",
                                 &name, NULL, FALSE, FALSE, INPUT_COMPLETE_NONE),
            QUICK_LABELED_INPUT (_ ("Build directory, from the root of the project:"),
                                 input_label_above, p->dir != NULL ? p->dir : "",
                                 "build-profile-dir", &dir, NULL, FALSE, FALSE,
                                 INPUT_COMPLETE_FILENAMES | INPUT_COMPLETE_CD),
            QUICK_LABELED_INPUT (_ ("Build command (empty: that of the build system):"),
                                 input_label_above, p->build != NULL ? p->build : "",
                                 "build-command", &command, NULL, FALSE, FALSE,
                                 INPUT_COMPLETE_COMMANDS),
            QUICK_LABELED_INPUT (_ ("Test command:"), input_label_above,
                                 p->test != NULL ? p->test : "", "build-profile-test", &test, NULL,
                                 FALSE, FALSE, INPUT_COMPLETE_COMMANDS),
            QUICK_CHECKBOX (_ ("&Run the tests after the build"), &after, NULL),
            QUICK_BUTTONS_OK_CANCEL,
            QUICK_END,
        };
        WRect r = { -1, -1, 0, MIN (76, COLS - 4) };
        quick_dialog_t qdlg = {
            .rect = r,
            .title = title,
            .help = "[Build]",
            .help_file = "build.md",
            .widgets = widgets,
            .callback = NULL,
            .mouse_callback = NULL,
        };

        ret = quick_dialog (&qdlg);
        if (ret == B_CANCEL || ret == B_EXIT)
        {
            g_free (name);
            g_free (dir);
            g_free (command);
            g_free (test);
            break;
        }
        (void) g_strstrip (name);
        (void) g_strstrip (dir);
        if (*name == '\0' || strchr (name, '[') != NULL || strchr (name, ']') != NULL)
            problem = _ ("A profile needs a name, without [ and ].");
        else if (strcmp (name, p->name) != 0 && build_profiles_find (profiles, name) != NULL)
            problem = _ ("There is a profile of that name.");
        else if (*dir == '\0')
            problem = _ ("A profile needs a build directory.");
        if (problem != NULL)
        {
            build->host->message (build->host, D_ERROR, _ ("Build"), problem);
            g_free (name);
            g_free (dir);
            g_free (command);
            g_free (test);
            continue;
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
            p->name = name;
        }
        else
            g_free (name);
        g_free (p->dir);
        p->dir = dir;
        {
            // the system of the directory when it is set up, else the one it had
            char *abs = build_profile_dir (p, root);
            const build_system_t system = build_system_of_dir (abs);

            if (system != BUILD_NONE)
                p->system = system;
            g_free (abs);
        }
        g_free (p->build);
        p->build = *g_strstrip (command) != '\0' ? command : (g_free (command), NULL);
        g_free (p->test);
        p->test = *g_strstrip (test) != '\0' ? test : (g_free (test), NULL);
        p->test_after_build = after;
        done = TRUE;
    }
    g_free (title);
    return done;
}

/* --------------------------------------------------------------------------------------------- */

/* A new profile made from @from, its name asked; its form after it; FALSE when none is made */
static gboolean
build_profile_new (build_t *build, const char *root, build_profiles_t *profiles,
                   const build_profile_t *from)
{
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
    if (!build_profile_form (build, root, profiles, p))
    {
        char *gone = g_strdup (p->name);

        (void) build_profiles_remove (profiles, gone);
        g_free (gone);
        return FALSE;
    }
    return TRUE;
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
   makes a new one from it, F4 edits it, Del deletes it, F5 builds it */
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

/* The profiles of the project in one dialog, by keys: the active one is marked; Space and Enter
   make the one of the cursor it, Ins makes one from it, F4 and Del change it, F5 builds it and
   leaves the active one.  The dialog comes back after all but Enter and F5 */
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
            {
                (void) build_profiles_save (build, root, profiles);
                cursor = profiles->list->len - 1;
            }
            build_profiles_free (profiles);
            continue;
        case BUILD_B_EDIT:
            if (p != NULL && build_profile_form (build, root, profiles, p))
                (void) build_profiles_save (build, root, profiles);
            build_profiles_free (profiles);
            continue;
        case BUILD_B_DELETE:
            if (p != NULL
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
    char *command = p != NULL ? build_profile_build_command (p, root) : NULL;
    char *chosen;

    // the command of the active profile, its tests left out: they run after it by themselves
    chosen = input_dialog (_ ("Build"), _ ("Command that builds the project:"), "build-command",
                           command != NULL ? command : "make", INPUT_COMPLETE_COMMANDS);
    if (chosen != NULL)
        build_save_command (build, root, chosen);
    build_profiles_free (profiles);
    g_free (chosen);
    g_free (command);
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
    const gboolean readable = build_project_read (build, root, keyfile, &has);
    gboolean done = FALSE;

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
        const gboolean started = build_start (build, root, profile);

        build->quiet = started;
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
