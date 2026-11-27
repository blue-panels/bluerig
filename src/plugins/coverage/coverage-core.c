/*
   The coverage of a project: what gcov and lcov files say of its lines.

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

/** \file coverage-core.c
 *  \brief Source: the coverage of a project: what gcov and lcov files say of its lines
 */

#include <config.h>

#include <stdlib.h>  // realpath ()
#include <string.h>

#include "coverage-core.h"

/*** global variables ****************************************************************************/

/*** file scope macro definitions ****************************************************************/

/*** file scope type declarations ****************************************************************/

struct coverage_reader_t
{
    char *root;      /* with a slash at the end */
    char *real_root; /* the same with its links resolved, NULL when it cannot be */
    char *gcov_dir;  /* where gcov ran */
    coverage_data_t data;
    GHashTable *files;
    GHashTable *current; /* the lines of the file being read; NULL: one outside the root */
};

/*** forward declarations (file scope functions) *************************************************/

/*** file scope variables ************************************************************************/

/* --------------------------------------------------------------------------------------------- */
/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/* The lines of the file @name, relative to @base, when it is in the root: NULL else */
static GHashTable *
coverage_reader_file (coverage_reader_t *reader, const char *name, const char *base)
{
    char *path, *real;
    gboolean inside;
    GHashTable *lines;

    path = g_canonicalize_filename (name, base);
    real = realpath (path, NULL);
    /* in the root as the project names it, or as its links resolve: a root reached through a
       link, a link in the tree to a file outside it */
    inside = g_str_has_prefix (path, reader->root)
        || (real != NULL
            && (g_str_has_prefix (real, reader->root)
                || (reader->real_root != NULL && g_str_has_prefix (real, reader->real_root))));
    // a link is the file it points to, which the editor has open
    if (real != NULL)
    {
        g_free (path);
        path = g_strdup (real);
        free (real);
    }
    if (!inside)
    {
        g_free (path);
        return NULL;
    }
    lines = g_hash_table_lookup (reader->files, path);
    if (lines == NULL)
    {
        lines = g_hash_table_new (g_direct_hash, g_direct_equal);
        g_hash_table_insert (reader->files, path, lines);
    }
    else
        g_free (path);
    return lines;
}

/* --------------------------------------------------------------------------------------------- */

/* A line that ran once in any of the programs that have it ran */
static void
coverage_reader_mark (coverage_reader_t *reader, long line, coverage_line_t state)
{
    gpointer key = GINT_TO_POINTER ((int) line);
    gpointer was;

    if (reader->current == NULL || line <= 0 || line > G_MAXINT)
        return;
    was = g_hash_table_lookup (reader->current, key);
    if (state == COVERAGE_LINE_COVERED || GPOINTER_TO_INT (was) != COVERAGE_LINE_COVERED)
        g_hash_table_insert (reader->current, key, GINT_TO_POINTER (state));
}

/* --------------------------------------------------------------------------------------------- */

/* A line of gcov -t: "count:number:text"; count "-" for no code, "#####" or "=====" for code
   that did not run, a number (with "*" after it, a block of it did not run) for code that did;
   the lines numbered 0 tell of the file: "Source:name" */
static void
coverage_reader_gcov (coverage_reader_t *reader, const char *line)
{
    const char *colon, *text;
    char *count, *end;
    long number;

    colon = strchr (line, ':');
    if (colon == NULL)
        return;
    number = strtol (colon + 1, &end, 10);
    if (end == colon + 1 || *end != ':')
        return;
    text = end + 1;
    count = g_strstrip (g_strndup (line, (gsize) (colon - line)));
    if (number == 0)
    {
        if (strcmp (count, "-") == 0 && g_str_has_prefix (text, "Source:"))
            reader->current =
                coverage_reader_file (reader, text + strlen ("Source:"), reader->gcov_dir);
    }
    else if (count[0] == '#' || count[0] == '=')
        coverage_reader_mark (reader, number, COVERAGE_LINE_MISSED);
    else if (g_ascii_isdigit (count[0]))
        coverage_reader_mark (reader, number,
                              strtoul (count, NULL, 10) > 0 ? COVERAGE_LINE_COVERED
                                                            : COVERAGE_LINE_MISSED);
    g_free (count);
}

/* --------------------------------------------------------------------------------------------- */

/* A line of an lcov tracefile: "SF:name" starts a file, "DA:line,count[,checksum]" is a line of
   it, "end_of_record" ends it */
