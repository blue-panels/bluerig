/*
   src/plugins/debugger - tests for the configurations of the debugger: kept and read back, the
   environment, the adapter, and the program a new one suggests

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

#include <config.h>

/* -Dassert=false sets G_DISABLE_ASSERT in config.h: g_test_init () would then end the test and
   g_assert () check nothing, and the checks are the point of a test */
#undef G_DISABLE_ASSERT

#include <sys/stat.h>

#include "src/plugins/debugger/debug-config.c"  // for its file scope functions

/* --------------------------------------------------------------------------------------------- */
/* The rest of the plugin, as much as the configurations ask of it */

static GPtrArray *errors;  // what the user is told: debug_error () and the messages

void
debug_error (debugger_t *debug, const char *message_text)
{
    (void) debug;
    g_ptr_array_add (errors, g_strdup (message_text));
}

GVariant *
debug_build_call (debugger_t *debug, const char *method, const char *key, const char *value)
{
    GVariantDict args;

    if (debug->host->service_call == NULL)
        return NULL;
    g_variant_dict_init (&args, NULL);
    g_variant_dict_insert (&args, key, "s", value);
    return debug->host->service_call (debug->host, "build", method, g_variant_dict_end (&args),
                                      NULL);
}

static int synced;

void
debug_breakpoints_sync (debugger_t *debug)
{
    (void) debug;
    synced++;
}

void
debug_marks_show (debugger_t *debug, const char *file)
{
    (void) debug;
    (void) file;
}

void
debug_requests_clear (debugger_t *debug)
{
    (void) debug;
}

void
debug_pty_close (debugger_t *debug)
{
    (void) debug;
}

void
debug_clear_current (debugger_t *debug)
{
    (void) debug;
}

void
debug_notes_clear (debugger_t *debug)
{
    (void) debug;
}

void
debug_output_show (debugger_t *debug)
{
    (void) debug;
}

void
debug_session_refresh (debugger_t *debug)
{
    (void) debug;
}

/* --------------------------------------------------------------------------------------------- */
/* The editor, and the build plugin by its service */

static char *current_file;  // the file in front
static GVariant *build_info;
static GVariant *build_preset;
static GVariant *build_elf;
static GPtrArray *preset_paths;  // the "path" each asking for a preset gave, "" for none

static void
host_message (mc_editor_host_t *host, int flags, const char *title, const char *text)
{
    (void) host;
    (void) flags;
    (void) title;
    g_ptr_array_add (errors, g_strdup (text));
}

static void *
host_window_top_file (mc_editor_host_t *host)
{
    (void) host;
    return current_file != NULL ? (void *) &current_file : NULL;
}

static char *
host_get_current_file (mc_editor_host_t *host, void *edit)
{
    (void) host;
    g_assert_true (edit == (void *) &current_file);
    return g_strdup (current_file);
}

static GVariant *
host_service_call (mc_editor_host_t *host, const char *name, const char *method, GVariant *args,
                   GError **error)
{
    GVariant *reply = NULL;

    (void) host;
    (void) error;
    g_variant_ref_sink (args);
    if (strcmp (name, "build") == 0 && strcmp (method, "info") == 0)
        reply = build_info;
    else if (strcmp (name, "build") == 0 && strcmp (method, "elf") == 0)
        reply = build_elf;
    else if (strcmp (name, "build") == 0 && strcmp (method, "preset") == 0)
    {
        const char *path = "";

        (void) g_variant_lookup (args, "path", "&s", &path);
        g_ptr_array_add (preset_paths, g_strdup (path));
        reply = build_preset;
    }
    g_variant_unref (args);
    return reply != NULL ? g_variant_ref (reply) : NULL;
}

static mc_editor_host_t host = {
    .message = host_message,
    .get_current_file = host_get_current_file,
    .window_top_file = host_window_top_file,
    .service_call = host_service_call,
};

/* --------------------------------------------------------------------------------------------- */

static char *config_home;  // XDG_CONFIG_HOME of the test, set before the first look at it
static char *root;         // the project, new for each test
static debugger_t *debug;

static void
breakpoint_free (gpointer data)
{
    debug_breakpoint_t *bp = (debug_breakpoint_t *) data;

    g_free (bp->file);
    g_free (bp->condition);
    g_free (bp);
}

static void
watch_free (gpointer data)
{
    debug_watch_t *watch = (debug_watch_t *) data;

    g_free (watch->expression);
    g_free (watch);
}

static void
remove_tree (const char *path)
{
    GDir *dir = g_dir_open (path, 0, NULL);
    const char *name;

    if (dir != NULL)
    {
        while ((name = g_dir_read_name (dir)) != NULL)
        {
            char *child = g_build_filename (path, name, (char *) NULL);

            remove_tree (child);
            g_free (child);
        }
        g_dir_close (dir);
    }
    (void) g_remove (path);
}

/* @text into the file @name of the project, its directories made */
static char *
project_file (const char *name, const char *text, int mode)
{
    char *path = g_build_filename (root, name, (char *) NULL);
    char *dir = g_path_get_dirname (path);

    g_assert_cmpint (g_mkdir_with_parents (dir, 0755), ==, 0);
    g_assert_true (g_file_set_contents (path, text, -1, NULL));
    g_assert_cmpint (g_chmod (path, mode), ==, 0);
    g_free (dir);
    return path;
}

