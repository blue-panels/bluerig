/*
   src/editor - tests for the keyboard macros: the file of the macros, recording, playing

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

#include <glib/gstdio.h>

#include "lib/charsets.h"
#include "lib/fileloc.h"  // MC_MACRO_FILE
#include "lib/keybind.h"
#include "lib/mcconfig.h"
#include "lib/tty/key.h"
#include "src/selcodepage.h"
#include "src/setup.h"  // macros_list

#include "src/editor/editwidget.h"
#include "src/editor/edit-impl.h"
#include "src/editor/editmacros.h"

/* A dialog, not a bare group: the editor reaches its owner as one, and a
 * WGroup has no room for the fields it reads there. */
static WDialog owner;
static WEdit *test_edit;
static char *config_home;
/* the key the dialog of a new macro answers with */
static int query_hotkey;
static int query_calls;

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
/* @Mock */
int
editcmd_dialog_raw_key_query (const char *heading, const char *query, gboolean cancel)
{
    (void) heading;
    (void) query;
    (void) cancel;

    query_calls++;
    return query_hotkey;
}

/* --------------------------------------------------------------------------------------------- */

static void
remove_tree (const char *dir)
{
    GDir *d;
    const char *name;

    d = g_dir_open (dir, 0, NULL);
    while (d != NULL && (name = g_dir_read_name (d)) != NULL)
    {
        char *path;

        path = g_build_filename (dir, name, (char *) NULL);
        if (g_file_test (path, G_FILE_TEST_IS_DIR) && !g_file_test (path, G_FILE_TEST_IS_SYMLINK))
            remove_tree (path);
        else
            (void) g_unlink (path);
        g_free (path);
    }
    if (d != NULL)
        g_dir_close (d);
    (void) g_rmdir (dir);
}

/* --------------------------------------------------------------------------------------------- */

static char *
macro_file_path (void)
{
    return mc_config_get_full_path (MC_MACRO_FILE);
}

/* --------------------------------------------------------------------------------------------- */

static void
write_macro_file (const char *text)
{
    char *path;

    path = macro_file_path ();
    ck_assert (g_file_set_contents (path, text, -1, NULL));
    g_free (path);
}

/* --------------------------------------------------------------------------------------------- */

static char *
read_macro_file (void)
{
    char *path, *text = NULL;

    path = macro_file_path ();
    if (!g_file_get_contents (path, &text, NULL, NULL))
        text = NULL;
    g_free (path);
    return text;
}

/* --------------------------------------------------------------------------------------------- */

