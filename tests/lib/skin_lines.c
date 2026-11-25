/*
   lib/skin - tests for the lines and the scrollbars of a skin, and for the files of the skins

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

#define TEST_SUITE_NAME "/lib/skin"

#include "tests/mctest.h"

#include <stdio.h>  // remove ()

#include "lib/global.h"
#include "lib/fileloc.h"  // MC_SKINS_DIR
#include "lib/mcconfig.h"
#include "lib/skin.h"
#include "lib/skin/internal.h"  // mc_skin_ini_file_load ()
#include "lib/strutil.h"
#include "lib/tty/tty.h"

/* --------------------------------------------------------------------------------------------- */

static mc_skin_t skin;
static char *home;  // the data of the user, the skins of the system and those of the package

/* --------------------------------------------------------------------------------------------- */

static void
setup (void)
{
    str_init_strings ("UTF-8");
    mc_global.utf8_display = TRUE;
    mc_global.tty.slow_terminal = FALSE;
    mc_global.tty.ugly_line_drawing = FALSE;
    memset (&skin, 0, sizeof (skin));
    skin.config = mc_config_init (NULL, FALSE);
}

/* --------------------------------------------------------------------------------------------- */

static void
teardown (void)
{
    mc_config_deinit (skin.config);
    str_uninit_strings ();
}

/* --------------------------------------------------------------------------------------------- */

static void
set_line (const char *section, const char *key, const char *value)
{
    mc_config_set_string_raw (skin.config, section, key, value);
}

/* --------------------------------------------------------------------------------------------- */

/* A file of the skins of @base, @text its content */
static void
skin_file (const char *base, const char *name, const char *text)
{
    char *dir = g_build_filename (home, base, MC_SKINS_DIR, (char *) NULL);
    char *path = g_build_filename (dir, name, (char *) NULL);

    ck_assert_int_eq (g_mkdir_with_parents (dir, 0700), 0);
    ck_assert (g_file_set_contents (path, text, -1, NULL));
    g_free (path);
    g_free (dir);
}

