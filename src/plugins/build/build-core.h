/** \file build-core.h
 *  \brief Header: how a project is built, what it builds, and what the compiler says
 */

#ifndef MC__BUILD_CORE_H
#define MC__BUILD_CORE_H

#include "lib/global.h"

/*** typedefs(not structures) and defined constants **********************************************/

/*** enums ***************************************************************************************/

typedef enum
{
    BUILD_NONE,
    BUILD_MESON,
    BUILD_CMAKE,
    BUILD_MAKE
} build_system_t;

/* What an option of a build directory takes */
typedef enum
{
    BUILD_OPTION_STRING,
    BUILD_OPTION_BOOLEAN,
    BUILD_OPTION_COMBO, /* one of its choices */
    BUILD_OPTION_INTEGER,
    BUILD_OPTION_ARRAY,   /* a list, its items joined by ',' in the value */
    BUILD_OPTION_FEATURE, /* enabled, disabled or auto */
    BUILD_OPTION_PATH
} build_option_type_t;

/*** structures declarations (and typedefs of structures)*****************************************/

/* How a project is built */
typedef struct
{
    build_system_t system;
    char *build_dir; /* where the build is: the build directory of meson or cmake, else the root */
    char *command;   /* the command that builds it, run in the root */
} build_info_t;

/* What an executable says of itself */
typedef struct
{
    gboolean executable; /* an ELF program, not a library or an object */
    gboolean debug_info; /* it has .debug_info */
    gboolean optimized;  /* the compiler was told -O1 or more */
} build_elf_t;

/* A line of the compiler about a file */
typedef struct
{
    char *file; /* absolute */
    long line;
    long column;
    gboolean error; /* else a warning or a note */
    char *message;
} build_diagnostic_t;

/* An option of a build directory, as the build system has it there */
typedef struct
{
    char *name;
    char *section; /* meson: core, base, compiler, user, ...; cmake: cmake for CMAKE_*, else user */
    build_option_type_t type;
    char *value;          /* as text: "true", "2", "a,b" for a list; "" for none */
    char *default_value;  /* NULL when it is not known */
    char **choices;       /* NULL when any value goes */
    char **items;         /* meson: the items of a list, which may have ',' in them; else NULL */
    char *description;    /* NULL when there is none */
    gboolean advanced;    /* cmake: kept out of the first sight */
    gboolean set_by_user; /* meson: given to setup or configure; cmake: the build type, the flags
                             and the options of the project, what sets a directory up the same */
} build_option_t;

/* What a build directory is built for, at a glance */
typedef struct
{
    char *buildtype; /* debug, release, ...; NULL when it is not known */
    gboolean coverage;
    char *sanitize; /* address, undefined, ...; NULL for none */
} build_summary_t;

/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/

/* How the project of @root is built: meson with a build directory in it or next to it whose
   source is @root, cmake likewise, else make with a Makefile in @root; NULL when none */
build_info_t *build_detect (const char *root);
void build_info_free (build_info_t *info);

/* Read the ELF headers of a file; FALSE when it is no ELF file */
gboolean build_elf_read (const char *path, build_elf_t *elf);

/* The programs with debug information under @dir, a few levels down, the newest first: a
   GPtrArray of absolute names */
GPtrArray *build_find_programs (const char *dir);

/* A line of gcc, clang or rustc: "file:line:col: error: message", or of go, an error with no
   kind: "file:line:col: message" of a file there; NULL when it is none.  A relative name is
   taken from @dir, where the compiler ran. */
build_diagnostic_t *build_parse_diagnostic (const char *line, const char *dir);
void build_diagnostic_free (build_diagnostic_t *d);

/* The options of the build directory @dir of @system: meson from meson-info and meson-private,
   cmake from CMakeCache.txt; a GPtrArray of build_option_t that frees them, NULL when the
   directory has none */
GPtrArray *build_options_read (const char *dir, build_system_t system);
/* The defaults from @json, what "meson introspect --buildoptions meson.build" prints for the
   source without a build: FALSE when it is no list of options */
gboolean build_options_set_defaults (GPtrArray *options, const char *json);
const build_option_t *build_options_find (const GPtrArray *options, const char *name);
/* @items as meson takes a list in -D, "['-O1', '-g']", each quoted */
char *build_option_meson_list (const char *const *items);
/* The value of @o as -Dname= takes it: a list of meson as a list, the items kept apart */
char *build_option_arg (const build_option_t *o);
/* The options of the source of a meson project from @json, what "meson introspect --buildoptions
   meson.build" prints, each with its default for its value; NULL when it is no list of options */
GPtrArray *build_options_of_source (const char *json);
/* The other value of a boolean: "true" and "false", "ON" and "OFF", ...; "true" for another */
const char *build_option_toggled (const char *value);

/* What @options of @system say the directory is built for */
build_summary_t *build_summary_of (const GPtrArray *options, build_system_t system);
/* "debug, coverage, address,undefined"; to be freed */
char *build_summary_text (const build_summary_t *summary);
void build_summary_free (build_summary_t *summary);

/*** inline functions ****************************************************************************/

#endif /* MC__BUILD_CORE_H */
