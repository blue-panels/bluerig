/*
   src/plugins/ctags - tests for the tags files of the repositories

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

#define TEST_SUITE_NAME "/src/plugins/ctags"

#include "tests/mctest.h"

#include <glib/gstdio.h>

#include "src/plugins/ctags/ctags-parser.h"
#include "src/plugins/ctags/ctags-repository.h"

/* a directory of the test, made anew for each test */
static char *top;

/* --------------------------------------------------------------------------------------------- */

static void
remove_tree (const char *dir)
{
    GDir *d;
    const char *name;

    d = g_dir_open (dir, 0, NULL);
    while (d != NULL && (name = g_dir_read_name (d)) != NULL)
    {
        char *path;

        path = g_build_filename (dir, name, (char *) NULL);
        if (g_file_test (path, G_FILE_TEST_IS_DIR) && !g_file_test (path, G_FILE_TEST_IS_SYMLINK))
            remove_tree (path);
        else
            (void) g_unlink (path);
        g_free (path);
    }
    if (d != NULL)
        g_dir_close (d);
    (void) g_rmdir (dir);
}

/* --------------------------------------------------------------------------------------------- */

/* Write @text to the file @rel under the directory of the test, its directories made; the full
   path, to be freed */
static char *
put_file (const char *rel, const char *text)
{
    char *path, *dir;

    path = g_build_filename (top, rel, (char *) NULL);
    dir = g_path_get_dirname (path);
    ck_assert_int_eq (g_mkdir_with_parents (dir, 0700), 0);
    g_free (dir);
    ck_assert (g_file_set_contents (path, text, -1, NULL));
    return path;
}

/* --------------------------------------------------------------------------------------------- */

static gboolean
has_entry (const GPtrArray *found, const char *name, const char *file)
{
    for (guint i = 0; found != NULL && i < found->len; i++)
    {
        const ctags_entry_t *e = g_ptr_array_index (found, i);

        if (strcmp (e->name, name) == 0 && strcmp (e->file, file) == 0)
            return TRUE;
    }
    return FALSE;
}

/* --------------------------------------------------------------------------------------------- */

/* @Before */
static void
setup (void)
{
    top = g_dir_make_tmp ("ctags_repo_XXXXXX", NULL);
    ck_assert_ptr_nonnull (top);
}

/* --------------------------------------------------------------------------------------------- */

/* @After */
static void
teardown (void)
{
    remove_tree (top);
    g_free (top);
}

/* --------------------------------------------------------------------------------------------- */

/* The line of an entry: its own number first; else the line its search pattern finds in the
   file; else 1, never 0 */
