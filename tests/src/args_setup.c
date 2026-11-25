/*
   src/args - tests for the command line: the options, the files with their lines, git and the
   queries of a database

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

#define TEST_SUITE_NAME "/src/args"

#include "tests/mctest.h"

#include <unistd.h>

#include "src/editor/edit-impl.h"  // edit_startup_option ()

#include "src/args.c"  // for its file scope variables and functions

/* --------------------------------------------------------------------------------------------- */

static char *dir;  // a directory of the test, the current one, with a file a.c in it
static char *start_dir;

static void
setup (void)
{
    char *path;

    str_init_strings ("UTF-8");
    start_dir = g_get_current_dir ();
    dir = g_dir_make_tmp ("args-XXXXXX", NULL);
    ck_assert_ptr_nonnull (dir);
    // by its real name, as the names of the command line are made whole: /var is /private/var
    // on macOS
    {
        char *real = realpath (dir, NULL);

        ck_assert_ptr_nonnull (real);
        g_free (dir);
        dir = g_strdup (real);
        free (real);
    }
    path = g_build_filename (dir, "a.c", (char *) NULL);
    ck_assert (g_file_set_contents (path, "", 0, NULL));
    g_free (path);
    path = g_build_filename (dir, "a.c:5", (char *) NULL);
    ck_assert (g_file_set_contents (path, "", 0, NULL));
    g_free (path);
    path = g_build_filename (dir, "b.c:7", (char *) NULL);
    ck_assert (g_file_set_contents (path, "", 0, NULL));
    g_free (path);
    ck_assert_int_eq (chdir (dir), 0);
    g_setenv ("XDG_DATA_HOME", dir, TRUE);
    g_unsetenv ("MC_QUERY_KIND");
    g_unsetenv ("MC_MONGO_URI");
    g_unsetenv ("MC_MONGO_DB");
    g_unsetenv ("MC_PG_CONNECTION");
    g_unsetenv ("PGSERVICE");
    g_unsetenv ("PGDATABASE");
    g_unsetenv ("PGUSER");
}

/* --------------------------------------------------------------------------------------------- */

static void
remove_tree (const char *path)
{
    GDir *d = g_dir_open (path, 0, NULL);
    const char *name;

    if (d != NULL)
    {
        while ((name = g_dir_read_name (d)) != NULL)
        {
            char *child = g_build_filename (path, name, (char *) NULL);

            remove_tree (child);
            g_free (child);
        }
        g_dir_close (d);
    }
    (void) remove (path);
}

/* --------------------------------------------------------------------------------------------- */

static void
teardown (void)
{
    ck_assert_int_eq (chdir (start_dir), 0);
    remove_tree (dir);
    g_free (dir);
    g_free (start_dir);
    g_list_free_full (mc_args__edit_files, (GDestroyNotify) edit_arg_free);
    mc_args__edit_files = NULL;
    // the options as they were, for the next test in the same process
    mc_args__git_log = mc_args__git_commit = mc_args__git_amend = mc_args__query = FALSE;
    MC_PTR_FREE (mc_args__git_dir);
    MC_PTR_FREE (mc_args__git_show);
    MC_PTR_FREE (mc_args__git_reword);
    MC_PTR_FREE (mc_args__debug_project);
    str_uninit_strings ();
}

/* --------------------------------------------------------------------------------------------- */

/* A command line g_option_context_parse () may change, @n with its NULL */
static char **
argv_copy (char **args, gsize n)
{
    char **argv = g_new (char *, n);

    memcpy (argv, args, n * sizeof (char *));
    return argv;
}

/* --------------------------------------------------------------------------------------------- */

/* The arguments of a command line, the program first; the files of the one before forgotten */
static gboolean
setup_by (GError **error, const char *first, ...)
{
    GPtrArray *argv = g_ptr_array_new ();
    const char *arg;
    va_list args;
    gboolean ok;

    g_list_free_full (mc_args__edit_files, (GDestroyNotify) edit_arg_free);
    mc_args__edit_files = NULL;
    g_ptr_array_add (argv, (char *) "bluerig");
    va_start (args, first);
    for (arg = first; arg != NULL; arg = va_arg (args, const char *))
        g_ptr_array_add (argv, (char *) arg);
    va_end (args);
    ok = mc_setup_by_args ((int) argv->len, (char **) argv->pdata, error);
    g_ptr_array_free (argv, TRUE);
    return ok;
}

/* --------------------------------------------------------------------------------------------- */

static const edit_arg_t *file_arg (guint i);

/* @name in the directory of the test, as a file of the command line is taken */
static void
ck_file (guint i, const char *name)
{
    char *path = g_build_filename (dir, name, (char *) NULL);

    ck_assert_str_eq (file_arg (i)->file_name, path);
    g_free (path);
}

/* --------------------------------------------------------------------------------------------- */

