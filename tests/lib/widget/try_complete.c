/*
   lib/widget - tests for the completion of an input line: file names, directories for cd,
   commands, variables, hosts and users

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

#define TEST_SUITE_NAME "/lib/widget"

#include "tests/mctest.h"

#include <stdio.h>  // remove ()
#include <sys/stat.h>
#include <unistd.h>

#include "lib/strutil.h"
#include "lib/widget.h"

GPtrArray *try_complete (char *text, int *lc_start, int *lc_end, input_complete_t flags);

/* --------------------------------------------------------------------------------------------- */

static char *dir;  // the current directory of the test, its files and a bin of its own
static char *start_dir;

/* --------------------------------------------------------------------------------------------- */

static void
make (const char *name, int mode)
{
    char *path = g_build_filename (dir, name, (char *) NULL);

    if (g_str_has_suffix (name, "/"))
        ck_assert_int_eq (g_mkdir_with_parents (path, 0755), 0);
    else
    {
        ck_assert (g_file_set_contents (path, "", 0, NULL));
        ck_assert_int_eq (chmod (path, mode), 0);
    }
    g_free (path);
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
setup (void)
{
    char *bin, *path_env;

    str_init_strings ("UTF-8");
    start_dir = g_get_current_dir ();
    dir = g_dir_make_tmp ("complete-XXXXXX", NULL);
    ck_assert_ptr_nonnull (dir);
    ck_assert_int_eq (chdir (dir), 0);
    make ("alpha.txt", 0644);
    make ("alpine.sh", 0755);
    make ("albums/", 0);
    make ("albums/inner/", 0);
    make ("beta", 0644);
    make ("a b c", 0644);
    make ("q?.txt", 0644);
    make ("bin/", 0);
    make ("bin/mytool", 0755);
    make ("bin/mytext", 0644);
    make ("far/", 0);
    make ("far/zone/", 0);
    bin = g_build_filename (dir, "bin", (char *) NULL);
    path_env = g_strconcat (bin, ":/nonexistent", (char *) NULL);
    g_setenv ("PATH", path_env, TRUE);
    g_free (path_env);
    g_free (bin);
}

/* --------------------------------------------------------------------------------------------- */

static void
teardown (void)
{
    ck_assert_int_eq (chdir (start_dir), 0);
    remove_tree (dir);
    g_free (dir);
    g_free (start_dir);
    str_uninit_strings ();
}

/* --------------------------------------------------------------------------------------------- */

/* The completions of @text with the cursor at its end and the word from @start: the common part
   first, then each; "-" for none.  @start becomes where the word completed starts. */
static char *
complete (const char *text, int *start, input_complete_t flags)
{
    char *t = g_strdup (text);
    int end = (int) strlen (text);
    GPtrArray *matches = try_complete (t, start, &end, flags);
    GString *joined = g_string_new (NULL);
    guint i;

    if (matches == NULL)
        g_string_append (joined, "-");
    for (i = 0; matches != NULL && i < matches->len; i++)
    {
        if (i > 0)
            g_string_append_c (joined, '|');
        g_string_append (joined, g_ptr_array_index (matches, i));
    }
    if (matches != NULL)
        g_ptr_array_free (matches, TRUE);
    g_free (t);
    return g_string_free (joined, FALSE);
}

#define ck_complete(text, start, flags, expected)                                                  \
    do                                                                                             \
    {                                                                                              \
        int ck_at = (start);                                                                       \
        char *ck_got = complete ((text), &ck_at, (flags));                                         \
                                                                                                   \
        ck_assert_str_eq (ck_got, (expected));                                                     \
        g_free (ck_got);                                                                           \
    }                                                                                              \
    while (0)

/* --------------------------------------------------------------------------------------------- */
/*** tests ***************************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/* File names: the common part first, then each, a directory with its slash */
START_TEST (test_filenames)
{
    ck_complete ("al", 0, INPUT_COMPLETE_FILENAMES, "al|albums/|alpha.txt|alpine.sh");
    ck_complete ("alph", 0, INPUT_COMPLETE_FILENAMES, "alpha.txt");
    ck_complete ("albums/", 0, INPUT_COMPLETE_FILENAMES, "albums/inner/");
    ck_complete ("bin/myt", 0, INPUT_COMPLETE_FILENAMES, "bin/myt|bin/mytext|bin/mytool");
    ck_complete ("zzz", 0, INPUT_COMPLETE_FILENAMES, "-");
    // the characters of a pattern escaped, as the manual says
    ck_complete ("q", 0, INPUT_COMPLETE_FILENAMES, "q\\?.txt");
    // a word after a space
    ck_complete ("x be", 2, INPUT_COMPLETE_FILENAMES, "beta");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* The command line: names escaped for the shell, a word of escaped spaces taken whole */
START_TEST (test_shell_escape)
{
    const input_complete_t flags =
        INPUT_COMPLETE_FILENAMES | INPUT_COMPLETE_SHELL_ESC | INPUT_COMPLETE_COMMANDS;

    ck_complete ("ls a\\ ", 3, flags, "a\\ b\\ c");
    ck_complete ("ls q", 3, flags, "q\\?.txt");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* cd: directories only, then those of CDPATH */
START_TEST (test_cd)
{
    const input_complete_t flags = INPUT_COMPLETE_FILENAMES | INPUT_COMPLETE_CD;
    char *cdpath = g_build_filename (dir, "far", (char *) NULL);

    ck_complete ("cd al", 3, flags, "albums/");
    // a file is no directory to go to
    ck_complete ("cd bet", 3, flags, "-");
    g_setenv ("CDPATH", cdpath, TRUE);
    {
        char *zone = g_build_filename (cdpath, "zone/", (char *) NULL);

        ck_complete ("cd zo", 3, flags, zone);
        g_free (zone);
    }
    g_unsetenv ("CDPATH");
    // the whole input a directory, as in a dialog of cd
    ck_complete ("al", 0, INPUT_COMPLETE_CD, "albums/");
    // not cd: files too
    ck_complete ("cdx al", 4, flags, "al|albums/|alpha.txt|alpine.sh");
    g_free (cdpath);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* Commands: the words of the shell, its builtins, the programs of PATH; at the start, after a
   separator, in backquotes */
START_TEST (test_commands)
{
    const input_complete_t flags = INPUT_COMPLETE_COMMANDS | INPUT_COMPLETE_FILENAMES;

    ck_complete ("whi", 0, flags, "while");
    ck_complete ("ec", 0, flags, "echo");
    // what can run in PATH, not the other files
    ck_complete ("myt", 0, flags, "mytool");
    ck_complete ("ls; myt", 4, flags, "mytool");
    ck_complete ("  myt", 2, flags, "mytool");
    // after a redirection it is a file
    ck_complete ("ls >& myt", 6, flags, "-");
    ck_complete ("ls >| myt", 6, flags, "-");
    // a path: files that run and directories
    ck_complete ("./alp", 0, flags, "./alpine.sh");
    ck_complete ("echo `myt", 5, flags, "mytool");
    // not a command line
    ck_complete ("ec", 0, INPUT_COMPLETE_FILENAMES, "-");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* Variables of the environment, $NAME and ${NAME}; \$ is no variable */
START_TEST (test_variables)
{
    const input_complete_t flags = INPUT_COMPLETE_VARIABLES | INPUT_COMPLETE_FILENAMES;
    int at = 5;
    char *got;

    g_setenv ("COMPLETE_TEST_ONE", "1", TRUE);
    g_setenv ("COMPLETE_TEST_TWO", "2", TRUE);
    ck_complete ("$COMPLETE_TEST_O", 0, flags, "$COMPLETE_TEST_ONE");
    ck_complete ("${COMPLETE_TEST_T", 0, flags, "${COMPLETE_TEST_TWO}");
    ck_complete ("$COMPLETE_TEST_", 0, flags,
                 "$COMPLETE_TEST_|$COMPLETE_TEST_ONE|$COMPLETE_TEST_TWO");
    // the word completed starts at the $
    got = complete ("echo x$COMPLETE_TEST_O", &at, flags);
    ck_assert_str_eq (got, "$COMPLETE_TEST_ONE");
    ck_assert_int_eq (at, 6);
    g_free (got);
    ck_complete ("\\$COMPLETE_TEST_O", 0, flags, "-");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* Hosts of HOSTFILE, its comments and includes, each once */
START_TEST (test_hosts)
{
    char *hosts = g_build_filename (dir, "hosts", (char *) NULL);
    char *more = g_build_filename (dir, "more", (char *) NULL);
    char *text = g_strdup_printf ("# a comment myhostx\n"
                                  "127.0.0.1 localhost myhost # myhostz\n"
                                  "$include %s\n"
                                  "  10.0.0.1\tmyhost2 myhost\n",
                                  more);

    ck_assert (g_file_set_contents (hosts, text, -1, NULL));
    ck_assert (g_file_set_contents (more, "10.0.0.2 myhost3\n", -1, NULL));
    g_setenv ("HOSTFILE", hosts, TRUE);
    ck_complete ("@myh", 0, INPUT_COMPLETE_HOSTNAMES, "@myhost|@myhost|@myhost2|@myhost3");
    ck_complete ("@loc", 0, INPUT_COMPLETE_HOSTNAMES, "@localhost");
    ck_complete ("ssh user@myhost3", 4, INPUT_COMPLETE_HOSTNAMES, "@myhost3");
    g_unsetenv ("HOSTFILE");
    g_free (text);
    g_free (more);
    g_free (hosts);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* Users after a tilde, with their slash */
START_TEST (test_users)
{
    {
        int at = 0;
        char *got = complete ("~roo", &at, INPUT_COMPLETE_USERNAMES);

        // macOS lists root twice
        ck_assert (g_str_has_prefix (got, "~root/"));
        g_free (got);
    }
    ck_complete ("~no-such-user-at-all", 0, INPUT_COMPLETE_USERNAMES, "-");
    // a tilde with a slash is a path
    ck_complete ("~root/", 0, INPUT_COMPLETE_USERNAMES, "-");
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
    tcase_add_test (tc_core, test_filenames);
    tcase_add_test (tc_core, test_shell_escape);
    tcase_add_test (tc_core, test_cd);
    tcase_add_test (tc_core, test_commands);
    tcase_add_test (tc_core, test_variables);
    tcase_add_test (tc_core, test_hosts);
    tcase_add_test (tc_core, test_users);
    // ***********************************

    return mctest_run_all (tc_core);
}

/* --------------------------------------------------------------------------------------------- */