/* --------------------------------------------------------------------------------------------- */
/*** tests ***************************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/* In UTF-8 the lines are the characters of the skin, or the box drawing ones */
START_TEST (test_lines_utf8)
{
    set_line ("lines", "horiz", "=");
    // the name of mc 4.8.33 and before
    set_line ("Lines", "vert", "!");
    set_line ("lines", "dhoriz", "\xe2\x94\x81");
    mc_skin_lines_parse_ini_file (&skin);
    ck_assert_uint_eq (mc_tty_frm[MC_TTY_FRM_HORIZ], '=');
    ck_assert_uint_eq (mc_tty_frm[MC_TTY_FRM_VERT], '!');
    ck_assert_uint_eq (mc_tty_frm[MC_TTY_FRM_LEFTTOP], 0x250C);
    ck_assert_uint_eq (mc_tty_frm[MC_TTY_FRM_CROSS], 0x253C);
    ck_assert_uint_eq (mc_tty_frm[MC_TTY_FRM_DHORIZ], 0x2501);
    ck_assert_uint_eq (mc_tty_frm[MC_TTY_FRM_DVERT], 0x2551);
    ck_assert_uint_eq (mc_tty_frm[MC_TTY_FRM_DRIGHTMIDDLE], 0x2562);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A line the display cannot show takes all of its kind back: the single ones to those of the
   terminal, the double ones to the single; what is no UTF-8 in the file is not there at all */
START_TEST (test_lines_invalid)
{
    set_line ("lines", "vert", "\xff");
    mc_skin_lines_parse_ini_file (&skin);
    ck_assert_uint_eq (mc_tty_frm[MC_TTY_FRM_VERT], 0x2502);

    str_uninit_strings ();
    str_init_strings ("KOI8-R");
    mc_global.utf8_display = FALSE;
    // a star, which KOI8-R has not
    set_line ("lines", "horiz", "-");
    set_line ("lines", "cross", "\xe2\x98\x85");
    set_line ("lines", "drightmiddle", "\xe2\x98\x85");
    mc_skin_lines_parse_ini_file (&skin);
    ck_assert_uint_eq (mc_tty_frm[MC_TTY_FRM_HORIZ], MC_ACS_HLINE);
    ck_assert_uint_eq (mc_tty_frm[MC_TTY_FRM_CROSS], MC_ACS_PLUS);
    ck_assert_uint_eq (mc_tty_frm[MC_TTY_FRM_LEFTMIDDLE], MC_ACS_LTEE);
    ck_assert_uint_eq (mc_tty_frm[MC_TTY_FRM_DHORIZ], MC_ACS_HLINE);
    ck_assert_uint_eq (mc_tty_frm[MC_TTY_FRM_DLEFTTOP], MC_ACS_ULCORNER);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* An 8-bit display: the single lines those of the terminal, the double ones of the codeset when
   it has them, else the single ones */
START_TEST (test_lines_8bit)
{
    str_uninit_strings ();
    str_init_strings ("KOI8-R");
    mc_global.utf8_display = FALSE;
    mc_skin_lines_parse_ini_file (&skin);
    ck_assert_uint_eq (mc_tty_frm[MC_TTY_FRM_HORIZ], MC_ACS_HLINE);
    ck_assert_uint_eq (mc_tty_frm[MC_TTY_FRM_RIGHTBOTTOM], MC_ACS_LRCORNER);
    ck_assert_uint_eq (mc_tty_frm[MC_TTY_FRM_CROSS], MC_ACS_PLUS);
    // U+2550 and U+2551 of KOI8-R
    ck_assert_uint_eq (mc_tty_frm[MC_TTY_FRM_DHORIZ], 0xA0);
    ck_assert_uint_eq (mc_tty_frm[MC_TTY_FRM_DVERT], 0xA1);
    // an ASCII line as it is
    set_line ("lines", "horiz", "-");
    // a double line the codeset has not
    set_line ("lines", "dvert", "\xe2\x98\x85");
    mc_skin_lines_parse_ini_file (&skin);
    ck_assert_uint_eq (mc_tty_frm[MC_TTY_FRM_HORIZ], '-');
    ck_assert_uint_eq (mc_tty_frm[MC_TTY_FRM_DHORIZ], '-');
    ck_assert_uint_eq (mc_tty_frm[MC_TTY_FRM_DVERT], MC_ACS_VLINE);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A slow terminal draws blanks, an ugly one ASCII, the skin before them */
START_TEST (test_lines_terminals)
{
    mc_global.tty.slow_terminal = TRUE;
    mc_skin_lines_parse_ini_file (&skin);
    ck_assert_uint_eq (mc_tty_frm[MC_TTY_FRM_HORIZ], ' ');
    ck_assert_uint_eq (mc_tty_frm[MC_TTY_FRM_DVERT], ' ');
    ck_assert_uint_eq (mc_skin_scrollbar_vert.thumb, '#');

    mc_config_deinit (skin.config);
    skin.config = mc_config_init (NULL, FALSE);
    mc_global.tty.slow_terminal = FALSE;
    mc_global.tty.ugly_line_drawing = TRUE;
    mc_skin_lines_parse_ini_file (&skin);
    ck_assert_uint_eq (mc_tty_frm[MC_TTY_FRM_LEFTTOP], '+');
    ck_assert_uint_eq (mc_tty_frm[MC_TTY_FRM_DHORIZ], '-');
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* The scrollbars: the arrows, the track, the thumb and its length, as the skin has them */
START_TEST (test_scrollbars)
{
    mc_skin_lines_parse_ini_file (&skin);
    ck_assert_uint_eq (mc_skin_scrollbar_vert.start, '^');
    ck_assert_uint_eq (mc_skin_scrollbar_vert.end, 'v');
    // no track in the skin: the line of the frame
    ck_assert_uint_eq (mc_skin_scrollbar_vert.track, 0x2502);
    ck_assert_uint_eq (mc_skin_scrollbar_vert.thumb, 0x2588);
    ck_assert_int_eq (mc_skin_scrollbar_vert.thumb_len, 6);
    ck_assert_uint_eq (mc_skin_scrollbar_horiz.start, '<');
    ck_assert_uint_eq (mc_skin_scrollbar_horiz.track, 0x2500);
    ck_assert_uint_eq (mc_skin_scrollbar_horiz.thumb, 0x25A0);
    ck_assert_int_eq (mc_skin_scrollbar_horiz.thumb_len, 4);

    set_line ("scrollbar", "vstart", "\xe2\x96\xb2");
    set_line ("scrollbar", "vtrack", " ");
    set_line ("scrollbar", "vthumblen", "0");
    mc_skin_lines_parse_ini_file (&skin);
    ck_assert_uint_eq (mc_skin_scrollbar_vert.start, 0x25B2);
    ck_assert_uint_eq (mc_skin_scrollbar_vert.track, ' ');
    ck_assert_int_eq (mc_skin_scrollbar_vert.thumb_len, 1);

    // what the display cannot show: '#' for the thumb, the default for an arrow
    str_uninit_strings ();
    str_init_strings ("KOI8-R");
    mc_global.utf8_display = FALSE;
    set_line ("scrollbar", "vthumb", "\xe2\x98\x85");
    set_line ("scrollbar", "hend", "\xe2\x98\x85");
    mc_skin_lines_parse_ini_file (&skin);
    ck_assert_uint_eq (mc_skin_scrollbar_vert.thumb, '#');
    ck_assert_uint_eq (mc_skin_scrollbar_horiz.end, '>');
    // the default thumb of the horizontal one, U+25A0, KOI8-R has
    ck_assert_uint_eq (mc_skin_scrollbar_horiz.thumb, 0x94);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* The skin of no files: ASCII lines, and its description */
START_TEST (test_hardcoded)
{
    mc_skin_t hard = { 0 };
    char *description;

    mc_skin_set_hardcoded_skin (&hard);
    description = mc_config_get_string (hard.config, "skin", "description", NULL);
    ck_assert_str_eq (description, "hardcoded skin");
    g_free (description);
    {
        mc_config_t *own = skin.config;

        skin.config = hard.config;
        mc_skin_lines_parse_ini_file (&skin);
        skin.config = own;
    }
    ck_assert_uint_eq (mc_tty_frm[MC_TTY_FRM_HORIZ], '-');
    ck_assert_uint_eq (mc_tty_frm[MC_TTY_FRM_DVERT], '|');
    ck_assert_uint_eq (mc_tty_frm[MC_TTY_FRM_CROSS], '+');
    ck_assert_uint_eq (mc_skin_scrollbar_vert.thumb, '#');
    mc_config_deinit (hard.config);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* The skins there are: of the user, of the system, of the package, each name once, sorted */
START_TEST (test_list)
{
    GPtrArray *list;
    guint i;
    GString *names = g_string_new (NULL);

    skin_file ("data/bluerig", "mine.ini", "");
    skin_file ("data/bluerig", "dark.ini", "");
    skin_file ("etc", "dark.ini", "");
    skin_file ("etc", "notes.txt", "");
    skin_file ("etc", ".ini", "");
    skin_file ("share", "blue.ini", "");
    list = mc_skin_list ();
    for (i = 0; i < list->len; i++)
        g_string_append_printf (names, "%s;", (const char *) g_ptr_array_index (list, i));
    ck_assert_str_eq (names->str, "blue;dark;mine;");
    g_ptr_array_free (list, TRUE);
    g_string_free (names, TRUE);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A skin by its name: of the user first, then of the system, then of the package; by the name of
   its file with .ini or without; by an absolute path */
START_TEST (test_load)
{
    mc_skin_t s = { 0 };
    char *value;
    char *path;

    skin_file ("data/bluerig", "dark.ini", "[skin]\ndescription=user\n");
    skin_file ("etc", "dark.ini", "[skin]\ndescription=system\n");
    skin_file ("etc", "light.ini", "[skin]\ndescription=light\n");
    skin_file ("share", "plain", "[skin]\ndescription=plain\n");

    s.name = (char *) "dark";
    ck_assert (mc_skin_ini_file_load (&s));
    value = mc_config_get_string (s.config, "skin", "description", NULL);
    ck_assert_str_eq (value, "user");
    g_free (value);
    mc_config_deinit (s.config);
    s.name = (char *) "light";
    ck_assert (mc_skin_ini_file_load (&s));
    mc_config_deinit (s.config);
    s.name = (char *) "plain";
    ck_assert (mc_skin_ini_file_load (&s));
    value = mc_config_get_string (s.config, "skin", "description", NULL);
    ck_assert_str_eq (value, "plain");
    g_free (value);
    mc_config_deinit (s.config);
    s.name = (char *) "none";
    ck_assert (!mc_skin_ini_file_load (&s));

    // a path: absolute only
    path = g_build_filename (home, "etc", MC_SKINS_DIR, "light.ini", (char *) NULL);
    s.name = path;
    ck_assert (mc_skin_ini_file_load (&s));
    mc_config_deinit (s.config);
    g_free (path);
    s.name = (char *) "etc/skins/light.ini";
    ck_assert (!mc_skin_ini_file_load (&s));
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

static void
remove_tree (const char *path)
{
    GDir *d = g_dir_open (path, 0, NULL);
    const char *name;

    if (d != NULL)
    {
        while ((name = g_dir_read_name (d)) != NULL)
        {
            char *child = g_build_filename (path, name, (char *) NULL);

            remove_tree (child);
            g_free (child);
        }
        g_dir_close (d);
    }
    (void) remove (path);
}

/* --------------------------------------------------------------------------------------------- */

int
main (void)
{
    TCase *tc_core;
    int result;
    char *data;

    // the places of the skins, before glib and mc read them once and for all
    home = g_dir_make_tmp ("skin-lines-XXXXXX", NULL);
    data = g_build_filename (home, "data", (char *) NULL);
    g_setenv ("XDG_DATA_HOME", data, TRUE);
    g_setenv ("XDG_CONFIG_HOME", home, TRUE);
    g_setenv ("XDG_CACHE_HOME", home, TRUE);
    g_free (data);
    mc_global.sysconfig_dir = g_build_filename (home, "etc", (char *) NULL);
    mc_global.share_data_dir = g_build_filename (home, "share", (char *) NULL);

    tc_core = tcase_create ("Core");

    tcase_add_checked_fixture (tc_core, setup, teardown);

    // Add new tests here: ***************
    tcase_add_test (tc_core, test_lines_utf8);
    tcase_add_test (tc_core, test_lines_invalid);
    tcase_add_test (tc_core, test_lines_8bit);
    tcase_add_test (tc_core, test_lines_terminals);
    tcase_add_test (tc_core, test_scrollbars);
    tcase_add_test (tc_core, test_hardcoded);
    tcase_add_test (tc_core, test_list);
    tcase_add_test (tc_core, test_load);
    // ***********************************

    result = mctest_run_all (tc_core);
    remove_tree (home);
    g_free (home);
    g_free (mc_global.sysconfig_dir);
    g_free (mc_global.share_data_dir);
    return result;
}

/* --------------------------------------------------------------------------------------------- */
