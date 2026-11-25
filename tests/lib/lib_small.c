/*
   lib - tests for the hooks, the dates of files and the log

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

#define TEST_SUITE_NAME "/lib"

#include "tests/mctest.h"

#include <stdio.h>  // remove ()

#include "lib/hook.h"
#include "lib/mcconfig.h"
#include "lib/strutil.h"
#include "lib/logging.c"  // for its state, read once
#include "lib/timefmt.c"  // for the length it keeps

/* --------------------------------------------------------------------------------------------- */

static GString *calls;  // what the hooks did, in turn
static hook_t *chain;

static void
setup (void)
{
    str_init_strings ("UTF-8");
    calls = g_string_new (NULL);
    chain = NULL;
    g_setenv ("TZ", "UTC", TRUE);
    tzset ();
    // the setting of the log read again, the length of a date found again
    logging_initialized = FALSE;
    i18n_timelength_cache = MAX_I18NTIMELENGTH + 1;
    user_recent_timeformat = NULL;
    user_old_timeformat = NULL;
}

/* --------------------------------------------------------------------------------------------- */

static void
teardown (void)
{
    while (chain != NULL)
    {
        hook_t *next = chain->next;

        g_free (chain);
        chain = next;
    }
    g_string_free (calls, TRUE);
    str_uninit_strings ();
}

/* --------------------------------------------------------------------------------------------- */

static void
hook_a (void *data)
{
    g_string_append_printf (calls, "a%s ", (const char *) data);
}

static void
hook_b (void *data)
{
    g_string_append_printf (calls, "b%s ", (const char *) data);
}

/* a hook that takes itself off while the hooks run */
static void
hook_leaving (void *data)
{
    (void) data;
    g_string_append (calls, "leaving ");
    delete_hook (&chain, hook_leaving);
}

