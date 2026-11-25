/*
   lib/widget - tests on the screen: a dialog drawn, its widgets, the keys it takes

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
static WInput *input;

static void
setup (void)
{
    screen_open (24, 80);
}

/* --------------------------------------------------------------------------------------------- */

static void
teardown (void)
{
    screen_close ();
}

/* --------------------------------------------------------------------------------------------- */

/* A dialog of a label, an input line and two buttons, as the editor makes them */
static void
dialog_new (void)
{
    dlg = dlg_create (TRUE, 0, 0, 8, 40, WPOS_CENTER, FALSE, dialog_colors, NULL, NULL, NULL,
                      "Title");
    group_add_widget (GROUP (dlg), label_new (2, 3, "Name:"));
    input = input_new (2, 10, input_colors, 20, "start", NULL, INPUT_COMPLETE_NONE);
    group_add_widget (GROUP (dlg), input);
    group_add_widget (GROUP (dlg), button_new (5, 8, B_ENTER, DEFPUSH_BUTTON, "&OK", NULL));
    group_add_widget (GROUP (dlg), button_new (5, 20, B_CANCEL, NORMAL_BUTTON, "&Cancel", NULL));
    dlg_init (dlg);
    // the line has the focus, as the dialogs of the editor give it
    widget_select (WIDGET (input));
}

/* --------------------------------------------------------------------------------------------- */

static void
dialog_free (void)
{
    dlg_run_done (dlg);
    widget_destroy (WIDGET (dlg));
}

/* --------------------------------------------------------------------------------------------- */

#define ck_row(row, expected)                                                                      \
    do                                                                                             \
    {                                                                                              \
        char *got = screen_row (row);                                                              \
                                                                                                   \
        ck_assert_str_eq (got, (expected));                                                        \
        g_free (got);                                                                              \
    }                                                                                              \
    while (0)

/* --------------------------------------------------------------------------------------------- */
/*** tests ***************************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/* The dialog drawn in the middle of the screen: its frame, its title, its widgets */
START_TEST (test_drawn)
{
    int row, col;
    const WRect *r;

    dialog_new ();
    r = &WIDGET (dlg)->rect;
    // in the middle of the screen
    ck_assert_int_eq (r->y, (24 - 8) / 2);
    ck_assert_int_eq (r->x, (80 - 40) / 2);
    // the title on the frame, inside the shadow of the dialog
    ck_assert (screen_find ("Title", &row, &col));
    ck_assert_int_eq (row, r->y + 1);
    ck_row (r->y + 1, "                     +-------------- Title ---------------+");
    ck_row (r->y + 2, "                     | Name:  start                       |");
    ck_row (r->y + 5, "                     |      [< OK >]    [ Cancel ]        |");
    ck_row (r->y + 6, "                     +------------------------------------+");
    // nothing around it
    ck_row (r->y - 1, "");
    ck_row (r->y + 7, "");
    // the text of the line not yet typed over in its own color, the label in that of the dialog
    ck_assert (screen_find ("start", &row, &col));
    ck_assert_int_eq (screen_color (row, col), input_colors[WINPUTC_UNCHANGED]);
    ck_assert (screen_find ("Name:", &row, &col));
    ck_assert_int_eq (screen_color (row, col), dialog_colors[DLG_COLOR_NORMAL]);
    dialog_free ();
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* Keys: typed into the line, Tab to the next widget, Enter on a button closes the dialog */
START_TEST (test_keys)
{
    int row, col;

    dialog_new ();
    // the first key takes the place of the text, the line in the color of the input
    dlg_process_event (dlg, 'x', NULL);
    ck_assert_str_eq (input_get_ctext (input), "x");
    ck_assert (!screen_find ("start", NULL, NULL));
    ck_assert (screen_find ("Name:  x", &row, &col));
    ck_assert_int_eq (screen_color (row, col + 7), input_colors[WINPUTC_MAIN]);

    dlg_process_event (dlg, '\t', NULL);
    dlg_process_event (dlg, '\t', NULL);
    dlg_process_event (dlg, '\n', NULL);
    ck_assert (widget_get_state (WIDGET (dlg), WST_CLOSED));
    ck_assert_int_eq (dlg->ret_value, B_CANCEL);
    dialog_free ();
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
    tcase_add_test (tc_core, test_drawn);
    tcase_add_test (tc_core, test_keys);
    // ***********************************

    return mctest_run_all (tc_core);
}

/* --------------------------------------------------------------------------------------------- */
