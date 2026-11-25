/*
   src/viewer/ansi - tests for the colors and the attributes of SGR, the strings that are not
   shown, and the faint colors

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

#define TEST_SUITE_NAME "/src/viewer/ansi"

#include "tests/mctest.h"

#include "src/viewer/ansi.h"

#define ESC "\033"
#define CSI ESC "["

/*** file scope functions ************************************************************************/

static mcview_ansi_state_t st;

/* --------------------------------------------------------------------------------------------- */

/* The bytes of @data fed, the ones shown kept */
static char *
parse (const char *data)
{
    GString *shown = g_string_new (NULL);
    const char *p;

    for (p = data; *p != '\0'; p++)
        if (mcview_ansi_parse_char (&st, (unsigned char) *p) == ANSI_RESULT_CHAR)
            g_string_append_c (shown, *p);
    return g_string_free (shown, FALSE);
}

/* --------------------------------------------------------------------------------------------- */

static void
sgr (const char *data)
{
    char *shown = parse (data);

    ck_assert_str_eq (shown, "");
    g_free (shown);
}

/* --------------------------------------------------------------------------------------------- */

static void
setup (void)
{
    mcview_ansi_state_init (&st);
}

/* --------------------------------------------------------------------------------------------- */
/*** tests ***************************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/* The attributes on and off, one by one and all at once */
START_TEST (test_attributes)
{
    sgr (CSI "1;2;3;4;5;7;8m");
    ck_assert (st.bold && st.dim && st.italic && st.underline && st.blink && st.reverse
               && st.conceal);
    sgr (CSI "22m");
    ck_assert (!st.bold && !st.dim);
    sgr (CSI "23;24;25;27;28m");
    ck_assert (!st.italic && !st.underline && !st.blink && !st.reverse && !st.conceal);
    // a double underline and a rapid blink as the plain ones
    sgr (CSI "21;6m");
    ck_assert (st.underline && st.blink);
    // 4:0 is no underline, 4:3 a curly one
    sgr (CSI "4:0m");
    ck_assert (!st.underline);
    sgr (CSI "4:3m");
    ck_assert (st.underline);
    sgr (CSI "31;42m" CSI "0m");
    ck_assert_int_eq (st.fg, MCVIEW_ANSI_COLOR_DEFAULT);
    ck_assert_int_eq (st.bg, MCVIEW_ANSI_COLOR_DEFAULT);
    ck_assert (!st.underline && !st.blink);
    sgr (CSI "1;33m" CSI "m");
    ck_assert (!st.bold);
    ck_assert_int_eq (st.fg, MCVIEW_ANSI_COLOR_DEFAULT);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* The colors: the eight, the bright ones, the 256 and the true ones */
START_TEST (test_colors)
{
    sgr (CSI "31;47m");
    ck_assert_int_eq (st.fg, 1);
    ck_assert_int_eq (st.bg, 7);
    sgr (CSI "92;104m");
    ck_assert_int_eq (st.fg, 10);
    ck_assert_int_eq (st.bg, 12);
    sgr (CSI "39;49m");
    ck_assert_int_eq (st.fg, MCVIEW_ANSI_COLOR_DEFAULT);
    ck_assert_int_eq (st.bg, MCVIEW_ANSI_COLOR_DEFAULT);
    sgr (CSI "0;38;5;196;48;5;21m");
    ck_assert_int_eq (st.fg, 196);
    ck_assert_int_eq (st.bg, 21);
    // 5 and 21 there are of the colors, not a blink and an underline
    ck_assert (!st.blink && !st.underline);
    sgr (CSI "38:5:82m");
    ck_assert_int_eq (st.fg, 82);
    // a true color as the nearest of the cube
    sgr (CSI "38;2;255;0;0m");
    ck_assert_int_eq (st.fg, 196);
    sgr (CSI "48;2;0;0;255m");
    ck_assert_int_eq (st.bg, 21);
    // colons, with the color space before the components
    sgr (CSI "38:2::0:255:0m");
    ck_assert_int_eq (st.fg, 46);
    sgr (CSI "48:2:0:100:100:100m");
    ck_assert_int_eq (st.bg, 16 + 36 * 1 + 6 * 1 + 1);
    // what follows a long color is a parameter of its own
    sgr (CSI "38;5;1;1m");
    ck_assert_int_eq (st.fg, 1);
    ck_assert (st.bold);
    sgr (CSI "0;38;2;10;20;30;4m");
    ck_assert_int_eq (st.fg, 16);
    ck_assert (st.underline);
    // too short: nothing
    sgr (CSI "0;38;5m");
    ck_assert_int_eq (st.fg, MCVIEW_ANSI_COLOR_DEFAULT);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* What is no SGR leaves the colors; the text is shown around the sequences */
START_TEST (test_not_sgr)
{
    char *shown;

    sgr (CSI "0;31m");
    shown = parse ("a" CSI "2J"
                   "b" CSI ">4;2m"
                   "c" CSI "?25l"
                   "d" ESC "c"
                   "e");
    ck_assert_str_eq (shown, "abcde");
    g_free (shown);
    ck_assert_int_eq (st.fg, 1);
    // modifyOtherKeys is no underline and no faint
    ck_assert (!st.underline && !st.dim);
    // ESC [ with a parameter too large stays a number
    sgr (CSI "99999999m");
    ck_assert_int_eq (st.fg, 1);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* OSC, DCS, APC, PM and SOS are not shown, to BEL or ST; OSC 8 with a URI is a link */
START_TEST (test_strings)
{
    char *shown;

    shown = parse ("a" ESC "]0;title"
                   "\a"
                   "b" ESC "P"
                   "q#0" ESC "\\"
                   "c" ESC "_x" ESC "\\"
                   "d");
    ck_assert_str_eq (shown, "abcd");
    g_free (shown);
    ck_assert (!st.link);

    shown = parse (ESC "]8;;https://x.org/" ESC "\\"
                       "link" ESC "]8;;" ESC "\\"
                       "after");
    ck_assert_str_eq (shown, "linkafter");
    g_free (shown);
    ck_assert (!st.link);
    g_free (parse (ESC "]8;id=1;file:///tmp"
                       "\a"));
    ck_assert (st.link);
    g_free (parse (ESC "]8;;"
                       "\a"));
    ck_assert (!st.link);
    // another OSC is no link, one with as many fields either
    g_free (parse (ESC "]7;file:///tmp"
                       "\a"));
    ck_assert (!st.link);
    g_free (parse (ESC "]52;c;Zm9v"
                       "\a"));
    ck_assert (!st.link);

    // CAN ends the string, a new line too, an ESC that is no ST starts its own sequence
    shown = parse (ESC "]0;x"
                       "\x18"
                       "y" ESC "]0;t\nz" ESC "]0;t" ESC "[31m"
                       "w");
    ck_assert_str_eq (shown, "y\nzw");
    g_free (shown);
    ck_assert_int_eq (st.fg, 1);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A color at about half its brightness: one of the eight darker, else on the 256 */
START_TEST (test_dim_color)
{
    ck_assert_int_eq (mcview_ansi_dim_color (MCVIEW_ANSI_COLOR_DEFAULT, FALSE), 8);
    ck_assert_int_eq (mcview_ansi_dim_color (7, FALSE), 8);
    ck_assert_int_eq (mcview_ansi_dim_color (15, FALSE), 7);
    ck_assert_int_eq (mcview_ansi_dim_color (9, FALSE), 1);
    ck_assert_int_eq (mcview_ansi_dim_color (2, FALSE), 2);

    // white, gray: on the gray ramp
    ck_assert_int_eq (mcview_ansi_dim_color (15, TRUE), 243);
    ck_assert_int_eq (mcview_ansi_dim_color (MCVIEW_ANSI_COLOR_DEFAULT, TRUE), 242);
    ck_assert_int_eq (mcview_ansi_dim_color (0, TRUE), 16);
    // red at half
    ck_assert_int_eq (mcview_ansi_dim_color (9, TRUE), 16 + 36 * 2);
    // of the cube, and of the ramp
    ck_assert_int_eq (mcview_ansi_dim_color (231, TRUE), 243);
    ck_assert_int_eq (mcview_ansi_dim_color (255, TRUE), 243);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

int
main (void)
{
    TCase *tc_core;

    tc_core = tcase_create ("Core");

    tcase_add_checked_fixture (tc_core, setup, NULL);

    // Add new tests here: ***************
    tcase_add_test (tc_core, test_attributes);
    tcase_add_test (tc_core, test_colors);
    tcase_add_test (tc_core, test_not_sgr);
    tcase_add_test (tc_core, test_strings);
    tcase_add_test (tc_core, test_dim_color);
    // ***********************************

    return mctest_run_all (tc_core);
}

/* --------------------------------------------------------------------------------------------- */
