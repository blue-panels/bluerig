#include <config.h>

/* -Dassert=false sets G_DISABLE_ASSERT in config.h: g_test_init () would then end the test and
   g_assert () check nothing, and the checks are the point of a test */
#undef G_DISABLE_ASSERT

#include <stdlib.h>  // realpath()
#include <string.h>
#include <unistd.h>  // symlink()

#include <glib.h>
#include <glib/gstdio.h>

#include "src/plugins/coverage/coverage-core.h"

static char *top = NULL;

/* --------------------------------------------------------------------------------------------- */

static void
put (const char *rel)
{
    char *path = g_build_filename (top, rel, (char *) NULL);
    char *dir = g_path_get_dirname (path);

    g_assert_cmpint (g_mkdir_with_parents (dir, 0700), ==, 0);
    g_assert_true (g_file_set_contents (path, "", -1, NULL));
    g_free (dir);
    g_free (path);
}

/* The output @text read as the reader of the root of the test reads it */
static coverage_data_t
read_text (const char *text, const char *gcov_dir, GHashTable **files)
{
    coverage_reader_t *reader = coverage_reader_new (top, gcov_dir);
    char **lines = g_strsplit (text, "\n", -1);
    char **l;

    for (l = lines; *l != NULL; l++)
        coverage_reader_line (reader, *l);
    g_strfreev (lines);
    return coverage_reader_finish (reader, files);
}

static coverage_line_t
state_of (GHashTable *files, const char *rel, int line)
{
    char *path = g_build_filename (top, rel, (char *) NULL);
    GHashTable *lines = coverage_file_lines (files, path);
    gpointer state;

    g_free (path);
    if (lines == NULL)
        return COVERAGE_LINE_NONE;
    state = g_hash_table_lookup (lines, GINT_TO_POINTER (line));
    return (coverage_line_t) GPOINTER_TO_INT (state);
}

/* --------------------------------------------------------------------------------------------- */

/* gcov -t: the names from the build directory, the lines that ran, those that did not, those
   without code; a header of two programs ran when one of them ran it */
static void
test_gcov (void)
{
    char *build = g_build_filename (top, "build", (char *) NULL);
    GHashTable *files = NULL;
    const char *text = COVERAGE_HEAD_GCOV "\n"
                                          "        -:    0:Source:../src/a.c\n"
                                          "        -:    0:Graph:./a.gcno\n"
                                          "        -:    1:#include <stdio.h>\n"
                                          "        1:    2:int f(int x)\n"
                                          "    #####:    6:    return 2;\n"
                                          "       3*:    7:    if (x) g ();\n"
                                          "    =====:    8:    throw;\n"
                                          "        -:    0:Source:../src/a.h\n"
                                          "    #####:    3:static int h;\n"
                                          "        -:    0:Source:/usr/include/stdio.h\n"
                                          "        1:   10:extern int printf;\n"
                                          "        -:    0:Source:../src/a.h\n"
                                          "        5:    3:static int h;\n"
                                          "File '../src/a.c'\n"
                                          "Lines executed:50.00% of 4\n";

    put ("src/a.c");
    put ("src/a.h");
    g_assert_cmpint (g_mkdir_with_parents (build, 0700), ==, 0);
    g_assert_cmpint (read_text (text, build, &files), ==, COVERAGE_DATA_GCOV);
    g_assert_nonnull (files);
    g_assert_cmpint (g_hash_table_size (files), ==, 2);
    g_assert_cmpint (state_of (files, "src/a.c", 1), ==, COVERAGE_LINE_NONE);
    g_assert_cmpint (state_of (files, "src/a.c", 2), ==, COVERAGE_LINE_COVERED);
    g_assert_cmpint (state_of (files, "src/a.c", 6), ==, COVERAGE_LINE_MISSED);
    g_assert_cmpint (state_of (files, "src/a.c", 7), ==, COVERAGE_LINE_COVERED);
    g_assert_cmpint (state_of (files, "src/a.c", 8), ==, COVERAGE_LINE_MISSED);
    g_assert_cmpint (state_of (files, "src/a.h", 3), ==, COVERAGE_LINE_COVERED);
    {
        // of the four lines of code of a.c two ran; a file not there has none
        char *path = g_build_filename (top, "src", "a.c", (char *) NULL);
        guint covered, code;

        coverage_lines_count (coverage_file_lines (files, path), &covered, &code);
        g_assert_cmpuint (covered, ==, 2);
        g_assert_cmpuint (code, ==, 4);
        coverage_lines_count (NULL, &covered, &code);
        g_assert_cmpuint (code, ==, 0);
        g_free (path);
    }
    g_hash_table_unref (files);
    g_free (build);
}

