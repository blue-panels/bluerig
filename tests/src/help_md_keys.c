/*
   tests/src - unit tests for the keys of the actions in the help markdown

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

#define TEST_SUITE_NAME "/src/help_md_keys"

#include "tests/mctest.h"

#include "lib/keybind.h"

#include "src/help.h"
#include "src/help_md.h"
#include "src/keymap.h"

/* the markers the help window reads, written out to compare */
#define B   "\010"
#define N   "\013"
#define COL "\017"

/* The keymaps of the editor: the keys are numbers of their own, only the captions are read.  F2
   is bound twice, and its second action is never called. */
static const global_keymap_t test_editor_map[] = {
    { 1001, CK_Save, "F2" },
    { 1002, CK_Save, "Ctrl-s" },
    { 1001, CK_Mark, "F2" },
    { 1001, CK_Save, "F2" },
    { 1003, CK_Help, "F1" },
    { 1004, CK_Quit, "F10" },
    { 1005, CK_ShowTabTws, "Alt-_" },
    { 1006, CK_ExtendedKeyMap, "Ctrl-x" },
    { 0, 0, "" },
};

static const global_keymap_t test_editor_x_map[] = {
    { 2001, CK_Remove, "d" },
    { 0, 0, "" },
};

/* --------------------------------------------------------------------------------------------- */

/* @Before */
static void
setup (void)
{
    editor_map = test_editor_map;
    editor_x_map = test_editor_x_map;
}

/* --------------------------------------------------------------------------------------------- */

/* The text of the node of a page with one heading, from the line after the heading to its end */
static char *
convert_body (const char *md)
{
    char *all, *p, *body;

    all = help_md_convert (md, NULL);
    p = strstr (all, "[keys]\n" B "Keys" N "\n\n");
    ck_assert_msg (p != NULL, "no node in \"%s\"", all);
    body = g_strdup (p + strlen ("[keys]\n" B "Keys" N "\n\n"));
    g_free (all);
    // up to the end of the node
    p = strchr (body, CHAR_NODE_END);
    if (p != NULL)
        *p = '\0';

    return body;
}

/* --------------------------------------------------------------------------------------------- */

static void
assert_keys (const char *marker, const char *expected)
{
    char *md, *body, *want;

    md = g_strdup_printf ("# Keys\n\n%s\n", marker);
    body = convert_body (md);
    want = g_strdup_printf ("%s\n\n", expected);
    mctest_assert_str_eq (body, want);
    g_free (want);
    g_free (body);
    g_free (md);
}

/* --------------------------------------------------------------------------------------------- */

/* every key that calls the action, each once */
START_TEST (test_keys_of_action) { assert_keys ("{{key:Save|F2}}", "F2, Ctrl-s"); }
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* a key another action has before it does not call this one: the default stays */
START_TEST (test_shadowed_key) { assert_keys ("{{key:Mark|F3}}", "F3"); }
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* no such action, or no key for it: the default */
START_TEST (test_default)
{
    assert_keys ("{{key:NoSuchAction|Ctrl-q}}", "Ctrl-q");
    assert_keys ("{{key:Redo|Alt-r}}", "Alt-r");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* F1 is no part of F10, and a character markdown reads is a character of the key */
START_TEST (test_caption_text)
{
    assert_keys ("{{key:Quit|Esc}}", "F10");
    assert_keys ("{{key:ShowTabTws|Alt-\\_}}", "Alt-_");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* a key after the prefix key has the prefix before it */
START_TEST (test_prefix_key) { assert_keys ("{{key:Remove|F8}}", "Ctrl-x d"); }
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* the bar of a marker is no column of a table */
START_TEST (test_marker_in_table)
{
    char *body;

    body = convert_body ("# Keys\n"
                         "\n"
                         "|  |  |\n"
                         "|--|--|\n"
                         "| {{key:Save|F2}} | Save the file |\n"
                         "| {{key:Help|F1}} | Help |\n");
    mctest_assert_str_eq (body,
                          "F2, Ctrl-s" COL "12" COL "Save the file\n"
                          "F1" COL "12" COL "Help\n"
                          "\n");
    g_free (body);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

int
main (void)
{
    TCase *tc_core;

    tc_core = tcase_create ("Core");
    tcase_add_checked_fixture (tc_core, setup, NULL);
    tcase_add_test (tc_core, test_keys_of_action);
    tcase_add_test (tc_core, test_shadowed_key);
    tcase_add_test (tc_core, test_default);
    tcase_add_test (tc_core, test_caption_text);
    tcase_add_test (tc_core, test_prefix_key);
    tcase_add_test (tc_core, test_marker_in_table);

    return mctest_run_all (tc_core);
}
