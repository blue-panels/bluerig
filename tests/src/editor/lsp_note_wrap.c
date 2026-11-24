/*
   src/plugins/lsp - tests for the lines of the note of the LSP: the text in the columns of the box

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

#define TEST_SUITE_NAME "/src/plugins/lsp"

#include "tests/mctest.h"

#include "src/plugins/lsp/lsp-note.c"  // for its file scope functions

/* the marks of the text, as the tests write them */
#define HOT  "\001"
#define COLD "\002"
#define HEAD "\003"
#define CODE "\004"
#define RULE "\005"

/* U+65E5 and U+672C, two columns each */
#define WIDE1 "\xe6\x97\xa5"
#define WIDE2 "\xe6\x9c\xac"

/* --------------------------------------------------------------------------------------------- */

/* The lines of @text in @cols columns, joined by '|' */
static char *
wrap (const char *text, int cols)
{
    GPtrArray *lines = lsp_note_wrap (text, cols);
    GString *joined = g_string_new (NULL);
    guint i;

    for (i = 0; i < lines->len; i++)
    {
        if (i > 0)
            g_string_append_c (joined, '|');
        g_string_append (joined, g_ptr_array_index (lines, i));
    }
    g_ptr_array_free (lines, TRUE);
    return g_string_free (joined, FALSE);
}

#define ck_wrap(text, cols, expected)                                                              \
    do                                                                                             \
    {                                                                                              \
        char *got = wrap ((text), (cols));                                                         \
                                                                                                   \
        ck_assert_str_eq (got, (expected));                                                        \
        g_free (got);                                                                              \
    }                                                                                              \
    while (0)

/* --------------------------------------------------------------------------------------------- */

static void
setup (void)
{
    str_init_strings ("UTF-8");
}

/* --------------------------------------------------------------------------------------------- */

static void
teardown (void)
{
    str_uninit_strings ();
}

/* --------------------------------------------------------------------------------------------- */

/* The columns of a line: no marks, no kind of the line, two for a wide character */
START_TEST (test_width)
{
    ck_assert_int_eq (lsp_note_width ("plain"), 5);
    ck_assert_int_eq (lsp_note_width (HOT "hot" COLD " x"), 5);
    ck_assert_int_eq (lsp_note_width (HEAD "Title"), 5);
    ck_assert_int_eq (lsp_note_width (CODE "int x;"), 6);
    ck_assert_int_eq (lsp_note_width (RULE), 0);
    ck_assert_int_eq (lsp_note_width (WIDE1 WIDE2 "a"), 5);
    ck_assert_int_eq (lsp_note_width (""), 0);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* Text: broken after the last space that fits, going on at a word */
START_TEST (test_wrap_text)
{
    ck_wrap ("short", 10, "short");
    ck_wrap ("hello world again", 10, "hello |world |again");
    ck_wrap ("hello     world", 8, "hello   |world");
    // a word too long for a line is broken in it
    ck_wrap ("abcdefghijkl", 5, "abcde|fghij|kl");
    // a line of the text each, the empty lines at the end left out
    ck_wrap ("one\ntwo\n\n\n", 10, "one|two");
    ck_wrap ("one\n\ntwo", 10, "one||two");
    ck_wrap ("", 10, "");
    ck_wrap ("\n\n", 10, "");
    // what went on under itself was spaces
    ck_wrap ("abcde     ", 5, "abcde");
    ck_wrap ("abcde     \nx", 5, "abcde|x");
    // the ends of the parts of code break a line too
    ck_wrap ("foo(bar,baz)", 8, "foo(bar,|baz)");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* Kinds of lines: a heading goes on as one, code moved in by two, a rule as it is */
START_TEST (test_wrap_kinds)
{
    ck_wrap (HEAD "The heading here", 8, HEAD "The |" HEAD "heading |" HEAD "here");
    ck_wrap (CODE "int foo(int a)", 8, CODE "int foo(|" CODE "  int a)");
    ck_wrap (CODE "abcdefghij", 6, CODE "abcdef|" CODE "  ghij");
    ck_wrap (RULE, 3, RULE);
    ck_wrap ("text\n" RULE "\n" CODE "x", 10, "text|" RULE "|" CODE "x");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* What is hot stays hot across a break: off at the end of the line, on again on the next */
START_TEST (test_wrap_hot)
{
    ck_wrap (HOT "hot text here" COLD " cold", 9, HOT "hot text " COLD "|" HOT "here" COLD " cold");
    ck_wrap ("a " HOT "b" COLD " c", 10, "a " HOT "b" COLD " c");
    // broken before a hot part: the line before it is not hot
    ck_wrap ("a " HOT "bcdef" COLD, 4, "a |" HOT "bcde" COLD "|" HOT "f" COLD);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* Wide characters by their columns; a line takes one at least */
START_TEST (test_wrap_wide)
{
    ck_wrap (WIDE1 WIDE2 WIDE1 WIDE2, 5, WIDE1 WIDE2 "|" WIDE1 WIDE2);
    ck_wrap (WIDE1 WIDE2, 1, WIDE1 "|" WIDE2);
    ck_wrap ("ab", 0, "a|b");
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
    tcase_add_test (tc_core, test_width);
    tcase_add_test (tc_core, test_wrap_text);
    tcase_add_test (tc_core, test_wrap_kinds);
    tcase_add_test (tc_core, test_wrap_hot);
    tcase_add_test (tc_core, test_wrap_wide);
    // ***********************************

    return mctest_run_all (tc_core);
}

/* --------------------------------------------------------------------------------------------- */
