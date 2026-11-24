/*
   lib/mcconfig - tests for the history of the inputs: kept in a profile and read back

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

#define TEST_SUITE_NAME "lib/mcconfig"

#include "tests/mctest.h"

#include <stdio.h>  // remove ()

#include "lib/fileloc.h"
#include "lib/mcconfig.h"
#include "lib/strutil.h"

/* --------------------------------------------------------------------------------------------- */

static char *home;  // the data and the settings of the user, of the test
static mc_config_t *cfg;
static char *ini;

/* --------------------------------------------------------------------------------------------- */

static GList *
list_of (const char *first, ...)
{
    GList *list = NULL;
    const char *item;
    va_list args;

    va_start (args, first);
    for (item = first; item != NULL; item = va_arg (args, const char *))
        list = g_list_append (list, g_strdup (item));
    va_end (args);
    return list;
}

/* --------------------------------------------------------------------------------------------- */

/* A history from its last item back to the first, as the inputs keep it: "a,b,c" */
static char *
joined (GList *last)
{
    GString *text = g_string_new (NULL);
    GList *l;

    for (l = g_list_first (last); l != NULL; l = g_list_next (l))
    {
        if (text->len > 0)
            g_string_append_c (text, ',');
        g_string_append (text, (const char *) l->data);
    }
    return g_string_free (text, FALSE);
}

/* --------------------------------------------------------------------------------------------- */

#define ck_history(last, expected)                                                                 \
    do                                                                                             \
    {                                                                                              \
        char *got = joined (last);                                                                 \
                                                                                                   \
        ck_assert_str_eq (got, (expected));                                                        \
        g_free (got);                                                                              \
    }                                                                                              \
    while (0)

/* --------------------------------------------------------------------------------------------- */

static void
setup (void)
{
    str_init_strings ("UTF-8");
    mc_global.utf8_display = TRUE;
    num_history_items_recorded = 60;
    ini = g_build_filename (home, "profile.ini", (char *) NULL);
    (void) remove (ini);
    cfg = mc_config_init (ini, FALSE);
}

/* --------------------------------------------------------------------------------------------- */

static void
teardown (void)
{
    mc_config_deinit (cfg);
    (void) remove (ini);
    g_free (ini);
    str_uninit_strings ();
}

/* --------------------------------------------------------------------------------------------- */
/*** tests ***************************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/* Kept and read back in order, the last item given; an item twice once, where it was last */
START_TEST (test_save_load)
{
    GList *h = list_of ("a", "b", "a", "c", NULL);
    GList *read;

    mc_config_history_save (cfg, "find", h);
    read = mc_config_history_load (cfg, "find");
    ck_assert_ptr_null (read->next);
    ck_assert_str_eq ((const char *) read->data, "c");
    ck_history (read, "b,a,c");
    g_list_free_full (g_list_first (read), g_free);

    // saved again over it: only what is given now
    g_list_free_full (h, g_free);
    h = list_of ("x", NULL);
    mc_config_history_save (cfg, "find", h);
    read = mc_config_history_load (cfg, "find");
    ck_history (read, "x");
    g_list_free_full (g_list_first (read), g_free);

    // none: the history goes
    mc_config_history_save (cfg, "find", NULL);
    ck_assert (!mc_config_has_group (cfg, "find"));
    ck_assert_ptr_null (mc_config_history_load (cfg, "find"));
    ck_assert_ptr_null (mc_config_history_load (cfg, ""));
    ck_assert_ptr_null (mc_config_history_load (cfg, NULL));

    // with no name nothing is kept
    mc_config_history_save (cfg, "", h);
    mc_config_history_save (cfg, NULL, h);
    ck_assert (!mc_config_has_group (cfg, ""));
    g_list_free_full (h, g_free);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* The newest items as many as are kept; the list given from anywhere in it */
START_TEST (test_limit)
{
    GList *h = list_of ("1", "2", "3", "4", "5", NULL);
    GList *read;

    num_history_items_recorded = 3;
    mc_config_history_save (cfg, "cmd", g_list_nth (h, 1));
    read = mc_config_history_load (cfg, "cmd");
    ck_history (read, "3,4,5");
    g_list_free_full (g_list_first (read), g_free);
    g_list_free_full (h, g_free);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A display of 8 bits: the history is kept in UTF-8 and read back in its codeset */
START_TEST (test_8bit)
{
    GList *h = list_of ("\xd6\xc1", "plain", NULL);
    GList *read;
    char *raw;

    str_uninit_strings ();
    str_init_strings ("KOI8-R");
    mc_global.utf8_display = FALSE;
    mc_config_history_save (cfg, "find", h);
    raw = mc_config_get_string_raw (cfg, "find", "0", "");
    ck_assert_str_eq (raw, "\xd0\xb6\xd0\xb0");
    g_free (raw);
    read = mc_config_history_load (cfg, "find");
    ck_history (read, "\xd6\xc1,plain");
    g_list_free_full (g_list_first (read), g_free);
    g_list_free_full (h, g_free);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* The history of the user, from the file of the histories; its newest item */
START_TEST (test_user_history)
{
    char *path = mc_config_get_full_path (MC_HISTORY_FILE);
    mc_config_t *user;
    GList *h = list_of ("old", "new", NULL);
    GList *read;
    char *item;
    GError *error = NULL;

    ck_assert_ptr_nonnull (path);
    ck_assert (g_str_has_prefix (path, home));
    user = mc_config_init (path, FALSE);
    mc_config_history_save (user, "grep", h);
    ck_assert (mc_config_save_file (user, &error));
    mc_config_deinit (user);

    read = mc_config_history_get ("grep");
    ck_history (read, "old,new");
    g_list_free_full (g_list_first (read), g_free);
    item = mc_config_history_get_recent_item ("grep");
    ck_assert_str_eq (item, "new");
    g_free (item);
    ck_assert_ptr_null (mc_config_history_get_recent_item ("none"));
    ck_assert_ptr_null (mc_config_history_get (""));
    // no history kept at all
    num_history_items_recorded = 0;
    ck_assert_ptr_null (mc_config_history_get ("grep"));

    (void) remove (path);
    g_free (path);
    g_list_free_full (h, g_free);
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

    // the places of the user, before glib and mc read them once and for all
    home = g_dir_make_tmp ("history-XXXXXX", NULL);
    g_setenv ("XDG_DATA_HOME", home, TRUE);
    g_setenv ("XDG_CONFIG_HOME", home, TRUE);
    g_setenv ("XDG_CACHE_HOME", home, TRUE);

    tc_core = tcase_create ("Core");

    tcase_add_checked_fixture (tc_core, setup, teardown);

    // Add new tests here: ***************
    tcase_add_test (tc_core, test_save_load);
    tcase_add_test (tc_core, test_limit);
    tcase_add_test (tc_core, test_8bit);
    tcase_add_test (tc_core, test_user_history);
    // ***********************************

    result = mctest_run_all (tc_core);
    remove_tree (home);
    g_free (home);
    return result;
}

/* --------------------------------------------------------------------------------------------- */
