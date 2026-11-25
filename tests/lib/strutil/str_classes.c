/*
   lib/strutil - tests for the strings of a terminal of ASCII and of an 8-bit codeset: the same
   functions as for UTF-8, a byte a character and a column

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
teardown (void)
{
    str_uninit_strings ();
}

/* --------------------------------------------------------------------------------------------- */

/* What the two codesets do alike with a text of ASCII */
static void
check_common (void)
{
    const char *text = "abc def";
    const char *p;
    char *s;
    char buf[8] = { 0 };
    char *out = buf;
    size_t remain = sizeof (buf) - 1;

    ck_assert (str_is_valid_string (text));
    ck_assert_int_eq (str_is_valid_char ("a", 1), 1);
    p = text;
    str_cnext_char (&p);
    ck_assert_ptr_eq (p, text + 1);
    str_cprev_char (&p);
    ck_assert_ptr_eq (p, text);
    ck_assert_ptr_eq (str_cget_next_char_safe (text), text + 1);
    ck_assert_ptr_eq (str_cget_prev_char_safe (text + 2), text + 1);
    p = text;
    ck_assert_int_eq (str_cnext_noncomb_char (&p), 1);
    ck_assert_int_eq (str_cprev_noncomb_char (&p, text), 1);
    ck_assert_ptr_eq (p, text);
    ck_assert_int_eq (str_cprev_noncomb_char (&p, text), 0);

    ck_assert (str_isspace (" "));
    ck_assert (!str_isspace ("a"));
    ck_assert (str_ispunct ("!"));
    ck_assert (!str_ispunct ("a"));
    ck_assert (str_isalnum ("7"));
    ck_assert (!str_isalnum ("-"));
    ck_assert (str_isdigit ("7"));
    ck_assert (!str_isdigit ("a"));
    ck_assert (str_isprint ("a"));
    ck_assert (!str_isprint ("\x01"));
    ck_assert (!str_iscombiningmark ("a"));
    ck_assert (str_toupper ("a", &out, &remain));
    ck_assert (str_tolower ("B", &out, &remain));
    ck_assert_str_eq (buf, "Ab");
    remain = 1;
    ck_assert (!str_toupper ("a", &out, &remain));

    ck_assert_int_eq (str_length (text), 7);
    ck_assert_int_eq (str_length2 (text, 3), 3);
    ck_assert_int_eq (str_length2 (text, -1), 7);
    ck_assert_int_eq (str_length_char (text), 1);
    ck_assert_int_eq (str_length_noncomb (text), 7);
    ck_assert_int_eq (str_term_width1 (text), 7);
    ck_assert_int_eq (str_term_width2 (text, 4), 4);
    ck_assert_int_eq (str_term_char_width ("a"), 1);
    ck_assert_int_eq (str_offset_to_pos (text, 3), 3);
    ck_assert_int_eq (str_column_to_pos (text, 5), 5);

    ck_assert_str_eq (str_fit_to_term ("abc", 6, J_LEFT), "abc   ");
    ck_assert_str_eq (str_fit_to_term ("abc", 6, J_RIGHT), "   abc");
    ck_assert_str_eq (str_fit_to_term ("abc", 6, J_CENTER), " abc  ");
    ck_assert_str_eq (str_fit_to_term ("abcdefgh", 5, J_LEFT), "abcde");
    ck_assert_str_eq (str_fit_to_term ("abcdefgh", 5, J_RIGHT), "defgh");
    ck_assert_str_eq (str_fit_to_term ("abcdefgh", 5, J_LEFT_FIT), "ab~gh");
    ck_assert_str_eq (str_term_trim ("abcdefgh", 6), "...fgh");
    ck_assert_str_eq (str_term_trim ("abcdefgh", 2), "..");
    ck_assert_str_eq (str_term_trim ("abcdefgh", 9), "abcdefgh");
    ck_assert_str_eq (str_term_substring ("abcdef", 2, 3), "cde");
    ck_assert_str_eq (str_term_substring ("ab", 1, 4), "b   ");
    ck_assert_str_eq (str_trunc ("abcdefgh", 5), "ab~gh");
    ck_assert_str_eq (str_trunc ("abc", 5), "abc");
    ck_assert_str_eq (str_term_form ("a\tb"), "a.b");

    s = g_strdup ("ab");
    str_fix_string (s);
    ck_assert_str_eq (s, "ab");
    g_free (s);

    {
        char *needle = str_create_search_needle ("DEF", FALSE);

        ck_assert_str_eq (str_search_first ("abc def DEF", needle, FALSE), "def DEF");
        ck_assert_str_eq (str_search_last ("abc def DEF", needle, FALSE), "DEF");
        str_release_search_needle (needle, FALSE);
        needle = str_create_search_needle ("DEF", TRUE);
        ck_assert_str_eq (str_search_first ("abc def DEF", needle, TRUE), "DEF");
        ck_assert_ptr_null (str_search_first ("abc def", needle, TRUE));
        str_release_search_needle (needle, TRUE);
    }

    ck_assert_int_lt (str_compare ("a", "b"), 0);
    ck_assert_int_eq (str_compare ("ab", "ab"), 0);
    ck_assert_int_eq (str_ncompare ("abx", "ab"), 0);
    ck_assert_int_ne (str_ncompare ("axb", "ab"), 0);
    ck_assert_int_eq (str_casecmp ("AbC", "aBc"), 0);
    ck_assert_int_eq (str_ncasecmp ("ABx", "ab"), 0);
    ck_assert_int_eq (str_prefix ("abcd", "abx"), 2);
    ck_assert_int_eq (str_caseprefix ("ABcd", "abx"), 2);
    {
        char *a = str_create_key ("apple", TRUE);
        char *b = str_create_key ("banana", TRUE);
        char *c = str_create_key ("BANANA", FALSE);
        char *d = str_create_key ("banana", FALSE);

        ck_assert_int_lt (str_key_collate (a, b, TRUE), 0);
        ck_assert_int_eq (str_key_collate (c, d, FALSE), 0);
        str_release_key (a, TRUE);
        str_release_key (b, TRUE);
        str_release_key (c, FALSE);
        str_release_key (d, FALSE);
    }
    {
        GString *g = g_string_new ("x");

        str_insert_replace_char (g);
        ck_assert_str_eq (g->str, "x?");
        str_printf (g, "%d-%s", 4, "y");
        ck_assert_str_eq (g->str, "x?4-y");
        g_string_free (g, TRUE);
    }
    {
        GError *error = g_error_new_literal (G_CONVERT_ERROR, 0, "the reason");
        char *msg = str_conv_gerror_message (error, "default");

        ck_assert_ptr_nonnull (msg);
        g_free (msg);
        msg = str_conv_gerror_message (NULL, "default");
        ck_assert_str_eq (msg, "default");
        g_free (msg);
        g_error_free (error);
    }
}

