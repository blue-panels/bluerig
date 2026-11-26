/** \file build-profiles.h
 *  \brief Header: the build profiles of a project, a build directory and how it is built each
 */

#ifndef MC__BUILD_PROFILES_H
#define MC__BUILD_PROFILES_H

#include "lib/global.h"

#include "build-core.h"

/*** typedefs(not structures) and defined constants **********************************************/

/* The groups of the profiles in the file of the project in the configuration of the user */
#define BUILD_PROFILES_GROUP        "project"
#define BUILD_PROFILES_GROUP_PREFIX "build:"
#define BUILD_PROFILES_DEFAULT      "Default"

/*** enums ***************************************************************************************/

/*** structures declarations (and typedefs of structures)*****************************************/

/* A build profile: a build directory and how it is built */
typedef struct
{
    char *name;
    char *dir; /* as given: from the root of the project, or absolute */
    build_system_t system;
    char *from;  /* the profile it was made from, NULL for none */
    char *setup; /* the options that set the directory up again, "-Dx=y ...", NULL for none */
    char *build; /* the command that builds it, run in the root; NULL: that of its system */
    char *test;  /* the command that runs its tests, NULL for none */
    gboolean test_after_build;
} build_profile_t;

/* The profiles of a project */
typedef struct
{
    GPtrArray *list; /* build_profile_t, in their order */
    char *active;    /* Ctrl-F9 builds it, the debugger runs what it built; NULL: the first */
    char *coverage;  /* the coverage plugin reads its data; NULL for none */
} build_profiles_t;

/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/

/* "meson", "cmake", "make", "none" and back */
const char *build_system_name (build_system_t system);
build_system_t build_system_of_name (const char *name);

/* The profiles in @keyfile, the groups "build:NAME" and the keys of "project"; none when it has
   none, and @keyfile NULL too */
build_profiles_t *build_profiles_load (GKeyFile *keyfile);
/* The profiles into @keyfile, those it had replaced, its other groups kept */
void build_profiles_store (const build_profiles_t *profiles, GKeyFile *keyfile);
void build_profiles_free (build_profiles_t *profiles);

/* One profile, "Default", for a project that has none: the build found in it, @found, NULL when
   none, built by @command, the one the user gave the project before it had profiles, NULL for
   that of the build found */
build_profiles_t *build_profiles_default (const char *root, const build_info_t *found,
                                          const char *command);

build_profile_t *build_profiles_find (const build_profiles_t *profiles, const char *name);
/* The active profile, else the first; NULL when there is none */
build_profile_t *build_profiles_active (const build_profiles_t *profiles);
/* A new profile @name of the project of @root made from @from, its options and commands, in
   @dir; NULL when the name is taken or empty.  The commands of @from that name its directory,
   as given or absolute, name @dir. */
build_profile_t *build_profiles_add (build_profiles_t *profiles, const char *root, const char *name,
                                     const build_profile_t *from, const char *dir);
/* The profile @name gone, and the active one and that of coverage when it was one of them */
gboolean build_profiles_remove (build_profiles_t *profiles, const char *name);

/* The build directory of @profile, absolute */
char *build_profile_dir (const build_profile_t *profile, const char *root);
/* The command that builds @profile, its tests after it when it says so: "meson compile -C dir"
   when it has none of its own; NULL when its system has no command either */
char *build_profile_command (const build_profile_t *profile, const char *root);
/* The command that runs the tests of @system in @dir, offered to a new profile */
char *build_profile_test_command (build_system_t system, const char *dir);

/*** inline functions ****************************************************************************/

#endif /* MC__BUILD_PROFILES_H */
