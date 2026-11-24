/*
   lib/widget - tests for the editing of an input line: moving, deleting, the kill buffer, the
   marks, characters of several bytes and the history

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

#define TEST_SUITE_NAME "lib/widget/input"

#include "tests/mctest.h"

#include "lib/widget/input.c"  // for its file scope functions

/* --------------------------------------------------------------------------------------------- */

static WInput *in;

/* --------------------------------------------------------------------------------------------- */

static void
setup (void)
{
    static const int colors[WINPUTC_COUNT_COLORS] = { 0 };

    str_init_strings ("UTF-8");
    // a line of no dialog: nothing drawn
    in = input_new (0, 0, colors, 20, "hello world", NULL, INPUT_COMPLETE_NONE);
    // as a line drawn once: its text no longer the default one a key replaces
    in->first = FALSE;
}

/* --------------------------------------------------------------------------------------------- */

static void
teardown (void)
{
    input_destroy (in);
    g_free (in);
    MC_PTR_FREE (kill_buffer);
    str_uninit_strings ();
}

/* --------------------------------------------------------------------------------------------- */

static void
cmd (long command)
{
    ck_assert_int_eq (input_execute_cmd (in, command), MSG_HANDLED);
}

/* --------------------------------------------------------------------------------------------- */

#define ck_line(text, at)                                                                          \
    do                                                                                             \
    {                                                                                              \
        ck_assert_str_eq (input_get_ctext (in), (text));                                           \
        ck_assert_int_eq (in->point, (at));                                                        \
    }                                                                                              \
    while (0)