static void
setup (void)
{
    errors = g_ptr_array_new_with_free_func (g_free);
    preset_paths = g_ptr_array_new_with_free_func (g_free);
    synced = 0;
    root = g_dir_make_tmp ("debug-config-XXXXXX", NULL);
    g_assert_nonnull (root);
    debug = g_new0 (debugger_t, 1);
    debug->host = &host;
    debug->launches = g_ptr_array_new_with_free_func (debug_launch_free);
    debug->breakpoints = g_ptr_array_new_with_free_func (breakpoint_free);
    debug->address_breakpoints = g_ptr_array_new ();
    debug->watches = g_ptr_array_new_with_free_func (watch_free);
    debug->frames = g_ptr_array_new ();
}

static void
teardown (void)
{
    g_ptr_array_free (debug->launches, TRUE);
    g_ptr_array_free (debug->breakpoints, TRUE);
    g_ptr_array_free (debug->address_breakpoints, TRUE);
    g_ptr_array_free (debug->watches, TRUE);
    g_ptr_array_free (debug->frames, TRUE);
    g_free (debug->project_dir);
    g_free (debug);
    g_clear_pointer (&build_info, g_variant_unref);
    g_clear_pointer (&build_preset, g_variant_unref);
    g_clear_pointer (&build_elf, g_variant_unref);
    g_clear_pointer (&current_file, g_free);
    g_ptr_array_free (preset_paths, TRUE);
    g_ptr_array_free (errors, TRUE);
    remove_tree (root);
    g_free (root);
    // the settings of the user, of every project
    {
        char *dir = g_build_filename (config_home, "bluerig", (char *) NULL);

        remove_tree (dir);
        g_free (dir);
    }
}

/* The user's file of the project, where debug_config_path () says */
static char *
user_config (void)
{
    char *hash = g_compute_checksum_for_string (G_CHECKSUM_SHA256, root, -1);
    char *name = g_strconcat (hash, ".ini", NULL);
    char *path = g_build_filename (config_home, "bluerig", "debug", name, (char *) NULL);

    g_free (name);
    g_free (hash);
    return path;
}

static void
write_user_config (const char *text)
{
    char *path = user_config ();
    char *dir = g_path_get_dirname (path);

    g_assert_cmpint (g_mkdir_with_parents (dir, 0700), ==, 0);
    g_assert_true (g_file_set_contents (path, text, -1, NULL));
    g_free (dir);
    g_free (path);
}

static int
file_mode (const char *path)
{
    struct stat st;

    g_assert_cmpint (stat (path, &st), ==, 0);
    return (int) (st.st_mode & 0777);
}

static debug_launch_t *
launch_new (const char *name, const char *executable, const char *directory)
{
    debug_launch_t *launch = g_new0 (debug_launch_t, 1);

    launch->name = g_strdup (name);
    launch->executable = g_strdup (executable);
    launch->directory = g_strdup (directory);
    return launch;
}

static void
breakpoint_add (const char *file, long line, gboolean disabled, const char *condition)
{
    debug_breakpoint_t *bp = g_new0 (debug_breakpoint_t, 1);

    bp->file = g_strdup (file);
    bp->line = line;
    bp->disabled = disabled;
    bp->condition = g_strdup (condition);
    g_ptr_array_add (debug->breakpoints, bp);
}

static debug_launch_t *
launch_at (guint i)
{
    g_assert_cmpuint (i, <, debug->launches->len);
    return g_ptr_array_index (debug->launches, i);
}

static debug_breakpoint_t *
breakpoint_at (guint i)
{
    g_assert_cmpuint (i, <, debug->breakpoints->len);
    return g_ptr_array_index (debug->breakpoints, i);
}

/* The project closed and opened again: all it has is read from the files */
static void
reopen (void)
{
    g_assert_cmpint (debug_project_switch (debug, NULL), ==, MC_EPR_OK);
    g_assert_null (debug->project_dir);
    g_assert_cmpuint (debug->launches->len, ==, 0);
    g_assert_cmpuint (debug->breakpoints->len, ==, 0);
    g_assert_cmpint (debug_project_switch (debug, g_strdup (root)), ==, MC_EPR_OK);
    g_assert_cmpstr (debug->project_dir, ==, root);
}

/* --------------------------------------------------------------------------------------------- */

/* The configurations of the user: all of a configuration, the breakpoints, their conditions and
   the disabled ones, the watches; the file the user's alone */
