/*
   src/plugins/etags - tests for the reading of a TAGS file

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

#define TEST_SUITE_NAME "/src/plugins/etags"

#include "tests/mctest.h"

#include "src/plugins/etags/etags.c"  // for its file scope functions

/* --------------------------------------------------------------------------------------------- */

/* A TAGS file as etags writes it: a form feed before the section of each file, the name of the
   file and the size of the section, then a line for each tag: the text of the line up to the
   tag, DEL, the name of the tag, SOH, the line and the offset in the file */
static const char tags[] = "a line before any file\n"
                           "\f\n"
                           "src/a.c,120\n"
                           "int foo (void)\x7f"
                           "foo\x01"
                           "12,345\n"
                           "static int foo_bar;\x7f"
                           "foo_bar\x01"
                           "20,400\n"
                           "#define BAR 1\x7f\x01"
                           "30,500\n"
                           "int other;\x7f"
                           "other\x01"
                           "31,520\n"
                           "\f\n"
                           "lib/b.c,60\n"
                           "void foo (int x)\x7f"
                           "foo\x01"
                           "7,99\n";

static char *dir;
static char *tagfile;

/* --------------------------------------------------------------------------------------------- */

static void
setup (void)
{
    dir = g_dir_make_tmp ("etags-XXXXXX", NULL);
    ck_assert_ptr_nonnull (dir);
    tagfile = g_build_filename (dir, "TAGS", (char *) NULL);
    ck_assert (g_file_set_contents (tagfile, tags, sizeof (tags) - 1, NULL));
}

/* --------------------------------------------------------------------------------------------- */

static void
teardown (void)
{
    (void) remove (tagfile);
    (void) rmdir (dir);
    g_free (tagfile);
    g_free (dir);
}

/* --------------------------------------------------------------------------------------------- */

static etags_hash_t *
entry (GPtrArray *found, guint i)
{
    ck_assert_uint_lt (i, found->len);
    return g_ptr_array_index (found, i);
}

/* --------------------------------------------------------------------------------------------- */

/* A line of a tag: from where the name was found, to the tag, its name and its line */
START_TEST (test_parse_define)
{
    char *long_name, *short_name;
    long line;

    ck_assert (parse_define ("foo (void)\x7f"
                             "foo\x01"
                             "12,345\n",
                             &long_name, &short_name, &line));
    ck_assert_str_eq (long_name, "foo (void)");
    ck_assert_str_eq (short_name, "foo");
    ck_assert_int_eq (line, 12);
    g_free (long_name);
    g_free (short_name);

    // no name of the tag: the text up to it stands for it
    ck_assert (parse_define ("BAR 1\x7f\x01"
                             "30,500\n",
                             &long_name, &short_name, &line));
    ck_assert_str_eq (long_name, "BAR 1");
    ck_assert_ptr_null (short_name);
    ck_assert_int_eq (line, 30);
    g_free (long_name);

    // no DEL either
    ck_assert (parse_define ("x\x01"
                             "4,5\n",
                             &long_name, &short_name, &line));
    ck_assert_str_eq (long_name, "x");
    ck_assert_ptr_null (short_name);
    ck_assert_int_eq (line, 4);
    g_free (long_name);

    // a line that is no tag
    ck_assert (!parse_define ("just text\n", &long_name, &short_name, &line));
    ck_assert_ptr_null (long_name);
    ck_assert_ptr_null (short_name);
    ck_assert_int_eq (line, 0);
    ck_assert (!parse_define ("", &long_name, &short_name, &line));
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* The tags of every file that have the name in them, the files by the directory of the tags */
START_TEST (test_definitions)
{
    GPtrArray *found;
    char *path;

    found = etags_set_definition_hash (tagfile, "/work", "foo");
    ck_assert_ptr_nonnull (found);
    // what has the name in it: foo_bar too, and the line before the first file is of none
    ck_assert_uint_eq (found->len, 3);
    ck_assert_str_eq (entry (found, 0)->short_define, "foo");
    ck_assert_str_eq (entry (found, 0)->filename, "src/a.c");
    ck_assert_str_eq (entry (found, 0)->fullpath, "/work/src/a.c");
    ck_assert_int_eq (entry (found, 0)->line, 12);
    ck_assert_str_eq (entry (found, 1)->short_define, "foo_bar");
    ck_assert_int_eq (entry (found, 1)->line, 20);
    ck_assert_str_eq (entry (found, 2)->filename, "lib/b.c");
    path = g_build_filename ("/work", "lib", "b.c", (char *) NULL);
    ck_assert_str_eq (entry (found, 2)->fullpath, path);
    g_free (path);
    ck_assert_int_eq (entry (found, 2)->line, 7);
    g_ptr_array_free (found, TRUE);

    // a tag with no name of its own: the text from the name found
    found = etags_set_definition_hash (tagfile, "/work", "BAR");
    ck_assert_uint_eq (found->len, 1);
    ck_assert_str_eq (entry (found, 0)->short_define, "BAR 1");
    ck_assert_int_eq (entry (found, 0)->line, 30);
    g_ptr_array_free (found, TRUE);

    // found only in the line number and the offset of a tag: no tag of that name
    ck_assert_ptr_null (etags_set_definition_hash (tagfile, "/work", "345"));
    ck_assert_ptr_null (etags_set_definition_hash (tagfile, "/work", "99"));
    ck_assert_ptr_null (etags_set_definition_hash (tagfile, "/work", "nothing_like_it"));
    ck_assert_ptr_null (etags_set_definition_hash (tagfile, "/work", NULL));
    ck_assert_ptr_null (etags_set_definition_hash (NULL, "/work", "foo"));
    path = g_build_filename (dir, "none", (char *) NULL);
    ck_assert_ptr_null (etags_set_definition_hash (path, "/work", "foo"));
    g_free (path);
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
    tcase_add_test (tc_core, test_parse_define);
    tcase_add_test (tc_core, test_definitions);
    // ***********************************

    return mctest_run_all (tc_core);
}

/* --------------------------------------------------------------------------------------------- */
