/*
   src/editor - tests for format_paragraph()

   Copyright (C) 2026
   Ilia Maslakov il.smind@gmail.com

   This file is part of bluerig,
   a text editor based on GNU Midnight Commander.

   bluerig is free software: you can redistribute it
   and/or modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of the License,
   or (at your option) any later version.

   bluerig is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see https://www.gnu.org/licenses/.
 */

#define TEST_SUITE_NAME "/src/editor"

#include "tests/mctest.h"

#include "lib/charsets.h"
#include "src/selcodepage.h"

#include "src/editor/editwidget.h"
#include "src/editor/edit-impl.h"

/* A dialog, not a bare group: the editor reaches its owner as one, and a
 * WGroup has no room for the fields it reads there. */
static WDialog owner;
static WEdit *test_edit;

/* --------------------------------------------------------------------------------------------- */
/* @Mock */
void
message (int flags, const char *title, const char *text, ...)
{
    (void) flags;
    (void) title;
    (void) text;
}

/* --------------------------------------------------------------------------------------------- */
/* @Mock */
void
status_msg_init (status_msg_t *sm, const char *title, double delay, status_msg_cb init_cb,
                 status_msg_update_cb update_cb, status_msg_cb deinit_cb)
{
    (void) sm;
    (void) title;
    (void) delay;
    (void) init_cb;
    (void) update_cb;
    (void) deinit_cb;
}

/* --------------------------------------------------------------------------------------------- */
/* @Mock */
void
status_msg_deinit (status_msg_t *sm)
{
    (void) sm;
}

/* --------------------------------------------------------------------------------------------- */

/* @Before */
static void
setup (void)
{
    WRect r;

    str_init_strings (NULL);

    mc_global.sysconfig_dir = (char *) TEST_SHARE_DIR;
    load_codepages_list ();

    edit_options.filesize_threshold = (char *) "64M";
    edit_options.word_wrap_line_length = 20;
    edit_options.stop_format_chars = NULL;
    edit_options.fill_tabs_with_spaces = FALSE;

    rect_init (&r, 0, 0, 24, 80);
    test_edit = edit_init (NULL, &r, NULL);
    memset (&owner, 0, sizeof (owner));
    group_add_widget (&owner.group, WIDGET (test_edit));

    mc_global.source_codepage = 0;
    mc_global.display_codepage = 0;
    cp_source = "ASCII";
    cp_display = "ASCII";

    do_set_codepage (0);
    edit_set_codeset (test_edit);
}

/* --------------------------------------------------------------------------------------------- */

/* @After */
static void
teardown (void)
{
    edit_options.stop_format_chars = NULL;

    group_remove_widget (test_edit);
    widget_destroy (WIDGET (test_edit));

    free_codepages_list ();
    str_uninit_strings ();
}

/* --------------------------------------------------------------------------------------------- */

/* Put @text into the editor and the cursor at the byte @cursor */
static void
put_text (const char *text, off_t cursor)
{
    for (const char *c = text; *c != '\0'; c++)
        edit_insert (test_edit, (unsigned char) *c);

    edit_cursor_move (test_edit, cursor - test_edit->buffer.curs1);
}

/* --------------------------------------------------------------------------------------------- */

static void
assert_text (const char *expected)
{
    GString *actual;

    actual = g_string_new ("");
    for (off_t i = 0; i < test_edit->buffer.size; i++)
        g_string_append_c (actual, edit_buffer_get_byte (&test_edit->buffer, i));

    mctest_assert_str_eq (actual->str, expected);
    g_string_free (actual, TRUE);
}

/* --------------------------------------------------------------------------------------------- */

