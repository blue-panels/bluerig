/*
   src/editor - tests for the block commands: copy, move and delete

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
    edit_options.cursor_after_inserted_block = FALSE;
    edit_options.cursor_beyond_eol = FALSE;

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
    edit_options.cursor_after_inserted_block = FALSE;

    group_remove_widget (test_edit);
    widget_destroy (WIDGET (test_edit));

    free_codepages_list ();
    str_uninit_strings ();
}

/* --------------------------------------------------------------------------------------------- */

/* @text in the editor, the block from @m1 to @m2, the cursor at @cursor */
static void
put_text (const char *text, off_t m1, off_t m2, off_t cursor)
{
    for (const char *c = text; *c != '\0'; c++)
        edit_insert (test_edit, (unsigned char) *c);

    edit_cursor_move (test_edit, cursor - test_edit->buffer.curs1);
    edit_set_markers (test_edit, m1, m2, 0, 0);
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

/* A copy goes in at the cursor, the cursor and the block stay where they were */
START_TEST (test_copy)
{
    put_text ("abc\ndef\n", 0, 3, 4);

    edit_block_copy_cmd (test_edit);

    assert_text ("abc\nabcdef\n");
    ck_assert_int_eq (test_edit->buffer.curs1, 4);
    ck_assert_int_eq (test_edit->mark1, 0);
    ck_assert_int_eq (test_edit->mark2, 3);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* With "cursor after inserted block" the cursor goes past the copy */
START_TEST (test_copy_cursor_after)
{
    edit_options.cursor_after_inserted_block = TRUE;
    put_text ("abc\ndef\n", 0, 3, 4);

    edit_block_copy_cmd (test_edit);

    assert_text ("abc\nabcdef\n");
    ck_assert_int_eq (test_edit->buffer.curs1, 7);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A copy into the block itself: the block grows over both */
START_TEST (test_copy_into_block)
{
    put_text ("abcdef\n", 1, 4, 2);

    edit_block_copy_cmd (test_edit);

    assert_text ("abbcdcdef\n");
    ck_assert_int_eq (test_edit->mark1, 1);
    ck_assert_int_eq (test_edit->mark2, 7);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* No block, no copy */
START_TEST (test_copy_no_block)
{
    put_text ("abc\n", 0, 0, 1);

    edit_block_copy_cmd (test_edit);

    assert_text ("abc\n");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A block moved forward: out of its place, in at the cursor, marked there */
START_TEST (test_move_forward)
{
    put_text ("abc\ndef\n", 0, 3, 8);

    edit_block_move_cmd (test_edit);

    assert_text ("\ndef\nabc");
    ck_assert_int_eq (test_edit->mark1, 5);
    ck_assert_int_eq (test_edit->mark2, 8);
    ck_assert_int_eq (test_edit->buffer.curs1, 5);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A block moved forward into the middle of the text: the cursor counted without the block */
START_TEST (test_move_forward_middle)
{
    put_text ("abc\ndef\nghi\n", 0, 3, 8);

    edit_block_move_cmd (test_edit);

    assert_text ("\ndef\nabcghi\n");
    ck_assert_int_eq (test_edit->mark1, 5);
    ck_assert_int_eq (test_edit->mark2, 8);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A block moved back */
START_TEST (test_move_back)
{
    put_text ("abc\ndef\n", 4, 7, 0);

    edit_block_move_cmd (test_edit);

    assert_text ("defabc\n\n");
    ck_assert_int_eq (test_edit->mark1, 0);
    ck_assert_int_eq (test_edit->mark2, 3);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* The cursor inside the block: nothing moves */
START_TEST (test_move_into_itself)
{
    put_text ("abc\ndef\n", 0, 3, 1);

    edit_block_move_cmd (test_edit);

    assert_text ("abc\ndef\n");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A block deleted, the marks cleared, the cursor at its place */
START_TEST (test_delete_block)
{
    put_text ("abc\ndef\n", 1, 3, 6);

    ck_assert (edit_block_delete_cmd (test_edit));

    assert_text ("a\ndef\n");
    ck_assert_int_eq (test_edit->mark1, 0);
    ck_assert_int_eq (test_edit->mark2, 0);
    ck_assert_int_eq (test_edit->buffer.curs1, 1);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* No block: the line of the cursor is deleted */
START_TEST (test_delete_line)
{
    put_text ("abc\ndef\nghi\n", 0, 0, 5);

    ck_assert (edit_block_delete_cmd (test_edit));

    assert_text ("abc\nghi\n");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A column block deleted: the same columns out of every line */
START_TEST (test_delete_column)
{
    put_text ("abcd\nefgh\nijkl\n", 0, 0, 0);
    test_edit->column_highlight = 1;
    // from line 0 column 1 to line 2 column 3
    edit_set_markers (test_edit, 1, 13, 1, 3);

    ck_assert (edit_block_delete_cmd (test_edit));

    assert_text ("ad\neh\nil\n");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A column block moved to the end of the first line */
START_TEST (test_move_column)
{
    put_text ("abcd\nefgh\nijkl\n", 0, 0, 4);
    test_edit->column_highlight = 1;
    // "bc" and "fg": line 0 column 1 to line 1 column 3
    edit_set_markers (test_edit, 1, 8, 1, 3);

    edit_block_move_cmd (test_edit);

    assert_text ("adbc\nehfg\nijkl\n");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

int
main (void)
{
    TCase *tc_core;

    tc_core = tcase_create ("Core");

    tcase_add_checked_fixture (tc_core, setup, teardown);

    tcase_add_test (tc_core, test_copy);
    tcase_add_test (tc_core, test_copy_cursor_after);
    tcase_add_test (tc_core, test_copy_into_block);
    tcase_add_test (tc_core, test_copy_no_block);
    tcase_add_test (tc_core, test_move_forward);
    tcase_add_test (tc_core, test_move_forward_middle);
    tcase_add_test (tc_core, test_move_back);
    tcase_add_test (tc_core, test_move_into_itself);
    tcase_add_test (tc_core, test_delete_block);
    tcase_add_test (tc_core, test_delete_line);
    tcase_add_test (tc_core, test_delete_column);
    tcase_add_test (tc_core, test_move_column);

    return mctest_run_all (tc_core);
}

/* --------------------------------------------------------------------------------------------- */
