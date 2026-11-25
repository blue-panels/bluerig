/*
   src/mcterm - tests for the filter of the output and the search in it

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

#define TEST_SUITE_NAME "/src/mcterm/mcterm_filter"

#include "tests/mctest.h"

#include "lib/strutil.h"

#include "src/mcterm/mcterm_filter.h"

#define TERM_ROWS 4
#define TERM_COLS 20

/* Six rows through a screen of four: rows 0 and 1 go to the history */
#define LOG        "alpha one\r\nbeta\r\nAlpha two\r\n\r\ngamma alpha\r\nend"
#define LOG_NEWEST 5

/*** file scope functions ************************************************************************/

/* --------------------------------------------------------------------------------------------- */

static void
feed (mcview_vterm_t *vt, const char *data)
{
    const char *p;

    for (p = data; *p != '\0'; p++)
    {
        vterm_event_t ev = mcview_vterm_feed (vt, (unsigned char) *p);

        mcview_vterm_apply_event (vt, &ev);
    }
}

/* --------------------------------------------------------------------------------------------- */

static mcview_vterm_t *
term_new (gboolean keep_history, const char *text)
{
    mcview_vterm_t *vt;

    vt = mcview_vterm_new ();
    mcview_vterm_set_size (vt, TERM_ROWS, TERM_COLS);
    mcview_vterm_set_keep_history (vt, keep_history);
    feed (vt, text);

    return vt;
}

/* --------------------------------------------------------------------------------------------- */

/* The match mcterm_filter_find() gives, as "row:col+width", or "none" */
static char *
find_to (mcview_vterm_t *vt, gint64 newest, const char *pattern, gint64 row, int col, gboolean up)
{
    gint64 found_row = -1;
    int found_col = -1, found_width = -1;

    if (!mcterm_filter_find (vt, TERM_COLS, newest, pattern, row, col, up, &found_row, &found_col,
                             &found_width))
        return g_strdup ("none");

    return g_strdup_printf ("%d:%d+%d", (int) found_row, found_col, found_width);
}

/* --------------------------------------------------------------------------------------------- */

