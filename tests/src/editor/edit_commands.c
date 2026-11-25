/*
   src/editor - tests for the commands of the editor: moving by words, paragraphs and pages,
   deleting, the indent and the tab, the brackets, shifting a block, marking and undoing

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

#define TEST_SUITE_NAME "/src/editor"

#include "tests/mctest.h"

#include "lib/charsets.h"
#include "lib/keybind.h"
#include "src/selcodepage.h"

#include "src/editor/editwidget.h"
#include "src/editor/edit-impl.h"

/* A dialog, not a bare group: the editor reaches its owner as one */
static WDialog owner;
static WEdit *test_edit;
static edit_options_t saved_options;

/* --------------------------------------------------------------------------------------------- */

/* @Mock */
void
message (int flags, const char *title, const char *text, ...)
{
    (void) flags;
    (void) title;
    (void) text;
}

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

/* @Mock */
void
status_msg_deinit (status_msg_t *sm)
{
    (void) sm;
}

/* --------------------------------------------------------------------------------------------- */

static void
setup (void)
{
    WRect r;

    str_init_strings (NULL);
    mc_global.sysconfig_dir = (char *) TEST_SHARE_DIR;
    load_codepages_list ();
    saved_options = edit_options;
    edit_options.filesize_threshold = (char *) "64M";
    edit_options.cursor_beyond_eol = FALSE;
    edit_options.persistent_selections = FALSE;
    edit_options.fill_tabs_with_spaces = FALSE;
    edit_options.fake_half_tabs = FALSE;
    edit_options.return_does_auto_indent = FALSE;
    edit_options.auto_para_formatting = FALSE;
    edit_options.typewriter_wrap = FALSE;
    edit_options.group_undo = FALSE;
    rect_init (&r, 0, 0, 10, 80);
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

static void
teardown (void)
{
    group_remove_widget (test_edit);
    widget_destroy (WIDGET (test_edit));
    edit_options = saved_options;
    free_codepages_list ();
    str_uninit_strings ();
}

/* --------------------------------------------------------------------------------------------- */

/* @t in the editor, the cursor at @at */
static void
put (const char *t, off_t at)
{
    for (; *t != '\0'; t++)
        edit_insert (test_edit, (unsigned char) *t);
    edit_cursor_move (test_edit, at - test_edit->buffer.curs1);
}

/* A command as a key gives it: a step of the undo each */
static void
cmd (long command)
{
    edit_execute_key_command (test_edit, command, -1);
}

static void
type (const char *t)
{
    for (; *t != '\0'; t++)
        edit_execute_key_command (test_edit, CK_InsertChar, (unsigned char) *t);
}

static char *
all_text (void)
{
    GString *s = g_string_new (NULL);
    off_t i;

    for (i = 0; i < test_edit->buffer.size; i++)
        g_string_append_c (s, (char) edit_buffer_get_byte (&test_edit->buffer, i));
    return g_string_free (s, FALSE);
}

#define ck_text(expected)                                                                          \
    do                                                                                             \
    {                                                                                              \
        char *got = all_text ();                                                                   \
                                                                                                   \
        ck_assert_str_eq (got, (expected));                                                        \
        g_free (got);                                                                              \
    }                                                                                              \
    while (0)

#define ck_at(offset) ck_assert_int_eq (test_edit->buffer.curs1, (offset))

/* --------------------------------------------------------------------------------------------- */
/*** tests ***************************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/* By words: right to the start of the next one, the spaces with the word; left to the start of
   one; '_' of a word, ',' not; over the end of a line */
START_TEST (test_words)
{
    put ("foo bar_baz, qux\n", 0);
    cmd (CK_WordRight);
    ck_at (4);
    cmd (CK_WordRight);
    ck_at (11);
    cmd (CK_WordRight);
    ck_at (13);
    cmd (CK_WordRight);
    ck_at (16);
    cmd (CK_WordRight);
    ck_at (17);
    cmd (CK_WordRight);
    ck_at (17);
    cmd (CK_WordLeft);
    ck_at (16);
    cmd (CK_WordLeft);
    ck_at (13);
    cmd (CK_WordLeft);
    ck_at (11);
    cmd (CK_WordLeft);
    ck_at (4);
    cmd (CK_WordLeft);
    ck_at (0);
    cmd (CK_WordLeft);
    ck_at (0);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* By paragraphs, to the top and the bottom, by pages */
START_TEST (test_lines_and_pages)
{
    GString *t = g_string_new (NULL);
    int i;

    put ("a\nb\n\nc\nd\n\ne\n", 0);
    cmd (CK_ParagraphDown);
    ck_assert_int_eq (test_edit->buffer.curs_line, 2);
    cmd (CK_ParagraphDown);
    ck_assert_int_eq (test_edit->buffer.curs_line, 5);
    cmd (CK_ParagraphUp);
    ck_assert_int_eq (test_edit->buffer.curs_line, 2);
    cmd (CK_Bottom);
    ck_at (test_edit->buffer.size);
    cmd (CK_Top);
    ck_at (0);

    // a text of 40 lines in a window of 10
    edit_cursor_move (test_edit, test_edit->buffer.size);
    for (i = 0; i < 40; i++)
        g_string_append_printf (t, "line %d\n", i);
    put (t->str, 0);
    g_string_free (t, TRUE);
    cmd (CK_Top);
    cmd (CK_PageDown);
    ck_assert_int_gt (test_edit->buffer.curs_line, 5);
    {
        const long after_page = test_edit->buffer.curs_line;

        cmd (CK_PageUp);
        ck_assert_int_lt (test_edit->buffer.curs_line, after_page);
    }
    cmd (CK_Top);
    cmd (CK_BottomOnScreen);
    ck_assert_int_eq (test_edit->buffer.curs_line, WIDGET (test_edit)->rect.lines - 1);
    cmd (CK_TopOnScreen);
    ck_assert_int_eq (test_edit->buffer.curs_line, 0);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* Deleting words, to the ends of the line, the line */
START_TEST (test_deleting)
{
    put ("one two three\nfour five\n", 4);
    cmd (CK_DeleteToWordEnd);
    ck_text ("one  three\nfour five\n");
    // back over the run before the cursor: the space
    cmd (CK_DeleteToWordBegin);
    ck_text ("one three\nfour five\n");
    ck_at (3);
    cmd (CK_DeleteToWordBegin);
    ck_text (" three\nfour five\n");
    ck_at (0);
    cmd (CK_DeleteToEnd);
    ck_text ("\nfour five\n");
    cmd (CK_Down);
    cmd (CK_End);
    cmd (CK_DeleteToHome);
    ck_text ("\n\n");
    edit_cursor_move (test_edit, -test_edit->buffer.curs1);
    put ("x\ny\n", 0);
    cmd (CK_DeleteLine);
    ck_text ("y\n\n\n");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A new line takes the indent of the line before when asked */
START_TEST (test_auto_indent)
{
    put ("    x\n", 5);
    cmd (CK_Enter);
    type ("y");
    ck_text ("    x\ny\n");

    edit_options.return_does_auto_indent = TRUE;
    cmd (CK_End);
    cmd (CK_Enter);
    type ("z");
    ck_text ("    x\ny\nz\n");
    edit_cursor_move (test_edit, -test_edit->buffer.curs1 + 5);
    cmd (CK_Enter);
    type ("w");
    ck_text ("    x\n    w\ny\nz\n");
    // a return of its own does not
    cmd (CK_Return);
    type ("v");
    ck_text ("    x\n    w\nv\ny\nz\n");

    // with the paragraphs formatted a new line in a line is two: a new paragraph
    edit_cursor_move (test_edit, -test_edit->buffer.curs1);
    cmd (CK_DeleteLine);
    cmd (CK_DeleteLine);
    cmd (CK_DeleteLine);
    cmd (CK_DeleteLine);
    cmd (CK_DeleteLine);
    put ("abc def\n", 3);
    edit_options.return_does_auto_indent = FALSE;
    edit_options.auto_para_formatting = TRUE;
    cmd (CK_Enter);
    ck_text ("abc\n\n def\n");
    // at an empty line one is enough
    cmd (CK_Enter);
    ck_text ("abc\n\n\n def\n");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A tab: the character, spaces to the next stop, half tabs in the indent */
START_TEST (test_tab)
{
    put ("ab\n", 2);
    cmd (CK_Tab);
    ck_text ("ab\t\n");
    cmd (CK_BackSpace);
    edit_options.fill_tabs_with_spaces = TRUE;
    cmd (CK_Tab);
    ck_text ("ab      \n");
    ck_at (8);

    cmd (CK_Home);
    cmd (CK_DeleteToEnd);
    edit_options.fill_tabs_with_spaces = FALSE;
    edit_options.fake_half_tabs = TRUE;
    cmd (CK_Tab);
    ck_text ("    \n");
    // a half tab behind: the two are a tab
    cmd (CK_Tab);
    ck_text ("\t\n");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* The bracket that matches the one under the cursor, either way, nested */
START_TEST (test_brackets)
{
    put ("f (a [b] {c (d)}) x\n", 2);
    cmd (CK_MatchBracket);
    ck_at (16);
    cmd (CK_MatchBracket);
    ck_at (2);
    edit_cursor_move (test_edit, 5 - test_edit->buffer.curs1);
    cmd (CK_MatchBracket);
    ck_at (7);
    // not on a bracket: the cursor stays
    edit_cursor_move (test_edit, 1 - test_edit->buffer.curs1);
    cmd (CK_MatchBracket);
    ck_at (1);
    // one with no match
    edit_cursor_move (test_edit, -test_edit->buffer.curs1);
    put ("((", 0);
    cmd (CK_MatchBracket);
    ck_at (0);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A block of lines shifted right by a tab, back left; blank lines left alone */
START_TEST (test_block_shift)
{
    put ("a\n\nb\nc\n", 0);
    edit_set_markers (test_edit, 0, 5, 0, 0);
    cmd (CK_BlockShiftRight);
    ck_text ("\ta\n\n\tb\nc\n");
    edit_set_markers (test_edit, 0, 7, 0, 0);
    cmd (CK_BlockShiftLeft);
    ck_text ("a\n\nb\nc\n");

    edit_options.fill_tabs_with_spaces = TRUE;
    edit_set_markers (test_edit, 0, 2, 0, 0);
    cmd (CK_BlockShiftRight);
    ck_text ("        a\n\nb\nc\n");
    edit_set_markers (test_edit, 0, 10, 0, 0);
    cmd (CK_BlockShiftLeft);
    ck_text ("a\n\nb\nc\n");
    // no block: nothing
    edit_set_markers (test_edit, -1, -1, 0, 0);
    cmd (CK_BlockShiftRight);
    ck_text ("a\n\nb\nc\n");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* The word and the line under the cursor marked */
START_TEST (test_mark_word_line)
{
    off_t m1, m2;

    put ("one two_x three\nnext\n", 6);
    cmd (CK_MarkWord);
    ck_assert (eval_marks (test_edit, &m1, &m2));
    ck_assert_int_eq (m1, 4);
    ck_assert_int_eq (m2, 9);
    cmd (CK_MarkLine);
    ck_assert (eval_marks (test_edit, &m1, &m2));
    ck_assert_int_eq (m1, 0);
    ck_assert_int_eq (m2, 15);
    cmd (CK_MarkAll);
    ck_assert (eval_marks (test_edit, &m1, &m2));
    ck_assert_int_eq (m1, 0);
    ck_assert_int_eq (m2, test_edit->buffer.size);
    cmd (CK_Unmark);
    ck_assert (!eval_marks (test_edit, &m1, &m2));
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* Undo and redo: by keys, or the run of the same kind as one */
START_TEST (test_undo)
{
    put ("", 0);
    type ("abc");
    cmd (CK_Undo);
    ck_text ("ab");
    cmd (CK_Redo);
    ck_text ("abc");
    cmd (CK_Undo);
    cmd (CK_Undo);
    cmd (CK_Undo);
    ck_text ("");
    ck_assert (!test_edit->modified);

    edit_options.group_undo = TRUE;
    type ("xyz");
    cmd (CK_Undo);
    ck_text ("");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* Insert and overwrite */
START_TEST (test_overwrite)
{
    put ("abcd\n", 1);
    cmd (CK_InsertOverwrite);
    type ("XY");
    ck_text ("aXYd\n");
    cmd (CK_InsertOverwrite);
    type ("Z");
    ck_text ("aXYZd\n");
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
    tcase_add_test (tc_core, test_words);
    tcase_add_test (tc_core, test_lines_and_pages);
    tcase_add_test (tc_core, test_deleting);
    tcase_add_test (tc_core, test_auto_indent);
    tcase_add_test (tc_core, test_tab);
    tcase_add_test (tc_core, test_brackets);
    tcase_add_test (tc_core, test_block_shift);
    tcase_add_test (tc_core, test_mark_word_line);
    tcase_add_test (tc_core, test_undo);
    tcase_add_test (tc_core, test_overwrite);
    // ***********************************

    return mctest_run_all (tc_core);
}

/* --------------------------------------------------------------------------------------------- */
