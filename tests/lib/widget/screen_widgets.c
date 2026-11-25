/*
   lib/widget - tests on the screen: the check boxes, radio buttons, lists, gauges, lines and
   group boxes as they are drawn, and the keys that change them

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

#define TEST_SUITE_NAME "/lib/widget/screen"

#include "tests/mctest.h"

#include "lib/widget.h"

#include "tests/screen.h"

/* --------------------------------------------------------------------------------------------- */

static WDialog *dlg;

static void
setup (void)
{
    screen_open (24, 80);
    dlg = dlg_create (TRUE, 0, 0, 20, 60, WPOS_CENTER, FALSE, dialog_colors, NULL, NULL, NULL,
                      "Widgets");
}

/* --------------------------------------------------------------------------------------------- */

static void
teardown (void)
{
    dlg_run_done (dlg);
    widget_destroy (WIDGET (dlg));
    screen_close ();
}

/* --------------------------------------------------------------------------------------------- */

#define ck_found(s, r, c) ck_assert_msg (screen_find ((s), (r), (c)), "%s", (s))

/* The row and the column on the screen of a place in the dialog */
#define AT_X(n) (WIDGET (dlg)->rect.x + (n))
#define AT_Y(n) (WIDGET (dlg)->rect.y + (n))

/* --------------------------------------------------------------------------------------------- */
/*** tests ***************************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/* Check boxes: their marks, the space and the hotkey that turn them over */
START_TEST (test_check)
{
    WCheck *yes, *no;
    int row, col;

    yes = check_new (2, 4, TRUE, "&Yes");
    no = check_new (3, 4, FALSE, "&No");
    group_add_widget (GROUP (dlg), yes);
    group_add_widget (GROUP (dlg), no);
    dlg_init (dlg);
    // the last widget added has the focus: give it to the first
    widget_select (WIDGET (yes));

    ck_found ("[x] Yes", &row, &col);
    ck_assert_int_eq (row, AT_Y (2));
    ck_assert_int_eq (col, AT_X (4));
    ck_found ("[ ] No", &row, &col);
    ck_assert_int_eq (row, AT_Y (3));
    // the one with the focus in the color of the focus, the other not
    ck_assert_int_eq (screen_color (AT_Y (2), AT_X (4)), dialog_colors[DLG_COLOR_FOCUS]);
    ck_assert_int_eq (screen_color (AT_Y (3), AT_X (4)), dialog_colors[DLG_COLOR_NORMAL]);
    // its hotkey in its own color
    ck_assert_int_eq (screen_color (AT_Y (3), AT_X (8)), dialog_colors[DLG_COLOR_HOT_NORMAL]);

    // the space turns the focused one over
    dlg_process_event (dlg, ' ', NULL);
    ck_assert (!yes->state);
    ck_found ("[ ] Yes", NULL, NULL);
    // the hotkey the other, and takes the focus to it
    dlg_process_event (dlg, 'n', NULL);
    ck_assert (no->state);
    ck_found ("[x] No", NULL, NULL);
    ck_assert_int_eq (screen_color (AT_Y (3), AT_X (4)), dialog_colors[DLG_COLOR_FOCUS]);
    ck_assert_int_eq (screen_color (AT_Y (2), AT_X (4)), dialog_colors[DLG_COLOR_NORMAL]);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* Radio buttons: the arrows move the cursor, the space chooses */
START_TEST (test_radio)
{
    const char *items[] = { "&One", "&Two", "T&hree" };
    WRadio *radio;

    radio = radio_new (2, 4, 3, items);
    group_add_widget (GROUP (dlg), radio);
    dlg_init (dlg);

    ck_found ("(*) One", NULL, NULL);
    ck_found ("( ) Two", NULL, NULL);
    ck_found ("( ) Three", NULL, NULL);

    // the cursor moves, the choice stays until the space
    dlg_process_event (dlg, KEY_DOWN, NULL);
    dlg_process_event (dlg, KEY_DOWN, NULL);
    ck_assert_int_eq (radio->pos, 2);
    ck_assert_int_eq (radio->sel, 0);
    ck_found ("(*) One", NULL, NULL);
    ck_assert_int_eq (screen_color (AT_Y (4), AT_X (4)), dialog_colors[DLG_COLOR_FOCUS]);
    ck_assert_int_eq (screen_color (AT_Y (2), AT_X (4)), dialog_colors[DLG_COLOR_NORMAL]);
    dlg_process_event (dlg, ' ', NULL);
    ck_assert_int_eq (radio->sel, 2);
    ck_found ("( ) One", NULL, NULL);
    ck_found ("(*) Three", NULL, NULL);

    // a hotkey moves and chooses at once
    dlg_process_event (dlg, 't', NULL);
    ck_assert_int_eq (radio->sel, 1);
    ck_found ("(*) Two", NULL, NULL);
    ck_found ("( ) Three", NULL, NULL);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A list: its items, the current one in the color of the selection, the arrows that move it */
START_TEST (test_listbox)
{
    WListbox *list;
    char *text = NULL;
    int row, col;

    list = listbox_new (2, 4, 3, 20, FALSE, NULL);
    listbox_add_item (list, LISTBOX_APPEND_AT_END, 0, "alpha", NULL, FALSE);
    listbox_add_item (list, LISTBOX_APPEND_AT_END, 0, "beta", NULL, FALSE);
    listbox_add_item (list, LISTBOX_APPEND_AT_END, 0, "gamma", NULL, FALSE);
    listbox_add_item (list, LISTBOX_APPEND_AT_END, 0, "delta", NULL, FALSE);
    group_add_widget (GROUP (dlg), list);
    dlg_init (dlg);

    // three rows: the fourth item is below them
    ck_found ("alpha", &row, &col);
    ck_assert_int_eq (row, AT_Y (2));
    ck_found ("gamma", NULL, NULL);
    ck_assert (!screen_find ("delta", NULL, NULL));
    // the text a column in from the edge, the bar of the current item as wide as the text
    ck_assert_int_eq (col, AT_X (5));
    ck_assert_int_eq (screen_color (AT_Y (2), AT_X (5)), dialog_colors[DLG_COLOR_SELECTED_FOCUS]);
    ck_assert_int_eq (screen_color (AT_Y (2), AT_X (5 + 17)),
                      dialog_colors[DLG_COLOR_SELECTED_FOCUS]);
    ck_assert_int_eq (screen_color (AT_Y (3), AT_X (5)), dialog_colors[DLG_COLOR_NORMAL]);

    dlg_process_event (dlg, KEY_DOWN, NULL);
    ck_assert_int_eq (screen_color (AT_Y (3), AT_X (5)), dialog_colors[DLG_COLOR_SELECTED_FOCUS]);
    ck_assert_int_eq (screen_color (AT_Y (2), AT_X (5)), dialog_colors[DLG_COLOR_NORMAL]);
    listbox_get_current (list, &text, NULL);
    ck_assert_str_eq (text, "beta");

    // past the last row the list scrolls: the first item goes, the fourth comes
    dlg_process_event (dlg, KEY_END, NULL);
    listbox_get_current (list, &text, NULL);
    ck_assert_str_eq (text, "delta");
    ck_assert (!screen_find ("alpha", NULL, NULL));
    ck_found ("delta", &row, NULL);
    ck_assert_int_eq (row, AT_Y (4));
    ck_assert_int_eq (screen_color (AT_Y (4), AT_X (5)), dialog_colors[DLG_COLOR_SELECTED_FOCUS]);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A gauge: as much of it filled as is done, the percentage after it, nothing when hidden */
START_TEST (test_gauge)
{
    WGauge *gauge;
    char *text;
    int row, col, i;

    // 30 columns: 7 for the brackets and the percentage, 23 for the bar
    gauge = gauge_new (2, 4, 30, TRUE, 100, 50);
    group_add_widget (GROUP (dlg), gauge);
    // nothing in the dialog takes the focus: it opens all the same
    dlg_init (dlg);
    ck_assert_ptr_eq (GROUP (dlg)->current->data, gauge);

    ck_found ("]  50%", &row, &col);
    text = screen_text (AT_Y (2), AT_X (4), 1);
    ck_assert_str_eq (text, "[");
    g_free (text);
    // half of 23 rounded: 12 cells filled
    for (i = 1; i <= 12; i++)
        ck_assert_int_eq (screen_color (AT_Y (2), AT_X (4 + i)), CORE_GAUGE_COLOR);
    ck_assert_int_eq (screen_color (AT_Y (2), AT_X (4 + 13)), dialog_colors[DLG_COLOR_NORMAL]);

    gauge_set_value (gauge, 200, 200);
    ck_found ("] 100%", NULL, NULL);
    ck_assert_int_eq (screen_color (AT_Y (2), AT_X (4 + 23)), CORE_GAUGE_COLOR);
    // more done than there is to do: all of it
    gauge_set_value (gauge, 10, 20);
    ck_found ("] 100%", NULL, NULL);
    gauge_set_value (gauge, 100, 0);
    ck_found ("]   0%", NULL, NULL);
    ck_assert_int_eq (screen_color (AT_Y (2), AT_X (4 + 1)), dialog_colors[DLG_COLOR_NORMAL]);

    gauge_show (gauge, FALSE);
    ck_assert (!screen_find ("%", NULL, NULL));
    ck_assert (!screen_find ("[", NULL, NULL));
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A line across the dialog joins its frame, its text in the middle; a group box has its title */
START_TEST (test_lines)
{
    WHLine *line;
    char *text;

    group_add_widget (GROUP (dlg), groupbox_new (2, 3, 4, 24, "Box"));
    line = hline_new (8, -1, -1);
    group_add_widget (GROUP (dlg), line);
    hline_set_text (line, "Middle");
    dlg_init (dlg);

    text = screen_text (AT_Y (2), AT_X (3), 24);
    ck_assert_str_eq (text, "+ Box -----------------+");
    g_free (text);
    text = screen_text (AT_Y (5), AT_X (3), 24);
    ck_assert_str_eq (text, "+----------------------+");
    g_free (text);

    // from one side of the frame to the other, a tee at each end
    text = screen_text (AT_Y (8), AT_X (1), 58);
    ck_assert_str_eq (text, "+-------------------------Middle-------------------------+");
    g_free (text);
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
    tcase_add_test (tc_core, test_check);
    tcase_add_test (tc_core, test_radio);
    tcase_add_test (tc_core, test_listbox);
    tcase_add_test (tc_core, test_gauge);
    tcase_add_test (tc_core, test_lines);
    // ***********************************

    return mctest_run_all (tc_core);
}

/* --------------------------------------------------------------------------------------------- */