static void
coverage_reader_lcov (coverage_reader_t *reader, const char *line)
{
    if (g_str_has_prefix (line, "SF:"))
        reader->current = coverage_reader_file (reader, line + 3, reader->root);
    else if (g_str_has_prefix (line, "DA:"))
    {
        char *end;
        const long number = strtol (line + 3, &end, 10);

        if (*end == ',')
            coverage_reader_mark (reader, number,
                                  strtod (end + 1, NULL) > 0 ? COVERAGE_LINE_COVERED
                                                             : COVERAGE_LINE_MISSED);
    }
    else if (strcmp (line, "end_of_record") == 0)
        reader->current = NULL;
}

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

coverage_reader_t *
coverage_reader_new (const char *root, const char *gcov_dir)
{
    coverage_reader_t *reader = g_new0 (coverage_reader_t, 1);

    reader->root = g_str_has_suffix (root, G_DIR_SEPARATOR_S)
        ? g_strdup (root)
        : g_strconcat (root, G_DIR_SEPARATOR_S, (char *) NULL);
    {
        char *real = realpath (root, NULL);

        if (real != NULL)
        {
            reader->real_root = g_str_has_suffix (real, G_DIR_SEPARATOR_S)
                ? g_strdup (real)
                : g_strconcat (real, G_DIR_SEPARATOR_S, (char *) NULL);
            free (real);
        }
    }
    reader->gcov_dir = g_strdup (gcov_dir != NULL ? gcov_dir : root);
    reader->files = g_hash_table_new_full (g_str_hash, g_str_equal, g_free,
                                           (GDestroyNotify) g_hash_table_unref);
    return reader;
}

/* --------------------------------------------------------------------------------------------- */

void
coverage_reader_line (coverage_reader_t *reader, const char *line)
{
    if (reader == NULL || line == NULL)
        return;
    switch (reader->data)
    {
    case COVERAGE_DATA_NONE:
        // the head: what the script found
        if (strcmp (line, COVERAGE_HEAD_GCOV) == 0)
            reader->data = COVERAGE_DATA_GCOV;
        else if (strcmp (line, COVERAGE_HEAD_LCOV) == 0)
            reader->data = COVERAGE_DATA_LCOV;
        else if (strcmp (line, COVERAGE_HEAD_UNCHANGED) == 0)
            reader->data = COVERAGE_DATA_UNCHANGED;
        break;
    case COVERAGE_DATA_GCOV:
        coverage_reader_gcov (reader, line);
        break;
    case COVERAGE_DATA_LCOV:
        coverage_reader_lcov (reader, line);
        break;
    default:
        break;
    }
}

/* --------------------------------------------------------------------------------------------- */

coverage_data_t
coverage_reader_finish (coverage_reader_t *reader, GHashTable **files)
{
    coverage_data_t data;

    *files = NULL;
    if (reader == NULL)
        return COVERAGE_DATA_NONE;
    data = reader->data;
    if (data == COVERAGE_DATA_GCOV || data == COVERAGE_DATA_LCOV)
        *files = reader->files;
    else
        g_hash_table_unref (reader->files);
    g_free (reader->root);
    g_free (reader->real_root);
    g_free (reader->gcov_dir);
    g_free (reader);
    return data;
}

/* --------------------------------------------------------------------------------------------- */

GHashTable *
coverage_file_lines (GHashTable *files, const char *path)
{
    GHashTable *lines;
    char *real;

    if (files == NULL || path == NULL)
        return NULL;
    lines = g_hash_table_lookup (files, path);
    if (lines != NULL)
        return lines;
    real = realpath (path, NULL);
    if (real != NULL)
        lines = g_hash_table_lookup (files, real);
    free (real);
    return lines;
}

/* --------------------------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------------------------- */

void
coverage_lines_count (GHashTable *lines, guint *covered, guint *code)
{
    GHashTableIter iter;
    gpointer value;

    *covered = 0;
    *code = 0;
    if (lines == NULL)
        return;
    g_hash_table_iter_init (&iter, lines);
    while (g_hash_table_iter_next (&iter, NULL, &value))
    {
        const coverage_line_t line = (coverage_line_t) GPOINTER_TO_INT (value);

        if (line == COVERAGE_LINE_COVERED)
            (*covered)++;
        if (line != COVERAGE_LINE_NONE)
            (*code)++;
    }
}

/* --------------------------------------------------------------------------------------------- */