static void
test_user_config (void)
{
    char *exe, *path;
    debug_launch_t *launch;
    const char *const exceptions[] = { "throw", NULL };

    setup ();
    exe = g_build_filename (root, "build", "app", (char *) NULL);
    g_assert_cmpint (debug_project_switch (debug, g_strdup (root)), ==, MC_EPR_OK);
    // a project new to the debugger keeps them in it
    g_assert_true (debug->launches_in_project);
    debug->launches_in_project = FALSE;

    launch = launch_new ("app", exe, "/tmp");
    launch->arguments = g_strdup ("-v 'a b'");
    launch->environment = g_strdup ("A=1 B=2");
    launch->gdb_path = g_strdup ("rust-gdb");
    launch->build = TRUE;
    launch->terminal = FALSE;
    launch->exceptions = g_strdupv ((char **) exceptions);
    g_ptr_array_add (debug->launches, launch);
    launch = launch_new ("script", "/opt/s.py", root);
    launch->backend = g_strdup ("dap");
    launch->adapter = g_strdup ("python3 -m debugpy.adapter");
    launch->address = g_strdup ("127.0.0.1:5678");
    launch->launch_extra = g_strdup ("{\"justMyCode\": false}");
    launch->exceptions = g_new0 (char *, 1);
    g_ptr_array_add (debug->launches, launch);
    debug->active_launch = 1;
    breakpoint_add ("/src/a.c", 7, FALSE, NULL);
    breakpoint_add ("/src/a.c", 9, TRUE, "x > 1");
    breakpoint_add ("/src/b:c.c", 12, FALSE, "s == \"a;b\"");
    g_ptr_array_add (debug->watches, g_new0 (debug_watch_t, 1));
    ((debug_watch_t *) g_ptr_array_index (debug->watches, 0))->expression = g_strdup ("p->next");

    debug_config_save (debug);
    g_assert_cmpint (synced, ==, 1);
    path = user_config ();
    g_assert_cmpint (file_mode (path), ==, 0600);
    g_free (path);
    // nothing in the project
    path = g_build_filename (root, ".bluerig", "debug.ini", (char *) NULL);
    g_assert_false (g_file_test (path, G_FILE_TEST_EXISTS));
    g_free (path);

    reopen ();
    g_assert_false (debug->launches_in_project);
    g_assert_cmpuint (debug->launches->len, ==, 2);
    g_assert_cmpuint (debug->active_launch, ==, 1);
    launch = launch_at (0);
    g_assert_cmpstr (launch->name, ==, "app");
    g_assert_cmpstr (launch->executable, ==, exe);
    g_assert_cmpstr (launch->arguments, ==, "-v 'a b'");
    g_assert_cmpstr (launch->directory, ==, "/tmp");
    g_assert_cmpstr (launch->environment, ==, "A=1 B=2");
    g_assert_cmpstr (launch->gdb_path, ==, "rust-gdb");
    g_assert_true (launch->build);
    g_assert_false (launch->terminal);
    g_assert_cmpstr (launch->backend, ==, "gdb-mi");
    g_assert_null (launch->adapter);
    g_assert_cmpuint (g_strv_length (launch->exceptions), ==, 1);
    g_assert_cmpstr (launch->exceptions[0], ==, "throw");
    g_assert_null (launch->exception_filters);
    launch = launch_at (1);
    g_assert_cmpstr (launch->backend, ==, "dap");
    g_assert_cmpstr (launch->adapter, ==, "python3 -m debugpy.adapter");
    g_assert_cmpstr (launch->address, ==, "127.0.0.1:5678");
    g_assert_cmpstr (launch->launch_extra, ==, "{\"justMyCode\": false}");
    g_assert_cmpstr (launch->gdb_path, ==, "gdb");
    // none of them, not those of the debugger
    g_assert_nonnull (launch->exceptions);
    g_assert_null (launch->exceptions[0]);

    g_assert_cmpuint (debug->breakpoints->len, ==, 3);
    g_assert_cmpstr (breakpoint_at (0)->file, ==, "/src/a.c");
    g_assert_cmpint (breakpoint_at (0)->line, ==, 7);
    g_assert_false (breakpoint_at (0)->disabled);
    g_assert_null (breakpoint_at (0)->condition);
    g_assert_true (breakpoint_at (1)->disabled);
    g_assert_cmpstr (breakpoint_at (1)->condition, ==, "x > 1");
    g_assert_cmpstr (breakpoint_at (2)->file, ==, "/src/b:c.c");
    g_assert_cmpint (breakpoint_at (2)->line, ==, 12);
    g_assert_cmpstr (breakpoint_at (2)->condition, ==, "s == \"a;b\"");
    g_assert_cmpuint (debug->watches->len, ==, 1);
    g_assert_cmpstr (((debug_watch_t *) g_ptr_array_index (debug->watches, 0))->expression, ==,
                     "p->next");
    g_assert_cmpuint (errors->len, ==, 0);
    teardown ();
    g_free (exe);
}

/* The configurations in the project: names from its root, for all who work on it, and made
   whole when read; the breakpoints stay the user's */
static void
test_project_config (void)
{
    char *exe, *shared_path, *text, *path;
    GKeyFile *keyfile;

    setup ();
    exe = g_build_filename (root, "build", "app", (char *) NULL);
    g_assert_cmpint (debug_project_switch (debug, g_strdup (root)), ==, MC_EPR_OK);
    g_assert_true (debug->launches_in_project);
    g_ptr_array_add (debug->launches, launch_new ("app", exe, root));
    // a name out of the project stays as it is
    g_ptr_array_add (debug->launches, launch_new ("tool", "/usr/bin/tool", "/var/tmp"));
    breakpoint_add ("/src/a.c", 7, FALSE, NULL);
    debug_config_save (debug);

    shared_path = g_build_filename (root, ".bluerig", "debug.ini", (char *) NULL);
    g_assert_cmpint (file_mode (shared_path), ==, 0644);
    keyfile = g_key_file_new ();
    g_assert_true (g_key_file_load_from_file (keyfile, shared_path, G_KEY_FILE_NONE, NULL));
    text = g_key_file_get_string (keyfile, "Launch 0", "executable", NULL);
    g_assert_cmpstr (text, ==, "build/app");
    g_free (text);
    text = g_key_file_get_string (keyfile, "Launch 0", "directory", NULL);
    g_assert_cmpstr (text, ==, ".");
    g_free (text);
    text = g_key_file_get_string (keyfile, "Launch 1", "executable", NULL);
    g_assert_cmpstr (text, ==, "/usr/bin/tool");
    g_free (text);
    g_assert_false (g_key_file_has_key (keyfile, "Debug", "breakpoints", NULL));
    g_key_file_free (keyfile);
    // the user's own file has the breakpoints, not the configurations
    keyfile = g_key_file_new ();
    path = user_config ();
    g_assert_true (g_key_file_load_from_file (keyfile, path, G_KEY_FILE_NONE, NULL));
    g_assert_true (g_key_file_get_boolean (keyfile, "Debug", "in_project", NULL));
    g_assert_false (g_key_file_has_group (keyfile, "Launch 0"));
    g_assert_true (g_key_file_has_key (keyfile, "Debug", "breakpoints", NULL));
    g_key_file_free (keyfile);
    g_free (path);

    reopen ();
    g_assert_true (debug->launches_in_project);
    g_assert_cmpuint (debug->launches->len, ==, 2);
    g_assert_cmpstr (launch_at (0)->executable, ==, exe);
    g_assert_cmpstr (launch_at (0)->directory, ==, root);
    g_assert_cmpstr (launch_at (1)->executable, ==, "/usr/bin/tool");
    g_assert_cmpuint (debug->breakpoints->len, ==, 1);

    // Keep in the project turned off: the user's own, the project's aside
    debug->launches_in_project = FALSE;
    g_ptr_array_set_size (debug->launches, 0);
    g_ptr_array_add (debug->launches, launch_new ("mine", "/bin/mine", "/"));
    debug_config_save (debug);
    reopen ();
    g_assert_false (debug->launches_in_project);
    g_assert_cmpuint (debug->launches->len, ==, 1);
    g_assert_cmpstr (launch_at (0)->name, ==, "mine");
    teardown ();
    g_free (shared_path);
    g_free (exe);
}