/* --------------------------------------------------------------------------------------------- */
/*** tests ***************************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/* Hooks added, run in the order they came, found and taken off, one taking itself off */
START_TEST (test_hooks)
{
    ck_assert (!hook_present (chain, hook_a));
    execute_hooks (chain);
    ck_assert_str_eq (calls->str, "");

    add_hook (&chain, hook_a, (void *) "1");
    add_hook (&chain, hook_b, (void *) "2");
    add_hook (&chain, hook_a, (void *) "3");
    ck_assert (hook_present (chain, hook_a));
    ck_assert (hook_present (chain, hook_b));
    execute_hooks (chain);
    ck_assert_str_eq (calls->str, "a1 b2 a3 ");

    // all of one function go, the others stay in their order
    g_string_truncate (calls, 0);
    delete_hook (&chain, hook_a);
    ck_assert (!hook_present (chain, hook_a));
    add_hook (&chain, hook_leaving, NULL);
    add_hook (&chain, hook_a, (void *) "4");
    execute_hooks (chain);
    ck_assert_str_eq (calls->str, "b2 leaving a4 ");
    ck_assert (!hook_present (chain, hook_leaving));
    // the rest in their order
    g_string_truncate (calls, 0);
    execute_hooks (chain);
    ck_assert_str_eq (calls->str, "b2 a4 ");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A date of a file: its time of day when it is recent, its year when it is old or to come */
START_TEST (test_file_date)
{
    const time_t now = time (NULL);
    char *expected;

    user_recent_timeformat = (char *) "R %Y-%m-%d %H:%M";
    user_old_timeformat = (char *) "O %Y-%m-%d";
    ck_assert (g_str_has_prefix (file_date (now - 60), "R "));
    ck_assert (g_str_has_prefix (file_date (now - 100L * 24 * 60 * 60), "R "));
    ck_assert (g_str_has_prefix (file_date (now - 200L * 24 * 60 * 60), "O "));
    // a little in the future, a clock of NFS, is recent; more is not
    ck_assert (g_str_has_prefix (file_date (now + 30 * 60), "R "));
    ck_assert (g_str_has_prefix (file_date (now + 2 * 60 * 60), "O "));
    ck_assert_str_eq (file_date (0), "O 1970-01-01");
    expected = g_strdup ("O 2001-09-09");
    ck_assert_str_eq (file_date (1000000000), expected);
    g_free (expected);

    // the formats of mc: the widest of them all, in the bounds
    user_recent_timeformat = NULL;
    user_old_timeformat = NULL;
    ck_assert_uint_ge (i18n_checktimelength (), MIN_I18NTIMELENGTH);
    ck_assert_uint_le (i18n_checktimelength (), MAX_I18NTIMELENGTH);
    ck_assert_str_eq (file_date (1000000000), "Sep  9  2001");

    // a format too wide or too narrow: the length of the standard one
    i18n_timelength_cache = MAX_I18NTIMELENGTH + 1;
    user_recent_timeformat = (char *) "%A %B %d %Y %H:%M:%S";
    ck_assert_uint_eq (i18n_checktimelength (), STD_I18NTIMELENGTH);
    i18n_timelength_cache = MAX_I18NTIMELENGTH + 1;
    user_recent_timeformat = (char *) "%y";
    user_old_timeformat = (char *) "%y";
    ck_assert_uint_eq (i18n_checktimelength (), STD_I18NTIMELENGTH);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* The log: to the file of the environment when it is on, always for what is always logged */
START_TEST (test_log)
{
    char *dir = g_dir_make_tmp ("log-XXXXXX", NULL);
    char *log = g_build_filename (dir, "test.log", (char *) NULL);
    char *text = NULL;

    g_setenv ("BLUERIG_LOG_FILE", log, TRUE);
    g_setenv ("BLUERIG_LOG_ENABLE", "true", TRUE);
    mc_log ("one %d\n", 1);
    mc_always_log ("two %s\n", "2");
    ck_assert (g_file_get_contents (log, &text, NULL, NULL));
    ck_assert_str_eq (text, "one 1\ntwo 2\n");
    g_free (text);
    (void) remove (log);
    (void) remove (dir);
    g_free (log);
    g_free (dir);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* Off by the environment, the log writes only what is always logged */
START_TEST (test_log_off)
{
    char *dir = g_dir_make_tmp ("log-XXXXXX", NULL);
    char *log = g_build_filename (dir, "test.log", (char *) NULL);
    char *text = NULL;

    g_setenv ("BLUERIG_LOG_FILE", log, TRUE);
    g_setenv ("BLUERIG_LOG_ENABLE", "0", TRUE);
    mc_log ("not this\n");
    ck_assert (!g_file_test (log, G_FILE_TEST_EXISTS));
    mc_always_log ("this\n");
    ck_assert (g_file_get_contents (log, &text, NULL, NULL));
    ck_assert_str_eq (text, "this\n");
    g_free (text);
    (void) remove (log);
    (void) remove (dir);
    g_free (log);
    g_free (dir);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* No environment: the setting of the main config says */
START_TEST (test_log_by_config)
{
    char *dir = g_dir_make_tmp ("log-XXXXXX", NULL);
    char *log = g_build_filename (dir, "conf.log", (char *) NULL);
    char *text = NULL;

    g_unsetenv ("BLUERIG_LOG_ENABLE");
    g_unsetenv ("BLUERIG_LOG_FILE");
    mc_global.main_config = mc_config_init (NULL, FALSE);
    mc_config_set_bool (mc_global.main_config, "Development", "logging", TRUE);
    mc_config_set_string (mc_global.main_config, "Development", "logfile", log);
    mc_log ("by config\n");
    ck_assert (g_file_get_contents (log, &text, NULL, NULL));
    ck_assert_str_eq (text, "by config\n");
    g_free (text);
    mc_config_deinit (mc_global.main_config);
    mc_global.main_config = NULL;
    (void) remove (log);
    (void) remove (dir);
    g_free (log);
    g_free (dir);
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
    tcase_add_test (tc_core, test_hooks);
    tcase_add_test (tc_core, test_file_date);
    tcase_add_test (tc_core, test_log);
    tcase_add_test (tc_core, test_log_off);
    tcase_add_test (tc_core, test_log_by_config);
    // ***********************************

    return mctest_run_all (tc_core);
}

/* --------------------------------------------------------------------------------------------- */
