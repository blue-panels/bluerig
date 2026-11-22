/*
   tests/src - unit tests for the tables of the help markdown

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

#define TEST_SUITE_NAME "/src/help_md_table"

#include "tests/mctest.h"

#include "src/help.h"
#include "src/help_md.h"

/* the markers the help window reads, written out to compare */
#define B   "\010"
#define N   "\013"
#define COL "\017"

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

/* a head with no text is left out; each row is a line, the second column after the widest
   first cell and two blanks */
START_TEST (test_table_no_head)
{
    char *body;

    body = convert_body ("# Keys\n"
                         "\n"
                         "|  |  |\n"
                         "|--|--|\n"
                         "| **F2** | Save |\n"
                         "| F12, Ctrl-F2 | Save under another name |\n"
                         "\n"
                         "After.\n");
    mctest_assert_str_eq (body,
                          B "F2" N COL "14" COL "Save\n"
                            "F12, Ctrl-F2" COL "14" COL "Save under another name\n"
                            "\n"
                            "After.\n\n");
    g_free (body);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* a head with text is bold; a column is as wide as its widest cell, the head too */
START_TEST (test_table_head)
{
    char *body;

    body = convert_body ("# Keys\n"
                         "\n"
                         "| Key | Action |\n"
                         "|:----|:-------|\n"
                         "| F2 | Save |\n");
    mctest_assert_str_eq (body,
                          B "Key" N COL "5" COL B "Action" N "\n"
                            "F2" COL "5" COL "Save\n"
                            "\n");
    g_free (body);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* \| is a bar inside a cell, and a link takes the columns of its text only */
START_TEST (test_table_bar_and_link)
{
    char *body;

    body = convert_body ("# Keys\n"
                         "\n"
                         "|  |  |\n"
                         "|--|--|\n"
                         "| a\\|b | [Go](#keys) |\n"
                         "| [Find](#keys) | x |\n");
    ck_assert_msg (strstr (body, "a|b" COL "6" COL) == body, "\"%s\"", body);
    g_free (body);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* a cell wider than 40 does not widen its column; the window puts the next cell on the next line */
START_TEST (test_table_wide_cell)
{
    char *body;

    body = convert_body ("# Keys\n"
                         "\n"
                         "|  |  |\n"
                         "|--|--|\n"
                         "| 01234567890123456789012345678901234567890123456789 | x |\n"
                         "| a | y |\n");
    mctest_assert_str_eq (body,
                          "01234567890123456789012345678901234567890123456789" COL "3" COL "x\n"
                          "a" COL "3" COL "y\n"
                          "\n");
    g_free (body);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* a line with bars that has no rule under it is a paragraph */
START_TEST (test_no_rule_no_table)
{
    char *body;

    body = convert_body ("# Keys\n"
                         "\n"
                         "| a | b |\n");
    mctest_assert_str_eq (body, "| a | b |\n\n");
    g_free (body);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

int
main (void)
{
    TCase *tc_core;

    tc_core = tcase_create ("Core");
    tcase_add_test (tc_core, test_table_no_head);
    tcase_add_test (tc_core, test_table_head);
    tcase_add_test (tc_core, test_table_bar_and_link);
    tcase_add_test (tc_core, test_table_wide_cell);
    tcase_add_test (tc_core, test_no_rule_no_table);

    return mctest_run_all (tc_core);
}
