/*
   src/usermenu - tests for the lines of conditions of the user menu

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

#define TEST_SUITE_NAME "/src/usermenu"

#include "tests/mctest.h"

#include <sys/stat.h>
#include <unistd.h>

#include "lib/charsets.h"

/* the name and the syntax of the edited file come from the test, not from an editor */
#define edit_get_file_name   test_edit_get_file_name
#define edit_get_syntax_type test_edit_get_syntax_type

#include "src/usermenu.c"

/* --------------------------------------------------------------------------------------------- */

static char *dir;  // the files the conditions look at
static char *edited_file_name;
static const char *edited_syntax;
static GString *told;  // what message () has shown

/* stands in for the editor the menu is shown for */
static Widget fake_editor;

/* --------------------------------------------------------------------------------------------- */

/* @Mock */
const char *
test_edit_get_file_name (const WEdit *edit)
{
    (void) edit;
    return edited_file_name;
}

/* @Mock */
const char *
test_edit_get_syntax_type (const WEdit *edit)
{
    (void) edit;
    return edited_syntax;
}

/* @Mock */
void
message (int flags, const char *title, const char *text, ...)
{
    va_list args;
    char *line;

    (void) flags;
    (void) title;
    va_start (args, text);
    line = g_strdup_vprintf (text, args);
    va_end (args);
    g_string_append (told, line);
    g_string_append_c (told, '\n');
    g_free (line);
}

/* --------------------------------------------------------------------------------------------- */

static char *
in_dir (const char *name)
{
    return g_build_filename (dir, name, (char *) NULL);
}

static void
make_file (const char *name, int mode)
{
    char *path = in_dir (name);

    ck_assert (g_file_set_contents (path, "#!/bin/sh\n", -1, NULL));
    ck_assert_int_eq (chmod (path, mode), 0);
    g_free (path);
}

static void
setup (void)
{
    char *target, *link;

    str_init_strings ("UTF-8");
    mc_global.utf8_display = TRUE;
    easy_patterns = FALSE;
    debug_flag = FALSE;
    debug_error = FALSE;
    told = g_string_new (NULL);
    dir = g_dir_make_tmp ("usermenu-XXXXXX", NULL);
    ck_assert_ptr_nonnull (dir);
    make_file ("a.c", 0644);
    make_file ("a b.c", 0644);
    make_file ("run.sh", 0755);
    make_file ("group.sh", 0610);
    target = in_dir ("sub");
    ck_assert_int_eq (mkdir (target, 0755), 0);
    g_free (target);
    target = in_dir ("a.c");
    link = in_dir ("link");
    ck_assert_int_eq (symlink (target, link), 0);
    g_free (link);
    g_free (target);
    edited_file_name = in_dir ("a.c");
    edited_syntax = "C Program";
}

static void
teardown (void)
{
    static const char *const names[] = { "a.c", "a b.c", "run.sh", "group.sh", "sub", "link" };
    size_t i;

    for (i = 0; i < G_N_ELEMENTS (names); i++)
    {
        char *path = in_dir (names[i]);

        (void) remove (path);
        g_free (path);
    }
    (void) rmdir (dir);
    g_free (dir);
    g_free (edited_file_name);
    edited_file_name = NULL;
    g_string_free (told, TRUE);
    str_uninit_strings ();
}

/* The truth of a line of conditions, as the menu file has it: one line, its end with it, and the
   end of the line before it, which the parser may look back at */
static gboolean
line_is (const char *line, gboolean before)
{
    char *text = g_strconcat ("\n", line, "\n", (char *) NULL);
    gboolean result = before;
    char *end;

    end = test_line (&fake_editor, text + 1, &result);
    // the point just before the end of the line
    ck_assert_int_eq (end[1], '\n');
    g_free (text);
    return result;
}

static void
edit (const char *name)
{
    g_free (edited_file_name);
    edited_file_name = name != NULL ? in_dir (name) : NULL;
}

/* --------------------------------------------------------------------------------------------- */