/* What a hand or an older version may have written: the one configuration of the first version,
   lines of breakpoints that are none, configurations with no name or program, an active one out
   of range */
static void
test_odd_config (void)
{
    char *text;

    setup ();
    write_user_config ("[Debug]\nexecutable=/bin/old\narguments=-x\n");
    g_assert_cmpint (debug_project_switch (debug, g_strdup (root)), ==, MC_EPR_OK);
    g_assert_cmpuint (debug->launches->len, ==, 1);
    g_assert_cmpstr (launch_at (0)->name, ==, "Default");
    g_assert_cmpstr (launch_at (0)->executable, ==, "/bin/old");
    g_assert_cmpstr (launch_at (0)->arguments, ==, "-x");
    g_assert_cmpstr (launch_at (0)->directory, ==, root);
    // they are the user's: the project is not new to the debugger
    g_assert_false (debug->launches_in_project);

    text = g_strdup_printf ("[Debug]\nin_project=false\nlaunch_count=3\nactive_launch=7\n"
                            "breakpoints=nofile;/a.c:;/a.c:0;/a.c:x;/a.c:-3;/a.c:5x;"
                            "/a:b.c:12;/c.c:3;\n"
                            "breakpoints_disabled=/c.c:3;/none.c:1;\n"
                            "breakpoint_conditions=/c.c:3;;/a:b.c:12;n > 0;/c.c:3;\n"
                            "watches=;x;\n"
                            "[Launch 0]\nexecutable=/bin/x\n"
                            "[Launch 1]\nname=nothing to run\n"
                            "[Launch 2]\nname=rel\nexecutable=sub/../bin/y\ndirectory=\n");
    g_assert_cmpint (debug_project_switch (debug, NULL), ==, MC_EPR_OK);
    write_user_config (text);
    g_free (text);
    g_assert_cmpint (debug_project_switch (debug, g_strdup (root)), ==, MC_EPR_OK);
    g_assert_cmpuint (debug->launches->len, ==, 1);
    g_assert_cmpstr (launch_at (0)->name, ==, "rel");
    {
        char *exe = g_build_filename (root, "bin", "y", (char *) NULL);

        // from the root of the project, made whole; no directory: the root
        g_assert_cmpstr (launch_at (0)->executable, ==, exe);
        g_assert_cmpstr (launch_at (0)->directory, ==, root);
        g_free (exe);
    }
    // no key of the terminal: one from before there was a terminal for the program has one
    g_assert_true (launch_at (0)->terminal);
    g_assert_cmpuint (debug->active_launch, ==, 0);
    g_assert_cmpuint (debug->breakpoints->len, ==, 2);
    g_assert_cmpstr (breakpoint_at (0)->file, ==, "/a:b.c");
    g_assert_cmpint (breakpoint_at (0)->line, ==, 12);
    g_assert_cmpstr (breakpoint_at (0)->condition, ==, "n > 0");
    g_assert_false (breakpoint_at (0)->disabled);
    g_assert_cmpstr (breakpoint_at (1)->file, ==, "/c.c");
    g_assert_true (breakpoint_at (1)->disabled);
    // an empty condition is none; the list ends on a location with no condition after it
    g_assert_null (breakpoint_at (1)->condition);
    g_assert_cmpuint (debug->watches->len, ==, 1);
    teardown ();
}

/* The kinds of exceptions an adapter offered: an id with ':' in it, a label with them, and
   entries that are not right left out */
