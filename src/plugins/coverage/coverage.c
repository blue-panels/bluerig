/*
   The plugin coverage: the lines that ran and those that did not, in the gutter.

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

/** \file coverage.c
 *  \brief Source: the plugin coverage: the lines that ran and those that did not, in the gutter
 *
 *  gcov and the reading of the files run in a process of their own, a shell script; its output
 *  comes in by the main loop, a piece at a time, and the marks go into the files when the editor
 *  is idle, some hundreds at a time: the editor does not stop for them.
 */

#include <config.h>

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "lib/global.h"
#include "lib/widget.h"
#include "lib/tty/key.h"  // add_select_channel ()

#include "src/editor/edit-impl.h"          // edit_widget_is_editor ()
#include "src/plugins/build/build-core.h"  // build_detect ()

#include "coverage-core.h"
#include "coverage.h"

/*** global variables ****************************************************************************/

/*** file scope macro definitions ****************************************************************/

/* the marks set at a time when the editor is idle */
#define COVERAGE_MARKS_STEP 300
/* a look at the data when a file window comes to the front: not more often, in microseconds */
#define COVERAGE_LOOK_EVERY (2 * G_USEC_PER_SEC)

/*** file scope type declarations ****************************************************************/

enum
{
    COVERAGE_MARK_COVERED,
    COVERAGE_MARK_MISSED,
    COVERAGE_MARK_COUNT
};

enum
{
    COVERAGE_ACT_TOGGLE,
    COVERAGE_ACT_REFRESH
};

/* The marks of an open file still to be set */
typedef struct
{
    char *file;
    GArray *lines; /* long, the line, negative for a line that did not run */
    guint done;
} coverage_job_t;

/* What a call for the idle editor reaches: the plugin, or NULL when it is closed */
typedef struct
{
    struct coverage_t *coverage;
} coverage_token_t;

typedef struct coverage_t
{
    mc_editor_host_t *host;
    int marks[COVERAGE_MARK_COUNT];
    gboolean shown;
    guint build_signal;
    guint terminal_signal;

    // the reading of the data
    GPid pid;
    int fd;
    gboolean forced;       /* the reading asked for by the user: say when there is none */
    gboolean again;        /* asked for again while it ran */
    gboolean again_forced; /* and then whatever the stamp says */
    char *stamp;           /* the last reading of this editor; a reading done renames .new to it */
    GString *pending;      /* the output after its last newline */
    coverage_reader_t *reader;
    char *root;
    char *profile; /* the build profile whose data is read, NULL for none */
    char *dir;     /* the directory it was read from */
    gint64 looked; /* when the data was looked at the last time */

    GHashTable *files; /* coverage_reader_finish (), NULL before the first data */
    GQueue jobs;       /* coverage_job_t */
    coverage_token_t *token;
} coverage_t;

/*** forward declarations (file scope functions) *************************************************/

static void coverage_read (coverage_t *coverage, gboolean forced);

/*** file scope variables ************************************************************************/

static const mc_ep_marker_kind_t coverage_mark_kinds[COVERAGE_MARK_COUNT] = {
    // the left half of a block, as the other editors draw it; a bar where there is no UTF-8
    { "coverage.covered", "coverage-covered-char", "\xe2\x96\x8c", "|", "coveragecovered", NULL,
      NULL, 1, "green" },
    { "coverage.missed", "coverage-missed-char", "\xe2\x96\x8c", "|", "coveragemissed", NULL, NULL,
      2, "yellow" },
};

/* What finds the data and prints it, the head first: $1 the build directory, $2 the root, $3 the
   stamp of the last reading, $4 1 to read it whatever the stamp says; $3.new stamps this reading,
   to be the stamp once its output is all read.  gcov of the .gcda files of
   the build, else the first lcov tracefile there is, the build directory there or not */
