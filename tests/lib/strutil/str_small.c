/*
   lib/strutil - tests for the words of a command, the escapes of the shell, of globs and of
   regular expressions, and the numbers with their suffixes

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

#define TEST_SUITE_NAME "/lib/strutil"

#include "tests/mctest.h"

#include "lib/strutil.h"

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

/* The words of @line joined by '|', "-" for none */
static char *
words (const char *line)
{
    GPtrArray *tokens = str_tokenize (line);
    GString *joined = g_string_new (NULL);
    guint i;

    if (tokens == NULL)
    {
        g_string_free (joined, TRUE);
        return g_strdup ("-");
    }
    for (i = 0; i < tokens->len; i++)
    {
        if (i > 0)
            g_string_append_c (joined, '|');
        g_string_append (joined, g_ptr_array_index (tokens, i));
    }
    g_ptr_array_set_free_func (tokens, g_free);
    g_ptr_array_free (tokens, TRUE);
    return g_string_free (joined, FALSE);
}

#define ck_words(line, expected)                                                                   \
    do                                                                                             \
    {                                                                                              \
        char *got = words (line);                                                                  \
                                                                                                   \
        ck_assert_str_eq (got, (expected));                                                        \
        g_free (got);                                                                              \
    }                                                                                              \
    while (0)

#define ck_escape(fn, text, expected)                                                              \
    do                                                                                             \
    {                                                                                              \
        char *got = fn (text);                                                                     \
                                                                                                   \
        ck_assert_str_eq (got, (expected));                                                        \
        g_free (got);                                                                              \
    }                                                                                              \
    while (0)