/* --------------------------------------------------------------------------------------------- */

/* lcov: the names from the root or absolute, a count of 0 a line that did not run */
static void
test_lcov (void)
{
    GHashTable *files = NULL;
    char *abs = g_build_filename (top, "pkg", "m.py", (char *) NULL);
    char *text;

    put ("pkg/m.py");
    put ("lib/x.js");
    text = g_strconcat (COVERAGE_HEAD_LCOV "\n"
                                           "TN:\n"
                                           "SF:",
                        abs,
                        "\n"
                        "DA:1,1\n"
                        "DA:2,0\n"
                        "DA:3,4,abcdef\n"
                        "end_of_record\n"
                        "DA:9,1\n"
                        "SF:lib/x.js\n"
                        "DA:5,0\n"
                        "end_of_record\n"
                        "SF:/elsewhere/y.c\n"
                        "DA:1,1\n"
                        "end_of_record\n",
                        (char *) NULL);
    g_assert_cmpint (read_text (text, NULL, &files), ==, COVERAGE_DATA_LCOV);
    g_assert_cmpint (g_hash_table_size (files), ==, 2);
    g_assert_cmpint (state_of (files, "pkg/m.py", 1), ==, COVERAGE_LINE_COVERED);
    g_assert_cmpint (state_of (files, "pkg/m.py", 2), ==, COVERAGE_LINE_MISSED);
    g_assert_cmpint (state_of (files, "pkg/m.py", 3), ==, COVERAGE_LINE_COVERED);
    // after end_of_record a line is of no file
    g_assert_cmpint (state_of (files, "pkg/m.py", 9), ==, COVERAGE_LINE_NONE);
    g_assert_cmpint (state_of (files, "lib/x.js", 5), ==, COVERAGE_LINE_MISSED);
    g_hash_table_unref (files);
    g_free (text);
    g_free (abs);
}

/* --------------------------------------------------------------------------------------------- */

/* The heads with no data: no files, and what was said */
static void
test_heads (void)
{
    GHashTable *files = NULL;

    g_assert_cmpint (read_text (COVERAGE_HEAD_UNCHANGED "\n", NULL, &files), ==,
                     COVERAGE_DATA_UNCHANGED);
    g_assert_null (files);
    g_assert_cmpint (read_text (COVERAGE_HEAD_NONE "\n", NULL, &files), ==, COVERAGE_DATA_NONE);
    g_assert_null (files);
    g_assert_cmpint (read_text ("", NULL, &files), ==, COVERAGE_DATA_NONE);
    g_assert_null (files);
    // the lines of gcov with no head before them are no data
    g_assert_cmpint (read_text ("        1:    2:int f(int x)\n", NULL, &files), ==,
                     COVERAGE_DATA_NONE);
    g_assert_null (files);
}

/* --------------------------------------------------------------------------------------------- */

/* A root the project names through a link: its files are in it all the same, and so is a link in
   it to a file outside it; the names are the real ones */
static void
test_link_root (void)
{
    char *real = g_build_filename (top, "real", (char *) NULL);
    char *via = g_build_filename (top, "via", (char *) NULL);
    char *outside = g_build_filename (top, "outside.c", (char *) NULL);
    char *in_link = g_build_filename (real, "src", "out.c", (char *) NULL);
    char *text, *lookup;
    coverage_reader_t *reader;
    GHashTable *files = NULL;

    put ("real/src/b.c");
    put ("outside.c");
    g_assert_cmpint (symlink (real, via), ==, 0);
    g_assert_cmpint (symlink (outside, in_link), ==, 0);
    text = g_strdup_printf ("%s\n"
                            "        -:    0:Source:%s/src/b.c\n"
                            "        1:    4:int b;\n"
                            "        -:    0:Source:src/out.c\n"
                            "    #####:    2:int o;\n",
                            COVERAGE_HEAD_GCOV, via);

    reader = coverage_reader_new (via, via);
    {
        char **lines = g_strsplit (text, "\n", -1);
        char **l;

        for (l = lines; *l != NULL; l++)
            coverage_reader_line (reader, *l);
        g_strfreev (lines);
    }
    g_assert_cmpint (coverage_reader_finish (reader, &files), ==, COVERAGE_DATA_GCOV);
    g_assert_cmpint (g_hash_table_size (files), ==, 2);
    lookup = g_build_filename (via, "src", "b.c", (char *) NULL);
    g_assert_nonnull (coverage_file_lines (files, lookup));
    g_free (lookup);
    g_assert_nonnull (g_hash_table_lookup (files, outside));

    g_hash_table_unref (files);
    g_free (text);
    g_free (in_link);
    g_free (outside);
    g_free (via);
    g_free (real);
}

