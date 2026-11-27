/** \file coverage-core.h
 *  \brief Header: the coverage of a project: what gcov and lcov files say of its lines
 */

#ifndef MC__COVERAGE_CORE_H
#define MC__COVERAGE_CORE_H

#include "lib/global.h"

/*** typedefs(not structures) and defined constants **********************************************/

/* The first line of the output of the reading script: what follows it */
#define COVERAGE_HEAD_GCOV      "#coverage-gcov"
#define COVERAGE_HEAD_LCOV      "#coverage-lcov"
#define COVERAGE_HEAD_UNCHANGED "#coverage-unchanged"
#define COVERAGE_HEAD_NONE      "#coverage-none"

/*** enums ***************************************************************************************/

/* What is known of a line */
typedef enum
{
    COVERAGE_LINE_NONE = 0, /* no code, or not in the data */
    COVERAGE_LINE_COVERED,  /* ran at least once */
    COVERAGE_LINE_MISSED    /* code that did not run */
} coverage_line_t;

/* What the output said of itself */
typedef enum
{
    COVERAGE_DATA_NONE = 0, /* no data of coverage in the project */
    COVERAGE_DATA_UNCHANGED,
    COVERAGE_DATA_GCOV,
    COVERAGE_DATA_LCOV
} coverage_data_t;

/*** structures declarations (and typedefs of structures)*****************************************/

/* Reads the output line by line, a file after a file */
typedef struct coverage_reader_t coverage_reader_t;

/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/

/* A reader of the files of @root: a relative name of gcov is from @gcov_dir, where gcov ran, one
   of lcov from @root; the files outside @root are left out */
coverage_reader_t *coverage_reader_new (const char *root, const char *gcov_dir);
/* One line of the output, without its newline */
void coverage_reader_line (coverage_reader_t *reader, const char *line);
/* What the output said of itself, the reader freed; @files gets the files read when it was data:
   absolute path -> GHashTable of GINT_TO_POINTER (line) -> GINT_TO_POINTER (coverage_line_t),
   free with g_hash_table_unref(); else NULL */
coverage_data_t coverage_reader_finish (coverage_reader_t *reader, GHashTable **files);

/* The lines of a file of @files, NULL when it is not there */
GHashTable *coverage_file_lines (GHashTable *files, const char *path);
/* The shell script that finds the data and prints it, the head first: $1 the build directory,
   $2 the root, $3 the stamp of the last reading, $4 1 to read it whatever the stamp says; $3.new
   stamps this reading, made before it reads, to be the stamp once its output is all read; $3.out
   holds the output while it is made */
const char *coverage_reading_script (void);
/* Whether a build that ended, of the step @step ("build", "configure"), the project @root and the
   profile @profile, makes the data of the project @front, that of the profile @coverage_profile,
   other: a build of it, not a setup; NULL for what is not known, which does not say no */
gboolean coverage_build_concerns (const char *step, const char *root, const char *front,
                                  const char *profile, const char *coverage_profile);
/* Of the lines of a file, @lines of coverage_file_lines(): those that ran, and those of code */
void coverage_lines_count (GHashTable *lines, guint *covered, guint *code);

#endif /* MC__COVERAGE_CORE_H */