/* The file and the line of the argument @i, in the order of the command line */
static const edit_arg_t *
file_arg (guint i)
{
    GList *l = g_list_nth (mc_args__edit_files, g_list_length (mc_args__edit_files) - 1 - i);

    ck_assert_ptr_nonnull (l);
    return (const edit_arg_t *) l->data;
}

/* --------------------------------------------------------------------------------------------- */
/*** tests ***************************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/* The options are taken off the command line, the files stay */
START_TEST (test_parse)
{
    char *args[] = { (char *) "bluerig",   (char *) "-x", (char *) "--nomouse",
                     (char *) "--git-log", (char *) "-K", (char *) "my.keymap",
                     (char *) "a.c",       (char *) "+3", NULL };
    char **argv = argv_copy (args, G_N_ELEMENTS (args));
    int argc = G_N_ELEMENTS (args) - 1;
    GError *error = NULL;

    ck_assert (mc_args_parse (&argc, &argv, "mc", &error));
    ck_assert_ptr_null (error);
    ck_assert (mc_args__force_xterm);
    ck_assert (mc_args__nomouse);
    ck_assert (mc_args__git_log);
    ck_assert_str_eq (mc_args__keymap_file, "my.keymap");
    ck_assert_int_eq (argc, 3);
    ck_assert_str_eq (argv[1], "a.c");
    ck_assert_str_eq (argv[2], "+3");
    ck_assert (mc_args_show_info ());
    g_free (argv);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* An option that is not there: the error with the help after it */
START_TEST (test_parse_unknown)
{
    char *args[] = { (char *) "bluerig", (char *) "--no-such-option", NULL };
    char **argv = argv_copy (args, G_N_ELEMENTS (args));
    int argc = G_N_ELEMENTS (args) - 1;
    GError *error = NULL;

    ck_assert (!mc_args_parse (&argc, &argv, "mc", &error));
    ck_assert_ptr_nonnull (error);
    /* the message in the codeset of the terminal: with no setlocale () the test has none of
       UTF-8 on musl and Solaris, and what is not ASCII in the help gets in the way */
    if (str_isutf8 (str_detect_termencoding ()))
    {
        ck_assert_ptr_nonnull (strstr (error->message, "--no-such-option"));
        ck_assert_ptr_nonnull (strstr (error->message, "--git-log"));
    }
    g_error_free (error);
    g_free (argv);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* Files with their lines: +N for the first one, file:N as compilers write it */
START_TEST (test_files)
{
    GError *error = NULL;

    ck_assert (setup_by (&error, "a.c:12", "b.c:7", "c.c:5:", "+30", "d.c", "+x", NULL));
    ck_assert_uint_eq (g_list_length (mc_args__edit_files), 5);
    // a file that is there, a line of it
    ck_file (0, "a.c");
    // +30 is for the first file
    ck_assert_int_eq (file_arg (0)->line_number, 30);
    // a file named so, that is there
    ck_file (1, "b.c:7");
    ck_assert_int_eq (file_arg (1)->line_number, 0);
    // a file that is not there keeps its name, to be made
    ck_file (2, "c.c:5:");
    ck_file (3, "d.c");
    // + with no number is a name
    ck_file (4, "+x");
    g_list_free_full (mc_args__edit_files, (GDestroyNotify) edit_arg_free);
    mc_args__edit_files = NULL;

    // a file named so is that one, though the file before the colon is there too
    ck_assert (setup_by (&error, "a.c:5", NULL));
    ck_file (0, "a.c:5");
    ck_assert_int_eq (file_arg (0)->line_number, 0);

    ck_assert (setup_by (&error, "a.c:9:", NULL));
    ck_file (0, "a.c");
    ck_assert_int_eq (file_arg (0)->line_number, 9);
    g_list_free_full (mc_args__edit_files, (GDestroyNotify) edit_arg_free);
    mc_args__edit_files = NULL;

    // none: one new file
    ck_assert (setup_by (&error, NULL));
    ck_assert_uint_eq (g_list_length (mc_args__edit_files), 1);
    ck_assert_ptr_null (file_arg (0)->file_name);
    ck_assert_ptr_null (error);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* The options of git: one action at a time, for the directory named or the current one */
START_TEST (test_git)
{
    GError *error = NULL;

    mc_args__git_log = TRUE;
    mc_args__git_commit = TRUE;
    ck_assert (!setup_by (&error, NULL));
    ck_assert_ptr_nonnull (strstr (error->message, "Only one of"));
    g_clear_error (&error);

    mc_args__git_commit = FALSE;
    ck_assert (setup_by (&error, NULL));
    ck_assert_str_eq (edit_startup_option ("git"), dir);
    ck_assert_str_eq (edit_startup_option ("git-open"), "log");

    mc_args__git_log = FALSE;
    mc_args__git_reword = g_strdup ("abc123");
    ck_assert (setup_by (&error, NULL));
    ck_assert_str_eq (edit_startup_option ("git-open"), "reword");
    ck_assert_str_eq (edit_startup_option ("git-commit"), "abc123");

    // a directory alone: the status of it, its name made whole
    MC_PTR_FREE (mc_args__git_reword);
    g_free (mc_args__git_dir);
    mc_args__git_dir = g_strdup ("./sub/..");
    ck_assert (setup_by (&error, NULL));
    ck_assert_str_eq (edit_startup_option ("git"), dir);
    ck_assert_str_eq (edit_startup_option ("git-open"), "status");

    g_free (mc_args__git_dir);
    mc_args__git_dir = g_strdup ("none");
    ck_assert (!setup_by (&error, NULL));
    ck_assert_ptr_nonnull (strstr (error->message, "no such directory"));
    g_clear_error (&error);
    MC_PTR_FREE (mc_args__git_dir);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* bluerig -D: a directory to debug, made whole */
START_TEST (test_debug_project)
{
    GError *error = NULL;

    mc_args__debug_project = g_strdup (".");
    ck_assert (setup_by (&error, NULL));
    ck_assert_str_eq (mc_args__debug_project, dir);
    ck_assert_str_eq (edit_startup_option ("debug"), dir);

    g_free (mc_args__debug_project);
    mc_args__debug_project = g_strdup ("a.c");
    ck_assert (!setup_by (&error, NULL));
    ck_assert_ptr_nonnull (strstr (error->message, "no such directory to debug"));
    g_clear_error (&error);
    MC_PTR_FREE (mc_args__debug_project);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* --query: the database by the environment, the file of its queries when none is named */
START_TEST (test_query)
{
    GError *error = NULL;
    char *expected;

    mc_args__query = TRUE;
    ck_assert (!setup_by (&error, NULL));
    ck_assert_ptr_nonnull (strstr (error->message, "MC_QUERY_KIND is not set"));
    g_clear_error (&error);

    // any other file says nothing
    ck_assert (!setup_by (&error, "q.txt", NULL));
    g_clear_error (&error);
    // a file .sql is of postgres
    ck_assert (setup_by (&error, "q.sql", NULL));
    ck_assert_str_eq (edit_startup_option ("query"), "postgres");
    ck_file (0, "q.sql");

    g_setenv ("MC_QUERY_KIND", "mysql", TRUE);
    ck_assert (!setup_by (&error, NULL));
    ck_assert_ptr_nonnull (strstr (error->message, "not mongo or postgres"));
    g_clear_error (&error);
    g_setenv ("MC_QUERY_KIND", "mongo", TRUE);
    ck_assert (!setup_by (&error, NULL));
    ck_assert_ptr_nonnull (strstr (error->message, "MC_MONGO_URI is not set"));
    g_clear_error (&error);

    // mongo: the file of the database, in a directory of the user alone
    g_setenv ("MC_MONGO_URI", "mongodb://localhost", TRUE);
    g_setenv ("MC_MONGO_DB", "shop", TRUE);
    ck_assert (setup_by (&error, NULL));
    ck_assert_str_eq (edit_startup_option ("query"), "mongo");
    expected = g_build_filename (dir, "bluerig", "queries", "shop.js", (char *) NULL);
    ck_assert_str_eq (file_arg (0)->file_name, expected);
    g_free (expected);
    {
        char *queries = g_build_filename (dir, "bluerig", "queries", (char *) NULL);
        struct stat st;

        ck_assert_int_eq (stat (queries, &st), 0);
        ck_assert_int_eq (st.st_mode & 0777, 0700);
        g_free (queries);
    }

    // postgres: by the connection and the database, names that are no file names passed by
    g_setenv ("MC_QUERY_KIND", "postgres", TRUE);
    g_setenv ("MC_PG_CONNECTION", "../etc", TRUE);
    g_setenv ("PGSERVICE", "prod", TRUE);
    g_setenv ("PGDATABASE", "..", TRUE);
    g_setenv ("PGUSER", "ann", TRUE);
    ck_assert (setup_by (&error, NULL));
    expected =
        g_build_filename (dir, "bluerig", "queries", "postgres", "prod", "ann.sql", (char *) NULL);
    ck_assert_str_eq (file_arg (0)->file_name, expected);
    g_free (expected);

    // a file named is that one
    ck_assert (setup_by (&error, "my.sql", NULL));
    ck_file (0, "my.sql");
    mc_args__query = FALSE;
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

int
main (void)
{
    TCase *tc_core;

    tc_core = tcase_create ("Core");

    tcase_add_checked_fixture (tc_core, setup, teardown);

    // Add new tests here: ***************
    tcase_add_test (tc_core, test_parse);
    tcase_add_test (tc_core, test_parse_unknown);
    tcase_add_test (tc_core, test_files);
    tcase_add_test (tc_core, test_git);
    tcase_add_test (tc_core, test_debug_project);
    tcase_add_test (tc_core, test_query);
    // ***********************************

    return mctest_run_all (tc_core);
}

/* --------------------------------------------------------------------------------------------- */