static void
test_exception_filters (void)
{
    debug_launch_t *launch;
    const debug_exception_filter_t *filter;
    GKeyFile *keyfile;
    GPtrArray *filters;
    const char *const odd[] = { "1:5:short", "2:3:abc:x", "1::abc:x", "0:0::x", "1:3:abcx",
                                "1:x:abc:x", "x",         "0:3:abc:", NULL };

    setup ();
    g_assert_cmpint (debug_project_switch (debug, g_strdup (root)), ==, MC_EPR_OK);
    debug->launches_in_project = FALSE;
    launch = launch_new ("py", "/s.py", "/");
    launch->exception_filters = g_ptr_array_new_with_free_func (debug_exception_filter_free);
    g_ptr_array_add (launch->exception_filters,
                     debug_exception_filter_new ("raised", "Raised Exceptions", FALSE));
    g_ptr_array_add (launch->exception_filters,
                     debug_exception_filter_new ("kind:uncaught", "Uncaught: all of them", TRUE));
    g_ptr_array_add (debug->launches, launch);
    debug_config_save (debug);
    reopen ();
    launch = launch_at (0);
    g_assert_nonnull (launch->exception_filters);
    g_assert_cmpuint (launch->exception_filters->len, ==, 2);
    filter = g_ptr_array_index (launch->exception_filters, 0);
    g_assert_cmpstr (filter->id, ==, "raised");
    g_assert_cmpstr (filter->label, ==, "Raised Exceptions");
    g_assert_false (filter->on);
    filter = g_ptr_array_index (launch->exception_filters, 1);
    g_assert_cmpstr (filter->id, ==, "kind:uncaught");
    g_assert_cmpstr (filter->label, ==, "Uncaught: all of them");
    g_assert_true (filter->on);

    keyfile = g_key_file_new ();
    g_key_file_set_string_list (keyfile, "L", "exception_filters", odd,
                                g_strv_length ((char **) odd));
    filters = debug_exception_filters_read (keyfile, "L");
    // only the last is right: an id of 3, an empty label
    g_assert_cmpuint (filters->len, ==, 1);
    filter = g_ptr_array_index (filters, 0);
    g_assert_cmpstr (filter->id, ==, "abc");
    g_assert_cmpstr (filter->label, ==, "");
    g_ptr_array_unref (filters);
    g_assert_null (debug_exception_filters_read (keyfile, "none"));
    g_key_file_free (keyfile);
    teardown ();
}

/* A project that is not there is not switched to; the same one again is nothing to do */
static void
test_switch (void)
{
    char *gone;

    setup ();
    gone = g_build_filename (root, "gone", (char *) NULL);
    g_assert_cmpint (debug_project_switch (debug, gone), ==, MC_EPR_FAILED);
    g_assert_cmpuint (errors->len, ==, 1);
    g_assert_cmpstr (g_ptr_array_index (errors, 0), ==, "The project directory does not exist.");
    g_assert_null (debug->project_dir);

    g_assert_cmpint (debug_project_switch (debug, g_strdup (root)), ==, MC_EPR_OK);
    breakpoint_add ("/src/a.c", 1, FALSE, NULL);
    g_assert_cmpint (debug_project_switch (debug, g_strdup (root)), ==, MC_EPR_OK);
    g_assert_cmpuint (debug->breakpoints->len, ==, 1);
    // no project, no file to keep them in
    g_assert_cmpint (debug_project_switch (debug, NULL), ==, MC_EPR_OK);
    synced = 0;
    debug_config_save (debug);
    g_assert_cmpint (synced, ==, 0);
    teardown ();
}

/* NAME=VALUE, by the words of the shell */
static void
test_environment (void)
{
    char **entries = NULL;

    setup ();
    g_assert_true (debug_parse_environment (debug, NULL, &entries));
    g_assert_null (entries);
    g_assert_true (debug_parse_environment (debug, "", &entries));
    g_assert_null (entries);
    g_assert_true (debug_parse_environment (debug, "A=1 _B2='x y' C=", &entries));
    g_assert_cmpuint (g_strv_length (entries), ==, 3);
    g_assert_cmpstr (entries[1], ==, "_B2=x y");
    g_assert_cmpstr (entries[2], ==, "C=");
    g_strfreev (entries);
    g_assert_cmpuint (errors->len, ==, 0);

    g_assert_false (debug_parse_environment (debug, "A=1 2B=3", &entries));
    g_assert_null (entries);
    g_assert_false (debug_parse_environment (debug, "NOVALUE", &entries));
    g_assert_false (debug_parse_environment (debug, "A-B=1", &entries));
    g_assert_false (debug_parse_environment (debug, "A='two\nlines'", &entries));
    g_assert_null (entries);
    g_assert_cmpuint (errors->len, ==, 4);
    g_assert_nonnull (strstr (g_ptr_array_index (errors, 0), "NAME=VALUE"));
    // what the shell would not read
    g_assert_false (debug_parse_environment (debug, "A='open", &entries));
    g_assert_null (entries);
    g_assert_cmpuint (errors->len, ==, 5);
    g_assert_null (strstr (g_ptr_array_index (errors, 4), "NAME=VALUE"));
    teardown ();
}

