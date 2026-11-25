/*
   src/viewer/vterm - tests for the screen of the terminal: the cursor, erasing, scrolling, the
   colors of the cells, the answers to the program and the character sets

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

#define TEST_SUITE_NAME "/src/mcterm/vterm_screen"

#include "tests/mctest.h"

#include "lib/strutil.h"

#include "src/viewer/vterm.h"
#include "src/viewer/terminal_buffer.h"

#define ROWS 5
#define COLS 10

#define ESC  "\033"
#define CSI  ESC "["

/*** file scope functions ************************************************************************/

static GString *replies;  // what the terminal answered the program

/* --------------------------------------------------------------------------------------------- */

static void
feed (mcview_vterm_t *vt, const char *data)
{
    const char *p;

    for (p = data; *p != '\0'; p++)
    {
        vterm_event_t ev = mcview_vterm_feed (vt, (unsigned char) *p);

        if (ev.type == VTERM_REPLY)
            g_string_append (replies, ev.reply);
        mcview_vterm_apply_event (vt, &ev);
    }
}

/* --------------------------------------------------------------------------------------------- */

static mcview_vterm_t *
term_new (void)
{
    mcview_vterm_t *vt;

    vt = mcview_vterm_new ();
    mcview_vterm_set_size (vt, ROWS, COLS);
    // a terminal, as mcterm makes it: the viewer of a log has its rows as long as they are
    mcview_vterm_set_autowrap (vt, TRUE);
    return vt;
}

/* --------------------------------------------------------------------------------------------- */

static const mcview_vterm_cell_t *
cell (mcview_vterm_t *vt, int row, int col)
{
    return mcview_terminal_buffer_get (mcview_vterm_buf (vt), row, col);
}

/* --------------------------------------------------------------------------------------------- */

/* The screen as text: a row each, a blank for a cell never written or erased, the blanks at the
   end of a row left out, rows joined by '|' */
static char *
screen (mcview_vterm_t *vt)
{
    GString *text = g_string_new (NULL);
    int row, col;

    for (row = 0; row < ROWS; row++)
    {
        gsize end;

        if (row > 0)
            g_string_append_c (text, '|');
        end = text->len;
        for (col = 0; col < COLS; col++)
        {
            const mcview_vterm_cell_t *c = cell (vt, row, col);

            if (c == NULL || c->ch == 0 || c->ch == ' ')
                g_string_append_c (text, ' ');
            else
            {
                g_string_append_unichar (text, c->ch);
                end = text->len;
            }
        }
        g_string_truncate (text, end);
    }
    return g_string_free (text, FALSE);
}

/* --------------------------------------------------------------------------------------------- */

#define ck_screen(vt, expected)                                                                    \
    do                                                                                             \
    {                                                                                              \
        char *got = screen (vt);                                                                   \
                                                                                                   \
        ck_assert_str_eq (got, (expected));                                                        \
        g_free (got);                                                                              \
    }                                                                                              \
    while (0)

#define ck_cursor(vt, row, col)                                                                    \
    do                                                                                             \
    {                                                                                              \
        ck_assert_int_eq (mcview_vterm_cursor_row (vt), (row));                                    \
        ck_assert_int_eq (mcview_vterm_cursor_col (vt), (col));                                    \
    }                                                                                              \
    while (0)

/* --------------------------------------------------------------------------------------------- */

static void
setup (void)
{
    str_init_strings ("UTF-8");
    replies = g_string_new (NULL);
}

/* --------------------------------------------------------------------------------------------- */

static void
teardown (void)
{
    g_string_free (replies, TRUE);
    str_uninit_strings ();
}