/* --------------------------------------------------------------------------------------------- */
/*** tests ***************************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/* The words of a command where the shell splits them */
START_TEST (test_tokenize)
{
    ck_words ("ls -l  /tmp", "ls|-l|/tmp");
    ck_words ("   ", "-");
    ck_words ("", "-");
    // quotes keep a word whole, a backslash a space
    ck_words ("echo 'a b' \"c d\" e\\ f", "echo|'a b'|\"c d\"|e\\ f");
    ck_words ("echo \"a \\\" b\"", "echo|\"a \\\" b\"");
    // the operators are words of their own
    ck_words ("a|b&&c;d", "a|||b|&&|c|;|d");
    ck_words ("cmd >out 2>&1 <in", "cmd|>|out|2>&1|<|in");
    ck_words ("cat <<EOF", "cat|<<|EOF");
    ck_words ("cat <<-EOF", "cat|<<-|EOF");
    ck_words ("x >| y &> z", "x|>||y|&>|z");
    ck_words ("x <&3- y", "x|<&3-|y");
    // a number is a word, and a file descriptor before a redirection
    ck_words ("head -n 10", "head|-n|10");
    ck_words ("echo 12abc", "echo|12abc");
    ck_words ("echo 7", "echo|7");
    // substitutions and their parentheses
    ck_words ("echo $(ls -l) done", "echo|$(ls -l)|done");
    ck_words ("diff <(a b) x", "diff|<(a b)|x");
    ck_words ("ls (a)", "ls|(|a|)");
    ck_words ("ls ((a))", "ls|(|(|a|)|)");
    ck_words ("echo $(a (b) c) d", "echo|$(a (b) c)|d");
    // quotes in a word keep its spaces
    ck_words ("a'b c'd e", "a'b c'd|e");
    ck_words ("echo `date`", "echo|`date`");
    // a backslash and a new line go on the line
    ck_words ("a\\\nb c", "a\\\nb|c");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* Escapes for the shell, for globs and for regular expressions, and back */
START_TEST (test_escapes)
{
    ck_escape (str_shell_escape, "a b$c", "a\\ b\\$c");
    ck_escape (str_shell_escape, "", "");
    ck_assert_ptr_null (str_shell_escape (NULL));
    ck_escape (str_glob_escape, "a*b?c$d\\e.f", "a\\*b\\?c\\$d\\\\e.f");
    // a glob and a regular expression show the characters that do not show
    ck_escape (str_glob_escape, "a\tb\nc", "a\\tb\\nc");
    ck_escape (str_regex_escape, "a.b+c^(d)", "a\\.b\\+c\\^\\(d\\)");

    ck_escape (str_shell_unescape, "a\\ b\\(c\\)", "a b(c)");
    // \$ stays: no variable there
    ck_escape (str_shell_unescape, "a\\$b", "a\\$b");
    // a backslash before what is not escaped stays
    ck_escape (str_shell_unescape, "a\\zb", "a\\zb");
    ck_escape (str_glob_unescape, "a\\*b\\tc", "a*b\tc");
    ck_escape (str_regex_unescape, "a\\.b\\nc", "a.b\nc");
    ck_escape (str_shell_unescape, "x", "x");
    ck_escape (str_shell_unescape, "a\\", "a\\");
    ck_escape (str_shell_unescape, "", "");
    ck_assert_ptr_null (str_shell_unescape (NULL));
    {
        char *round = str_shell_escape ("we're (here) & $there");
        char *back = str_shell_unescape (round);

        // all of it back, the dollar kept escaped
        ck_assert_str_eq (back, "we're (here) & \\$there");
        g_free (back);
        g_free (round);
    }
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* Whether a character has an odd number of backslashes before it */
START_TEST (test_is_char_escaped)
{
    const char *s = "a\\b\\\\c\\\\\\d";

    ck_assert (!str_is_char_escaped (s, s));
    ck_assert (str_is_char_escaped (s, s + 2));
    ck_assert (!str_is_char_escaped (s, s + 5));
    ck_assert (str_is_char_escaped (s, s + 9));
    ck_assert (!str_is_char_escaped (NULL, s));
    ck_assert (!str_is_char_escaped (s, NULL));
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

static void
ck_number (const char *text, const char *suffixes, strtol_error_t err, uintmax_t expected)
{
    uintmax_t val = 12345;

    ck_assert_int_eq (xstrtoumax (text, NULL, 10, &val, suffixes), err);
    if (err == LONGINT_OK || err == LONGINT_INVALID_SUFFIX_CHAR || err == LONGINT_OVERFLOW)
        ck_assert_uint_eq (val, expected);
}

/* A number and its suffix: of 1024, of 1000 with B, blocks and words; what is not one */
START_TEST (test_xstrtoumax)
{
    const char *all = "bBcEGgkKMmPQRTtwYZ0";

    ck_number ("42", NULL, LONGINT_OK, 42);
    ck_number ("  42", "", LONGINT_OK, 42);
    ck_number ("42x", NULL, LONGINT_OK, 42);
    ck_number ("-1", all, LONGINT_INVALID, 0);
    ck_number ("abc", all, LONGINT_INVALID, 0);
    ck_number ("", all, LONGINT_INVALID, 0);
    ck_number ("3k", all, LONGINT_OK, 3 * 1024);
    ck_number ("3K", all, LONGINT_OK, 3 * 1024);
    ck_number ("3kB", all, LONGINT_OK, 3000);
    ck_number ("3KiB", all, LONGINT_OK, 3 * 1024);
    ck_number ("2M", all, LONGINT_OK, 2 * 1024 * 1024);
    ck_number ("2MB", all, LONGINT_OK, 2000000);
    ck_number ("1G", all, LONGINT_OK, UINTMAX_C (1) << 30);
    ck_number ("1T", all, LONGINT_OK, UINTMAX_C (1) << 40);
    ck_number ("1P", all, LONGINT_OK, UINTMAX_C (1) << 50);
    ck_number ("1E", all, LONGINT_OK, UINTMAX_C (1) << 60);
    ck_number ("4b", all, LONGINT_OK, 4 * 512);
    ck_number ("4B", all, LONGINT_OK, 4 * 1024);
    ck_number ("4c", all, LONGINT_OK, 4);
    ck_number ("4w", all, LONGINT_OK, 8);
    // a suffix alone is one of it
    ck_number ("k", all, LONGINT_OK, 1024);
    // a suffix not allowed, or more after it
    ck_number ("4k", "M", LONGINT_INVALID_SUFFIX_CHAR, 4);
    ck_number ("4kx", all, LONGINT_INVALID_SUFFIX_CHAR, 4096);
    // too large
    ck_number ("16Z", all, LONGINT_OVERFLOW, UINTMAX_MAX);
    ck_number ("1Q", all, LONGINT_OVERFLOW, UINTMAX_MAX);
    {
        char *end = NULL;
        uintmax_t val = 0;

        ck_assert_int_eq (xstrtoumax ("7m rest", &end, 10, &val, NULL), LONGINT_OK);
        ck_assert_str_eq (end, "m rest");
        ck_assert_int_eq (xstrtoumax ("0x1f", &end, 16, &val, ""), LONGINT_OK);
        ck_assert_uint_eq (val, 31);
    }
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
    tcase_add_test (tc_core, test_tokenize);
    tcase_add_test (tc_core, test_escapes);
    tcase_add_test (tc_core, test_is_char_escaped);
    tcase_add_test (tc_core, test_xstrtoumax);
    // ***********************************

    return mctest_run_all (tc_core);
}

/* --------------------------------------------------------------------------------------------- */
