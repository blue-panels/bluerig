/*
   src/editor - tests for the search of the editor and for its replace asked at each match

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
#include "src/selcodepage.h"

#include "src/editor/editwidget.h"
#include "src/editor/editmacros.h"  // edit_load_macro_cmd()
#include "src/editor/editsearch.h"

/* A dialog, not a bare group: the editor reaches its owner as one */
static WDialog owner;
static WEdit *test_edit;

static GString *told;  // what message () showed

// the replace: what the dialog gives, and the answers at the matches, in turn
static const char *replace_from;
static const char *replace_to;
static const int *answers;

/* --------------------------------------------------------------------------------------------- */

void edit_dialog_replace_show (WEdit *edit, const char *search_default, const char *replace_default,
                               char **search_text, char **replace_text);
int edit_dialog_replace_prompt_show (WEdit *edit, char *from_text, char *to_text, int xpos,
                                     int ypos);

/* --------------------------------------------------------------------------------------------- */

/* @Mock */
void
message (int flags, const char *title, const char *text, ...)
{
    va_list args;
    char *line;

    (void) flags;
    (void) title;
    va_start (args, text);
    line = g_strdup_vprintf (text, args);
    va_end (args);
    g_string_append_printf (told, "%s\n", line);
    g_free (line);
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

/* @Mock */
mc_search_cbret_t
edit_search_update_callback (const void *user_data, off_t char_offset)
{
    (void) user_data;
    (void) char_offset;
    return MC_SEARCH_CB_OK;
}

/* @Mock */
void
edit_dialog_replace_show (WEdit *edit, const char *search_default, const char *replace_default,
                          char **search_text, char **replace_text)
{
    (void) edit;
    (void) search_default;
    (void) replace_default;
    *search_text = g_strdup (replace_from);
    *replace_text = g_strdup (replace_to);
}

/* @Mock */
int
edit_dialog_replace_prompt_show (WEdit *edit, char *from_text, char *to_text, int xpos, int ypos)
{
    (void) edit;
    (void) from_text;
    (void) to_text;
    (void) xpos;
    (void) ypos;
    return *answers++;
}

/* @Mock */
void
edit_load_syntax (WEdit *edit, GPtrArray *pnames, const char *type)
{
    (void) edit;
    (void) pnames;
    (void) type;
}

/* @Mock */
int
edit_get_syntax_color (WEdit *edit, off_t byte_index)
{
    (void) edit;
    (void) byte_index;
    return 0;
}

/* @Mock */
gboolean
edit_load_macro_cmd (WEdit *edit)
{
    (void) edit;
    return FALSE;
}

/* --------------------------------------------------------------------------------------------- */

static void
setup (void)
{
    WRect r;

    str_init_strings (NULL);
    mc_global.sysconfig_dir = (char *) TEST_SHARE_DIR;
    load_codepages_list ();
    edit_options.filesize_threshold = (char *) "64M";
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
    told = g_string_new (NULL);
    memset (&edit_search_options, 0, sizeof (edit_search_options));
    edit_search_options.type = MC_SEARCH_T_NORMAL;
    edit_search_options.case_sens = TRUE;
}

/* --------------------------------------------------------------------------------------------- */

static void
teardown (void)
{
    edit_replace_cmd (NULL, FALSE);
    group_remove_widget (test_edit);
    widget_destroy (WIDGET (test_edit));
    free_codepages_list ();
    str_uninit_strings ();
    g_string_free (told, TRUE);
}

/* --------------------------------------------------------------------------------------------- */

static void
text (const char *t)
{
    for (; *t != '\0'; t++)
        edit_buffer_insert (&test_edit->buffer, *t);
    edit_cursor_move (test_edit, -test_edit->buffer.curs1);
}

/* --------------------------------------------------------------------------------------------- */

static char *
buffer_text (void)
{
    GString *s = g_string_new (NULL);
    off_t i;

    for (i = 0; i < test_edit->buffer.size; i++)
        g_string_append_c (s, (char) edit_buffer_get_byte (&test_edit->buffer, i));
    return g_string_free (s, FALSE);
}

/* --------------------------------------------------------------------------------------------- */

/* A search as a key again gives it: the string of the search before, from the point */
static void
search_for (const char *what)
{
    edit_search_deinit (test_edit);
    test_edit->last_search_string = g_strdup (what);
    test_edit->search_start = test_edit->buffer.curs1;
    test_edit->found_len = 0;
    ck_assert (edit_search_init (test_edit, test_edit->last_search_string));
    edit_search_cmd (test_edit, TRUE);
}

/* --------------------------------------------------------------------------------------------- */

static void
again (void)
{
    edit_search_cmd (test_edit, TRUE);
}

/* --------------------------------------------------------------------------------------------- */

#define ck_found(at, length)                                                                       \
    do                                                                                             \
    {                                                                                              \
        ck_assert_int_eq (test_edit->buffer.curs1, (at));                                          \
        ck_assert_int_eq (test_edit->found_start, (at));                                           \
        ck_assert_int_eq (test_edit->found_len, (length));                                         \
    }                                                                                              \
    while (0)

/* --------------------------------------------------------------------------------------------- */
/*** tests ***************************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/* Forward from the point, again from after the match, and the end said */
START_TEST (test_forward)
{
    text ("one two one two\nthree one\n");
    search_for ("one");
    ck_found (0, 3);
    again ();
    ck_found (8, 3);
    again ();
    ck_found (22, 3);
    again ();
    ck_assert_str_eq (told->str, "Search string not found\n");
    // what was found last stays; the next search starts at the point
    ck_assert_int_eq (test_edit->buffer.curs1, 22);
    ck_assert_int_eq (test_edit->search_start, 22);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* Backward from the point */
START_TEST (test_backward)
{
    text ("one two one two\nthree one\n");
    edit_cursor_move (test_edit, test_edit->buffer.size);
    edit_search_options.backwards = TRUE;
    search_for ("one");
    ck_found (22, 3);
    again ();
    ck_found (8, 3);
    again ();
    ck_found (0, 3);
    again ();
    ck_assert_str_eq (told->str, "Search string not found\n");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* The case of the letters, whole words */
START_TEST (test_options)
{
    text ("bone One one\n");
    search_for ("one");
    ck_found (1, 3);
    edit_cursor_move (test_edit, -test_edit->buffer.curs1);
    edit_search_options.case_sens = FALSE;
    edit_search_options.whole_words = TRUE;
    search_for ("ONE");
    ck_found (5, 3);
    again ();
    ck_found (9, 3);

    edit_cursor_move (test_edit, -test_edit->buffer.curs1);
    edit_search_options.case_sens = TRUE;
    edit_search_options.whole_words = FALSE;
    search_for ("ONE");
    ck_assert_str_eq (told->str, "Search string not found\n");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A regular expression of the start of a line: only where a line starts, forward and back */
START_TEST (test_regex_line_start)
{
    text ("two\nxtwo\ntwo\n");
    edit_search_options.type = MC_SEARCH_T_REGEX;
    edit_cursor_move (test_edit, 1);
    search_for ("^two");
    ck_found (9, 3);
    again ();
    ck_assert_str_eq (told->str, "Search string not found\n");

    g_string_truncate (told, 0);
    edit_cursor_move (test_edit, test_edit->buffer.size - test_edit->buffer.curs1);
    edit_search_options.backwards = TRUE;
    search_for ("^two");
    ck_found (9, 3);
    again ();
    ck_found (0, 3);

    // a regular expression the search cannot take
    g_string_truncate (told, 0);
    edit_search_options.backwards = FALSE;
    edit_search_deinit (test_edit);
    test_edit->last_search_string = g_strdup ("(");
    test_edit->search_start = 0;
    ck_assert (edit_search_init (test_edit, test_edit->last_search_string));
    edit_search_cmd (test_edit, TRUE);
    ck_assert_str_ne (told->str, "");
    ck_assert_str_ne (told->str, "Search string not found\n");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* Only in the block marked */
START_TEST (test_in_selection)
{
    text ("one one one one\n");
    test_edit->mark1 = 4;
    test_edit->mark2 = 11;
    edit_search_options.only_in_selection = TRUE;
    search_for ("one");
    ck_found (4, 3);
    again ();
    ck_found (8, 3);
    again ();
    ck_assert_str_eq (told->str, "Search string not found\n");

    // no block: nothing
    g_string_truncate (told, 0);
    test_edit->mark1 = test_edit->mark2 = -1;
    search_for ("one");
    ck_assert_str_eq (told->str, "Search string not found\n");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* Bytes in hex */
START_TEST (test_hex)
{
    text ("abc one\n");
    edit_search_options.type = MC_SEARCH_T_HEX;
    search_for ("6f 6e 65");
    ck_found (4, 3);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* Find all: a bookmark on each line with a match, the cursor on the first */
START_TEST (test_bookmarks)
{
    text ("a\none\nb\none one\nc\n");
    edit_cursor_move (test_edit, test_edit->buffer.size);
    search_create_bookmark = TRUE;
    search_for ("one");
    ck_assert (!search_create_bookmark);
    ck_assert_int_eq (test_edit->buffer.curs1, 2);
    ck_assert (book_mark_query_color (test_edit, 1, EDITOR_BOOKMARK_FOUND_COLOR));
    ck_assert (book_mark_query_color (test_edit, 3, EDITOR_BOOKMARK_FOUND_COLOR));
    ck_assert (!book_mark_query_color (test_edit, 0, EDITOR_BOOKMARK_FOUND_COLOR));
    ck_assert (!book_mark_query_color (test_edit, 2, EDITOR_BOOKMARK_FOUND_COLOR));

    search_create_bookmark = TRUE;
    search_for ("none");
    ck_assert_str_eq (told->str, "Search string not found\n");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* The replace asked at each match: one, skipped, all the rest; cancelled */
START_TEST (test_replace_asked)
{
    static const int one_skip_all[] = { B_REPLACE_ONE, B_SKIP_REPLACE, B_REPLACE_ALL };
    static const int cancel[] = { B_REPLACE_ONE, B_CANCEL };
    char *got;

    text ("a a a a a\n");
    replace_from = "a";
    replace_to = "bb";
    answers = one_skip_all;
    test_edit->replace_mode = 0;
    edit_replace_cmd (test_edit, FALSE);
    got = buffer_text ();
    ck_assert_str_eq (got, "bb a bb bb bb\n");
    g_free (got);
    // all of them: how many were replaced, the one asked for with them
    ck_assert_str_eq (told->str, "4 replacements made\n");
    g_string_truncate (told, 0);

    // again: the same strings, from the point; one replaced, then cancelled
    edit_cursor_move (test_edit, -test_edit->buffer.curs1);
    edit_insert (test_edit, 'a');
    edit_insert (test_edit, 'a');
    edit_cursor_move (test_edit, -test_edit->buffer.curs1);
    test_edit->search_start = 0;
    replace_to = "c";
    answers = cancel;
    test_edit->replace_mode = 0;
    edit_replace_cmd (test_edit, TRUE);
    got = buffer_text ();
    ck_assert_str_eq (got, "bbabb a bb bb bb\n");
    g_free (got);
    // cancelled: not told
    ck_assert_str_eq (told->str, "");

    // nothing to replace: said once
    edit_cursor_move (test_edit, -test_edit->buffer.curs1);
    test_edit->search_start = 0;
    replace_from = "zz";
    test_edit->replace_mode = 0;
    edit_replace_cmd (test_edit, FALSE);
    ck_assert_str_eq (told->str, "Search string not found\n");
    // no string: nothing, and the strings before kept for again
    g_string_truncate (told, 0);
    replace_from = "";
    edit_replace_cmd (test_edit, FALSE);
    ck_assert_str_eq (told->str, "");
    test_edit->search_start = 0;
    test_edit->replace_mode = 0;
    edit_replace_cmd (test_edit, TRUE);
    ck_assert_str_eq (told->str, "Search string not found\n");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* Again with no replace before: the dialog asks for the strings */
START_TEST (test_replace_again_first)
{
    static const int all[] = { B_REPLACE_ALL };
    char *got;

    text ("x y x\n");
    replace_from = "x";
    replace_to = "z";
    answers = all;
    test_edit->replace_mode = 0;
    edit_replace_cmd (test_edit, TRUE);
    got = buffer_text ();
    ck_assert_str_eq (got, "z y z\n");
    g_free (got);
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
    tcase_add_test (tc_core, test_forward);
    tcase_add_test (tc_core, test_backward);
    tcase_add_test (tc_core, test_options);
    tcase_add_test (tc_core, test_regex_line_start);
    tcase_add_test (tc_core, test_in_selection);
    tcase_add_test (tc_core, test_hex);
    tcase_add_test (tc_core, test_bookmarks);
    tcase_add_test (tc_core, test_replace_asked);
    tcase_add_test (tc_core, test_replace_again_first);
    // ***********************************

    return mctest_run_all (tc_core);
}

/* --------------------------------------------------------------------------------------------- */