/* The operators: + and = set, & and | take the one before, from left to right, ! turns one */
START_TEST (check_operators)
{
    ck_assert (line_is ("+ f \\.c$", FALSE));
    ck_assert (!line_is ("= f \\.h$", TRUE));
    ck_assert (!line_is ("+ f \\.c$ & t d", TRUE));
    ck_assert (line_is ("+ f \\.h$ | t r", FALSE));
    ck_assert (line_is ("+ ! f \\.h$", FALSE));
    ck_assert (!line_is ("+ !f \\.c$", TRUE));
    // no priority: (d | f) & x, not d | (f & x)
    ck_assert (!line_is ("+ t d | f \\.c$ & t x", TRUE));
    ck_assert (line_is ("+ t d & t x | f \\.c$", FALSE));
    // & and | alone take what was before the line
    ck_assert (line_is ("& f \\.c$", TRUE));
    ck_assert (!line_is ("& f \\.c$", FALSE));
    ck_assert (line_is ("| t d", TRUE));
    // the letters as capitals too
    ck_assert (line_is ("+ F \\.C$", FALSE) == FALSE);
    ck_assert (line_is ("+ F \\.c$ & T r", FALSE));
    // nothing on the line: as before
    ck_assert (line_is ("", TRUE));
    ck_assert (!line_is ("   ", FALSE));
    ck_assert (line_is ("+", TRUE));
}
END_TEST

/* The type of the file edited, by lstat (): a link is not what it points to */
START_TEST (check_types)
{
    ck_assert (line_is ("+ t r", FALSE));
    ck_assert (line_is ("+ t n", FALSE));
    ck_assert (!line_is ("+ t x", TRUE));
    ck_assert (!line_is ("+ t l", TRUE));
    // any of the letters
    ck_assert (line_is ("+ t dlr", FALSE));
    edit ("run.sh");
    ck_assert (line_is ("+ t x", FALSE));
    // of the group alone
    edit ("group.sh");
    ck_assert (line_is ("+ t x", FALSE));
    edit ("link");
    ck_assert (line_is ("+ t l", FALSE));
    ck_assert (!line_is ("+ t r", TRUE));
    // no directory, though no regular file
    ck_assert (line_is ("+ t n", FALSE));
    edit ("sub");
    ck_assert (line_is ("+ t d", FALSE));
    ck_assert (!line_is ("+ t n", TRUE));
    ck_assert (!line_is ("+ t cbfs", TRUE));
    // a file that is not there, and no file: of no type
    edit ("gone");
    ck_assert (!line_is ("+ t n", TRUE));
    edit (NULL);
    ck_assert (!line_is ("+ t n", TRUE));
    ck_assert (!line_is ("+ f .*", TRUE));
    ck_assert (!debug_error);
    // a letter that is no type
    edit ("a.c");
    ck_assert (line_is ("+ t rq", FALSE));
    ck_assert (debug_error);
}
END_TEST

/* The directory, a program, the syntax of the file */
START_TEST (check_other_conditions)
{
    char *cwd = g_get_current_dir ();
    char *line;

    line = g_strdup_printf ("+ d ^%s$", cwd);
    ck_assert (line_is (line, FALSE));
    g_free (line);
    ck_assert (!line_is ("+ d ^/nonexistent-dir$", TRUE));
    g_free (cwd);

    line = g_strdup_printf ("+ x %s/run.sh", dir);
    ck_assert (line_is (line, FALSE));
    g_free (line);
    line = g_strdup_printf ("+ x %s/a.c", dir);
    ck_assert (!line_is (line, TRUE));
    g_free (line);
    ck_assert (!line_is ("+ x /nonexistent/program", TRUE));

    // a part of the name of the syntax
    ck_assert (line_is ("+ y Program", FALSE));
    ck_assert (!line_is ("+ y Python", TRUE));
}
END_TEST