/* The debugger of a configuration, and its adapter when it is there to run */
static void
test_adapter (void)
{
    debug_launch_t *gdb, *dap;
    char *command;

    setup ();
    g_assert_null (debug_adapter_hint (NULL));
    g_assert_null (debug_adapter_hint ("my-own-adapter"));
    g_assert_nonnull (strstr (debug_adapter_hint ("python3 -m debugpy.adapter"), "pip install"));
    g_assert_nonnull (strstr (debug_adapter_hint ("/opt/dlv dap"), "go install"));
    g_assert_nonnull (strstr (debug_adapter_hint ("lldb-dap"), "lldb-vscode"));

    gdb = launch_new ("c", "/bin/c", "/");
    g_assert_false (debug_launch_is_dap (gdb));
    command = debug_launch_debugger (gdb);
    g_assert_cmpstr (command, ==, "gdb-mi");
    g_free (command);
    dap = launch_new ("py", "/s.py", "/");
    dap->backend = g_strdup ("dap");
    g_assert_true (debug_launch_is_dap (dap));
    command = debug_launch_debugger (dap);
    g_assert_cmpstr (command, ==, "dap\x1f\x1f");
    g_free (command);

#ifdef ENABLE_DAP
    // none, unless it listens already
    g_assert_null (debug_adapter_check (debug, dap));
    g_assert_cmpstr (g_ptr_array_index (errors, 0), ==, "Enter the command of the debug adapter.");
    dap->address = g_strdup ("127.0.0.1:5678");
    command = debug_adapter_check (debug, dap);
    g_assert_cmpstr (command, ==, "");
    g_free (command);
    command = debug_launch_debugger (dap);
    g_assert_cmpstr (command, ==,
                     "dap\x1f\x1f"
                     "127.0.0.1:5678");
    g_free (command);
    // by its path, or in PATH; how to get it when it is not there
    dap->adapter = g_strdup ("sh -c 'exec cat'");
    command = debug_adapter_check (debug, dap);
    g_assert_cmpstr (command, ==, "sh -c 'exec cat'");
    g_free (command);
    g_free (dap->adapter);
    dap->adapter = g_strdup ("/nonexistent/debugpy-adapter --port 1");
    g_assert_null (debug_adapter_check (debug, dap));
    g_assert_cmpuint (errors->len, ==, 2);
    g_assert_nonnull (strstr (g_ptr_array_index (errors, 1), "/nonexistent/debugpy-adapter"));
    g_assert_nonnull (strstr (g_ptr_array_index (errors, 1), "pip install debugpy"));
    g_free (dap->adapter);
    dap->adapter = g_strdup ("no-such-adapter-anywhere");
    g_assert_null (debug_adapter_check (debug, dap));
    g_assert_cmpstr (g_ptr_array_index (errors, 2), ==,
                     "The debug adapter no-such-adapter-anywhere was not found.");
    g_free (dap->adapter);
    dap->adapter = g_strdup ("'open");
    g_assert_null (debug_adapter_check (debug, dap));
    g_assert_cmpuint (errors->len, ==, 4);
#endif
    debug_launch_free (gdb);
    debug_launch_free (dap);
    teardown ();
}

/* The Python of the virtual environment of the project, with its debugpy; without debugpy the
   user is told once how to have it */
static void
test_launch_python (void)
{
    debug_launch_t *launch;
    char *python, *expected;

    setup ();
    debug->project_dir = g_strdup (root);
    launch = launch_new ("s", "/p/s.py", "/");
    launch->adapter = g_strdup ("python3 -I -m debugpy.adapter --log-stderr");
    // no environment: as it was
    debug_launch_python (debug, launch);
    g_assert_cmpstr (launch->adapter, ==, "python3 -I -m debugpy.adapter --log-stderr");

    // a Python that has debugpy
    python = project_file (".venv/bin/python", "#!/bin/sh\nexit 0\n", 0755);
    debug_launch_python (debug, launch);
    expected = g_strdup_printf ("%s -I -m debugpy.adapter --log-stderr", python);
    g_assert_cmpstr (launch->adapter, ==, expected);
    g_free (expected);
    // one by its path is the user's choice
    g_free (launch->adapter);
    launch->adapter = g_strdup ("/usr/bin/python3 -m debugpy.adapter");
    debug_launch_python (debug, launch);
    g_assert_cmpstr (launch->adapter, ==, "/usr/bin/python3 -m debugpy.adapter");
    g_assert_cmpuint (errors->len, ==, 0);
    // -mdebugpy.adapter, one word; another module is not debugpy's
    g_free (launch->adapter);
    launch->adapter = g_strdup ("python3 -mdebugpy.adapter 'a b'");
    debug_launch_python (debug, launch);
    expected = g_strdup_printf ("%s -mdebugpy.adapter 'a b'", python);
    g_assert_cmpstr (launch->adapter, ==, expected);
    g_free (expected);
    g_free (launch->adapter);
    launch->adapter = g_strdup ("python3 -m other.adapter");
    debug_launch_python (debug, launch);
    g_assert_cmpstr (launch->adapter, ==, "python3 -m other.adapter");
    // not a script of Python
    g_free (launch->adapter);
    launch->adapter = g_strdup ("python3 -m debugpy.adapter");
    g_free (launch->executable);
    launch->executable = g_strdup ("/p/s.sh");
    debug_launch_python (debug, launch);
    g_assert_cmpstr (launch->adapter, ==, "python3 -m debugpy.adapter");

    // a Python with no debugpy: told once
    g_free (python);
    python = project_file (".venv/bin/python", "#!/bin/sh\nexit 1\n", 0755);
    g_free (launch->executable);
    launch->executable = g_strdup ("/p/s.py");
    debug_launch_python (debug, launch);
    debug_launch_python (debug, launch);
    g_assert_cmpstr (launch->adapter, ==, "python3 -m debugpy.adapter");
    g_assert_cmpuint (errors->len, ==, 1);
    g_assert_nonnull (strstr (g_ptr_array_index (errors, 0), "-m pip install debugpy"));
    debug_launch_free (launch);
    g_free (python);
    teardown ();
}

/* A new configuration: the program the build has made, those the language keeps for itself
   left out, the gdb of the language; a word when it was built for no debugging */