/* A line longer than the width breaks between words, no line longer than the width */
START_TEST (test_wrap_long_line)
{
    put_text ("aaa bbb ccc ddd eee fff ggg hhh\n", 0);

    format_paragraph (test_edit);

    assert_text ("aaa bbb ccc ddd eee\n"
                 "fff ggg hhh\n");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* Short lines of a paragraph are joined into one */
START_TEST (test_join_short_lines)
{
    put_text ("aaa\nbbb\nccc\n", 0);

    format_paragraph (test_edit);

    assert_text ("aaa bbb ccc\n");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* The paragraph is the one under the cursor, found from any of its lines; a blank line ends it,
   the next paragraph stays as it was */
START_TEST (test_paragraph_ends_at_blank_line)
{
    // the cursor on "bbb", the second line of the first paragraph
    put_text ("aaa\nbbb\n\nccc\nddd\n", 4);

    format_paragraph (test_edit);

    assert_text ("aaa bbb\n\nccc\nddd\n");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A paragraph whose lines share an indent keeps it on every line, the width counted with it */
START_TEST (test_indent_kept)
{
    put_text ("    aaa bbb ccc ddd eee fff\n    ggg\n", 0);

    format_paragraph (test_edit);

    assert_text ("    aaa bbb ccc ddd\n"
                 "    eee fff ggg\n");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A word longer than the width is not cut: it takes a line of its own */
START_TEST (test_long_word_whole)
{
    put_text ("aaaaaaaaaaaaaaaaaaaaaaaaa bbb\n", 0);

    format_paragraph (test_edit);

    assert_text ("aaaaaaaaaaaaaaaaaaaaaaaaa\n"
                 "bbb\n");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* The cursor stays on its character */
START_TEST (test_cursor_kept)
{
    // the cursor on the first 'c'
    put_text ("aaa\nbbb\nccc\n", 8);

    format_paragraph (test_edit);

    assert_text ("aaa bbb ccc\n");
    ck_assert_int_eq (test_edit->buffer.curs1, 8);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A paragraph that starts with a stop character is left alone, and so is one with a line
   that starts with it: a list */
START_TEST (test_stop_format_chars)
{
    edit_options.stop_format_chars = (char *) "-+*";

    put_text ("- aaa\nbbb\n", 0);
    format_paragraph (test_edit);
    assert_text ("- aaa\nbbb\n");

    edit_cursor_move (test_edit, test_edit->buffer.size - test_edit->buffer.curs1);
    put_text ("\naaa\n* bbb\n", 11);
    format_paragraph (test_edit);
    assert_text ("- aaa\nbbb\n\naaa\n* bbb\n");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A line that starts with a tab, with no stop characters set, is not joined */
START_TEST (test_tab_line_stops)
{
    put_text ("aaa\n\tbbb\n", 0);

    format_paragraph (test_edit);

    assert_text ("aaa\n\tbbb\n");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* Nothing is done on a blank line, nor with a width under 2 */
START_TEST (test_nothing_to_do)
{
    put_text ("aaa\nbbb\n\nccc\n", 8);
    format_paragraph (test_edit);
    assert_text ("aaa\nbbb\n\nccc\n");

    edit_options.word_wrap_line_length = 1;
    edit_cursor_move (test_edit, -test_edit->buffer.curs1);
    format_paragraph (test_edit);
    assert_text ("aaa\nbbb\n\nccc\n");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

int
main (void)
{
    TCase *tc_core;

    tc_core = tcase_create ("Core");

    tcase_add_checked_fixture (tc_core, setup, teardown);

    tcase_add_test (tc_core, test_wrap_long_line);
    tcase_add_test (tc_core, test_join_short_lines);
    tcase_add_test (tc_core, test_paragraph_ends_at_blank_line);
    tcase_add_test (tc_core, test_indent_kept);
    tcase_add_test (tc_core, test_long_word_whole);
    tcase_add_test (tc_core, test_cursor_kept);
    tcase_add_test (tc_core, test_stop_format_chars);
    tcase_add_test (tc_core, test_tab_line_stops);
    tcase_add_test (tc_core, test_nothing_to_do);

    return mctest_run_all (tc_core);
}

/* --------------------------------------------------------------------------------------------- */