static void
free_macros (void)
{
    for (guint i = 0; i < macros_list->len; i++)
        g_array_free (g_array_index (macros_list, macros_t, i).macro, TRUE);
    g_array_set_size (macros_list, 0);
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

static int
key (const char *name)
{
    int code;

    code = tty_keyname_to_keycode (name, NULL);
    ck_assert_int_gt (code, 0);
    return code;
}

/* --------------------------------------------------------------------------------------------- */

/* @Before */
static void
setup (void)
{
    WRect r;
    char *path;

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

    // a test that fails skips its teardown: no file of the macros is left from it
    path = macro_file_path ();
    (void) g_unlink (path);
    g_free (path);

    macros_list = g_array_new (TRUE, FALSE, sizeof (macros_t));
    macro_index = -1;
    query_hotkey = 0;
    query_calls = 0;
}

/* --------------------------------------------------------------------------------------------- */

/* @After */
static void
teardown (void)
{
    char *path;

    free_macros ();
    g_array_free (macros_list, TRUE);
    macros_list = NULL;

    path = macro_file_path ();
    (void) g_unlink (path);
    g_free (path);

    group_remove_widget (test_edit);
    widget_destroy (WIDGET (test_edit));

    free_codepages_list ();
    str_uninit_strings ();
}

/* --------------------------------------------------------------------------------------------- */

/* A macro of the file plays its actions: here two characters typed */
START_TEST (test_load_and_play)
{
    write_macro_file ("[editor]\nctrl-t=InsertChar:104;InsertChar:105;\n");

    ck_assert (edit_load_macro_cmd (test_edit));
    ck_assert_uint_eq (macros_list->len, 1);

    ck_assert (edit_execute_macro (test_edit, key ("ctrl-t")));
    assert_text ("hi");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* An action the editor does not know is dropped, the rest of the macro stays */
START_TEST (test_load_unknown_action)
{
    write_macro_file ("[editor]\nctrl-t=NoSuchAction:1;InsertChar:120;\n");

    ck_assert (edit_load_macro_cmd (test_edit));
    ck_assert (edit_execute_macro (test_edit, key ("ctrl-t")));
    assert_text ("x");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* The macros are sorted by their keys, whatever the order of the file: each key finds its own */
START_TEST (test_load_several)
{
    write_macro_file ("[editor]\n"
                      "ctrl-z=InsertChar:122;\n"
                      "ctrl-a=InsertChar:97;\n"
                      "ctrl-k=InsertChar:107;\n");

    ck_assert (edit_load_macro_cmd (test_edit));
    ck_assert_uint_eq (macros_list->len, 3);

    ck_assert (edit_execute_macro (test_edit, key ("ctrl-k")));
    ck_assert (edit_execute_macro (test_edit, key ("ctrl-a")));
    ck_assert (edit_execute_macro (test_edit, key ("ctrl-z")));
    assert_text ("kaz");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* The file is read once: a second load, the list not empty, does nothing */
START_TEST (test_load_once)
{
    write_macro_file ("[editor]\nctrl-t=InsertChar:104;\n");

    ck_assert (edit_load_macro_cmd (test_edit));
    ck_assert (!edit_load_macro_cmd (test_edit));
    ck_assert_uint_eq (macros_list->len, 1);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* "ExecuteScript:N" is the shell command N of the menu, not a character */
START_TEST (test_load_script)
{
    const macros_t *m;
    const macro_action_t *act;

    write_macro_file ("[editor]\nctrl-t=ExecuteScript:3;\n");

    ck_assert (edit_load_macro_cmd (test_edit));
    ck_assert_uint_eq (macros_list->len, 1);

    m = &g_array_index (macros_list, macros_t, 0);
    ck_assert_uint_eq (m->macro->len, 1);
    act = &g_array_index (m->macro, macro_action_t, 0);
    ck_assert_int_eq (act->action, CK_PipeBlock (3));
    ck_assert_int_eq (act->ch, -1);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A key without a macro plays nothing, nor does key 0 */
START_TEST (test_play_no_macro)
{
    write_macro_file ("[editor]\nctrl-t=InsertChar:104;\n");
    ck_assert (edit_load_macro_cmd (test_edit));

    ck_assert (!edit_execute_macro (test_edit, key ("ctrl-y")));
    ck_assert (!edit_execute_macro (test_edit, 0));
    assert_text ("");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* Record, stop: the macro is asked a key, written to the file, and plays from it after a load */
START_TEST (test_record_store_reload)
{
    char *text;

    query_hotkey = key ("ctrl-t");

    edit_execute_key_command (test_edit, CK_MacroStartRecord, -1);
    edit_execute_key_command (test_edit, CK_InsertChar, 'a');
    edit_execute_key_command (test_edit, CK_InsertChar, 'b');
    edit_execute_key_command (test_edit, CK_MacroStopRecord, -1);

    ck_assert_int_eq (query_calls, 1);
    ck_assert_int_eq (macro_index, -1);
    assert_text ("ab");

    text = read_macro_file ();
    ck_assert_ptr_nonnull (text);
    ck_assert_msg (strstr (text, "InsertChar:97;InsertChar:98;") != NULL, "the file: %s", text);
    g_free (text);

    // what a new session reads
    free_macros ();
    ck_assert (edit_load_macro_cmd (test_edit));
    ck_assert (edit_execute_macro (test_edit, key ("ctrl-t")));
    assert_text ("abab");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A new macro on the key of an old one takes its place, in the list and in the file */
START_TEST (test_store_replaces)
{
    char *text;

    // the name of the key as bluerig writes it; a name spelled otherwise ("ctrl-t") stays in
    // the file next to the new one
    write_macro_file ("[editor]\nCtrl-T=InsertChar:120;\n");
    ck_assert (edit_load_macro_cmd (test_edit));

    query_hotkey = key ("ctrl-t");
    edit_execute_key_command (test_edit, CK_MacroStartRecord, -1);
    edit_execute_key_command (test_edit, CK_InsertChar, 'y');
    edit_execute_key_command (test_edit, CK_MacroStopRecord, -1);

    ck_assert_uint_eq (macros_list->len, 1);

    text = read_macro_file ();
    ck_assert_msg (strstr (text, "InsertChar:121;") != NULL, "the file: %s", text);
    ck_assert_msg (strstr (text, "InsertChar:120;") == NULL, "the file: %s", text);
    g_free (text);

    ck_assert (edit_execute_macro (test_edit, key ("ctrl-t")));
    assert_text ("yy");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* Esc in the dialog of the key keeps nothing */
START_TEST (test_store_cancelled)
{
    char *text;

    query_hotkey = ESC_CHAR;

    edit_execute_key_command (test_edit, CK_MacroStartRecord, -1);
    edit_execute_key_command (test_edit, CK_InsertChar, 'a');
    edit_execute_key_command (test_edit, CK_MacroStopRecord, -1);

    ck_assert_int_eq (query_calls, 1);
    ck_assert_uint_eq (macros_list->len, 0);
    text = read_macro_file ();
    ck_assert_ptr_null (text);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

int
main (void)
{
    TCase *tc_core;
    int result;

    // the file of the macros goes to a directory of the test, not to the user's
    config_home = g_dir_make_tmp ("edit_macros_XXXXXX", NULL);
    g_assert_nonnull (config_home);
    g_setenv ("HOME", config_home, TRUE);
    g_setenv ("XDG_CONFIG_HOME", config_home, TRUE);
    g_setenv ("XDG_DATA_HOME", config_home, TRUE);
    g_setenv ("XDG_CACHE_HOME", config_home, TRUE);
    mc_config_init_config_paths (NULL);

    tc_core = tcase_create ("Core");

    tcase_add_checked_fixture (tc_core, setup, teardown);

    tcase_add_test (tc_core, test_load_and_play);
    tcase_add_test (tc_core, test_load_unknown_action);
    tcase_add_test (tc_core, test_load_several);
    tcase_add_test (tc_core, test_load_once);
    tcase_add_test (tc_core, test_load_script);
    tcase_add_test (tc_core, test_play_no_macro);
    tcase_add_test (tc_core, test_record_store_reload);
    tcase_add_test (tc_core, test_store_replaces);
    tcase_add_test (tc_core, test_store_cancelled);

    result = mctest_run_all (tc_core);

    mc_config_deinit_config_paths ();
    remove_tree (config_home);
    g_free (config_home);

    return result;
}

/* --------------------------------------------------------------------------------------------- */