/* A space with a backslash before it is of the argument */
START_TEST (check_quoted_space)
{
    edit ("a b.c");
    ck_assert (line_is ("+ f a\\ b\\.c$", FALSE));
    ck_assert (!line_is ("+ f ^a\\.c$", TRUE));
}
END_TEST

/* shell_patterns=1: the patterns of the shell, not regular expressions */
START_TEST (check_shell_patterns)
{
    char text[] = "shell_patterns=1  + f *.c\n";
    char *p;

    p = check_patterns (text);
    ck_assert (easy_patterns);
    ck_assert_str_eq (p, "+ f *.c\n");
    ck_assert (line_is ("+ f *.c", FALSE));
    ck_assert (!line_is ("+ f \\.c$", TRUE));

    ck_assert_ptr_eq (check_patterns (text + 1), text + 1);
    {
        char off[] = "shell_patterns=0\n+ f x";
        char bad[] = "shell_patterns=2 x";

        p = check_patterns (off);
        ck_assert (!easy_patterns);
        ck_assert_str_eq (p, "+ f x");
        ck_assert_ptr_eq (check_patterns (bad), bad);
        ck_assert (!easy_patterns);
    }
}
END_TEST

/* With ? after the operator the user is shown how the line came out */
START_TEST (check_debug)
{
    ck_assert (line_is ("+? f \\.c$ & t d", FALSE) == FALSE);
    ck_assert_str_eq (told->str, " True: f \\.c$  \n True: + \n False: t d \n False: & \n");
    ck_assert (!debug_flag);
    // an error in a condition is shown as one
    g_string_truncate (told, 0);
    (void) line_is ("+? q & f \\.c$", FALSE);
    ck_assert_ptr_nonnull (strstr (told->str, "ERROR: q"));
    ck_assert (!debug_error);
    // without ?, nothing
    g_string_truncate (told, 0);
    (void) line_is ("+ q & f \\.c$", FALSE);
    ck_assert_str_eq (told->str, "");
    // an operator that is none
    g_string_truncate (told, 0);
    debug_error = FALSE;
    (void) line_is ("*? f x", TRUE);
    ck_assert_ptr_nonnull (strstr (told->str, "ERROR: *"));
}
END_TEST

/* The name of a file without its extension; a dot of a directory is no extension */
START_TEST (check_strip_ext)
{
    char a[] = "a.tar.gz", b[] = "dir.d/file", c[] = ".hidden", d[] = "noext";

    ck_assert_str_eq (strip_ext (a), "a.tar");
    ck_assert_str_eq (strip_ext (b), "dir.d/file");
    ck_assert_ptr_null (strip_ext (c));
    ck_assert_str_eq (strip_ext (d), "noext");
    ck_assert_ptr_null (strip_ext (NULL));
}
END_TEST

/* An argument up to a space or the end of the line, as much as fits */
START_TEST (check_extract_arg)
{
    char text[] = "   abc\\ def ghi\n";
    char small[] = "abcdef\n";
    char arg[16];
    char *p;

    p = extract_arg (text, arg, sizeof (arg));
    ck_assert_str_eq (arg, "abc\\ def");
    ck_assert_str_eq (p, " ghi\n");
    p = extract_arg (p, arg, sizeof (arg));
    ck_assert_str_eq (arg, "ghi");
    // before the end of the line
    ck_assert_int_eq (*p, 'i');
    p = extract_arg (small, arg, 4);
    ck_assert_str_eq (arg, "abc");
    ck_assert_str_eq (p, "def\n");
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
    tcase_add_test (tc_core, check_operators);
    tcase_add_test (tc_core, check_types);
    tcase_add_test (tc_core, check_other_conditions);
    tcase_add_test (tc_core, check_quoted_space);
    tcase_add_test (tc_core, check_shell_patterns);
    tcase_add_test (tc_core, check_debug);
    tcase_add_test (tc_core, check_strip_ext);
    tcase_add_test (tc_core, check_extract_arg);
    // ***********************************

    return mctest_run_all (tc_core);
}

/* --------------------------------------------------------------------------------------------- */