#define ck_find(vt, pattern, row, col, up, expected)                                               \
    do                                                                                             \
    {                                                                                              \
        char *got = find_to ((vt), LOG_NEWEST, (pattern), (row), (col), (up));                     \
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
/*** tests ***************************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/* The text of a row: no blanks of the padding, one character for a wide one */
START_TEST (test_row_text)
{
    mcview_vterm_t *vt;
    char *text;

    vt = term_new (TRUE, LOG);
    text = mcterm_filter_row_text (vt, 1, TERM_COLS);
    ck_assert_str_eq (text, "beta");
    g_free (text);
    // a row of the history and an empty one
    text = mcterm_filter_row_text (vt, 0, TERM_COLS);
    ck_assert_str_eq (text, "alpha one");
    g_free (text);
    text = mcterm_filter_row_text (vt, 3, TERM_COLS);
    ck_assert_str_eq (text, "");
    g_free (text);
    ck_assert_ptr_null (mcterm_filter_row_text (vt, 0, 0));
    mcview_vterm_free (vt);

    // the cells a cursor moved over were never written: blanks
    vt = term_new (TRUE, "a\x1b[3Cb");
    text = mcterm_filter_row_text (vt, 0, TERM_COLS);
    ck_assert_str_eq (text, "a   b");
    g_free (text);
    ck_assert_ptr_null (mcterm_filter_row_text (NULL, 0, TERM_COLS));
    mcview_vterm_free (vt);

    vt = term_new (TRUE, "\xe6\x97\xa5\xe6\x9c\xac alpha  b");
    text = mcterm_filter_row_text (vt, 0, TERM_COLS);
    ck_assert_str_eq (text, "\xe6\x97\xa5\xe6\x9c\xac alpha  b");
    g_free (text);
    mcview_vterm_free (vt);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* The rows that match, of either case, the history with them; the set as a view of its own */
START_TEST (test_apply)
{
    mcview_vterm_t *vt;
    mcterm_filter_t f = { 0 };

    vt = term_new (TRUE, LOG);
    ck_assert_int_eq ((int) mcview_vterm_scrolled_rows (vt), 2);
    ck_assert (!mcterm_filter_active (&f));
    ck_assert_int_eq (mcterm_filter_len (&f), 0);

    ck_assert (mcterm_filter_apply (&f, vt, TERM_COLS, LOG_NEWEST, "ALPHA"));
    ck_assert (mcterm_filter_active (&f));
    ck_assert_str_eq (f.pattern, "ALPHA");
    ck_assert_int_eq (mcterm_filter_len (&f), 3);
    ck_assert_int_eq ((int) mcterm_filter_row (&f, 0), 0);
    ck_assert_int_eq ((int) mcterm_filter_row (&f, 1), 2);
    ck_assert_int_eq ((int) mcterm_filter_row (&f, 2), 4);
    ck_assert_int_eq ((int) mcterm_filter_row (&f, 3), -1);
    ck_assert_int_eq ((int) mcterm_filter_row (&f, -1), -1);
    // the newest row with them
    ck_assert (mcterm_filter_apply (&f, vt, TERM_COLS, LOG_NEWEST, "end"));
    ck_assert_int_eq (mcterm_filter_len (&f), 1);
    ck_assert_int_eq ((int) mcterm_filter_row (&f, 0), LOG_NEWEST);

    // rows up to @newest only
    f.top = 2;
    ck_assert (mcterm_filter_apply (&f, vt, TERM_COLS, 3, "alpha"));
    ck_assert_int_eq (mcterm_filter_len (&f), 2);
    ck_assert_int_eq (f.top, 0);

    // no row matches: the filter stays as it was
    ck_assert (!mcterm_filter_apply (&f, vt, TERM_COLS, LOG_NEWEST, "delta"));
    ck_assert_str_eq (f.pattern, "alpha");
    ck_assert_int_eq (mcterm_filter_len (&f), 2);
    ck_assert (!mcterm_filter_apply (&f, vt, TERM_COLS, LOG_NEWEST, ""));
    ck_assert (!mcterm_filter_apply (&f, vt, TERM_COLS, LOG_NEWEST, NULL));
    ck_assert (!mcterm_filter_apply (&f, vt, 0, LOG_NEWEST, "alpha"));
    ck_assert (!mcterm_filter_apply (NULL, vt, TERM_COLS, LOG_NEWEST, "alpha"));
    ck_assert_int_eq (mcterm_filter_len (&f), 2);

    mcterm_filter_clear (&f);
    ck_assert (!mcterm_filter_active (&f));
    ck_assert_ptr_null (f.rows);
    ck_assert_int_eq (mcterm_filter_len (&f), 0);
    mcterm_filter_clear (&f);
    mcterm_filter_clear (NULL);
    ck_assert (!mcterm_filter_active (NULL));
    mcview_vterm_free (vt);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* Rows the terminal has not kept are not looked at */
START_TEST (test_apply_without_history)
{
    mcview_vterm_t *vt;
    mcterm_filter_t f = { 0 };

    vt = term_new (FALSE, LOG);
    ck_assert_int_eq (mcview_vterm_history_len (vt), 0);
    ck_assert (mcterm_filter_apply (&f, vt, TERM_COLS, LOG_NEWEST, "alpha"));
    ck_assert_int_eq (mcterm_filter_len (&f), 2);
    ck_assert_int_eq ((int) mcterm_filter_row (&f, 0), 2);
    mcterm_filter_clear (&f);
    mcview_vterm_free (vt);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* The place of a row among those that matched, or of the first one after it */
START_TEST (test_index)
{
    mcview_vterm_t *vt;
    mcterm_filter_t f = { 0 };

    ck_assert_int_eq (mcterm_filter_index (&f, 7), 0);
    vt = term_new (TRUE, LOG);
    ck_assert (mcterm_filter_apply (&f, vt, TERM_COLS, LOG_NEWEST, "alpha"));
    // rows 0, 2 and 4
    ck_assert_int_eq (mcterm_filter_index (&f, -5), 0);
    ck_assert_int_eq (mcterm_filter_index (&f, 0), 0);
    ck_assert_int_eq (mcterm_filter_index (&f, 1), 1);
    ck_assert_int_eq (mcterm_filter_index (&f, 2), 1);
    ck_assert_int_eq (mcterm_filter_index (&f, 3), 2);
    ck_assert_int_eq (mcterm_filter_index (&f, 4), 2);
    // past them all: the last
    ck_assert_int_eq (mcterm_filter_index (&f, 9), 2);
    mcterm_filter_clear (&f);
    mcview_vterm_free (vt);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* Down from a place: there or after it on its row, then on the rows below, round to the top */
START_TEST (test_find_down)
{
    mcview_vterm_t *vt;

    vt = term_new (TRUE, LOG);
    ck_find (vt, "alpha", 0, 0, FALSE, "0:0+5");
    ck_find (vt, "alpha", 0, 1, FALSE, "2:0+5");
    ck_find (vt, "ALPHA", 2, 1, FALSE, "4:6+5");
    ck_find (vt, "alpha", 4, 6, FALSE, "4:6+5");
    // past the last row, round to the first
    ck_find (vt, "alpha", 4, 7, FALSE, "0:0+5");
    ck_find (vt, "end", 0, 0, FALSE, "5:0+3");
    // a column past the text of the row: the next row
    ck_find (vt, "one", 0, 15, FALSE, "0:6+3");
    ck_find (vt, "delta", 0, 0, FALSE, "none");
    ck_find (vt, "", 0, 0, FALSE, "none");
    mcview_vterm_free (vt);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* Up from a place: there or before it on its row, then on the rows above, round to the bottom */
START_TEST (test_find_up)
{
    mcview_vterm_t *vt;

    vt = term_new (TRUE, LOG);
    ck_find (vt, "alpha", 4, 6, TRUE, "4:6+5");
    ck_find (vt, "alpha", 4, 5, TRUE, "2:0+5");
    ck_find (vt, "alpha", 2, 0, TRUE, "2:0+5");
    ck_find (vt, "alpha", 1, 19, TRUE, "0:0+5");
    // above the first row, round to the last
    ck_find (vt, "gamma", 0, 0, TRUE, "4:0+5");
    ck_find (vt, "end", 4, 19, TRUE, "5:0+3");
    mcview_vterm_free (vt);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* The row the search starts on comes again at the end of the round, for what it has not run over;
   a column outside the row leaves the row out till then */
START_TEST (test_find_round)
{
    mcview_vterm_t *vt;

    vt = term_new (TRUE, "x alpha y alpha");
    ck_find (vt, "alpha", 0, 3, FALSE, "0:10+5");
    ck_find (vt, "alpha", 0, 11, FALSE, "0:2+5");
    ck_find (vt, "alpha", 0, 9, TRUE, "0:2+5");
    ck_find (vt, "alpha", 0, 1, TRUE, "0:10+5");
    ck_find (vt, "x", 0, -1, FALSE, "0:0+1");
    mcview_vterm_free (vt);

    // one row, the newest
    vt = term_new (TRUE, "ab ab");
    {
        char *got = find_to (vt, 0, "ab", 0, -1, TRUE);

        ck_assert_str_eq (got, "0:3+2");
        g_free (got);
    }
    mcview_vterm_free (vt);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* The cursor off the output: from the end the search runs away from */
START_TEST (test_find_from_off_the_output)
{
    mcview_vterm_t *vt;

    vt = term_new (TRUE, LOG);
    ck_find (vt, "alpha", 100, 3, FALSE, "0:0+5");
    ck_find (vt, "alpha", -100, 3, FALSE, "0:0+5");
    ck_find (vt, "alpha", 100, 0, TRUE, "4:6+5");
    mcview_vterm_free (vt);
    // up: from the end of the newest row
    vt = term_new (TRUE, "ab ab");
    {
        char *got = find_to (vt, 0, "ab", 100, 0, TRUE);

        ck_assert_str_eq (got, "0:3+2");
        g_free (got);
    }
    ck_assert (
        !mcterm_filter_find (NULL, TERM_COLS, LOG_NEWEST, "a", 0, 0, FALSE, NULL, NULL, NULL));
    mcview_vterm_free (vt);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A wide character takes two columns: the match is where it is on the screen, as wide as it is */
START_TEST (test_find_wide)
{
    mcview_vterm_t *vt;

    // two wide characters, U+65E5 in columns 0 and 1 and U+672C in 2 and 3, then " alpha"
    vt = term_new (TRUE, "\xe6\x97\xa5\xe6\x9c\xac alpha");
    ck_find (vt, "alpha", 0, 0, FALSE, "0:5+5");
    ck_find (vt, "\xe6\x9c\xac", 0, 0, FALSE, "0:2+2");
    ck_find (vt, "\xe6\x97\xa5\xe6\x9c\xac a", 0, 0, FALSE, "0:0+6");
    // from a column after U+672C: only the round brings it back
    ck_find (vt, "\xe6\x9c\xac", 0, 4, FALSE, "0:2+2");
    ck_find (vt, "alpha", 0, 19, TRUE, "0:5+5");
    ck_find (vt, "\xe6\x97\xa5", 0, 4, TRUE, "0:0+2");
    mcview_vterm_free (vt);

    // a column after a wide character is that many characters less into the text
    vt = term_new (TRUE, "\xe6\x97\xa5\xe6\x9c\xac ab ab");
    ck_find (vt, "ab", 0, 5, FALSE, "0:5+2");
    ck_find (vt, "ab", 0, 6, FALSE, "0:8+2");
    ck_find (vt, "ab", 0, 7, TRUE, "0:5+2");
    mcview_vterm_free (vt);
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
    tcase_add_test (tc_core, test_row_text);
    tcase_add_test (tc_core, test_apply);
    tcase_add_test (tc_core, test_apply_without_history);
    tcase_add_test (tc_core, test_index);
    tcase_add_test (tc_core, test_find_down);
    tcase_add_test (tc_core, test_find_up);
    tcase_add_test (tc_core, test_find_round);
    tcase_add_test (tc_core, test_find_from_off_the_output);
    tcase_add_test (tc_core, test_find_wide);
    // ***********************************

    return mctest_run_all (tc_core);
}

/* --------------------------------------------------------------------------------------------- */
