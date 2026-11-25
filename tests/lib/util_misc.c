/*
   lib/util - tests for the small helpers: sizes as text, a line of a text, errors, control
   characters, printable bytes, backups and the files of a language

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

#define TEST_SUITE_NAME "/lib/util"

#include "tests/mctest.h"

#include <errno.h>
#include <stdio.h>  // remove ()

#include "lib/strutil.h"
#include "lib/util.h"

/* --------------------------------------------------------------------------------------------- */

static char *dir;

static void
setup (void)
{
    str_init_strings ("UTF-8");
    dir = g_dir_make_tmp ("util-XXXXXX", NULL);
    ck_assert_ptr_nonnull (dir);
}

/* --------------------------------------------------------------------------------------------- */

static void
teardown (void)
{
    GDir *d = g_dir_open (dir, 0, NULL);
    const char *name;

    while ((name = g_dir_read_name (d)) != NULL)
    {
        char *path = g_build_filename (dir, name, (char *) NULL);

        (void) remove (path);
        g_free (path);
    }
    g_dir_close (d);
    (void) remove (dir);
    g_free (dir);
    str_uninit_strings ();
}

/* --------------------------------------------------------------------------------------------- */

static char *
file (const char *name, const char *text)
{
    char *path = g_build_filename (dir, name, (char *) NULL);

    if (text != NULL)
        ck_assert (g_file_set_contents (path, text, -1, NULL));
    return path;
}

/* --------------------------------------------------------------------------------------------- */

static void
ck_size (uintmax_t size, unsigned int len, int units, gboolean si, const char *expected)
{
    char buffer[32];

    memset (buffer, 'x', sizeof (buffer));
    size_trunc_len (buffer, len, size, units, si);
    ck_assert_str_eq (buffer, expected);
}

/* --------------------------------------------------------------------------------------------- */
/*** tests ***************************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/* A size in a field of so many characters: the number as it is while it fits, then in the next
   unit, rounded, a character of the field for its letter; of 1024 or of 1000 */