/* --------------------------------------------------------------------------------------------- */
/*** tests ***************************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/* By characters, by words, to the ends */
START_TEST (test_moving)
{
    // a text assigned puts the point at its end
    ck_line ("hello world", 11);
    cmd (CK_Home);
    ck_line ("hello world", 0);
    cmd (CK_WordRight);
    ck_line ("hello world", 5);
    cmd (CK_WordRight);
    ck_line ("hello world", 11);
    cmd (CK_WordLeft);
    ck_line ("hello world", 6);
    cmd (CK_WordLeft);
    ck_line ("hello world", 0);
    cmd (CK_WordLeft);
    ck_line ("hello world", 0);
    cmd (CK_Right);
    cmd (CK_Right);
    ck_line ("hello world", 2);
    cmd (CK_Left);
    ck_line ("hello world", 1);
    cmd (CK_End);
    cmd (CK_Right);
    ck_line ("hello world", 11);

    // punctuation stops a word
    input_assign_text (in, "a.b,  c");
    cmd (CK_Home);
    cmd (CK_WordRight);
    ck_line ("a.b,  c", 1);
    cmd (CK_WordRight);
    ck_line ("a.b,  c", 3);
    cmd (CK_End);
    cmd (CK_WordLeft);
    ck_line ("a.b,  c", 6);

    // from just after a punctuation mark: over it, then over the word
    input_assign_text (in, "ab.");
    cmd (CK_WordLeft);
    ck_line ("ab.", 0);

    input_assign_text (in, "a.b,  c");
    input_set_point (in, 99);
    ck_line ("a.b,  c", 7);
    input_set_point (in, 2);
    ck_line ("a.b,  c", 2);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* Deleting a character, words, to the end, all */
START_TEST (test_deleting)
{
    cmd (CK_BackSpace);
    ck_line ("hello worl", 10);
    cmd (CK_Home);
    cmd (CK_BackSpace);
    ck_line ("hello worl", 0);
    cmd (CK_Delete);
    ck_line ("ello worl", 0);
    cmd (CK_DeleteToWordEnd);
    ck_line (" worl", 0);
    cmd (CK_End);
    cmd (CK_DeleteToWordBegin);
    ck_line (" ", 1);
    cmd (CK_End);
    cmd (CK_Delete);
    ck_line (" ", 1);

    input_assign_text (in, "one two three");
    input_set_point (in, 4);
    cmd (CK_DeleteToEnd);
    ck_line ("one ", 4);
    ck_assert_str_eq (kill_buffer, "two three");
    cmd (CK_Clear);
    ck_line ("", 0);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* The kill buffer: what is cut or copied, put back where the point is */
START_TEST (test_kill_buffer)
{
    // nothing to put
    cmd (CK_Yank);
    ck_line ("hello world", 11);

    // copied: the text stays
    cmd (CK_Home);
    cmd (CK_MarkToWordEnd);
    cmd (CK_Store);
    ck_assert_str_eq (kill_buffer, "hello");
    ck_line ("hello world", 5);
    cmd (CK_End);
    cmd (CK_Yank);
    ck_line ("hello worldhello", 16);

    // cut: the text goes
    input_assign_text (in, "hello world");
    input_set_point (in, 6);
    cmd (CK_MarkToEnd);
    cmd (CK_Cut);
    ck_assert_str_eq (kill_buffer, "world");
    ck_line ("hello ", 6);
    cmd (CK_Home);
    cmd (CK_Yank);
    ck_line ("worldhello ", 5);
    // no mark: the whole line copied, a password not: the buffer keeps what it had
    cmd (CK_Store);
    ck_assert_str_eq (kill_buffer, "worldhello ");
    in->is_password = TRUE;
    input_assign_text (in, "secret");
    cmd (CK_Store);
    ck_assert_str_eq (kill_buffer, "worldhello ");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A marked part: by the keys that mark, typed over, removed */
START_TEST (test_marks)
{
    long m1, m2;

    ck_assert (!input_eval_marks (in, &m1, &m2));
    cmd (CK_MarkToWordBegin);
    ck_assert (input_eval_marks (in, &m1, &m2));
    ck_assert_int_eq (m1, 6);
    ck_assert_int_eq (m2, 11);
    // a character typed takes the place of the marked part
    ck_assert_int_eq (insert_char (in, 'X'), MSG_HANDLED);
    ck_line ("hello X", 7);
    ck_assert (!input_eval_marks (in, &m1, &m2));

    cmd (CK_MarkToHome);
    cmd (CK_MarkRight);
    ck_assert (input_eval_marks (in, &m1, &m2));
    ck_assert_int_eq (m1, 1);
    ck_assert_int_eq (m2, 7);
    cmd (CK_Remove);
    ck_line ("h", 1);

    // a move that does not mark takes the mark off
    input_assign_text (in, "abc");
    cmd (CK_MarkLeft);
    cmd (CK_Left);
    ck_assert (!input_eval_marks (in, &m1, &m2));
    cmd (CK_MarkToEnd);
    insert_text (in, "Z");
    ck_line ("aZ", 2);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* Characters of several bytes: a step each, whole; a byte that makes none dropped */
START_TEST (test_multibyte)
{
    input_assign_text (in, "\xd0\xb6\xd1\x83\xd0\xba");
    ck_line ("\xd0\xb6\xd1\x83\xd0\xba", 3);
    cmd (CK_BackSpace);
    ck_line ("\xd0\xb6\xd1\x83", 2);
    cmd (CK_Left);
    cmd (CK_Delete);
    ck_line ("\xd0\xb6", 1);
    // a character byte by byte, as the keys bring it
    ck_assert_int_eq (insert_char (in, 0xd1), MSG_HANDLED);
    ck_line ("\xd0\xb6", 1);
    ck_assert_int_eq (insert_char (in, 0x8f), MSG_HANDLED);
    ck_line ("\xd0\xb6\xd1\x8f", 2);
    insert_text (in,
                 "a\xff"
                 "b");
    ck_line ("\xd0\xb6\xd1\x8f"
             "ab",
             4);
    ck_assert_int_eq (insert_char (in, -1), MSG_NOT_HANDLED);

    // a letter and its combining mark go as one
    input_assign_text (in, "e\xcc\x81x");
    cmd (CK_Home);
    cmd (CK_Delete);
    ck_line ("x", 0);
    input_assign_text (in, "xe\xcc\x81");
    cmd (CK_BackSpace);
    ck_line ("x", 1);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* The history: the line typed is kept when going back, the items in turn, an empty line past
   the newest */
START_TEST (test_history)
{
    in->history.list = list_append_unique (NULL, g_strdup ("one"));
    in->history.list = list_append_unique (in->history.list, g_strdup ("two"));
    in->history.current = in->history.list;
    input_assign_text (in, "");
    insert_text (in, "new");

    cmd (CK_HistoryPrev);
    ck_line ("two", 3);
    cmd (CK_HistoryPrev);
    ck_line ("one", 3);
    cmd (CK_HistoryPrev);
    ck_line ("one", 3);
    cmd (CK_HistoryNext);
    ck_line ("two", 3);
    cmd (CK_HistoryNext);
    ck_line ("new", 3);
    cmd (CK_HistoryNext);
    ck_line ("", 0);
    ck_assert_uint_eq (g_list_length (g_list_first (in->history.list)), 3);
    // typed and pushed on: kept, the line empty for the next
    insert_text (in, "more");
    cmd (CK_HistoryNext);
    ck_line ("", 0);
    ck_assert_str_eq ((const char *) g_list_last (in->history.list)->data, "more");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A password is not kept in the history of URLs */
START_TEST (test_strip_password)
{
    char url[] = "ftp://user:secret@host:21/dir";
    char port[] = "sftp://user@host:22/";
    char plain[] = "user:secret@host";
    char none[] = "no at here";
    char two[] = "ftp://a@b:pw@host/";
    char *s;

    s = input_history_strip_password (url);
    ck_assert_str_eq (s, "ftp://user@host:21/dir");
    g_free (s);
    s = input_history_strip_password (port);
    ck_assert_str_eq (s, "sftp://user@host:22/");
    g_free (s);
    s = input_history_strip_password (plain);
    ck_assert_str_eq (s, "user@host");
    g_free (s);
    // the user may have an @ of its own: the last one is that of the host
    s = input_history_strip_password (two);
    ck_assert_str_eq (s, "ftp://a@b@host/");
    g_free (s);
    s = input_history_strip_password (none);
    ck_assert_str_eq (s, "no at here");
    g_free (s);

    in->history.name = g_strdup ("ftp");
    in->strip_password = TRUE;
    input_assign_text (in, "");
    insert_text (in, "ftp://me:pw@x/");
    input_push_history (in);
    ck_assert_str_eq ((const char *) in->history.list->data, "ftp://me@x/");
    // the same again is not kept twice
    in->history.changed = FALSE;
    input_push_history (in);
    ck_assert_uint_eq (g_list_length (g_list_first (in->history.list)), 1);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A text put in at the point as if typed, a space after it when asked */
START_TEST (test_insert)
{
    // the space after the text, for what comes next
    input_insert (in, "x", TRUE);
    ck_line ("hello worldx ", 13);
    input_insert (in, "y", FALSE);
    ck_line ("hello worldx y", 14);
    input_set_point (in, 0);
    input_insert (in, "z", TRUE);
    ck_line ("z hello worldx y", 2);
    input_clean (in);
    ck_line ("", 0);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* The text a line starts with is replaced by the first key typed, deleted by the first delete;
   a move keeps it */
START_TEST (test_first)
{
    in->first = TRUE;
    ck_assert_int_eq (input_handle_char (in, 'x'), MSG_HANDLED);
    ck_line ("x", 1);

    input_assign_text (in, "default");
    in->first = TRUE;
    cmd (CK_Delete);
    ck_line ("", 0);
    ck_assert (!in->first);

    input_assign_text (in, "default");
    in->first = TRUE;
    cmd (CK_Left);
    ck_line ("default", 6);
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
    tcase_add_test (tc_core, test_moving);
    tcase_add_test (tc_core, test_deleting);
    tcase_add_test (tc_core, test_kill_buffer);
    tcase_add_test (tc_core, test_marks);
    tcase_add_test (tc_core, test_multibyte);
    tcase_add_test (tc_core, test_history);
    tcase_add_test (tc_core, test_strip_password);
    tcase_add_test (tc_core, test_insert);
    tcase_add_test (tc_core, test_first);
    // ***********************************

    return mctest_run_all (tc_core);
}

/* --------------------------------------------------------------------------------------------- */