/* --------------------------------------------------------------------------------------------- */
/*** tests ***************************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/* The cursor moved by the sequences, 1-based on the wire, kept on the screen */
START_TEST (test_cursor)
{
    mcview_vterm_t *vt = term_new ();

    feed (vt, CSI "3;5H");
    ck_cursor (vt, 2, 4);
    feed (vt, CSI "A");
    ck_cursor (vt, 1, 4);
    feed (vt, CSI "2B");
    ck_cursor (vt, 3, 4);
    feed (vt, CSI "3C");
    ck_cursor (vt, 3, 7);
    feed (vt, CSI "5D");
    ck_cursor (vt, 3, 2);
    feed (vt, CSI "E");
    ck_cursor (vt, 4, 0);
    feed (vt, CSI "8G");
    ck_cursor (vt, 4, 7);
    feed (vt, CSI "2F");
    ck_cursor (vt, 2, 0);
    feed (vt, CSI "4d");
    ck_cursor (vt, 3, 0);
    feed (vt, CSI "H");
    ck_cursor (vt, 0, 0);
    feed (vt, CSI "2;3f");
    ck_cursor (vt, 1, 2);
    // not off the screen
    feed (vt, CSI "99;99H");
    ck_cursor (vt, ROWS - 1, COLS - 1);
    feed (vt, CSI "99A" CSI "99D");
    ck_cursor (vt, 0, 0);
    // a tab to the next stop of eight, a backspace back
    feed (vt, "ab\t");
    ck_cursor (vt, 0, 8);
    feed (vt, "\b");
    ck_cursor (vt, 0, 7);
    feed (vt, "\r");
    ck_cursor (vt, 0, 0);
    mcview_vterm_free (vt);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* The text written, wrapped at the right edge, or not when autowrap is off */
START_TEST (test_text_and_wrap)
{
    mcview_vterm_t *vt = term_new ();

    feed (vt, "hello\r\nworld");
    ck_screen (vt, "hello|world|||");
    ck_cursor (vt, 1, 5);
    feed (vt,
          CSI "H"
              "0123456789AB");
    ck_screen (vt, "0123456789|ABrld|||");
    ck_cursor (vt, 1, 2);
    // the last column of a full row: the wrap waits for the next character
    feed (vt,
          CSI "3;1H"
              "abcdefghij");
    ck_cursor (vt, 2, COLS - 1);
    feed (vt, "k");
    ck_screen (vt, "0123456789|ABrld|abcdefghij|k|");
    // no autowrap: the row goes on past the screen, as in the viewer
    feed (vt,
          CSI "?7l" CSI "5;1H"
              "abcdefghijXYZ");
    ck_screen (vt, "0123456789|ABrld|abcdefghij|k|abcdefghij");
    ck_assert_int_eq (cell (vt, 4, 12)->ch, 'Z');
    ck_cursor (vt, 4, 13);
    mcview_vterm_free (vt);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* Erasing: in the line, in the screen, characters; deleting and inserting characters */
START_TEST (test_erase)
{
    mcview_vterm_t *vt = term_new ();

    feed (vt, "0123456789\r\n0123456789\r\n0123456789\r\n0123456789\r\n0123456789");
    feed (vt, CSI "1;4H" CSI "K");
    feed (vt, CSI "2;4H" CSI "1K");
    feed (vt, CSI "3;4H" CSI "2K");
    ck_screen (vt, "012|    456789||0123456789|0123456789");
    feed (vt, CSI "4;3H" CSI "3X");
    ck_screen (vt, "012|    456789||01   56789|0123456789");
    ck_cursor (vt, 3, 2);
    feed (vt, CSI "5;3H" CSI "2P");
    ck_screen (vt, "012|    456789||01   56789|01456789");
    feed (vt, CSI "5;2H" CSI "3@");
    ck_screen (vt, "012|    456789||01   56789|0   145678");
    // the screen: from the cursor down, up to it, all of it
    feed (vt, CSI "4;5H" CSI "J");
    ck_screen (vt, "012|    456789||01|");
    feed (vt, CSI "2;3H" CSI "1J");
    ck_screen (vt, "|    456789||01|");
    feed (vt, CSI "2J");
    ck_screen (vt, "||||");
    mcview_vterm_free (vt);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A scroll region: a line feed at its bottom scrolls it alone; a reverse index at its top
   scrolls it down */
START_TEST (test_scroll_region)
{
    mcview_vterm_t *vt = term_new ();

    feed (vt, "r0\r\nr1\r\nr2\r\nr3\r\nr4");
    feed (vt, CSI "2;4r");
    // the region set puts the cursor home
    ck_cursor (vt, 0, 0);
    feed (vt,
          CSI "4;1H"
              "\n");
    ck_screen (vt, "r0|r2|r3||r4");
    ck_cursor (vt, 3, 0);
    feed (vt, CSI "2;1H" ESC "M");
    ck_screen (vt, "r0||r2|r3|r4");
    ck_cursor (vt, 1, 0);
    // a region with its top under its bottom is none: the cursor stays, the region too
    feed (vt, CSI "3;4H" CSI "4;2r");
    ck_cursor (vt, 2, 3);
    feed (vt,
          CSI "4;1H"
              "\n");
    ck_screen (vt, "r0|r2|r3||r4");
    // the region taken off: the whole screen scrolls
    feed (vt,
          CSI "r" CSI "5;1H"
              "\n"
              "new");
    ck_screen (vt, "r2|r3||r4|new");
    mcview_vterm_free (vt);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* The colors and the attributes of the cells, as SGR set them */
START_TEST (test_cell_attributes)
{
    mcview_vterm_t *vt = term_new ();
    const mcview_vterm_cell_t *c;

    feed (vt,
          CSI "1;31m"
              "A" CSI "0m"
              "B" CSI "4;44;7m"
              "C" CSI "38;5;200m"
              "D" CSI "m"
              "E");
    c = cell (vt, 0, 0);
    ck_assert_int_eq (c->ch, 'A');
    ck_assert (c->attr.bold);
    ck_assert_int_eq (c->attr.fg, 1);
    c = cell (vt, 0, 1);
    ck_assert (!c->attr.bold);
    ck_assert_int_eq (c->attr.fg, -1);
    c = cell (vt, 0, 2);
    ck_assert (c->attr.underline);
    ck_assert (c->attr.reverse);
    ck_assert_int_eq (c->attr.bg, 4);
    c = cell (vt, 0, 3);
    ck_assert_int_eq (c->attr.fg, 200);
    ck_assert_int_eq (c->attr.bg, 4);
    c = cell (vt, 0, 4);
    ck_assert_int_eq (c->attr.fg, -1);
    ck_assert_int_eq (c->attr.bg, -1);
    ck_assert (!c->attr.underline);
    mcview_vterm_free (vt);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* What the terminal says when the program asks: what it is, how it is, where the cursor is */
START_TEST (test_replies)
{
    mcview_vterm_t *vt = term_new ();

    feed (vt, CSI "c");
    ck_assert_str_eq (replies->str, ESC "[?1;2c");
    g_string_truncate (replies, 0);
    feed (vt, CSI ">c");
    ck_assert_str_eq (replies->str, ESC "[>0;0;0c");
    g_string_truncate (replies, 0);
    feed (vt, CSI "5n");
    ck_assert_str_eq (replies->str, ESC "[0n");
    g_string_truncate (replies, 0);
    feed (vt, CSI "3;7H" CSI "6n");
    ck_assert_str_eq (replies->str, ESC "[3;7R");
    g_string_truncate (replies, 0);
    // no pictures: the sizes are not told
    feed (vt, CSI "18t" CSI "7n");
    ck_assert_str_eq (replies->str, "");
    mcview_vterm_set_sixel (vt, TRUE);
    mcview_vterm_set_cell_size (vt, 8, 16);
    feed (vt, CSI "c" CSI "18t" CSI "16t" CSI "14t");
    ck_assert_str_eq (replies->str, ESC "[?62;4;22c" ESC "[8;5;10t" ESC "[6;16;8t" ESC "[4;80;80t");
    mcview_vterm_free (vt);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* The modes of the private sequences, several of them at once */
START_TEST (test_modes)
{
    mcview_vterm_t *vt = term_new ();

    ck_assert (!mcview_vterm_app_cursor_keys (vt));
    feed (vt, CSI "?1;2004h");
    ck_assert (mcview_vterm_app_cursor_keys (vt));
    ck_assert (mcview_vterm_bracketed_paste (vt));
    feed (vt, CSI "?1l");
    ck_assert (!mcview_vterm_app_cursor_keys (vt));
    // a query changes nothing
    feed (vt, CSI "?2004$p");
    ck_assert (mcview_vterm_bracketed_paste (vt));

    feed (vt, "main" CSI "?1049h");
    ck_assert (mcview_vterm_in_alt_screen (vt));
    // the program clears the screen it draws on
    feed (vt,
          CSI "2J" CSI "H"
              "alt");
    ck_screen (vt, "alt||||");
    feed (vt, CSI "?1049l");
    ck_assert (!mcview_vterm_in_alt_screen (vt));
    ck_screen (vt, "main||||");
    mcview_vterm_free (vt);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* The line drawing set of DEC, by ESC ( 0, and by SO from G1; UTF-8 by its bytes */
START_TEST (test_charsets)
{
    mcview_vterm_t *vt = term_new ();

    feed (vt,
          ESC "(0"
              "lqk" ESC "(B"
              "q");
    ck_assert_int_eq (cell (vt, 0, 0)->ch, 0x250C);
    ck_assert_int_eq (cell (vt, 0, 1)->ch, 0x2500);
    ck_assert_int_eq (cell (vt, 0, 2)->ch, 0x2510);
    ck_assert_int_eq (cell (vt, 0, 3)->ch, 'q');
    feed (vt,
          ESC ")0"
              "\x0e"
              "x"
              "\x0f"
              "x");
    ck_assert_int_eq (cell (vt, 0, 4)->ch, 0x2502);
    ck_assert_int_eq (cell (vt, 0, 5)->ch, 'x');
    // outside the letters the set is ASCII
    feed (vt,
          ESC "(0"
              "A" ESC "(B");
    ck_assert_int_eq (cell (vt, 0, 6)->ch, 'A');

    feed (vt,
          "\r\n"
          "\xd0\x96"
          "\xe2\x82\xac"
          "z");
    ck_assert_int_eq (cell (vt, 1, 0)->ch, 0x0416);
    ck_assert_int_eq (cell (vt, 1, 1)->ch, 0x20AC);
    ck_assert_int_eq (cell (vt, 1, 2)->ch, 'z');
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
    tcase_add_test (tc_core, test_cursor);
    tcase_add_test (tc_core, test_text_and_wrap);
    tcase_add_test (tc_core, test_erase);
    tcase_add_test (tc_core, test_scroll_region);
    tcase_add_test (tc_core, test_cell_attributes);
    tcase_add_test (tc_core, test_replies);
    tcase_add_test (tc_core, test_modes);
    tcase_add_test (tc_core, test_charsets);
    // ***********************************

    return mctest_run_all (tc_core);
}

/* --------------------------------------------------------------------------------------------- */
