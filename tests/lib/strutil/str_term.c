/*
   lib/strutil - tests for the strings on the terminal: their columns, fitted, trimmed and cut,
   and the characters of UTF-8 and of an 8-bit codeset

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

/* U+65E5, U+672C and U+8A9E, two columns each */
#define W1 "\xe6\x97\xa5"
#define W2 "\xe6\x9c\xac"
#define W3 "\xe8\xaa\x9e"
/* e and U+0301, the combining acute: one column; and U+00E9, the same composed */
#define E_COMBINED "e\xcc\x81"
#define E_COMPOSED "\xc3\xa9"
/* U+0416 and U+0436, a capital and a small letter */
#define ZHE_CAP   "\xd0\x96"
#define ZHE_SMALL "\xd0\xb6"

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

/* The columns of a text: two for a wide character, none for a combining mark */
START_TEST (test_width)
{
    ck_assert_int_eq (str_term_width1 ("abc"), 3);
    ck_assert_int_eq (str_term_width1 (W1 W2), 4);
    ck_assert_int_eq (str_term_width1 (E_COMBINED "x"), 2);
    ck_assert_int_eq (str_term_width1 (""), 0);
    // a byte that is no UTF-8 takes a column
    ck_assert_int_eq (str_term_width1 ("a\xff"
                                       "b"),
                      3);
    ck_assert_int_eq (str_term_width2 ("abcdef", 3), 3);
    ck_assert_int_eq (str_term_width2 (W1 W2 W3, 2), 4);

    ck_assert_int_eq (str_term_char_width ("a"), 1);
    ck_assert_int_eq (str_term_char_width (W1), 2);
    ck_assert_int_eq (str_term_char_width ("\xcc\x81"), 0);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A text in a field of columns: padded where it is shorter, as it is put; cut where it is longer,
   in the middle with a tilde when it has to fit */
START_TEST (test_fit_to_term)
{
    ck_assert_str_eq (str_fit_to_term ("abc", 6, J_LEFT), "abc   ");
    ck_assert_str_eq (str_fit_to_term ("abc", 6, J_RIGHT), "   abc");
    ck_assert_str_eq (str_fit_to_term ("abc", 6, J_CENTER), " abc  ");
    ck_assert_str_eq (str_fit_to_term ("abc", 6, J_CENTER_LEFT), " abc  ");
    ck_assert_str_eq (str_fit_to_term ("abc", 3, J_LEFT), "abc");
    ck_assert_str_eq (str_fit_to_term (W1 W2, 6, J_RIGHT), "  " W1 W2);

    ck_assert_str_eq (str_fit_to_term ("abcdefgh", 5, J_LEFT), "abcde");
    ck_assert_str_eq (str_fit_to_term ("abcdefgh", 5, J_LEFT_FIT), "ab~gh");
    ck_assert_str_eq (str_fit_to_term ("abcdefgh", 6, J_RIGHT_FIT), "abc~gh");
    // a wide character that does not fit leaves its columns blank
    ck_assert_str_eq (str_fit_to_term (W1 W2 W3, 5, J_LEFT), W1 W2 " ");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A text cut at the start, dots for what is gone; cut in the middle, a tilde */
START_TEST (test_trim_and_trunc)
{
    ck_assert_str_eq (str_term_trim ("abcdefgh", 8), "abcdefgh");
    ck_assert_str_eq (str_term_trim ("abcdefgh", 6), "...fgh");
    ck_assert_str_eq (str_term_trim ("abcdefgh", 3), "...");
    ck_assert_str_eq (str_term_trim ("abcdefgh", 2), "..");
    ck_assert_str_eq (str_term_trim ("abcdefgh", 0), "");
    ck_assert_str_eq (str_term_trim (W1 W2 W3, 5), "..." W3);

    ck_assert_str_eq (str_trunc ("abcdefgh", 5), "ab~gh");
    ck_assert_str_eq (str_trunc ("abcdefgh", 8), "abcdefgh");
    ck_assert_str_eq (str_trunc ("abcdefgh", -1), "abcdefgh");
    ck_assert_str_eq (str_trunc ("abc", 10), "abc");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* The columns of a text from one of them, blanks where it has none */
START_TEST (test_substring)
{
    ck_assert_str_eq (str_term_substring ("abcdef", 2, 3), "cde");
    ck_assert_str_eq (str_term_substring ("ab", 1, 4), "b   ");
    ck_assert_str_eq (str_term_substring ("abcdef", 0, 0), "");
    ck_assert_str_eq (str_term_substring (W1 W2 "x", 2, 3), W2 "x");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* Bytes, characters and columns of one another */
START_TEST (test_positions)
{
    ck_assert_int_eq (str_offset_to_pos (W1 W2 "a", 2), 6);
    ck_assert_int_eq (str_offset_to_pos ("abc", 0), 0);
    // a byte that is no UTF-8 is a character
    ck_assert_int_eq (str_offset_to_pos ("\xff"
                                         "ab",
                                         1),
                      1);
    // nor the start of one cut short
    ck_assert_int_eq (str_offset_to_pos ("\xe6"
                                         "ab",
                                         1),
                      1);

    ck_assert_int_eq (str_column_to_pos (W1 W2 "a", 0), 0);
    ck_assert_int_eq (str_column_to_pos (W1 W2 "a", 1), 0);
    ck_assert_int_eq (str_column_to_pos (W1 W2 "a", 2), 1);
    ck_assert_int_eq (str_column_to_pos (W1 W2 "a", 4), 2);
    ck_assert_int_eq (str_column_to_pos ("ab", 9), 2);
    ck_assert_int_eq (str_column_to_pos (E_COMBINED "x", 1), 2);

    ck_assert_int_eq (str_length (W1 W2 "a"), 3);
    ck_assert_int_eq (str_length (E_COMBINED), 2);
    ck_assert_int_eq (str_length_noncomb (E_COMBINED "x"), 2);
    ck_assert_int_eq (str_length2 (W1 W2 "a", 3), 1);
    ck_assert_int_eq (str_length2 ("abc", -1), 3);
    ck_assert_int_eq (str_length_char (W1), 3);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* Going over the characters: a whole one, a combining mark with the one before */
START_TEST (test_characters)
{
    const char *text = W1 "a" E_COMBINED "b";
    const char *p = text;
    char *end;

    str_cnext_char (&p);
    ck_assert_ptr_eq (p, text + 3);
    ck_assert_int_eq (str_cnext_noncomb_char (&p), 1);
    // e and its mark are one step, of two characters
    ck_assert_int_eq (str_cnext_noncomb_char (&p), 2);
    ck_assert_str_eq (p, "b");
    ck_assert_int_eq (str_cprev_noncomb_char (&p, text), 2);
    ck_assert_ptr_eq (p, text + 4);
    end = g_strdup (text);
    ck_assert_ptr_eq (str_get_prev_char (end + 3), end);
    g_free (end);

    ck_assert (str_isprint ("a"));
    ck_assert (str_isprint (W1));
    ck_assert (!str_isprint ("\x01"));
    ck_assert (str_isspace (" "));
    ck_assert (str_isspace ("\xe3\x80\x80"));
    ck_assert (!str_isspace ("a"));
    ck_assert (str_isalnum (ZHE_SMALL));
    ck_assert (!str_isalnum ("-"));
    ck_assert (str_isdigit ("7"));
    ck_assert (!str_isdigit ("x"));
    ck_assert (str_ispunct (","));
    ck_assert (!str_ispunct ("a"));
    ck_assert (str_iscombiningmark ("\xcc\x81"));
    ck_assert (!str_iscombiningmark ("e"));

    ck_assert (str_is_valid_string ("ab" W1));
    ck_assert (!str_is_valid_string ("a\xff"));
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A character to the other case, written where @out points, @remain the room there */
START_TEST (test_case)
{
    char buf[8] = { 0 };
    char *out = buf;
    size_t remain = sizeof (buf) - 1;

    ck_assert (str_toupper (ZHE_SMALL, &out, &remain));
    ck_assert (str_tolower ("Q", &out, &remain));
    ck_assert_str_eq (buf, ZHE_CAP "q");
    ck_assert_uint_eq (remain, sizeof (buf) - 1 - 3);
    // no room
    remain = 1;
    ck_assert (!str_toupper (ZHE_SMALL, &out, &remain));
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* Texts compared as they read: a letter composed or with its mark the same, of either case when
   asked */
START_TEST (test_compare)
{
    ck_assert_int_eq (str_compare (E_COMPOSED, E_COMBINED), 0);
    ck_assert_int_lt (str_compare ("a", "b"), 0);
    ck_assert_int_gt (str_compare ("b", "a"), 0);
    ck_assert_int_ne (str_compare ("A", "a"), 0);
    ck_assert_int_eq (str_casecmp ("A" ZHE_CAP, "a" ZHE_SMALL), 0);
    ck_assert_int_eq (str_ncompare ("abcx", "abc"), 0);
    ck_assert_int_ne (str_ncompare ("abx", "abc"), 0);
    ck_assert_int_eq (str_ncasecmp ("ABCx", "abc"), 0);

    // the length of what the text starts with, in its own bytes
    ck_assert_int_eq (str_prefix ("abcdef", "abc"), 3);
    ck_assert_int_eq (str_prefix ("abcdef", "abx"), 2);
    ck_assert_int_eq (str_prefix (E_COMBINED "x", E_COMPOSED), 3);
    ck_assert_int_eq (str_caseprefix ("ABCdef", "abc"), 3);

    {
        char *a = str_create_key ("apple", TRUE);
        char *b = str_create_key ("banana", TRUE);
        char *c = str_create_key ("banana", FALSE);
        char *d = str_create_key ("BANANA", FALSE);

        ck_assert_int_lt (str_key_collate (a, b, TRUE), 0);
        ck_assert_int_eq (str_key_collate (c, d, FALSE), 0);
        str_release_key (a, TRUE);
        str_release_key (b, TRUE);
        str_release_key (c, FALSE);
        str_release_key (d, FALSE);
    }
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A text looked for: the first or the last time, of either case when asked */
START_TEST (test_search)
{
    const char *text = "one Two one TWO";
    char *needle;

    needle = str_create_search_needle ("two", FALSE);
    ck_assert_ptr_eq (str_search_first (text, needle, FALSE), text + 4);
    ck_assert_ptr_eq (str_search_last (text, needle, FALSE), text + 12);
    str_release_search_needle (needle, FALSE);
    needle = str_create_search_needle ("two", TRUE);
    ck_assert_ptr_null (str_search_first (text, needle, TRUE));
    str_release_search_needle (needle, TRUE);
    needle = str_create_search_needle ("one", TRUE);
    ck_assert_ptr_eq (str_search_last (text, needle, TRUE), text + 8);
    str_release_search_needle (needle, TRUE);
    // a letter composed is found by its mark
    needle = str_create_search_needle (E_COMPOSED, TRUE);
    {
        const char *with_mark = "ab" E_COMBINED;

        ck_assert_ptr_eq (str_search_first (with_mark, needle, TRUE), with_mark + 2);
    }
    str_release_search_needle (needle, TRUE);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* An 8-bit codeset: a byte a character and a column */
START_TEST (test_8bit)
{
    str_uninit_strings ();
    str_init_strings ("KOI8-R");
    ck_assert_int_eq (str_term_width1 ("\xd6\xc1"
                                       "b"),
                      3);
    ck_assert_int_eq (str_length ("\xd6\xc1"
                                  "b"),
                      3);
    ck_assert_str_eq (str_fit_to_term ("abc", 5, J_RIGHT), "  abc");
    ck_assert_str_eq (str_fit_to_term ("abcdefgh", 5, J_LEFT_FIT), "ab~gh");
    ck_assert_str_eq (str_term_trim ("abcdefgh", 6), "...fgh");
    ck_assert_str_eq (str_trunc ("abcdefgh", 5), "ab~gh");
    ck_assert_str_eq (str_term_substring ("abcdef", 2, 3), "cde");
    ck_assert_int_eq (str_column_to_pos ("abc", 1), 1);
    // the case of the letters of ASCII; those of KOI8-R as the locale has them, C here
    ck_assert_int_eq (str_casecmp ("aBc", "AbC"), 0);
    ck_assert_int_ne (str_compare ("aBc", "AbC"), 0);
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
    tcase_add_test (tc_core, test_fit_to_term);
    tcase_add_test (tc_core, test_trim_and_trunc);
    tcase_add_test (tc_core, test_substring);
    tcase_add_test (tc_core, test_positions);
    tcase_add_test (tc_core, test_characters);
    tcase_add_test (tc_core, test_case);
    tcase_add_test (tc_core, test_compare);
    tcase_add_test (tc_core, test_search);
    tcase_add_test (tc_core, test_8bit);
    // ***********************************

    return mctest_run_all (tc_core);
}

/* --------------------------------------------------------------------------------------------- */