/* --------------------------------------------------------------------------------------------- */
/*** tests ***************************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/* A terminal of ASCII: a byte above it is shown as '?' */
START_TEST (test_ascii)
{
    char *s;

    str_init_strings ("ASCII");
    check_common ();
    // any byte is taken; what is not ASCII is shown as '?'
    ck_assert (str_is_valid_string ("a\xd6"));
    ck_assert_int_eq (str_is_valid_char ("\xd6", 1), 1);
    ck_assert_str_eq (str_term_form ("a\xd6"), "a?");
    ck_assert_str_eq (str_fit_to_term ("\xd6"
                                       "b",
                                       3, J_LEFT),
                      "?b ");
    s = g_strdup ("a\xd6");
    str_fix_string (s);
    ck_assert_str_eq (s, "a?");
    g_free (s);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* An 8-bit codeset: every byte a character of its own, its letters of either case */
START_TEST (test_8bit)
{
    char buf[4] = { 0 };
    char *out = buf;
    size_t remain = sizeof (buf) - 1;

    str_init_strings ("KOI8-R");
    check_common ();
    ck_assert (str_is_valid_string ("a\xd6"));
    ck_assert_int_eq (str_is_valid_char ("\xd6", 1), 1);
    ck_assert_int_eq (str_length ("\xd6\xc1"), 2);
    // what does not print shown as '.': the letters of KOI8-R as the locale has them, C here
    ck_assert_str_eq (str_term_form ("a\x01"), "a.");
    // the case of ASCII; the others as the locale has them, C here
    ck_assert (str_toupper ("q", &out, &remain));
    ck_assert_str_eq (buf, "Q");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A character from one codeset to another; the last match of a text but so many */
START_TEST (test_translate)
{
    GIConv conv;
    char out[8];

    str_init_strings ("UTF-8");
    conv = str_crt_conv_to ("KOI8-R");
    ck_assert (conv != INVALID_CONV);
    ck_assert_int_eq (str_translate_char (conv, "\xd0\xb6", 2, out, sizeof (out)), ESTR_SUCCESS);
    ck_assert_int_eq ((unsigned char) out[0], 0xd6);
    /* a character KOI8-R has not: refused by glibc, a replacement of its own by the iconv of
       musl and of Solaris, one byte either way */
    if (str_translate_char (conv, "\xe2\x98\x85", 3, out, sizeof (out)) == ESTR_SUCCESS)
        ck_assert_uint_eq (strlen (out), 1);
    str_close_conv (conv);

    ck_assert_str_eq (strrstr_skip_count ("a-b-c-d", "-", 0), "-d");
    ck_assert_str_eq (strrstr_skip_count ("a-b-c-d", "-", 1), "-c-d");
    ck_assert_ptr_null (strrstr_skip_count ("a-b", "-", 3));
    ck_assert_ptr_null (strrstr_skip_count ("abc", "-", 0));
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

int
main (void)
{
    TCase *tc_core;

    tc_core = tcase_create ("Core");

    tcase_add_checked_fixture (tc_core, NULL, teardown);

    // Add new tests here: ***************
    tcase_add_test (tc_core, test_ascii);
    tcase_add_test (tc_core, test_8bit);
    tcase_add_test (tc_core, test_translate);
    // ***********************************

    return mctest_run_all (tc_core);
}

/* --------------------------------------------------------------------------------------------- */