static const char coverage_script[] =
    "if cd \"$1\" 2>/dev/null && [ -n \"$(find . -name '*.gcda' -print 2>/dev/null | head -n 1)\" "
    "]; "
    "then\n"
    "    if [ \"$4\" != 1 ] && [ -f \"$3\" ] && [ -z \"$(find . -name '*.gcda' -newer \"$3\" "
    "-print 2>/dev/null | head -n 1)\" ]; then\n"
    "        echo '" COVERAGE_HEAD_UNCHANGED "'\n"
    "        exit 0\n"
    "    fi\n"
    "    touch \"$3.new\"\n"
    "    echo '" COVERAGE_HEAD_GCOV "'\n"
    "    find . -name '*.gcda' -exec gcov -t {} + 2>/dev/null\n"
    "    exit 0\n"
    "fi\n"
    "for f in \"$2/coverage.info\" \"$2/lcov.info\" \"$2/coverage/lcov.info\" "
    "\"$1/meson-logs/coverage.info\"; do\n"
    "    [ -f \"$f\" ] || continue\n"
    "    if [ \"$4\" != 1 ] && [ -f \"$3\" ] && [ -z \"$(find \"$f\" -newer \"$3\" -print)\" ]; "
    "then\n"
    "        echo '" COVERAGE_HEAD_UNCHANGED "'\n"
    "        exit 0\n"
    "    fi\n"
    "    touch \"$3.new\"\n"
    "    echo '" COVERAGE_HEAD_LCOV "'\n"
    "    cat \"$f\"\n"
    "    exit 0\n"
    "done\n"
    "echo '" COVERAGE_HEAD_NONE "'\n";

/* --------------------------------------------------------------------------------------------- */
/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

static void
coverage_job_free (gpointer p)
{
    coverage_job_t *job = (coverage_job_t *) p;

    g_free (job->file);
    g_array_free (job->lines, TRUE);
    g_free (job);
}

/* --------------------------------------------------------------------------------------------- */

static int
coverage_line_cmp (gconstpointer a, gconstpointer b)
{
    const long la = labs (*(const long *) a), lb = labs (*(const long *) b);

    return la < lb ? -1 : la > lb ? 1 : 0;
}

/* --------------------------------------------------------------------------------------------- */