static void
test_guess_built (void)
{
    debug_launch_t *launch = g_new0 (debug_launch_t, 1);
    char *app, *hidden, *build_dir;
    const char *programs[] = { NULL, NULL, NULL };
    const char *hide[] = { "target/debug/build/*", NULL };
    GVariantDict dict;

    setup ();
    app = g_build_filename (root, "build", "app", (char *) NULL);
    hidden = g_build_filename (root, "target", "debug", "build", "x", "build-script-build",
                               (char *) NULL);
    build_dir = g_build_filename (root, "build", (char *) NULL);
    programs[0] = hidden;
    programs[1] = app;
    debug->project_dir = g_strdup (root);
    g_variant_dict_init (&dict, NULL);
    g_variant_dict_insert_value (&dict, "programs", g_variant_new_strv (programs, -1));
    g_variant_dict_insert (&dict, "system", "s", "meson");
    g_variant_dict_insert (&dict, "command", "s", "ninja");
    g_variant_dict_insert (&dict, "dir", "s", build_dir);
    build_info = g_variant_ref_sink (g_variant_dict_end (&dict));
    g_variant_dict_init (&dict, NULL);
    g_variant_dict_insert (&dict, "id", "s", "rust");
    g_variant_dict_insert (&dict, "gdb", "s", "rust-gdb");
    g_variant_dict_insert_value (&dict, "hide", g_variant_new_strv (hide, -1));
    build_preset = g_variant_ref_sink (g_variant_dict_end (&dict));
    g_variant_dict_init (&dict, NULL);
    g_variant_dict_insert (&dict, "debug_info", "b", TRUE);
    g_variant_dict_insert (&dict, "optimized", "b", TRUE);
    build_elf = g_variant_ref_sink (g_variant_dict_end (&dict));

    debug_launch_guess (debug, launch, NULL);
    g_assert_cmpstr (launch->executable, ==, app);
    g_assert_cmpstr (launch->name, ==, "app");
    g_assert_cmpstr (launch->directory, ==, root);
    g_assert_cmpstr (launch->gdb_path, ==, "rust-gdb");
    g_assert_true (launch->build);
    g_assert_true (launch->terminal);
    g_assert_null (launch->backend);
    // built with optimization: how to build it without, for meson in its directory
    g_assert_cmpuint (errors->len, ==, 1);
    g_assert_nonnull (strstr (g_ptr_array_index (errors, 0), "built with optimization"));
    g_assert_nonnull (strstr (g_ptr_array_index (errors, 0), "meson configure -Dbuildtype=debug"));
    g_assert_nonnull (strstr (g_ptr_array_index (errors, 0), build_dir));
    debug_launch_free (launch);

    // nothing built yet, the program of the preset; no build plugin: GDB and the root
    g_clear_pointer (&build_info, g_variant_unref);
    g_clear_pointer (&build_preset, g_variant_unref);
    g_variant_dict_init (&dict, NULL);
    g_variant_dict_insert (&dict, "id", "s", "rust");
    g_variant_dict_insert (&dict, "program", "s", "/p/target/debug/p");
    build_preset = g_variant_ref_sink (g_variant_dict_end (&dict));
    g_variant_dict_init (&dict, NULL);
    g_variant_dict_insert (&dict, "debug_info", "b", FALSE);
    g_clear_pointer (&build_elf, g_variant_unref);
    build_elf = g_variant_ref_sink (g_variant_dict_end (&dict));
    launch = g_new0 (debug_launch_t, 1);
    debug_launch_guess (debug, launch, NULL);
    g_assert_cmpstr (launch->executable, ==, "/p/target/debug/p");
    g_assert_false (launch->build);
    g_assert_cmpstr (launch->gdb_path, ==, "gdb");
    g_assert_cmpuint (errors->len, ==, 2);
    g_assert_nonnull (strstr (g_ptr_array_index (errors, 1), "no debug information"));
    g_assert_nonnull (strstr (g_ptr_array_index (errors, 1), "CFLAGS='-g -O0'"));
    debug_launch_free (launch);

    g_clear_pointer (&build_preset, g_variant_unref);
    launch = g_new0 (debug_launch_t, 1);
    debug_launch_guess (debug, launch, NULL);
    g_assert_null (launch->executable);
    g_assert_cmpstr (launch->name, ==, "Debug");
    debug_launch_free (launch);
    teardown ();
    g_free (app);
    g_free (hidden);
    g_free (build_dir);
}

/* A new configuration of a script an adapter runs: the file in front, the adapter of its
   language, and of a language with a package main, that of the file in front */
