/*
   tests/src - unit tests for the index of the plugins in the help markdown

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

#define TEST_SUITE_NAME "/src/help_md_plugins"

#include "tests/mctest.h"

#include "lib/editor-plugin.h"

#include "src/help.h"
#include "src/help_md.h"

/* the markers the help window reads, written out to compare */
#define B   "\010"
#define N   "\013"
#define COL "\017"
#define LS  "\001"
#define LP  "\002"
#define LE  "\003"

/* --------------------------------------------------------------------------------------------- */

static void *
test_open (mc_editor_host_t *host, void *editor_dialog)
{
    (void) editor_dialog;
    return host;
}

/* --------------------------------------------------------------------------------------------- */

static void
test_close (void *plugin_data)
{
    (void) plugin_data;
}

/* --------------------------------------------------------------------------------------------- */

/* Two plugins with a help, given in the order the index does not keep, and one without */
static const mc_editor_plugin_t plugin_zeta = {
    .api_version = MC_EDITOR_PLUGIN_API_VERSION,
    .name = "zeta",
    .display_name = "&Zeta...",
    .open = test_open,
    .close = test_close,
    .help_file = "zeta.md",
    .help_node = "zeta",
    .summary = "Does z",
};

static const mc_editor_plugin_t plugin_alpha = {
    .api_version = MC_EDITOR_PLUGIN_API_VERSION,
    .name = "alpha",
    .display_name = "Alpha",
    .open = test_open,
    .close = test_close,
    .help_file = "alpha.md",
    .help_node = "start",
    .summary = "Does a_b",
};

static const mc_editor_plugin_t plugin_nohelp = {
    .api_version = MC_EDITOR_PLUGIN_API_VERSION,
    .name = "nohelp",
    .display_name = "No help",
    .open = test_open,
    .close = test_close,
};

/* --------------------------------------------------------------------------------------------- */

/* The text of the node of a page with one heading, from the line after the heading to its end */
static char *
convert_body (const char *md)
{
    char *all, *p, *body;

    all = help_md_convert (md, NULL);
    p = strstr (all, "[plugins]\n" B "Plugins" N "\n\n");
    ck_assert_msg (p != NULL, "no node in \"%s\"", all);
    body = g_strdup (p + strlen ("[plugins]\n" B "Plugins" N "\n\n"));
    g_free (all);
    // up to the end of the node
    p = strchr (body, CHAR_NODE_END);
    if (p != NULL)
        *p = '\0';

    return body;
}

/* --------------------------------------------------------------------------------------------- */

/* a row for each plugin with a help, by name, the name without & and dots a link to its node */
START_TEST (test_plugins_index)
{
    char *body;

    mctest_assert_true (mc_editor_plugin_add (&plugin_zeta));
    mctest_assert_true (mc_editor_plugin_add (&plugin_nohelp));
    mctest_assert_true (mc_editor_plugin_add (&plugin_alpha));

    body = convert_body ("# Plugins\n"
                         "\n"
                         "{{plugins}}\n"
                         "After.\n");
    mctest_assert_str_eq (body,
                          LS "Alpha" LP "alpha.md#start" LE COL "7" COL "Does a_b\n" LS "Zeta" LP
                             "zeta.md#zeta" LE COL "7" COL "Does z\n"
                             "\n"
                             "After.\n\n");
    g_free (body);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* with no plugin the marker leaves nothing */
START_TEST (test_no_plugins)
{
    char *body;

    body = convert_body ("# Plugins\n"
                         "\n"
                         "{{plugins}}\n"
                         "After.\n");
    mctest_assert_str_eq (body, "After.\n\n");
    g_free (body);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

int
main (void)
{
    TCase *tc_core;

    tc_core = tcase_create ("Core");
    // the registry has no removal: the test without plugins runs first
    tcase_add_test (tc_core, test_no_plugins);
    tcase_add_test (tc_core, test_plugins_index);

    return mctest_run_all (tc_core);
}
