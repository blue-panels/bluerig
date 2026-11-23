/*
   lib/skin - the colors of a skin

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

#define TEST_SUITE_NAME "/lib/skin"

#include "tests/mctest.h"

#include <stdlib.h>

#include "lib/global.h"
#include "lib/skin.h"
#include "lib/skin/internal.h"  // mc_skin_color_parse_ini_file()
#include "lib/tty/color.h"

/* --------------------------------------------------------------------------------------------- */

/* A skin of a few groups: the defaults of the core and of a group, a key of its own, a key
   that gives only the foreground, aliases (one through another, two in a loop), and the
   groups that are not made into pairs */
static const char *skin_text = "[skin]\n"
                               "description = test\n"
                               "[aliases]\n"
                               "main = blue\n"
                               "accent = hot\n"
                               "hot = red\n"
                               "ping = pong\n"
                               "pong = ping\n"
                               "[core]\n"
                               "_default_ = lightgray;main\n"
                               "selected = black;cyan\n"
                               "[dialog]\n"
                               "_default_ = black;lightgray\n"
                               "dfocus = white;accent\n"
                               "dhotnormal = yellow\n"
                               "looped = ping;black\n"
                               "[syntax]\n"
                               "keyword = brightcyan;;bold\n"
                               "[lines]\n"
                               "horiz = -\n"
                               "[widget-scrollbar]\n"
                               "thumb = white;blue\n";

/* --------------------------------------------------------------------------------------------- */

/* the value of the hash of the colors, as the skin engine frees it */
static void
color_free (gpointer data)
{
    tty_color_pair_t *color = (tty_color_pair_t *) data;

    g_free (color->fg);
    g_free (color->bg);
    g_free (color->attrs);
    g_free (color);
}

/* --------------------------------------------------------------------------------------------- */

static void
load_skin (const char *text)
{
    mc_config_t *config;
    gboolean ok;

    config = mc_config_init (NULL, FALSE);
    ck_assert_ptr_nonnull (config);
    ck_assert (g_key_file_load_from_data (config->handle, text, -1, G_KEY_FILE_NONE, NULL));

    mc_skin__default.name = g_strdup ("test");
    mc_skin__default.config = config;
    mc_skin__default.colors = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, color_free);

    ok = mc_skin_color_parse_ini_file (&mc_skin__default);
    ck_assert_msg (ok, "the skin parses");
}

/* --------------------------------------------------------------------------------------------- */

/* @Before */
static void
setup (void)
{
    setenv ("TERM", "xterm", 1);
    mc_global.tty.disable_colors = FALSE;
    mc_global.tty.color_terminal_string = (char *) "";
    tty_init_colors (FALSE, TRUE);
    ck_assert_msg (tty_use_colors (), "colors are on: the skin is not read as black and white");

    load_skin (skin_text);
}

/* --------------------------------------------------------------------------------------------- */

/* @After */
static void
teardown (void)
{
    mc_skin_deinit ();
    tty_colors_done ();
}

/* --------------------------------------------------------------------------------------------- */

static void
assert_pair (int pair, const char *fg, const char *bg)
{
    ck_assert_int_gt (pair, 0);
    mctest_assert_str_eq (tty_color_pair_foreground (pair), fg);
    mctest_assert_str_eq (tty_color_pair_background (pair), bg);
}

/* --------------------------------------------------------------------------------------------- */

/* A key of its own: both colors as the skin gives them, the background through two aliases */
START_TEST (test_key_of_its_own)
{
    assert_pair (mc_skin_color_get ("dialog", "dfocus"), "white", "red");
    assert_pair (mc_skin_color_get ("core", "selected"), "black", "cyan");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A key the group lacks takes the default of the group; a group the skin lacks takes the
   default of the core, its alias resolved */
START_TEST (test_defaults)
{
    assert_pair (mc_skin_color_get ("dialog", "nosuchkey"), "black", "lightgray");
    assert_pair (mc_skin_color_get ("nosuchgroup", "nosuchkey"), "lightgray", "blue");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A key that gives only the foreground is drawn on the background of its group */
START_TEST (test_foreground_only)
{
    assert_pair (mc_skin_color_get ("dialog", "dhotnormal"), "yellow", "lightgray");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* Aliases in a loop do not hang the parse: the name stays as it was written */
START_TEST (test_alias_loop)
{
    char *fg, *bg, *attrs;

    ck_assert (mc_skin_color_names ("dialog", "looped", &fg, &bg, &attrs));
    mctest_assert_str_eq (fg, "ping");
    mctest_assert_str_eq (bg, "black");
    ck_assert_ptr_null (attrs);

    g_free (fg);
    g_free (bg);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* Whether the skin gives a key itself, not by the defaults */
START_TEST (test_is_set)
{
    ck_assert (mc_skin_color_is_set ("dialog", "dfocus"));
    ck_assert (mc_skin_color_is_set ("dialog", "_default_"));
    ck_assert (!mc_skin_color_is_set ("dialog", "nosuchkey"));
    ck_assert (!mc_skin_color_is_set ("nosuchgroup", "_default_"));
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* [syntax], [lines], [aliases], [skin] and widget-* are not colors of the skin: no pairs */
START_TEST (test_sections_without_pairs)
{
    ck_assert (!mc_skin_color_is_set ("syntax", "keyword"));
    ck_assert (!mc_skin_color_is_set ("lines", "horiz"));
    ck_assert (!mc_skin_color_is_set ("aliases", "main"));
    ck_assert (!mc_skin_color_is_set ("skin", "description"));
    ck_assert (!mc_skin_color_is_set ("widget-scrollbar", "thumb"));
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* The names of a key, aliases resolved, an empty part NULL; FALSE for a key that is not there */
START_TEST (test_color_names)
{
    char *fg, *bg, *attrs;

    ck_assert (mc_skin_color_names ("syntax", "keyword", &fg, &bg, &attrs));
    mctest_assert_str_eq (fg, "brightcyan");
    ck_assert_ptr_null (bg);
    mctest_assert_str_eq (attrs, "bold");
    g_free (fg);
    g_free (attrs);

    ck_assert (mc_skin_color_names ("dialog", "dfocus", &fg, &bg, &attrs));
    mctest_assert_str_eq (fg, "white");
    mctest_assert_str_eq (bg, "red");
    ck_assert_ptr_null (attrs);
    g_free (fg);
    g_free (bg);

    ck_assert (!mc_skin_color_names ("dialog", "nosuchkey", &fg, &bg, &attrs));
    ck_assert_ptr_null (fg);
    ck_assert_ptr_null (bg);
    ck_assert_ptr_null (attrs);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A foreground of its own on the background of a color of the skin */
START_TEST (test_color_on)
{
    assert_pair (mc_skin_color_on ("dialog", "dfocus", "green"), "green", "red");
    assert_pair (mc_skin_color_on ("nosuchgroup", "nosuchkey", "green"), "green", "blue");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

int
main (void)
{
    TCase *tc_core;

    tc_core = tcase_create ("Core");

    tcase_add_checked_fixture (tc_core, setup, teardown);

    tcase_add_test (tc_core, test_key_of_its_own);
    tcase_add_test (tc_core, test_defaults);
    tcase_add_test (tc_core, test_foreground_only);
    tcase_add_test (tc_core, test_alias_loop);
    tcase_add_test (tc_core, test_is_set);
    tcase_add_test (tc_core, test_sections_without_pairs);
    tcase_add_test (tc_core, test_color_names);
    tcase_add_test (tc_core, test_color_on);

    return mctest_run_all (tc_core);
}

/* --------------------------------------------------------------------------------------------- */