/* --------------------------------------------------------------------------------------------- */

static void
remove_tree (const char *dir)
{
    GDir *d = g_dir_open (dir, 0, NULL);
    const char *name;

    while (d != NULL && (name = g_dir_read_name (d)) != NULL)
    {
        char *path = g_build_filename (dir, name, (char *) NULL);

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

/* The build that ended: of the project in front and of the profile of its data, not a setup */
static void
test_build_concerns (void)
{
    g_assert_true (coverage_build_concerns ("build", "/p", "/p", "Coverage", "Coverage"));
    // what is not told does not say no
    g_assert_true (coverage_build_concerns (NULL, NULL, "/p", NULL, NULL));
    g_assert_true (coverage_build_concerns ("build", "/p", "/p", "Coverage", NULL));
    // a setup, another project, another profile
    g_assert_false (coverage_build_concerns ("configure", "/p", "/p", "Coverage", "Coverage"));
    g_assert_false (coverage_build_concerns ("build", "/q", "/p", "Coverage", "Coverage"));
    g_assert_false (coverage_build_concerns ("build", "/p", NULL, "Coverage", "Coverage"));
    g_assert_false (coverage_build_concerns ("build", "/p", "/p", "Default", "Coverage"));
}

/* --------------------------------------------------------------------------------------------- */

/* A gcov of the test in @bin that runs @body: prints a line of a source, fails, or does both */
static void
fake_gcov (const char *bin, const char *body)
{
    char *path = g_build_filename (bin, "gcov", (char *) NULL);
    char *text = g_strconcat ("#!/bin/sh\n", body, "\n", (char *) NULL);

    g_assert_cmpint (g_mkdir_with_parents (bin, 0700), ==, 0);
    g_assert_true (g_file_set_contents (path, text, -1, NULL));
    g_assert_cmpint (g_chmod (path, 0700), ==, 0);
    g_free (text);
    g_free (path);
}

/* The first line the reading script prints for the build directory @dir and the root @root, the
   stamp @stamp, read whatever it says when @forced; its gcov that of @bin */
static char *
script_head (const char *dir, const char *root, const char *stamp, gboolean forced, const char *bin)
{
    const char *argv[] = { "/bin/sh", "-c",  coverage_reading_script (), "coverage", dir,
                           root,      stamp, forced ? "1" : "0",         NULL };
    char *path = g_strconcat (bin, ":", g_getenv ("PATH"), (char *) NULL);
    char **env = g_environ_setenv (g_get_environ (), "PATH", path, TRUE);
    char *out = NULL, *eol;

    g_assert_true (g_spawn_sync (root, (char **) argv, env, G_SPAWN_STDERR_TO_DEV_NULL, NULL, NULL,
                                 &out, NULL, NULL, NULL));
    eol = strchr (out, '\n');
    if (eol != NULL)
        *eol = '\0';
    g_strfreev (env);
    g_free (path);
    return out;
}

/* The head of the script is @head; the stamp of the reading made the stamp, as when it is read */
static void
script_is (const char *dir, const char *root, const char *stamp, gboolean forced, const char *bin,
           const char *head)
{
    char *got = script_head (dir, root, stamp, forced, bin);
    char *fresh = g_strconcat (stamp, ".new", (char *) NULL);

    g_assert_cmpstr (got, ==, head);
    if (g_file_test (fresh, G_FILE_TEST_EXISTS))
        g_assert_cmpint (rename (fresh, stamp), ==, 0);
    g_free (fresh);
    g_free (got);
}

/* The script: gcov data in the build directory, else an lcov tracefile in the root, the build
   directory there or not; nothing new since the stamp; a set of data files other than the one
   read; a gcov that prints nothing; nothing at all */
static void
test_script (void)
{
    char *root = g_build_filename (top, "s", (char *) NULL);
    char *build = g_build_filename (root, "build", (char *) NULL);
    char *stamp = g_build_filename (top, "stamp", (char *) NULL);
    char *gcov = g_build_filename (top, "bin", (char *) NULL);
    char *silent = g_build_filename (top, "bin-silent", (char *) NULL);
    char *partial = g_build_filename (top, "bin-partial", (char *) NULL);
    char *gone = g_build_filename (build, "b.p", "b.c.gcda", (char *) NULL);
    char *info = g_build_filename (top, "real.info", (char *) NULL);
    char *link = g_build_filename (root, "lcov.info", (char *) NULL);

    fake_gcov (gcov, "echo '        -:    0:Source:a.c'");
    fake_gcov (silent, "exit 1");
    fake_gcov (partial, "echo '        -:    0:Source:a.c'; exit 1");
    put ("s/coverage.info");
    // the build directory not there yet: the tracefile of the root
    script_is (build, root, stamp, TRUE, gcov, COVERAGE_HEAD_LCOV);
    put ("s/build/a.p/a.c.gcda");
    put ("s/build/b.p/b.c.gcda");
    script_is (build, root, stamp, TRUE, gcov, COVERAGE_HEAD_GCOV);
    // read, nothing new since
    script_is (build, root, stamp, FALSE, gcov, COVERAGE_HEAD_UNCHANGED);
    // a data file gone: the set is another, read again
    g_assert_cmpint (g_remove (gone), ==, 0);
    script_is (build, root, stamp, FALSE, gcov, COVERAGE_HEAD_GCOV);
    script_is (build, root, stamp, FALSE, gcov, COVERAGE_HEAD_UNCHANGED);
    // gcov prints nothing, or a part and fails: no data of it, the tracefile of the root
    script_is (build, root, stamp, TRUE, silent, COVERAGE_HEAD_LCOV);
    script_is (build, root, stamp, TRUE, partial, COVERAGE_HEAD_LCOV);
    remove_tree (root);
    /* a tracefile through a link, the link older than the stamp: the time of what it leads to,
       written again after the reading */
    g_assert_cmpint (g_mkdir_with_parents (root, 0700), ==, 0);
    g_assert_true (g_file_set_contents (info, "SF:a.c\n", -1, NULL));
    g_assert_cmpint (symlink (info, link), ==, 0);
    script_is (build, root, stamp, TRUE, gcov, COVERAGE_HEAD_LCOV);
    script_is (build, root, stamp, FALSE, gcov, COVERAGE_HEAD_UNCHANGED);
    g_usleep (1100 * 1000);
    g_assert_true (g_file_set_contents (info, "SF:b.c\n", -1, NULL));
    script_is (build, root, stamp, FALSE, gcov, COVERAGE_HEAD_LCOV);
    remove_tree (root);
    g_assert_cmpint (g_mkdir_with_parents (root, 0700), ==, 0);
    script_is (build, root, stamp, TRUE, gcov, COVERAGE_HEAD_NONE);
    g_free (link);
    g_free (info);
    g_free (gone);
    g_free (partial);
    g_free (silent);
    g_free (gcov);
    g_free (stamp);
    g_free (build);
    g_free (root);
}

/* --------------------------------------------------------------------------------------------- */

int
main (int argc, char **argv)
{
    char *tmp;
    int ret;

    g_test_init (&argc, &argv, NULL);
    tmp = g_dir_make_tmp ("coverage-core-XXXXXX", NULL);
    g_assert_nonnull (tmp);
    // the reader gives the real names, those of a link resolved: /tmp may be one
    top = realpath (tmp, NULL);
    g_assert_nonnull (top);

    g_test_add_func ("/coverage/gcov", test_gcov);
    g_test_add_func ("/coverage/lcov", test_lcov);
    g_test_add_func ("/coverage/heads", test_heads);
    g_test_add_func ("/coverage/link-root", test_link_root);
    g_test_add_func ("/coverage/build-concerns", test_build_concerns);
    g_test_add_func ("/coverage/script", test_script);
    ret = g_test_run ();

    remove_tree (top);
    free (top);
    g_free (tmp);
    return ret;
}