START_TEST (test_size_trunc_len)
{
    ck_size (1234, 4, 0, FALSE, "1234");
    ck_size (12345, 4, 0, FALSE, "12K");
    ck_size (12345, 4, 0, TRUE, "12k");
    ck_size (12600, 4, 0, TRUE, "13k");
    ck_size (1536, 3, 0, FALSE, "2K");
    ck_size (UINTMAX_C (5) * 1024 * 1024 * 1024, 4, 0, FALSE, "5G");
    // in units of K already
    ck_size (2048, 4, 1, FALSE, "2M");
    ck_size (999, 4, 1, FALSE, "999K");
    ck_size (20480, 4, 1, FALSE, "20M");
    ck_size (1000, 4, 1, TRUE, "1m");
    ck_size (1001, 9, 1, TRUE, "1025k");
    // nothing: 0 in its own unit, else about one of the unit before
    ck_size (0, 4, 0, FALSE, "0");
    ck_size (0, 4, 2, FALSE, "0");
    ck_size (100, 1, 0, FALSE, "B");
    ck_size (100, 2, 0, FALSE, "~B");
    ck_size (600000, 2, 0, FALSE, "1M");
    // no width: nine
    ck_size (123456789, 0, 0, FALSE, "123456789");
    ck_size (1234567890, 0, 0, FALSE, "1205633K");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A size as bytes while it fits nine digits, then in larger units */
START_TEST (test_size_trunc)
{
    ck_assert_str_eq (size_trunc (500, FALSE), "500 B");
    ck_assert_str_eq (size_trunc (999999999, FALSE), "999999999 B");
    ck_assert_str_eq (size_trunc (2000000000, FALSE), "1953125 KiB");
    ck_assert_str_eq (size_trunc (2000000000, TRUE), "2000000 kB");
    ck_assert_str_eq (size_trunc (UINTMAX_C (3000000000000000), TRUE), "3000000 GB");
    ck_assert_str_eq (path_trunc ("abcdefghij", 5), "ab~ij");
    ck_assert_str_eq (path_trunc ("abcdefghij", -1), "abcdefghij");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A line of a text: to its end, or the end the caller says */
START_TEST (test_extract_line)
{
    const char *text = "abc\ndef";
    size_t len = 99;

    ck_assert_str_eq (extract_line (text, text + 7, &len), "abc");
    ck_assert_uint_eq (len, 3);
    ck_assert_str_eq (extract_line (text, text + 2, &len), "ab");
    ck_assert_uint_eq (len, 2);
    ck_assert_str_eq (extract_line (text + 4, text + 7, NULL), "def");
    ck_assert_str_eq (extract_line ("", text, &len), "");
    ck_assert_uint_eq (len, 0);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* An error as its text and its number; a letter as its control character; a text after '|' */
START_TEST (test_small)
{
    const char *e = unix_error_string (ENOENT);
    char *number = g_strdup_printf ("(%d)", ENOENT);

    ck_assert (g_str_has_suffix (e, number));
    ck_assert_uint_gt (strlen (e), strlen (number) + 1);
    g_free (number);

    ck_assert_int_eq (ascii_alpha_to_cntrl ('a'), 1);
    ck_assert_int_eq (ascii_alpha_to_cntrl ('A'), 1);
    ck_assert_int_eq (ascii_alpha_to_cntrl ('z'), 26);
    ck_assert_int_eq (ascii_alpha_to_cntrl ('Z'), 26);
    ck_assert_int_eq (ascii_alpha_to_cntrl ('1'), '1');
    ck_assert_int_eq (ascii_alpha_to_cntrl ('['), '[');

    ck_assert_str_eq (Q_ ("Context|Text"), "Text");
    ck_assert_str_eq (Q_ ("Plain"), "Plain");
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A byte to show as it is: xterm has no C1 controls to show, the locale says the rest */
START_TEST (test_is_printable)
{
    mc_global.utf8_display = TRUE;
    mc_global.tty.xterm_flag = TRUE;
    ck_assert (is_printable ('a'));
    ck_assert (!is_printable (0x7f));
    ck_assert (!is_printable (0x85));
    ck_assert (is_printable (0xa0));
    ck_assert (is_printable (0x1a0));
    // a byte of an int
    ck_assert (!is_printable (0x185));
    ck_assert (!is_printable (0x1f));
    mc_global.tty.xterm_flag = FALSE;
    ck_assert (is_printable ('a'));
    ck_assert (!is_printable ('\t'));
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A backup made, the file brought back from it, the backup gone */
START_TEST (test_backup)
{
    char *path = file ("f.txt", "first");
    char *backup = g_strconcat (path, "~", (char *) NULL);
    char *text;

    ck_assert (mc_util_make_backup_if_possible (path, "~"));
    ck_assert (g_file_get_contents (backup, &text, NULL, NULL));
    ck_assert_str_eq (text, "first");
    g_free (text);
    g_free (file ("f.txt", "second"));
    ck_assert (mc_util_restore_from_backup_if_possible (path, "~"));
    ck_assert (g_file_get_contents (path, &text, NULL, NULL));
    ck_assert_str_eq (text, "first");
    g_free (text);
    ck_assert (mc_util_unlink_backup_if_possible (path, "~"));
    ck_assert (!g_file_test (backup, G_FILE_TEST_EXISTS));
    // nothing to back up
    g_free (path);
    path = file ("none", NULL);
    ck_assert (!mc_util_make_backup_if_possible (path, "~"));
    g_free (backup);
    g_free (path);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* The file of the language of the user: of its whole name, of its two letters, else the plain
   one; the tag before the extension, or at the end of a name with none */
START_TEST (test_home_file)
{
    char *name = NULL;
    char *data;
    size_t length = 0;

    g_free (file ("hint.md", "plain"));
    g_free (file ("hint.ru.md", "ru"));
    g_free (file ("hint.de_AT.md", "de_AT"));
    g_free (file ("TAGS.ru", "tags ru"));
    g_free (file (".hidden.ru", "hidden ru"));
    g_unsetenv ("LC_ALL");
    g_unsetenv ("LC_MESSAGES");

    g_setenv ("LANG", "ru_RU.UTF-8", TRUE);
    data = load_mc_home_file (dir, "hint.md", &name, &length);
    ck_assert_str_eq (data, "ru");
    ck_assert_uint_eq (length, 2);
    ck_assert (g_str_has_suffix (name, "/hint.ru.md"));
    g_free (data);
    g_free (name);
    data = load_mc_home_file (dir, "TAGS", NULL, NULL);
    ck_assert_str_eq (data, "tags ru");
    g_free (data);
    // the dot of a hidden file is no extension
    data = load_mc_home_file (dir, ".hidden", NULL, NULL);
    ck_assert_str_eq (data, "hidden ru");
    g_free (data);

    // LC_ALL before LANG, the whole name of it
    g_setenv ("LC_ALL", "de_AT", TRUE);
    data = load_mc_home_file (dir, "hint.md", &name, NULL);
    ck_assert_str_eq (data, "de_AT");
    g_free (data);
    g_free (name);
    g_unsetenv ("LC_ALL");

    g_setenv ("LANG", "fr_FR.UTF-8", TRUE);
    data = load_mc_home_file (dir, "hint.md", &name, NULL);
    ck_assert_str_eq (data, "plain");
    ck_assert (g_str_has_suffix (name, "/hint.md"));
    g_free (data);
    g_free (name);
    g_unsetenv ("LANG");
    data = load_mc_home_file (dir, "hint.md", NULL, NULL);
    ck_assert_str_eq (data, "plain");
    g_free (data);
    ck_assert_ptr_null (load_mc_home_file (dir, "none.md", NULL, NULL));
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
    tcase_add_test (tc_core, test_size_trunc_len);
    tcase_add_test (tc_core, test_size_trunc);
    tcase_add_test (tc_core, test_extract_line);
    tcase_add_test (tc_core, test_small);
    tcase_add_test (tc_core, test_is_printable);
    tcase_add_test (tc_core, test_backup);
    tcase_add_test (tc_core, test_home_file);
    // ***********************************

    return mctest_run_all (tc_core);
}

/* --------------------------------------------------------------------------------------------- */