/* The project of the file in front, as the project plugin sees it, else its directory */
static char *
coverage_root_of (coverage_t *coverage, void *edit)
{
    mc_editor_host_t *host = coverage->host;
    char *file = edit != NULL ? host->get_current_file (host, edit) : NULL;
    GVariantDict args;
    GVariant *reply;
    char *root = NULL;

    g_variant_dict_init (&args, NULL);
    if (file != NULL)
        g_variant_dict_insert (&args, "file", "s", file);
    reply = host->service_call (host, "project", "root", g_variant_dict_end (&args), NULL);
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

/* A call of the service build about the project of @root, @profile NULL for none; NULL when
   there is no such service or it says no */
static GVariant *
coverage_build_call (coverage_t *coverage, const char *method, const char *root,
                     const char *profile, GVariantDict *more)
{
    GVariantDict own;
    GVariantDict *args = more != NULL ? more : &own;
    GVariant *in;

    if (more == NULL)
        g_variant_dict_init (&own, NULL);
    if (coverage->host->service_call == NULL)
    {
        g_variant_dict_clear (args);
        return NULL;
    }
    g_variant_dict_insert (args, "root", "s", root);
    if (profile != NULL)
        g_variant_dict_insert (args, "profile", "s", profile);
    in = g_variant_dict_end (args);
    return coverage->host->service_call (coverage->host, "build", method, in, NULL);
}

/* --------------------------------------------------------------------------------------------- */

/* The directory of the data of the project of @root: that of its profile of coverage, which
   @profile gets, NULL for none; without one the build directory found in it, else the root */
static char *
coverage_data_dir (coverage_t *coverage, const char *root, char **profile)
{
    GVariant *reply = coverage_build_call (coverage, "coverage", root, NULL, NULL);
    const char *dir = NULL, *name = NULL;
    char *found;

    *profile = NULL;
    if (reply != NULL && g_variant_lookup (reply, "dir", "&s", &dir)
        && g_variant_lookup (reply, "profile", "&s", &name))
    {
        found = g_strdup (dir);
        *profile = g_strdup (name);
    }
    else
    {
        build_info_t *info = build_detect (root);

        found = g_strdup (info != NULL && info->build_dir != NULL ? info->build_dir : root);
        build_info_free (info);
    }
    if (reply != NULL)
        g_variant_unref (reply);
    return found;
}

/* --------------------------------------------------------------------------------------------- */

/* Build the profile @name of @root, its tests after it whatever the profile says: they make the
   data */
static void
coverage_build (coverage_t *coverage, const char *root, const char *name)
{
    GVariantDict more;
    GVariant *reply;

    g_variant_dict_init (&more, NULL);
    g_variant_dict_insert (&more, "tests", "b", TRUE);
    reply = coverage_build_call (coverage, "run", root, name, &more);

    if (reply != NULL)
        g_variant_unref (reply);
}

/* --------------------------------------------------------------------------------------------- */

/* The options that make the build of the active profile of @root one for coverage: b_coverage
   of meson, --coverage in the flags of cmake; NULL, said why, for another build system, which
   has no build directory of its own to set up */
static const char *
coverage_setup_of (coverage_t *coverage, const char *root)
{
    GVariant *reply = coverage_build_call (coverage, "info", root, NULL, NULL);
    const char *system = NULL, *setup = NULL;

    if (reply != NULL)
        (void) g_variant_lookup (reply, "system", "&s", &system);
    if (g_strcmp0 (system, "meson") == 0)
        setup = "-Db_coverage=true";
    else if (g_strcmp0 (system, "cmake") == 0)
        setup = "-DCMAKE_C_FLAGS=--coverage -DCMAKE_CXX_FLAGS=--coverage "
                "-DCMAKE_EXE_LINKER_FLAGS=--coverage -DCMAKE_SHARED_LINKER_FLAGS=--coverage";
    else
        coverage->host->message (
            coverage->host, D_NORMAL, _ ("Coverage"),
            _ ("Only a build of meson or cmake has a build directory of its own for coverage.\n"
               "Build the project with --coverage (gcc, clang) and run the tests: the data is\n"
               "read in the project."));
    if (reply != NULL)
        g_variant_unref (reply);
    return setup;
}

/* --------------------------------------------------------------------------------------------- */

/* A profile of coverage for the project of @root, made from the active one with b_coverage and
   its tests after its build, in its window; built when the user says so.  One there is opened
   in its window. */
static void
coverage_make_profile (coverage_t *coverage, const char *root)
{
    GVariant *reply = coverage_build_call (coverage, "coverage", root, NULL, NULL);
    const char *there = NULL;
    char *name = NULL;
    gboolean is_new = FALSE, saved = FALSE;

    if (reply == NULL)
    {
        coverage->host->message (coverage->host, D_ERROR, _ ("Coverage"),
                                 _ ("The plugin Build makes the profiles: it is not loaded."));
        return;
    }
    if (g_variant_lookup (reply, "profile", "&s", &there))
        name = g_strdup (there);
    g_variant_unref (reply);
    // a new one, by a name not taken, its options for coverage those of its build system
    if (name == NULL)
    {
        const char *setup = coverage_setup_of (coverage, root);
        gboolean taken = TRUE;
        int i;

        if (setup == NULL)
            return;

        // the next name while the one tried is taken; another failure, said by build, ends it
        for (i = 1; name == NULL && taken && i < 100; i++)
        {
            GVariantDict more;
            gboolean ok = FALSE;
            char *candidate = i == 1 ? g_strdup ("Coverage") : g_strdup_printf ("Coverage %d", i);

            g_variant_dict_init (&more, NULL);
            g_variant_dict_insert (&more, "setup", "s", setup);
            // the flags of cmake take --coverage after those they have; b_coverage is set
            g_variant_dict_insert (&more, "append", "b", strstr (setup, "_FLAGS=") != NULL);
            g_variant_dict_insert (&more, "coverage", "b", TRUE);
            g_variant_dict_insert (&more, "tests", "b", TRUE);
            reply = coverage_build_call (coverage, "create", root, candidate, &more);
            taken = FALSE;
            if (reply != NULL && g_variant_lookup (reply, "ok", "b", &ok) && ok)
                name = candidate;
            else
            {
                if (reply != NULL)
                    (void) g_variant_lookup (reply, "taken", "b", &taken);
                g_free (candidate);
            }
            if (reply != NULL)
                g_variant_unref (reply);
        }
        is_new = TRUE;
        if (name == NULL)
        {
            coverage->host->message (coverage->host, D_ERROR, _ ("Coverage"),
                                     _ ("No profile of coverage could be made: see Build "
                                        "profiles."));
            return;
        }
    }
    {
        GVariantDict more;

        g_variant_dict_init (&more, NULL);
        g_variant_dict_insert (&more, "new", "b", is_new);
        reply = coverage_build_call (coverage, "window", root, name, &more);
        if (reply != NULL)
        {
            const char *saved_as = NULL;

            (void) g_variant_lookup (reply, "saved", "b", &saved);
            // renamed in the window: built by its name now
            if (saved && g_variant_lookup (reply, "profile", "&s", &saved_as))
            {
                g_free (name);
                name = g_strdup (saved_as);
            }
            g_variant_unref (reply);
        }
    }
    if (saved || !is_new)
    {
        char *text = g_strdup_printf (_ ("Build %s and run its tests now?"), name);

        if (query_dialog (_ ("Coverage"), text, D_NORMAL, 2, _ ("&Build"), _ ("&Later")) == 0)
        {
            coverage->shown = TRUE;
            coverage_build (coverage, root, name);
        }
        g_free (text);
    }
    g_free (name);
}

/* --------------------------------------------------------------------------------------------- */

/* No data in the project of @root: what makes it */
static void
coverage_no_data (coverage_t *coverage, const char *root)
{
    char *profile = NULL;
    char *dir = coverage_data_dir (coverage, root, &profile);

    if (profile != NULL)
    {
        char *text = g_strdup_printf (_ ("No coverage data in %s, the build profile of coverage:\n"
                                         "its tests have not run yet."),
                                      profile);

        if (query_dialog (_ ("Coverage"), text, D_NORMAL, 2, _ ("&Build and test"), _ ("&Cancel"))
            == 0)
            coverage_build (coverage, root, profile);
        g_free (text);
    }
    else if (coverage->host->service_call != NULL
             && query_dialog (_ ("Coverage"),
                              _ ("No coverage data: no build profile of the project builds for "
                                 "coverage."),
                              D_NORMAL, 2, _ ("&Make a coverage profile"), _ ("&Cancel"))
                 == 0)
        coverage_make_profile (coverage, root);
    else if (coverage->host->service_call == NULL)
        coverage->host->message (
            coverage->host, D_NORMAL, _ ("Coverage"),
            _ ("No coverage data in the project: no .gcda files in the build directory,\n"
               "no coverage.info or lcov.info. Build with coverage and run the tests."));
    g_free (profile);
    g_free (dir);
}

/* --------------------------------------------------------------------------------------------- */

/* The stamp of the readings of one editor, its directory made: another editor or another
   process reading the same project has its own, and a new root is read whatever it says */
static char *
coverage_stamp_path (void)
{
    static guint count = 0;
    char *dir = g_build_filename (g_get_user_runtime_dir (), "bluerig", "coverage", (char *) NULL);
    char *name = g_strdup_printf ("%ld-%u", (long) getpid (), ++count);
    char *path = g_build_filename (dir, name, (char *) NULL);

    (void) g_mkdir_with_parents (dir, 0700);
    g_free (name);
    g_free (dir);
    return path;
}

/* --------------------------------------------------------------------------------------------- */

static void coverage_jobs_run (void *data);

/* The marks still to be set wait for the editor to be idle */
static void
coverage_jobs_later (coverage_t *coverage)
{
    if (coverage->token != NULL || g_queue_is_empty (&coverage->jobs))
        return;
    coverage->token = g_new0 (coverage_token_t, 1);
    coverage->token->coverage = coverage;
    coverage->host->call_later (coverage->host, coverage_jobs_run, coverage->token);
}

/* --------------------------------------------------------------------------------------------- */

/* Some hundreds of marks, then the editor reads its keys again */
static void
coverage_jobs_run (void *data)
{
    coverage_token_t *token = (coverage_token_t *) data;
    coverage_t *coverage = token->coverage;
    int left = COVERAGE_MARKS_STEP;

    g_free (token);
    if (coverage == NULL)
        return;
    coverage->token = NULL;
    while (left > 0 && !g_queue_is_empty (&coverage->jobs))
    {
        coverage_job_t *job = (coverage_job_t *) g_queue_peek_head (&coverage->jobs);

        for (; left > 0 && job->done < job->lines->len; left--, job->done++)
        {
            const long line = g_array_index (job->lines, long, job->done);

            coverage->host->set_marker (
                coverage->host, job->file, labs (line),
                coverage->marks[line > 0 ? COVERAGE_MARK_COVERED : COVERAGE_MARK_MISSED], TRUE);
        }
        if (job->done >= job->lines->len)
            coverage_job_free (g_queue_pop_head (&coverage->jobs));
    }
    coverage_jobs_later (coverage);
}

/* --------------------------------------------------------------------------------------------- */

/* The marks of the file of @edit, as the data has them */
static void
coverage_mark_file (coverage_t *coverage, void *edit)
{
    char *file = coverage->host->get_current_file (coverage->host, edit);
    GHashTable *lines = coverage_file_lines (coverage->files, file);
    coverage_job_t *job;
    GHashTableIter iter;
    gpointer key, value;

    if (lines == NULL || g_hash_table_size (lines) == 0)
    {
        g_free (file);
        return;
    }
    job = g_new0 (coverage_job_t, 1);
    // the name of the window itself: set_marker () finds it at once
    job->file = file;
    job->lines = g_array_sized_new (FALSE, FALSE, sizeof (long), g_hash_table_size (lines));
    g_hash_table_iter_init (&iter, lines);
    while (g_hash_table_iter_next (&iter, &key, &value))
    {
        const long line = GPOINTER_TO_INT (key);
        const long mark = GPOINTER_TO_INT (value) == COVERAGE_LINE_COVERED ? line : -line;

        g_array_append_val (job->lines, mark);
    }
    // from the top of the file: what is on the screen first, mostly
    g_array_sort (job->lines, coverage_line_cmp);
    g_queue_push_tail (&coverage->jobs, job);
    coverage_jobs_later (coverage);
}

/* --------------------------------------------------------------------------------------------- */

/* The marks off every file, those still to be set forgotten */
static void
coverage_clear (coverage_t *coverage)
{
    int i;

    g_queue_clear_full (&coverage->jobs, coverage_job_free);
    for (i = 0; i < COVERAGE_MARK_COUNT; i++)
        if (coverage->marks[i] >= 0)
            coverage->host->clear_markers (coverage->host, NULL, coverage->marks[i]);
}

/* --------------------------------------------------------------------------------------------- */

/* The marks of the open files, from the data */
static void
coverage_mark_all (coverage_t *coverage)
{
    GList *item;

    coverage_clear (coverage);
    for (item = GROUP (coverage->host->host_data)->widgets; item != NULL; item = g_list_next (item))
        if (edit_widget_is_editor (CONST_WIDGET (item->data)))
            coverage_mark_file (coverage, item->data);
}

/* --------------------------------------------------------------------------------------------- */

/* The reading stopped: its process killed, its output let go */
static void
coverage_read_stop (coverage_t *coverage)
{
    GHashTable *files;

    if (coverage->fd >= 0)
    {
        delete_select_channel (coverage->fd);
        close (coverage->fd);
        coverage->fd = -1;
    }
    if (coverage->pid != 0)
    {
        // its own process group: gcov with the shell, nothing else
        (void) kill (-coverage->pid, SIGTERM);
        (void) waitpid (coverage->pid, NULL, 0);
        g_spawn_close_pid (coverage->pid);
        coverage->pid = 0;
    }
    (void) coverage_reader_finish (coverage->reader, &files);
    coverage->reader = NULL;
    if (files != NULL)
        g_hash_table_unref (files);
    g_string_truncate (coverage->pending, 0);
}

/* --------------------------------------------------------------------------------------------- */

/* The output is over: the data replaces the one there was */
static void
coverage_read_done (coverage_t *coverage)
{
    GHashTable *files;
    coverage_data_t data;
    const gboolean forced = coverage->forced;

    if (coverage->pending->len > 0)
        coverage_reader_line (coverage->reader, coverage->pending->str);
    g_string_truncate (coverage->pending, 0);
    data = coverage_reader_finish (coverage->reader, &files);
    coverage->reader = NULL;
    (void) waitpid (coverage->pid, NULL, 0);
    g_spawn_close_pid (coverage->pid);
    coverage->pid = 0;

    if (data == COVERAGE_DATA_GCOV || data == COVERAGE_DATA_LCOV)
    {
        char *read_one = g_strconcat (coverage->stamp, ".new", (char *) NULL);

        // all of it read: the data of then is that of the stamp
        (void) rename (read_one, coverage->stamp);
        g_free (read_one);
        if (coverage->files != NULL)
            g_hash_table_unref (coverage->files);
        coverage->files = files;
        if (coverage->shown)
            coverage_mark_all (coverage);
    }
    else if (data == COVERAGE_DATA_NONE)
    {
        // the data is gone, or the project in front has none: no marks of the one before
        if (coverage->files != NULL)
        {
            g_hash_table_unref (coverage->files);
            coverage->files = NULL;
            coverage_clear (coverage);
        }
        if (forced && coverage->shown && coverage->root != NULL)
        {
            // a copy: a reading the dialogs let run in the meantime takes the root of before
            char *root = g_strdup (coverage->root);

            coverage_no_data (coverage, root);
            g_free (root);
        }
    }

    if (coverage->again)
    {
        const gboolean again_forced = coverage->again_forced;

        coverage->again = FALSE;
        coverage->again_forced = FALSE;
        coverage_read (coverage, again_forced);
    }
}

/* --------------------------------------------------------------------------------------------- */

static int
coverage_read_ready (int fd, void *data)
{
    coverage_t *coverage = (coverage_t *) data;
    char buf[65536];
    const ssize_t n = read (fd, buf, sizeof (buf));

    if (n > 0)
    {
        char *start, *newline;

        g_string_append_len (coverage->pending, buf, n);
        start = coverage->pending->str;
        while ((newline = strchr (start, '\n')) != NULL)
        {
            *newline = '\0';
            coverage_reader_line (coverage->reader, start);
            start = newline + 1;
        }
        g_string_erase (coverage->pending, 0, start - coverage->pending->str);
    }
    else if (n == 0 || (errno != EAGAIN && errno != EINTR))
    {
        delete_select_channel (fd);
        close (fd);
        coverage->fd = -1;
        coverage_read_done (coverage);
        // the loop of the editor runs then: the marks are set while it is idle, not at a key
        return 1;
    }
    return 0;
}

/* --------------------------------------------------------------------------------------------- */

static void
coverage_child_setup (gpointer data)
{
    (void) data;
    (void) setsid ();
}

/* --------------------------------------------------------------------------------------------- */

/* Read the data of the project in front again; @forced: whatever the stamp says, and say when
   there is none */
static void
coverage_read (coverage_t *coverage, gboolean forced)
{
    char *root, *dir;
    const char *argv[9];
    GError *error = NULL;
    int out = -1;

    if (coverage->pid != 0)
    {
        coverage->again = TRUE;
        coverage->again_forced = coverage->again_forced || forced;
        return;
    }
    root = coverage_root_of (coverage, coverage->host->window_top_file (coverage->host));
    // a project of another root: its data, whatever the stamp of it says
    if (g_strcmp0 (root, coverage->root) != 0)
        forced = TRUE;
    // the directory of the profile of coverage, it is the one whose builds are waited for
    g_free (coverage->profile);
    dir = coverage_data_dir (coverage, root, &coverage->profile);
    // another directory, of another profile: its data, whatever the stamp of the last one says
    if (g_strcmp0 (dir, coverage->dir) != 0)
        forced = TRUE;
    g_free (coverage->dir);
    coverage->dir = g_strdup (dir);

    // $0 is the name of the script, $1 to $4 its arguments
    argv[0] = "/bin/sh";
    argv[1] = "-c";
    argv[2] = coverage_script;
    argv[3] = "coverage";
    argv[4] = dir;
    argv[5] = root;
    argv[6] = coverage->stamp;
    argv[7] = forced ? "1" : "0";
    argv[8] = NULL;
    if (!g_spawn_async_with_pipes (
            root, (char **) argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_STDERR_TO_DEV_NULL,
            coverage_child_setup, NULL, &coverage->pid, NULL, &out, NULL, &error))
    {
        coverage->host->message (coverage->host, D_ERROR, _ ("Coverage"), error->message);
        g_clear_error (&error);
        coverage->pid = 0;
        g_free (root);
        g_free (dir);
        return;
    }
    g_free (coverage->root);
    coverage->root = root;
    coverage->reader = coverage_reader_new (root, dir);
    coverage->forced = forced;
    coverage->looked = g_get_monotonic_time ();
    coverage->fd = out;
    (void) fcntl (out, F_SETFL, fcntl (out, F_GETFL) | O_NONBLOCK);
    add_select_channel (out, coverage_read_ready, coverage);
    g_free (dir);
}

/* --------------------------------------------------------------------------------------------- */

/* A build or a command of the terminal is over: the data may be new */
static void
coverage_signal (const char *name, const char *signal, GVariant *args, void *user_data)
{
    coverage_t *coverage = (coverage_t *) user_data;

    if (!coverage->shown || strcmp (signal, "finished") != 0)
        return;
    // a build of another project or profile, or a setup, leaves the data as it is
    if (strcmp (name, "build") == 0 && args != NULL)
    {
        const char *step = NULL, *profile = NULL, *root = NULL;

        if (g_variant_lookup (args, "step", "&s", &step) && strcmp (step, "build") != 0)
            return;
        // the project read again is that of the window in front: the build is of it, or not
        if (g_variant_lookup (args, "root", "&s", &root))
        {
            char *front =
                coverage_root_of (coverage, coverage->host->window_top_file (coverage->host));
            const gboolean other = g_strcmp0 (root, front) != 0;

            g_free (front);
            if (other)
                return;
        }
        // the profile of coverage by its name now: it may have been renamed since the reading
        if (coverage->profile != NULL && g_variant_lookup (args, "profile", "&s", &profile)
            && strcmp (profile, coverage->profile) != 0)
        {
            char *front =
                coverage_root_of (coverage, coverage->host->window_top_file (coverage->host));
            char *now = NULL;
            char *dir = coverage_data_dir (coverage, front, &now);
            const gboolean other = g_strcmp0 (profile, now) != 0;

            g_free (dir);
            g_free (now);
            g_free (front);
            if (other)
                return;
        }
    }
    // a build makes the lines of the data others: read it whatever the stamp says
    coverage_read (coverage, strcmp (name, "build") == 0);
}

/* --------------------------------------------------------------------------------------------- */

static mc_ep_result_t
coverage_act_toggle (void *data, void *edit)
{
    coverage_t *coverage = (coverage_t *) data;

    (void) edit;
    coverage->shown = !coverage->shown;
    if (!coverage->shown)
    {
        coverage_clear (coverage);
        return MC_EPR_OK;
    }
    if (coverage->files != NULL)
        coverage_mark_all (coverage);
    coverage_read (coverage, TRUE);
    return MC_EPR_OK;
}

/* --------------------------------------------------------------------------------------------- */

static mc_ep_result_t
coverage_act_refresh (void *data, void *edit)
{
    coverage_t *coverage = (coverage_t *) data;

    (void) edit;
    coverage->shown = TRUE;
    coverage_read (coverage, TRUE);
    return MC_EPR_OK;
}

/* --------------------------------------------------------------------------------------------- */

static mc_ep_result_t
coverage_handle_event (void *data, void *edit, int event_id, void *payload)
{
    coverage_t *coverage = (coverage_t *) data;

    (void) payload;
    if (event_id != MC_EP_EVENT_FOCUS_IN || edit == NULL || !coverage->shown)
        return MC_EPR_NOT_SUPPORTED;
    // back from the terminal, where the tests ran: a look at the data, which is cheap when the
    // stamp says it is the same
    if (coverage->pid == 0 && g_get_monotonic_time () - coverage->looked >= COVERAGE_LOOK_EVERY)
        coverage_read (coverage, FALSE);
    return MC_EPR_OK;
}

/* --------------------------------------------------------------------------------------------- */

static mc_ep_result_t
coverage_file_open (void *data, void *edit)
{
    coverage_t *coverage = (coverage_t *) data;

    if (coverage->shown && coverage->files != NULL)
        coverage_mark_file (coverage, edit);
    return MC_EPR_OK;
}

/* --------------------------------------------------------------------------------------------- */

static void *
coverage_open (mc_editor_host_t *host, void *editor_dialog)
{
    coverage_t *coverage = g_new0 (coverage_t, 1);
    int i;

    (void) editor_dialog;
    coverage->host = host;
    coverage->fd = -1;
    coverage->pending = g_string_new (NULL);
    coverage->stamp = coverage_stamp_path ();
    g_queue_init (&coverage->jobs);
    for (i = 0; i < COVERAGE_MARK_COUNT; i++)
        coverage->marks[i] =
            host->marker_kind != NULL ? host->marker_kind (host, &coverage_mark_kinds[i]) : -1;
    if (host->service_connect != NULL)
    {
        coverage->build_signal = host->service_connect (host, "build", coverage_signal, coverage);
        coverage->terminal_signal =
            host->service_connect (host, "terminal", coverage_signal, coverage);
    }
    return coverage;
}

/* --------------------------------------------------------------------------------------------- */

static void
coverage_close (void *data)
{
    coverage_t *coverage = (coverage_t *) data;

    if (coverage->build_signal != 0)
        coverage->host->service_disconnect (coverage->host, coverage->build_signal);
    if (coverage->terminal_signal != 0)
        coverage->host->service_disconnect (coverage->host, coverage->terminal_signal);
    coverage_read_stop (coverage);
    // a call for the idle editor still to come finds no plugin
    if (coverage->token != NULL)
        coverage->token->coverage = NULL;
    g_queue_clear_full (&coverage->jobs, coverage_job_free);
    if (coverage->files != NULL)
        g_hash_table_unref (coverage->files);
    g_string_free (coverage->pending, TRUE);
    {
        char *read_one = g_strconcat (coverage->stamp, ".new", (char *) NULL);

        (void) unlink (read_one);
        (void) unlink (coverage->stamp);
        g_free (read_one);
    }
    g_free (coverage->stamp);
    g_free (coverage->root);
    g_free (coverage->profile);
    g_free (coverage->dir);
    g_free (coverage);
}

/* --------------------------------------------------------------------------------------------- */

static const mc_ep_action_t coverage_actions[] = {
    [COVERAGE_ACT_TOGGLE] = { "Toggle", coverage_act_toggle },
    [COVERAGE_ACT_REFRESH] = { "Refresh", coverage_act_refresh },
};

static const mc_ep_cmd_menu_entry_t coverage_menu[] = {
    { MC_EP_MENU_PROJECT, NULL, 0, NULL },
    { MC_EP_MENU_PROJECT, N_ ("Show/hide co&verage"), COVERAGE_ACT_TOGGLE, NULL },
    { MC_EP_MENU_PROJECT, N_ ("Re&read coverage"), COVERAGE_ACT_REFRESH, NULL },
};

static const mc_editor_plugin_t coverage_plugin = {
    .api_version = MC_EDITOR_PLUGIN_API_VERSION,
    .name = "coverage",
    .display_name = N_ ("Coverage"),
    .flags = MC_EPF_NONE,
    .open = coverage_open,
    .close = coverage_close,
    .handle_event = coverage_handle_event,
    .on_file_open = coverage_file_open,
    .actions = coverage_actions,
    .action_count = G_N_ELEMENTS (coverage_actions),
    .cmd_menu_entries = coverage_menu,
    .cmd_menu_entry_count = G_N_ELEMENTS (coverage_menu),
    .help_file = "coverage.md",
    .help_node = "coverage",
    .summary = N_ ("Marks in the gutter the lines the tests ran and those they did not"),
};

/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

const mc_editor_plugin_t *
coverage_get_plugin (void)
{
    return &coverage_plugin;
}

/* --------------------------------------------------------------------------------------------- */