static void
test_guess_script (void)
{
    debug_launch_t *launch;
    GVariantDict dict;
    const char *extensions[] = { ".go", NULL };
    char *tool, *server, *expected;

    setup ();
    debug->project_dir = g_strdup (root);
    current_file = g_build_filename (root, "s.py", (char *) NULL);
    g_variant_dict_init (&dict, NULL);
    g_variant_dict_insert (&dict, "id", "s", "python");
    g_variant_dict_insert (&dict, "adapter", "s", "python3 -m debugpy.adapter");
    g_variant_dict_insert (&dict, "launch", "s", "{\"justMyCode\": true}");
    build_preset = g_variant_ref_sink (g_variant_dict_end (&dict));
    launch = g_new0 (debug_launch_t, 1);
    debug_launch_guess (debug, launch, NULL);
    g_assert_cmpstr (launch->executable, ==, current_file);
    g_assert_cmpstr (launch->name, ==, "s.py");
    g_assert_cmpstr (launch->backend, ==, "dap");
    g_assert_cmpstr (launch->adapter, ==, "python3 -m debugpy.adapter");
    g_assert_null (launch->address);
    g_assert_cmpstr (launch->launch_extra, ==, "{\"justMyCode\": true}");
    g_assert_false (launch->build);
    // the preset asked of the file in front, then of the script
    g_assert_cmpstr (g_ptr_array_index (preset_paths, 0), ==, current_file);
    debug_launch_free (launch);

    // a preset with no adapter: not a script, nothing to run
    g_clear_pointer (&build_preset, g_variant_unref);
    g_variant_dict_init (&dict, NULL);
    g_variant_dict_insert (&dict, "id", "s", "c");
    build_preset = g_variant_ref_sink (g_variant_dict_end (&dict));
    launch = g_new0 (debug_launch_t, 1);
    debug_launch_guess (debug, launch, NULL);
    g_assert_null (launch->executable);
    g_assert_null (launch->backend);
    debug_launch_free (launch);

    // go: the package main of the file in front, of those in the project
    tool = project_file ("cmd/tool/main.go", "package main\n", 0644);
    server = project_file ("cmd/server/main.go", "// x\n  package main  \n", 0644);
    g_free (project_file ("lib/lib.go", "package lib\n", 0644));
    g_free (project_file ("vendor/v/main.go", "package main\n", 0644));
    g_free (project_file (".hidden/main.go", "package main\n", 0644));
    g_free (project_file ("cmd/tool/notes.txt", "package main\n", 0644));
    g_clear_pointer (&build_preset, g_variant_unref);
    g_variant_dict_init (&dict, NULL);
    g_variant_dict_insert (&dict, "id", "s", "go");
    g_variant_dict_insert (&dict, "adapter", "s", "dlv dap");
    g_variant_dict_insert (&dict, "program", "s", root);
    g_variant_dict_insert (&dict, "entry", "s", "package main");
    g_variant_dict_insert_value (&dict, "extensions", g_variant_new_strv (extensions, -1));
    build_preset = g_variant_ref_sink (g_variant_dict_end (&dict));
    g_free (current_file);
    current_file = g_strdup (tool);
    launch = g_new0 (debug_launch_t, 1);
    debug_launch_guess (debug, launch, NULL);
    expected = g_path_get_dirname (tool);
    g_assert_cmpstr (launch->executable, ==, expected);
    g_assert_cmpstr (launch->name, ==, "tool");
    g_assert_cmpstr (launch->adapter, ==, "dlv dap");
    g_free (expected);
    debug_launch_free (launch);
    // only one package main: that one, whatever is in front
    g_assert_cmpint (g_remove (tool), ==, 0);
    launch = g_new0 (debug_launch_t, 1);
    debug_launch_guess (debug, launch, NULL);
    expected = g_path_get_dirname (server);
    g_assert_cmpstr (launch->executable, ==, expected);
    g_free (expected);
    debug_launch_free (launch);
    // none: the program of the preset
    g_assert_cmpint (g_remove (server), ==, 0);
    launch = g_new0 (debug_launch_t, 1);
    debug_launch_guess (debug, launch, NULL);
    g_assert_cmpstr (launch->executable, ==, root);
    debug_launch_free (launch);
    g_free (tool);
    g_free (server);
    teardown ();
}

/* The programs a language keeps for itself, by patterns from the root */
static void
test_program_hidden (void)
{
    const char *const hide[] = { "target/*/build/*", "*.d", NULL };
    char *path;

    setup ();
    debug->project_dir = g_strdup ("/p");
    g_assert_true (debug_program_hidden (debug, "/p/target/debug/build/x/y", hide));
    g_assert_true (debug_program_hidden (debug, "/p/target/a.d", hide));
    g_assert_false (debug_program_hidden (debug, "/p/target/debug/app", hide));
    g_assert_false (debug_program_hidden (debug, "/p/target/debug/app", NULL));
    // of another project, one whose name starts the same
    g_assert_false (debug_program_hidden (debug, "/px/a.d", hide));
    g_assert_false (debug_program_hidden (debug, "/q/a.d", hide));

    path = debug_path_from_root (debug, "/p/a/b", TRUE);
    g_assert_cmpstr (path, ==, "a/b");
    g_free (path);
    path = debug_path_from_root (debug, "/p", TRUE);
    g_assert_cmpstr (path, ==, ".");
    g_free (path);
    path = debug_path_from_root (debug, "/px/a", TRUE);
    g_assert_cmpstr (path, ==, "/px/a");
    g_free (path);
    path = debug_path_from_root (debug, "/p/a", FALSE);
    g_assert_cmpstr (path, ==, "/p/a");
    g_free (path);
    path = debug_path_from_root (debug, NULL, TRUE);
    g_assert_cmpstr (path, ==, "");
    g_free (path);
    teardown ();
}

/* --------------------------------------------------------------------------------------------- */

int
main (int argc, char **argv)
{
    int result;

    config_home = g_dir_make_tmp ("debug-config-home-XXXXXX", NULL);
    g_assert_nonnull (config_home);
    // before glib reads it once and for all
    g_setenv ("XDG_CONFIG_HOME", config_home, TRUE);
    g_assert_cmpstr (g_get_user_config_dir (), ==, config_home);
    g_test_init (&argc, &argv, NULL);
    g_test_add_func ("/debugger/config/user", test_user_config);
    g_test_add_func ("/debugger/config/project", test_project_config);
    g_test_add_func ("/debugger/config/odd", test_odd_config);
    g_test_add_func ("/debugger/config/exception-filters", test_exception_filters);
    g_test_add_func ("/debugger/config/switch", test_switch);
    g_test_add_func ("/debugger/config/environment", test_environment);
    g_test_add_func ("/debugger/config/adapter", test_adapter);
    g_test_add_func ("/debugger/config/launch-python", test_launch_python);
    g_test_add_func ("/debugger/config/guess-built", test_guess_built);
    g_test_add_func ("/debugger/config/guess-script", test_guess_script);
    g_test_add_func ("/debugger/config/program-hidden", test_program_hidden);
    result = g_test_run ();
    remove_tree (config_home);
    g_free (config_home);
    return result;
}