START_TEST (test_resolve_line)
{
    char *src;
    ctags_entry_t e = { 0 };

    src = put_file ("main.c",
                    "#include <stdio.h>\n"
                    "\n"
                    "int main (void)\n"
                    "{\n"
                    "    return 0;\n"
                    "}\n");

    ck_assert_int_eq (ctags_entry_resolve_line (NULL, src), 1);

    e.line = 7;
    ck_assert_int_eq (ctags_entry_resolve_line (&e, src), 7);

    e.line = 0;
    e.excmd = (char *) "/^int main (void)$/";
    ck_assert_int_eq (ctags_entry_resolve_line (&e, src), 3);

    // without ^ and $ the pattern is the start of the line all the same
    e.excmd = (char *) "/    return 0;/";
    ck_assert_int_eq (ctags_entry_resolve_line (&e, src), 5);

    // not found, an empty pattern, not a search, no file: line 1
    e.excmd = (char *) "/^int nothing (void)$/";
    ck_assert_int_eq (ctags_entry_resolve_line (&e, src), 1);
    e.excmd = (char *) "/^$/";
    ck_assert_int_eq (ctags_entry_resolve_line (&e, src), 1);
    e.excmd = (char *) "?^int main?";
    ck_assert_int_eq (ctags_entry_resolve_line (&e, src), 1);
    e.excmd = (char *) "/^int main (void)$/";
    ck_assert_int_eq (ctags_entry_resolve_line (&e, NULL), 1);
    ck_assert_int_eq (ctags_entry_resolve_line (&e, "/nonexistent/main.c"), 1);

    g_free (src);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* The tags file is looked for from the file up: the nearest wins */
START_TEST (test_discover)
{
    char *src, *outer, *inner, *found, *dir;

    src = put_file ("a/b/c/file.c", "");
    outer = put_file ("a/tags", "");

    found = ctags_repo_discover (src);
    mctest_assert_str_eq (found, outer);
    g_free (found);

    // from a directory as well as from a file
    dir = g_build_filename (top, "a", "b", (char *) NULL);
    found = ctags_repo_discover (dir);
    mctest_assert_str_eq (found, outer);
    g_free (found);

    inner = put_file ("a/b/tags", "");
    found = ctags_repo_discover (src);
    mctest_assert_str_eq (found, inner);
    g_free (found);

    ck_assert_ptr_null (ctags_repo_discover (NULL));

    g_free (dir);
    g_free (inner);
    g_free (outer);
    g_free (src);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* No tags file up to the root: nothing of the test is found (a tags file above the directory of
   the test, in /tmp, is not ours to judge) */
START_TEST (test_discover_none)
{
    char *src, *found;

    src = put_file ("x/y/file.c", "");
    found = ctags_repo_discover (src);
    if (found != NULL)
        ck_assert_msg (!g_str_has_prefix (found, top), "found %s", found);
    g_free (found);
    g_free (src);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A file is under a root only at a boundary of the path */
START_TEST (test_path_is_under)
{
    ck_assert (ctags_path_is_under ("/tmp/proj/a.c", "/tmp/proj"));
    ck_assert (ctags_path_is_under ("/tmp/proj/a.c", "/tmp/proj/"));
    ck_assert (ctags_path_is_under ("/tmp/proj", "/tmp/proj"));
    ck_assert (!ctags_path_is_under ("/tmp/proj2/a.c", "/tmp/proj"));
    ck_assert (!ctags_path_is_under ("/tmp/pro", "/tmp/proj"));
    ck_assert (!ctags_path_is_under ("/var/a.c", "/tmp/proj"));
    ck_assert (ctags_path_is_under ("/anything", ""));
    ck_assert (!ctags_path_is_under (NULL, "/tmp"));
    ck_assert (!ctags_path_is_under ("/tmp/a", NULL));
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* A tags file loads with its directory for the root, or with a root given; a file without
   entries, or none at all, loads nothing */
START_TEST (test_load)
{
    char *tags, *empty, *root;
    ctags_repo_t *repo;

    tags = put_file ("p/tags",
                     "!_TAG_FILE_FORMAT\t2\t/extended format/\n"
                     "main\tmain.c\t/^int main (void)$/;\"\tf\n"
                     "helper\tutil.c\t42;\"\tf\n");

    repo = ctags_repo_load (tags);
    ck_assert_ptr_nonnull (repo);
    root = g_path_get_dirname (tags);
    mctest_assert_str_eq (repo->root_dir, root);
    mctest_assert_str_eq (repo->tags_path, tags);
    ck_assert_uint_eq (repo->entries->len, 2);
    ck_assert_ptr_nonnull (repo->index);
    ctags_repo_free (repo);

    repo = ctags_repo_load_root (tags, top);
    ck_assert_ptr_nonnull (repo);
    mctest_assert_str_eq (repo->root_dir, top);
    ctags_repo_free (repo);

    empty = put_file ("q/tags", "!_TAG_FILE_FORMAT\t2\t/extended format/\n");
    ck_assert_ptr_null (ctags_repo_load (empty));
    ck_assert_ptr_null (ctags_repo_load ("/nonexistent/tags"));
    ck_assert_ptr_null (ctags_repo_load (NULL));
    ck_assert_ptr_null (ctags_repo_load_root (tags, NULL));

    ctags_repo_free (NULL);

    g_free (root);
    g_free (empty);
    g_free (tags);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

/* Two repositories: a file finds its own, a name is looked for in both */
START_TEST (test_find_in_repos)
{
    char *tags1, *tags2, *file;
    ctags_repo_t *r1, *r2;
    GSList *repos = NULL;
    GPtrArray *found;

    tags1 = put_file ("one/tags",
                      "main\tmain.c\t1;\"\tf\n"
                      "helper\tutil.c\t2;\"\tf\n"
                      "Widget\twidget.h\t3;\"\ts\n");
    tags2 = put_file ("two/tags",
                      "helper\tlib/help.c\t4;\"\tf\n"
                      "helpful\tlib/help.c\t5;\"\tf\n");
    r1 = ctags_repo_load (tags1);
    r2 = ctags_repo_load (tags2);
    ck_assert_ptr_nonnull (r1);
    ck_assert_ptr_nonnull (r2);
    repos = g_slist_append (repos, r1);
    repos = g_slist_append (repos, r2);

    file = g_build_filename (top, "two", "lib", "help.c", (char *) NULL);
    ck_assert_ptr_eq (ctags_repos_find_for_file (repos, file), r2);
    g_free (file);
    ck_assert_ptr_null (ctags_repos_find_for_file (repos, "/elsewhere/a.c"));

    // an exact name from both
    found = ctags_repos_find_exact (repos, "helper");
    ck_assert_ptr_nonnull (found);
    ck_assert_uint_eq (found->len, 2);
    ck_assert (has_entry (found, "helper", "util.c"));
    ck_assert (has_entry (found, "helper", "lib/help.c"));
    g_ptr_array_free (found, FALSE);

    // no name in that case: the name in any case
    found = ctags_repos_find_exact (repos, "widget");
    ck_assert_ptr_nonnull (found);
    ck_assert_uint_eq (found->len, 1);
    ck_assert (has_entry (found, "Widget", "widget.h"));
    g_ptr_array_free (found, FALSE);

    ck_assert_ptr_null (ctags_repos_find_exact (repos, "nothing"));
    ck_assert_ptr_null (ctags_repos_find_exact (repos, NULL));

    // a prefix from both
    found = ctags_repos_find_prefix (repos, "help", FALSE);
    ck_assert_ptr_nonnull (found);
    ck_assert_uint_eq (found->len, 3);
    ck_assert (has_entry (found, "helpful", "lib/help.c"));
    g_ptr_array_free (found, FALSE);

    found = ctags_repos_find_prefix (repos, "WID", TRUE);
    ck_assert_ptr_nonnull (found);
    ck_assert (has_entry (found, "Widget", "widget.h"));
    g_ptr_array_free (found, FALSE);
    ck_assert_ptr_null (ctags_repos_find_prefix (repos, "WID", FALSE));
    ck_assert_ptr_null (ctags_repos_find_prefix (repos, "", FALSE));

    // the entries of a file by its name
    found = ctags_repos_find_basename (repos, "help.c");
    ck_assert_ptr_nonnull (found);
    ck_assert_uint_eq (found->len, 2);
    ck_assert (has_entry (found, "helpful", "lib/help.c"));
    g_ptr_array_free (found, FALSE);
    ck_assert_ptr_null (ctags_repos_find_basename (repos, "none.c"));
    ck_assert_ptr_null (ctags_repos_find_basename (repos, ""));

    g_slist_free_full (repos, (GDestroyNotify) ctags_repo_free);
    g_free (tags2);
    g_free (tags1);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

int
main (void)
{
    TCase *tc_core;

    tc_core = tcase_create ("Core");

    tcase_add_checked_fixture (tc_core, setup, teardown);

    tcase_add_test (tc_core, test_resolve_line);
    tcase_add_test (tc_core, test_discover);
    tcase_add_test (tc_core, test_discover_none);
    tcase_add_test (tc_core, test_path_is_under);
    tcase_add_test (tc_core, test_load);
    tcase_add_test (tc_core, test_find_in_repos);

    return mctest_run_all (tc_core);
}

/* --------------------------------------------------------------------------------------------- */
