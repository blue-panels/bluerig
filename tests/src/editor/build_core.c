/*
   src/plugins/build - tests for the core of the build plugin

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

#include <config.h>

/* -Dassert=false sets G_DISABLE_ASSERT in config.h: g_test_init () would then end the test and
   g_assert () check nothing, and the checks are the point of a test */
#undef G_DISABLE_ASSERT

#include <string.h>

#include <glib.h>
#include <glib/gstdio.h>

#include "src/plugins/build/build-core.h"
#include "src/plugins/build/build-presets.h"
#include "src/plugins/build/build-profiles.h"

static char *top = NULL;

static char *
at (const char *rel)
{
    return g_build_filename (top, rel, (char *) NULL);
}

static void
write_file (const char *rel, const char *text)
{
    char *path = at (rel);
    char *dir = g_path_get_dirname (path);

    g_assert_cmpint (g_mkdir_with_parents (dir, 0700), ==, 0);
    g_assert_true (g_file_set_contents (path, text, -1, NULL));
    g_free (dir);
    g_free (path);
}

static void
test_detect (void)
{
    char *root = at ("proj");
    char *build = at ("proj-build");
    char *json;
    build_info_t *info;

    // a meson build next to the project, which names it as its source
    write_file ("proj/meson.build", "project('x', 'c')\n");
    json =
        g_strdup_printf ("{\"directories\": {\"source\": \"%s\", \"build\": \"%s\"}}", root, build);
    write_file ("proj-build/meson-info/meson-info.json", json);
    g_free (json);
    // and one of another project
    write_file ("other-build/meson-info/meson-info.json",
                "{\"directories\": {\"source\": \"/elsewhere\"}}");

    info = build_detect (root);
    g_assert_nonnull (info);
    g_assert_cmpint (info->system, ==, BUILD_MESON);
    g_assert_cmpstr (info->build_dir, ==, build);
    g_assert_nonnull (strstr (info->command, "meson compile -C"));
    build_info_free (info);

    // make
    write_file ("mk/Makefile", "all:\n");
    g_free (root);
    root = at ("mk");
    info = build_detect (root);
    g_assert_nonnull (info);
    g_assert_cmpint (info->system, ==, BUILD_MAKE);
    g_assert_cmpstr (info->command, ==, "make");
    build_info_free (info);

    // nothing
    write_file ("bare/a.c", "");
    g_free (root);
    root = at ("bare");
    g_assert_null (build_detect (root));

    g_free (root);
    g_free (build);
}

static gboolean
compile (const char *flags, const char *out)
{
    char *src = at ("prog/main.c");
    char *command;
    gint status = 0;
    gboolean ok;

    command = g_strdup_printf ("cc %s -o '%s' '%s'", flags, out, src);
    ok = g_spawn_command_line_sync (command, NULL, NULL, &status, NULL) && status == 0;
    g_free (command);
    g_free (src);
    return ok;
}

static gboolean
is_elf (const char *path)
{
    char *data = NULL;
    gsize len = 0;
    gboolean elf;

    elf = g_file_get_contents (path, &data, &len, NULL) && len >= 4
        && memcmp (data, "\177ELF", 4) == 0;
    g_free (data);
    return elf;
}

static void
test_elf (void)
{
    char *debug = at ("prog/out/debug");
    char *fast = at ("prog/out/fast");
    char *dir = at ("prog");
    build_elf_t elf;
    GPtrArray *programs;
    char *cc = g_find_program_in_path ("cc");

    write_file ("prog/main.c", "int main (void) { return 0; }\n");
    write_file ("prog/out/.keep", "");
    // the flags in the debug information, which clang leaves out by default
    if (cc == NULL || !compile ("-g -O0 -grecord-gcc-switches", debug)
        || !compile ("-g -O2 -grecord-gcc-switches", fast))
    {
        g_test_skip ("no C compiler");
        goto out;
    }
    if (!is_elf (debug))
    {
        g_test_skip ("the C compiler makes no ELF");
        goto out;
    }

    g_assert_true (build_elf_read (debug, &elf));
    g_assert_true (elf.executable);
    g_assert_true (elf.debug_info);
    g_assert_false (elf.optimized);

    g_assert_true (build_elf_read (fast, &elf));
    g_assert_true (elf.debug_info);
    g_assert_true (elf.optimized);

    g_assert_false (build_elf_read (dir, &elf));

    programs = build_find_programs (dir);
    g_assert_cmpuint (programs->len, ==, 2);
    g_ptr_array_free (programs, TRUE);

out:
    g_free (cc);
    g_free (debug);
    g_free (fast);
    g_free (dir);
}

static void
test_diagnostic (void)
{
    build_diagnostic_t *d;

    d = build_parse_diagnostic ("../bluerig/src/a.c:12:5: error: 'x' undeclared",
                                "/b/bluerig-build");
    g_assert_nonnull (d);
    g_assert_cmpstr (d->file, ==, "/b/bluerig/src/a.c");
    g_assert_cmpint (d->line, ==, 12);
    g_assert_cmpint (d->column, ==, 5);
    g_assert_true (d->error);
    g_assert_cmpstr (d->message, ==, "'x' undeclared");
    build_diagnostic_free (d);

    d = build_parse_diagnostic ("/x/b.h:3: warning: unused", NULL);
    g_assert_nonnull (d);
    g_assert_false (d->error);
    g_assert_cmpint (d->column, ==, 0);
    build_diagnostic_free (d);

    // cargo build --message-format=short
    d = build_parse_diagnostic ("src/main.rs:3:9: error[E0425]: cannot find value `x`", "/r");
    g_assert_nonnull (d);
    g_assert_cmpstr (d->file, ==, "/r/src/main.rs");
    g_assert_true (d->error);
    g_assert_cmpstr (d->message, ==, "cannot find value `x`");
    build_diagnostic_free (d);

    g_assert_null (build_parse_diagnostic ("[12/30] Compiling C object a.o", "/b"));
    g_assert_null (build_parse_diagnostic ("ninja: build stopped: subcommand failed.", "/b"));

    // go build: no kind, an error of a file there
    {
        char *go = g_dir_make_tmp ("build_core_XXXXXX", NULL);
        char *main_go = g_build_filename (go, "main.go", NULL);

        g_assert_true (g_file_set_contents (main_go, "package main\n", -1, NULL));
        d = build_parse_diagnostic ("./main.go:12:2: undefined: x", go);
        g_assert_nonnull (d);
        g_assert_cmpstr (d->file, ==, main_go);
        g_assert_cmpint (d->line, ==, 12);
        g_assert_cmpint (d->column, ==, 2);
        g_assert_true (d->error);
        g_assert_cmpstr (d->message, ==, "undefined: x");
        build_diagnostic_free (d);
        // not a file there, or the context of gcc
        g_assert_null (build_parse_diagnostic ("./other.go:12:2: undefined: x", go));
        g_assert_null (build_parse_diagnostic ("./main.go:5:10:   required from here", go));
        g_unlink (main_go);
        g_rmdir (go);
        g_free (main_go);
        g_free (go);
    }
}

static void
test_presets (void)
{
    const char *user = "[rust]\nmarkers = Cargo.toml\ngdb = my-gdb\n\n"
                       "[zig]\nextensions = .zig\nbuild = zig build\n";
    char *system = NULL, *root, *program;
    GPtrArray *presets;
    const build_preset_t *p;
    GError *error = NULL;

    g_assert_true (g_file_get_contents (PRESETS_INI, &system, NULL, NULL));

    // those of bluerig alone
    presets = build_presets_parse (NULL, NULL, system, NULL, &error);
    g_assert_no_error (error);
    root = at ("presets");
    write_file ("presets/Cargo.toml", "[package]\nname = \"try-rust\"\nversion = \"0.1.0\"\n");
    p = build_presets_find (presets, root, "/x/notes.py");
    g_assert_nonnull (p);
    g_assert_cmpstr (p->id, ==, "rust");
    g_assert_cmpstr (p->gdb[0], ==, "rust-gdb");
    g_assert_cmpstr (p->gdb[1], ==, "gdb");
    g_assert_cmpstr (p->build, ==, "cargo build --message-format=short");
    g_assert_true (g_strv_contains ((const char *const *) p->hide, "target/*/deps/*"));
    program = build_presets_expand (p->program, root);
    g_assert_cmpstr (program, ==, "target/debug/try-rust");
    g_free (program);
    // no marker: the ending of the file
    p = build_presets_find (presets, "/nowhere", "/x/notes.py");
    g_assert_nonnull (p);
    g_assert_cmpstr (p->id, ==, "python");
    g_assert_cmpstr (p->adapter, ==, "python3 -m debugpy.adapter");
    g_assert_null (p->gdb);
    p = build_presets_find (presets, NULL, "/x/main.go");
    g_assert_cmpstr (p->launch, ==, "{\"mode\": \"debug\"}");
    g_assert_cmpstr (build_presets_find (presets, NULL, "/x/a.cpp")->id, ==, "c");
    g_assert_null (build_presets_find (presets, "/nowhere", "/x/README"));
    g_assert_null (build_presets_find (presets, "/nowhere", NULL));
    g_ptr_array_free (presets, TRUE);

    // the user first: a group of the name replaces that of bluerig, a new one is added
    presets = build_presets_parse (NULL, user, system, NULL, &error);
    g_assert_no_error (error);
    p = build_presets_find (presets, root, NULL);
    g_assert_cmpstr (p->id, ==, "rust");
    g_assert_cmpstr (p->gdb[0], ==, "my-gdb");
    g_assert_null (p->build);
    g_assert_cmpstr (build_presets_find (presets, NULL, "/x/a.zig")->build, ==, "zig build");
    g_assert_cmpstr (build_presets_find (presets, NULL, "/x/a.py")->id, ==, "python");
    g_ptr_array_free (presets, TRUE);

    // a wrong file of the user is left out, and said
    presets = build_presets_parse (NULL, "[rust\n", system, NULL, &error);
    g_assert_nonnull (error);
    g_clear_error (&error);
    g_assert_cmpstr (build_presets_find (presets, root, NULL)->gdb[0], ==, "rust-gdb");
    g_ptr_array_free (presets, TRUE);

    // the copy of a project: its language whatever its markers, and first by the endings
    presets = build_presets_parse (NULL, NULL, system, NULL, &error);
    program = build_presets_project_text (build_presets_find (presets, root, NULL), FALSE);
    g_assert_null (strstr (program, "\nmarkers="));
    g_assert_null (strstr (program, "\nbuild="));
    g_assert_nonnull (strstr (program, "language=rust"));
    g_assert_nonnull (strstr (program, "gdb=rust-gdb;gdb;"));
    g_ptr_array_free (presets, TRUE);
    {
        char *text = g_strconcat (program, "command=make me\n", (char *) NULL);
        GError *project_error = NULL;

        presets = build_presets_parse (text, user, system, &project_error, &error);
        g_assert_no_error (project_error);
        g_assert_no_error (error);
        p = build_presets_find (presets, "/nowhere", "/x/a.py");
        g_assert_true (p->project);
        g_assert_cmpstr (p->id, ==, "rust");
        // the copy, not the rust of the user
        g_assert_cmpstr (p->gdb[0], ==, "rust-gdb");
        g_assert_cmpstr (build_presets_find (presets, NULL, "/x/a.rs")->gdb[0], ==, "rust-gdb");
        g_assert_cmpstr (build_presets_find (presets, NULL, "/x/a.py")->id, ==, "python");
        g_ptr_array_free (presets, TRUE);
        g_free (text);

        // a file of the project with the command alone is no copy
        presets = build_presets_parse ("[project]\ncommand=make\n", NULL, system, NULL, NULL);
        g_assert_cmpstr (build_presets_find (presets, "/nowhere", "/x/a.py")->id, ==, "python");
        g_ptr_array_free (presets, TRUE);

        // a copy made before bluerig had a key: the key of its language, the user's first; one
        // the copy has, empty too, stays
        presets = build_presets_parse ("[project]\nlanguage=go\nadapter=my-dlv\nbuild=\n", NULL,
                                       system, NULL, NULL);
        p = build_presets_find (presets, "/nowhere", NULL);
        g_assert_true (p->project);
        g_assert_cmpstr (p->program, ==, ".");
        g_assert_cmpstr (p->entry, ==, "package main");
        g_assert_cmpstr (p->launch, ==, "{\"mode\": \"debug\"}");
        g_assert_cmpstr (p->adapter, ==, "my-dlv");
        g_assert_null (p->build);
        g_ptr_array_free (presets, TRUE);
        presets = build_presets_parse ("[project]\nlanguage=rust\n", user, system, NULL, NULL);
        p = build_presets_find (presets, "/nowhere", NULL);
        g_assert_cmpstr (p->gdb[0], ==, "my-gdb");
        g_assert_null (p->build);
        g_ptr_array_free (presets, TRUE);

        presets = build_presets_parse ("[project\n", NULL, system, &project_error, NULL);
        g_assert_nonnull (project_error);
        g_clear_error (&project_error);
        g_ptr_array_free (presets, TRUE);
    }
    g_free (program);

    // ${package} without a package
    g_assert_null (build_presets_expand ("target/debug/${package}", "/nowhere"));
    program = build_presets_expand ("bin/app", "/nowhere");
    g_assert_cmpstr (program, ==, "bin/app");
    g_free (program);

    g_free (root);
    g_free (system);
}

/* --------------------------------------------------------------------------------------------- */

/* The options of a meson build: their types, values, lists, choices, what the user gave, the
   defaults of the source, and what they say the build is for */
static void
test_options_meson (void)
{
    char *dir = at ("opt-meson");
    GPtrArray *options;
    const build_option_t *o;
    build_summary_t *summary;
    char *text;

    write_file (
        "opt-meson/meson-info/intro-buildoptions.json",
        "[{\"name\": \"buildtype\", \"value\": \"debug\", \"section\": \"core\", "
        "\"machine\": \"any\", \"choices\": [\"plain\", \"debug\", \"release\"], "
        "\"type\": \"combo\", \"description\": \"Build type to use\"},\n"
        " {\"name\": \"werror\", \"value\": false, \"section\": \"core\", \"type\": \"boolean\"},\n"
        " {\"name\": \"b_coverage\", \"value\": true, \"section\": \"base\", \"type\": "
        "\"boolean\"},\n"
        " {\"name\": \"b_sanitize\", \"value\": \"address,undefined\", \"section\": \"base\", "
        "\"choices\": [\"none\", \"address\", \"address,undefined\"], \"type\": \"combo\"},\n"
        " {\"name\": \"c_args\", \"value\": [\"-DX=1\", \"-O1\"], \"section\": \"compiler\", "
        "\"type\": \"array\"},\n"
        " {\"name\": \"backend_max_links\", \"value\": 0, \"section\": \"backend\", "
        "\"type\": \"integer\"},\n"
        " {\"name\": \"lua\", \"value\": \"auto\", \"section\": \"user\", \"type\": \"feature\", "
        "\"choices\": [\"enabled\", \"disabled\", \"auto\"]},\n"
        " {\"name\": \"motd\", \"value\": \"caf\\u00e9 \\\"x\\\"\\n\\ud83d\\ude00\", "
        "\"section\": \"user\", \"type\": \"string\", \"description\": null},\n"
        " {\"value\": \"no name: left out\"}]\n");
    write_file ("opt-meson/meson-private/cmd_line.txt",
                "[options]\nbuildtype = debug\nb_coverage = true\n\n[properties]\n");

    options = build_options_read (dir, BUILD_MESON);
    g_assert_nonnull (options);
    g_assert_cmpuint (options->len, ==, 8);

    o = build_options_find (options, "buildtype");
    g_assert_nonnull (o);
    g_assert_cmpint (o->type, ==, BUILD_OPTION_COMBO);
    g_assert_cmpstr (o->value, ==, "debug");
    g_assert_cmpstr (o->section, ==, "core");
    g_assert_cmpstr (o->description, ==, "Build type to use");
    g_assert_cmpuint (g_strv_length (o->choices), ==, 3);
    g_assert_cmpstr (o->choices[2], ==, "release");
    g_assert_true (o->set_by_user);
    g_assert_null (o->default_value);

    o = build_options_find (options, "werror");
    g_assert_cmpint (o->type, ==, BUILD_OPTION_BOOLEAN);
    g_assert_cmpstr (o->value, ==, "false");
    g_assert_false (o->set_by_user);
    g_assert_null (o->choices);
    o = build_options_find (options, "c_args");
    g_assert_cmpint (o->type, ==, BUILD_OPTION_ARRAY);
    g_assert_cmpstr (o->value, ==, "-DX=1,-O1");
    o = build_options_find (options, "backend_max_links");
    g_assert_cmpint (o->type, ==, BUILD_OPTION_INTEGER);
    g_assert_cmpstr (o->value, ==, "0");
    o = build_options_find (options, "lua");
    g_assert_cmpint (o->type, ==, BUILD_OPTION_FEATURE);
    // the escapes of JSON, a pair of surrogates one character
    o = build_options_find (options, "motd");
    g_assert_cmpstr (o->value, ==, "caf\xc3\xa9 \"x\"\n\xf0\x9f\x98\x80");
    g_assert_null (o->description);
    g_assert_null (build_options_find (options, "none"));

    summary = build_summary_of (options, BUILD_MESON);
    text = build_summary_text (summary);
    g_assert_cmpstr (text, ==, "debug, coverage, address,undefined");
    g_free (text);
    build_summary_free (summary);

    // the defaults of the source: those it names, the others not known
    g_assert_true (build_options_set_defaults (
        options,
        "[{\"name\": \"buildtype\", \"value\": \"plain\", \"type\": \"combo\"},"
        " {\"name\": \"werror\", \"value\": true, \"type\": \"boolean\"},"
        " {\"name\": \"only_in_source\", \"value\": 1, \"type\": \"integer\"}]"));
    g_assert_cmpstr (build_options_find (options, "buildtype")->default_value, ==, "plain");
    g_assert_cmpstr (build_options_find (options, "werror")->default_value, ==, "true");
    g_assert_null (build_options_find (options, "lua")->default_value);
    g_assert_null (build_options_find (options, "only_in_source"));
    // what is no list of options changes nothing
    g_assert_false (build_options_set_defaults (options, "{\"name\": \"x\"}"));
    g_assert_false (build_options_set_defaults (options, "[{\"name\": \"x\", \"value\": tru}]"));
    g_assert_false (build_options_set_defaults (options, "[1, 2] trailing"));
    g_assert_cmpstr (build_options_find (options, "buildtype")->default_value, ==, "plain");
    g_ptr_array_free (options, TRUE);

    // meson 1.8 gives b_sanitize as a list; nothing on: no sanitizer, no coverage
    write_file ("opt-meson/meson-info/intro-buildoptions.json",
                "[{\"name\": \"b_sanitize\", \"value\": [\"address\", \"undefined\"], "
                "\"type\": \"array\"},"
                " {\"name\": \"buildtype\", \"value\": \"release\", \"type\": \"combo\"}]");
    options = build_options_read (dir, BUILD_MESON);
    summary = build_summary_of (options, BUILD_MESON);
    g_assert_cmpstr (summary->sanitize, ==, "address,undefined");
    g_assert_false (summary->coverage);
    build_summary_free (summary);
    g_ptr_array_free (options, TRUE);
    write_file ("opt-meson/meson-info/intro-buildoptions.json",
                "[{\"name\": \"b_sanitize\", \"value\": \"none\", \"type\": \"combo\"},"
                " {\"name\": \"b_coverage\", \"value\": false, \"type\": \"boolean\"}]");
    options = build_options_read (dir, BUILD_MESON);
    summary = build_summary_of (options, BUILD_MESON);
    text = build_summary_text (summary);
    g_assert_cmpstr (text, ==, "");
    g_free (text);
    build_summary_free (summary);
    g_ptr_array_free (options, TRUE);

    // a broken file, a directory without meson-info, a system with no options
    write_file ("opt-meson/meson-info/intro-buildoptions.json", "[{\"name\": \"x\",");
    g_assert_null (build_options_read (dir, BUILD_MESON));
    g_assert_null (build_options_read (top, BUILD_MESON));
    g_assert_null (build_options_read (dir, BUILD_MAKE));
    g_assert_null (build_options_read (NULL, BUILD_MESON));
    g_free (dir);
}

/* --------------------------------------------------------------------------------------------- */

/* The options of a cmake build: help, types, advanced ones, the values a STRING takes, and the
   flags that make coverage and sanitizers */
static void
test_options_cmake (void)
{
    char *dir = at ("opt-cmake");
    GPtrArray *options;
    const build_option_t *o;
    build_summary_t *summary;
    char *text;

    write_file ("opt-cmake/CMakeCache.txt",
                "# This is the CMakeCache file.\n"
                "\n"
                "########################\n"
                "# EXTERNAL cache entries\n"
                "########################\n"
                "\n"
                "//Choose the type of build, options are: None Debug Release\n"
                "// RelWithDebInfo MinSizeRel ...\n"
                "CMAKE_BUILD_TYPE:STRING=Debug\n"
                "\n"
                "//Flags used by the C compiler during all build types.\n"
                "CMAKE_C_FLAGS:STRING=-Wall --coverage -fsanitize=address\n"
                "\n"
                "CMAKE_CXX_FLAGS_DEBUG:STRING=-g -fsanitize=undefined,address\n"
                "\n"
                "//Build the tests\n"
                "BUILD_TESTING:BOOL=ON\n"
                "\n"
                "//Where zlib is\n"
                "ZLIB_INCLUDE_DIR:PATH=/usr/include\n"
                "\n"
                "\"ODD NAME\":STRING=x=y\n"
                "\n"
                "SCREEN:STRING=slang\n"
                "\n"
                "WITH_THING:UNINITIALIZED=1\n"
                "\n"
                "CMAKE_HOME:STATIC=/src\n"
                "\n"
                "########################\n"
                "# INTERNAL cache entries\n"
                "########################\n"
                "\n"
                "//ADVANCED property for variable: ZLIB_INCLUDE_DIR\n"
                "ZLIB_INCLUDE_DIR-ADVANCED:INTERNAL=1\n"
                "CMAKE_C_FLAGS-ADVANCED:INTERNAL=0\n"
                "SCREEN-STRINGS:INTERNAL=slang;ncurses\n"
                "CMAKE_HOME_DIRECTORY:INTERNAL=/src\n");

    options = build_options_read (dir, BUILD_CMAKE);
    g_assert_nonnull (options);
    g_assert_cmpuint (options->len, ==, 8);

    o = build_options_find (options, "CMAKE_BUILD_TYPE");
    g_assert_cmpint (o->type, ==, BUILD_OPTION_STRING);
    g_assert_cmpstr (o->section, ==, "cmake");
    g_assert_cmpstr (o->description, ==,
                     "Choose the type of build, options are: None Debug Release RelWithDebInfo "
                     "MinSizeRel ...");
    o = build_options_find (options, "CMAKE_CXX_FLAGS_DEBUG");
    g_assert_null (o->description);
    o = build_options_find (options, "BUILD_TESTING");
    g_assert_cmpint (o->type, ==, BUILD_OPTION_BOOLEAN);
    g_assert_cmpstr (o->value, ==, "ON");
    g_assert_cmpstr (o->section, ==, "user");
    o = build_options_find (options, "ZLIB_INCLUDE_DIR");
    g_assert_cmpint (o->type, ==, BUILD_OPTION_PATH);
    g_assert_true (o->advanced);
    g_assert_false (build_options_find (options, "CMAKE_C_FLAGS")->advanced);
    o = build_options_find (options, "ODD NAME");
    g_assert_cmpstr (o->value, ==, "x=y");
    o = build_options_find (options, "SCREEN");
    g_assert_cmpint (o->type, ==, BUILD_OPTION_COMBO);
    g_assert_cmpuint (g_strv_length (o->choices), ==, 2);
    g_assert_cmpstr (o->choices[1], ==, "ncurses");
    g_assert_nonnull (build_options_find (options, "WITH_THING"));
    // the static and internal entries are no options
    g_assert_null (build_options_find (options, "CMAKE_HOME"));
    g_assert_null (build_options_find (options, "CMAKE_HOME_DIRECTORY"));
    g_assert_null (build_options_find (options, "SCREEN-STRINGS"));

    /* what sets another directory up the same: the build type and the flags given, the options of
       the project; not the paths found, nor the flags of a build type cmake gives */
    text = build_profile_setup_of (dir, BUILD_CMAKE);
    g_assert_cmpstr (
        text, ==,
        "-DCMAKE_BUILD_TYPE=Debug '-DCMAKE_C_FLAGS=-Wall --coverage -fsanitize=address' "
        "-DBUILD_TESTING=ON '-DODD NAME=x=y' -DSCREEN=slang -DWITH_THING=1");
    g_free (text);

    // the flags of all builds and of the build type, each sanitizer once
    summary = build_summary_of (options, BUILD_CMAKE);
    text = build_summary_text (summary);
    g_assert_cmpstr (text, ==, "debug, coverage, address,undefined");
    g_free (text);
    build_summary_free (summary);
    g_ptr_array_free (options, TRUE);

    write_file ("opt-cmake/CMakeCache.txt",
                "CMAKE_BUILD_TYPE:STRING=\nCMAKE_C_FLAGS:STRING=-O2\n"
                "MODE:STRING=x\nMODE-STRINGS:INTERNAL=\n");
    options = build_options_read (dir, BUILD_CMAKE);
    // an empty list of the values it takes: any value goes
    g_assert_cmpint (build_options_find (options, "MODE")->type, ==, BUILD_OPTION_STRING);
    g_assert_null (build_options_find (options, "MODE")->choices);
    summary = build_summary_of (options, BUILD_CMAKE);
    g_assert_null (summary->buildtype);
    g_assert_false (summary->coverage);
    g_assert_null (summary->sanitize);
    build_summary_free (summary);
    g_ptr_array_free (options, TRUE);
    g_free (dir);
}

/* --------------------------------------------------------------------------------------------- */

/* Profiles read and written in the file of the project, its other groups kept; the default one
   of a project without them; a new one made from another; one taken away */
static void
test_profiles (void)
{
    GKeyFile *keyfile = g_key_file_new ();
    build_profiles_t *profiles;
    build_profile_t *p, *cov;
    char *text, *command;
    build_info_t found = { BUILD_MESON, (char *) "/src/proj-build", (char *) "meson compile" };

    g_assert_true (g_key_file_load_from_data (
        keyfile,
        "[Build]\ncommand=make all\n\n"
        "[project]\nprofile=Debug\ncoverage_profile=Gone\n\n"
        "[build:Debug]\ndir=../proj-build\nsystem=meson\n"
        "setup=-Dbuildtype=debug -Dscreen=slang\nbuild=meson compile -C ../proj-build\n"
        "test=meson test -C ../proj-build\ntest_after_build=true\n\n"
        "[build:Plain]\ndir=build\nsystem=make\nbuild=  \n\n"
        "[build:]\ndir=x\n",
        -1, G_KEY_FILE_NONE, NULL));
    profiles = build_profiles_load (keyfile);
    g_assert_cmpuint (profiles->list->len, ==, 2);
    g_assert_cmpstr (profiles->active, ==, "Debug");
    // a profile of coverage that is not there is none
    g_assert_null (profiles->coverage);
    p = build_profiles_find (profiles, "Debug");
    g_assert_cmpint (p->system, ==, BUILD_MESON);
    g_assert_cmpstr (p->setup, ==, "-Dbuildtype=debug -Dscreen=slang");
    g_assert_true (p->test_after_build);
    g_assert_true (build_profiles_active (profiles) == p);
    // its own command, its directory not set up yet: set up first with its options
    command = build_profile_command (p, "/src/proj");
    g_assert_cmpstr (command, ==,
                     "(meson setup '/src/proj-build' -Dbuildtype=debug -Dscreen=slang && "
                     "meson compile -C ../proj-build\n) && meson test -C ../proj-build");
    g_free (command);
    // a command that sets up itself, or one that names no build directory, as it is
    {
        char *own = p->build;
        char *had = p->setup;

        // the setup with "&&" in a value in quotes stays whole
        p->setup = (char *) "'-Dlabel=A && B'";
        command = build_profile_build_command (p, "/src/proj");
        g_assert_cmpstr (command, ==,
                         "meson setup '/src/proj-build' '-Dlabel=A && B' && "
                         "meson compile -C ../proj-build");
        g_free (command);
        p->setup = had;

        p->build = (char *) "meson setup ../proj-build && meson compile -C ../proj-build";
        command = build_profile_build_command (p, "/src/proj");
        g_assert_cmpstr (command, ==, p->build);
        g_free (command);
        p->build = (char *) "make";
        command = build_profile_build_command (p, "/src/proj");
        g_assert_cmpstr (command, ==, "make");
        g_free (command);
        p->build = own;
    }
    text = build_profile_dir (p, "/src/proj");
    g_assert_cmpstr (text, ==, "/src/proj-build");
    g_free (text);
    // an empty command is that of its system
    p = build_profiles_find (profiles, "Plain");
    g_assert_null (p->build);
    g_assert_cmpint (p->system, ==, BUILD_MAKE);
    command = build_profile_command (p, "/src/proj");
    g_assert_cmpstr (command, ==, "make");
    g_free (command);

    // made from Debug: its options and commands, in a directory of its own
    cov = build_profiles_add (profiles, "/src/proj", "Coverage",
                              build_profiles_find (profiles, "Debug"), "../proj-build-cov");
    g_assert_nonnull (cov);
    g_assert_cmpstr (cov->from, ==, "Debug");
    g_assert_cmpstr (cov->setup, ==, "-Dbuildtype=debug -Dscreen=slang");
    // "meson compile -C ../proj-build" is what meson does: the new one sets its directory up
    g_assert_null (cov->build);
    g_assert_cmpstr (cov->test, ==, "meson test -C ../proj-build-cov");
    g_assert_true (cov->test_after_build);
    {
        // the directory named absolute and quoted, a whole word only: "build" not in "--build"
        build_profile_t from = { 0 };
        build_profile_t *q;

        from.name = (char *) "Abs";
        from.dir = (char *) "build";
        from.system = BUILD_CMAKE;
        from.build = (char *) "cmake --build build && cp build/x '/src/proj/build/y' /src/build";
        from.test = (char *) "ctest --test-dir '/src/proj/build'";
        q = build_profiles_add (profiles, "/src/proj", "Abs2", &from, "build/abs2");
        g_assert_cmpstr (q->build, ==,
                         "cmake --build build/abs2 && cp build/abs2/x '/src/proj/build/abs2/y' "
                         "/src/build");
        g_assert_cmpstr (q->test, ==, "ctest --test-dir '/src/proj/build/abs2'");
        // the command of its system for it: none, the new one sets its directory up first
        from.system = BUILD_MESON;
        from.dir = (char *) "../proj-build";
        from.build = NULL;
        text = build_profile_build_command (&from, "/src/proj");
        from.build = text;
        q = build_profiles_add (profiles, "/src/proj", "Abs3", &from, "../proj-build-3");
        g_assert_null (q->build);
        g_free (text);
        // a command of its own: the directory after an option, after "=" or as a path only
        from.dir = (char *) "build";
        from.build = (char *) "cd build && ninja -j 4 B=build && ninja -C 'build' x && ./build.sh";
        from.test = (char *) "cd build && meson test";
        q = build_profiles_add (profiles, "/src/proj", "Abs4", &from, "build/x");
        g_assert_cmpstr (
            q->build, ==,
            "cd build/x && ninja -j 4 B=build/x && ninja -C 'build/x' x && ./build.sh");
        g_assert_cmpstr (q->test, ==, "cd build/x && meson test");
        // "./build" is "build"; a new directory with a space is quoted where it stands alone
        from.build = (char *) "cd ./build && ninja -j 4";
        from.test = NULL;
        q = build_profiles_add (profiles, "/src/proj", "Abs7", &from, "build coverage");
        g_assert_cmpstr (q->build, ==, "cd 'build coverage' && ninja -j 4");
        g_assert_true (build_profiles_remove (profiles, "Abs7"));
        // an absolute directory named from the root; an option glued to the directory
        from.dir = (char *) "/opt/b/proj";
        from.build = (char *) "meson compile -C ../../opt/b/proj -j4 && ninja -C../../opt/b/proj";
        q = build_profiles_add (profiles, "/src/proj", "Abs8", &from, "/opt/b/proj2");
        g_assert_cmpstr (q->build, ==, "meson compile -C /opt/b/proj2 -j4 && ninja -C/opt/b/proj2");
        g_assert_true (build_profiles_remove (profiles, "Abs8"));
        from.dir = (char *) "build";
        // the window moves the directory of a profile: its command follows
        text = build_profile_follow_dir ("meson compile -C ../proj-build-new -j4", "/src/proj",
                                         "../proj-build-new", "../x");
        g_assert_cmpstr (text, ==, "meson compile -C ../x -j4");
        g_free (text);
        text = build_profile_follow_dir ("./run.sh && make", "/src/proj", ".", "build/x");
        g_assert_cmpstr (text, ==, "./run.sh && make");
        g_free (text);
        /* "build" of "make build" may be the directory or a target: not told for sure, the new
           profile builds as its system does, its tests by their command of the system */
        from.build = (char *) "make build";
        from.test = (char *) "make -C build check && ls build";
        from.system = BUILD_MESON;
        q = build_profiles_add (profiles, "/src/proj", "Abs6", &from, "build/y");
        g_assert_null (q->build);
        g_assert_cmpstr (q->test, ==, "meson test -C '/src/proj/build/y'");
        g_assert_true (build_profiles_remove (profiles, "Abs6"));
        from.system = BUILD_CMAKE;
        // a profile in the root: its commands name no build directory, they stay
        from.dir = (char *) ".";
        from.build = (char *) "./run_tests.sh . && make";
        from.test = (char *) "make check";
        q = build_profiles_add (profiles, "/src/proj", "Abs5", &from, "build/y");
        g_assert_cmpstr (q->build, ==, "./run_tests.sh . && make");
        g_assert_cmpstr (q->test, ==, "make check");
        g_assert_true (build_profiles_remove (profiles, "Abs2"));
        g_assert_true (build_profiles_remove (profiles, "Abs3"));
        g_assert_true (build_profiles_remove (profiles, "Abs4"));
        g_assert_true (build_profiles_remove (profiles, "Abs5"));
    }
    // a name taken, empty or with brackets is none
    g_assert_null (build_profiles_add (profiles, "/src/proj", "Coverage", NULL, "x"));
    g_assert_null (build_profiles_add (profiles, "/src/proj", "", NULL, "x"));
    g_assert_null (build_profiles_add (profiles, "/src/proj", "a]b", NULL, "x"));
    profiles->coverage = g_strdup ("Coverage");
    g_free (profiles->active);
    profiles->active = g_strdup ("Coverage");

    // written back: the group of before kept, the names in their places
    build_profiles_store (profiles, keyfile);
    text = g_key_file_get_string (keyfile, "Build", "command", NULL);
    g_assert_cmpstr (text, ==, "make all");
    g_free (text);
    build_profiles_free (profiles);
    profiles = build_profiles_load (keyfile);
    g_assert_cmpuint (profiles->list->len, ==, 3);
    g_assert_cmpstr (profiles->coverage, ==, "Coverage");
    g_assert_cmpstr (build_profiles_active (profiles)->name, ==, "Coverage");
    g_assert_cmpstr (build_profiles_find (profiles, "Coverage")->dir, ==, "../proj-build-cov");
    g_assert_null (build_profiles_find (profiles, "Plain")->build);

    // taken away: active and coverage go with it, the first is active then
    g_assert_true (build_profiles_remove (profiles, "Coverage"));
    g_assert_false (build_profiles_remove (profiles, "Coverage"));
    g_assert_null (profiles->active);
    g_assert_null (profiles->coverage);
    g_assert_cmpstr (build_profiles_active (profiles)->name, ==, "Debug");
    build_profiles_store (profiles, keyfile);
    g_assert_false (g_key_file_has_group (keyfile, "build:Coverage"));
    g_assert_false (g_key_file_has_key (keyfile, "project", "profile", NULL));
    build_profiles_free (profiles);
    g_key_file_free (keyfile);

    // none in the file: none; a project without them gets Default, the command given kept
    profiles = build_profiles_load (NULL);
    g_assert_cmpuint (profiles->list->len, ==, 0);
    g_assert_null (build_profiles_active (profiles));
    build_profiles_free (profiles);
    profiles = build_profiles_default ("/src/proj", &found, "make -j4");
    p = build_profiles_active (profiles);
    g_assert_cmpstr (p->name, ==, BUILD_PROFILES_DEFAULT);
    // named from the root, as the user names it
    g_assert_cmpstr (p->dir, ==, "../proj-build");
    g_assert_cmpint (p->system, ==, BUILD_MESON);
    g_assert_cmpstr (p->build, ==, "make -j4");
    build_profiles_free (profiles);
    // a directory not set up yet is set up first, with the options of the profile
    profiles = build_profiles_default ("/src/proj", &found, NULL);
    command = build_profile_command (build_profiles_active (profiles), "/src/proj");
    g_assert_cmpstr (command, ==,
                     "meson setup '/src/proj-build' && meson compile -C '/src/proj-build'");
    g_free (command);
    p = build_profiles_active (profiles);
    p->setup = g_strdup ("-Db_coverage=true");
    command = build_profile_command (p, "/src/proj");
    g_assert_cmpstr (command, ==,
                     "meson setup '/src/proj-build' -Db_coverage=true && "
                     "meson compile -C '/src/proj-build'");
    g_free (command);
    p->system = BUILD_CMAKE;
    command = build_profile_command (p, "/src/proj");
    g_assert_cmpstr (command, ==,
                     "cmake -S '/src/proj' -B '/src/proj-build' -Db_coverage=true && "
                     "cmake --build '/src/proj-build'");
    g_free (command);
    build_profiles_free (profiles);
    {
        // set up already: built as it is
        char *dir = at ("prof-build");
        build_info_t there = { BUILD_MESON, dir, NULL };

        write_file ("prof-build/meson-info/meson-info.json", "{}");
        g_assert_cmpint (build_system_of_dir (dir), ==, BUILD_MESON);
        profiles = build_profiles_default ("/src/proj", &there, NULL);
        command = build_profile_command (build_profiles_active (profiles), "/src/proj");
        text = g_strdup_printf ("meson compile -C '%s'", dir);
        g_assert_cmpstr (command, ==, text);
        g_free (text);
        g_free (command);
        build_profiles_free (profiles);

        // what set it up: the options given, those with odd characters quoted
        write_file ("prof-build/meson-info/intro-buildoptions.json",
                    "[{\"name\": \"buildtype\", \"value\": \"debug\", \"type\": \"combo\"},"
                    " {\"name\": \"c_args\", \"value\": [\"-DA=1\", \"-DB=a b\"], "
                    "\"type\": \"array\"},"
                    " {\"name\": \"werror\", \"value\": false, \"type\": \"boolean\"}]");
        write_file ("prof-build/meson-private/cmd_line.txt",
                    "[options]\nbuildtype = debug\nc_args = -DA=1,'-DB=a b'\n");
        text = build_profile_setup_of (dir, BUILD_MESON);
        g_assert_cmpstr (text, ==, "-Dbuildtype=debug \"-Dc_args=['-DA=1', '-DB=a b']\"");
        g_free (text);
        g_assert_null (build_profile_setup_of (top, BUILD_MESON));
        g_assert_cmpint (build_system_of_dir (top), ==, BUILD_NONE);
        g_free (dir);
    }
    profiles = build_profiles_default ("/src/proj", NULL, NULL);
    p = build_profiles_active (profiles);
    g_assert_cmpstr (p->dir, ==, ".");
    g_assert_cmpint (p->system, ==, BUILD_NONE);
    g_assert_null (build_profile_command (p, "/src/proj"));
    build_profiles_free (profiles);

    {
        static const char *const cases[][3] = {
            { "/src/proj", "/src/proj", "." },
            { "/src/proj", "/src/proj/build", "build" },
            { "/src/proj", "/src/proj-build", "../proj-build" },
            { "/src/proj", "/src/projx/b", "../projx/b" },
            { "/src/proj", "/other/b", "/other/b" },
            { "/proj", "/proj-build", "/proj-build" },
        };
        size_t i;

        for (i = 0; i < G_N_ELEMENTS (cases); i++)
        {
            text = build_profile_relative (cases[i][0], cases[i][1]);
            g_assert_cmpstr (text, ==, cases[i][2]);
            g_free (text);
        }
    }

    {
        // the options added take the place of those of the same name, the rest kept in order
        static const char *const cases[][3] = {
            { "-Dx=1 -Db_coverage=false -Dy=2", "-Db_coverage=true",
              "-Dx=1 -Dy=2 -Db_coverage=true" },
            { NULL, "-Db_coverage=true", "-Db_coverage=true" },
            { "-Dx=1", NULL, "-Dx=1" },
            { "'-Dc_args=a b' -Dx=1", "-Dx=2 -Wno", "'-Dc_args=a b' -Dx=2 -Wno" },
            { "", "", NULL },
        };
        size_t i;

        for (i = 0; i < G_N_ELEMENTS (cases); i++)
        {
            text = build_profile_setup_merge (cases[i][0], cases[i][1]);
            g_assert_cmpstr (text, ==, cases[i][2]);
            g_free (text);
        }
        g_assert_null (build_profile_setup_merge (NULL, NULL));
    }

    command = build_profile_test_command (BUILD_CMAKE, "/b d");
    g_assert_cmpstr (command, ==, "ctest --test-dir '/b d'");
    g_free (command);
    g_assert_null (build_profile_test_command (BUILD_NONE, "/b"));
    g_assert_cmpint (build_system_of_name (build_system_name (BUILD_CMAKE)), ==, BUILD_CMAKE);
    g_assert_cmpint (build_system_of_name ("scons"), ==, BUILD_NONE);
}

/* --------------------------------------------------------------------------------------------- */

static void
test_configure (void)
{
    build_profile_t p = { 0 };
    char *dir = at ("conf-build");
    char *text, *setup;
    GPtrArray *options;
    const build_option_t *o;

    // an option added, one of a type, one with a space: each its own word
    setup = build_profile_setup_add ("-Dx=1 -DCMAKE_BUILD_TYPE:STRING=Debug", "x", "2");
    g_assert_cmpstr (setup, ==, "-DCMAKE_BUILD_TYPE:STRING=Debug -Dx=2");
    text = build_profile_setup_add (setup, "CMAKE_BUILD_TYPE", "Release");
    g_assert_cmpstr (text, ==, "-Dx=2 -DCMAKE_BUILD_TYPE=Release");
    g_free (text);
    text = build_profile_setup_add (NULL, "c_args", "-DA=a b");
    g_assert_cmpstr (text, ==, "'-Dc_args=-DA=a b'");
    g_free (text);
    g_free (setup);

    // not set up: the options of the profile and the changes; set up: the changes alone
    p.name = (char *) "Cov";
    p.dir = dir;
    p.system = BUILD_MESON;
    p.setup = (char *) "-Dbuildtype=debug -Db_coverage=false";
    text = build_profile_configure_command (&p, "/src/proj", "-Db_coverage=true");
    setup = g_strdup_printf ("meson setup '%s' -Dbuildtype=debug -Db_coverage=true", dir);
    g_assert_cmpstr (text, ==, setup);
    g_free (setup);
    g_free (text);
    write_file ("conf-build/meson-info/meson-info.json", "{}");
    text = build_profile_configure_command (&p, "/src/proj", "-Db_coverage=true");
    setup = g_strdup_printf ("meson configure '%s' -Db_coverage=true", dir);
    g_assert_cmpstr (text, ==, setup);
    g_free (setup);
    g_free (text);
    p.system = BUILD_CMAKE;
    p.setup = NULL;
    text = build_profile_configure_command (&p, "/src/proj", NULL);
    setup = g_strdup_printf ("cmake -S '/src/proj' -B '%s'", dir);
    g_assert_cmpstr (text, ==, setup);
    g_free (setup);
    g_free (text);
    p.system = BUILD_MAKE;
    g_assert_null (build_profile_configure_command (&p, "/src/proj", "-Dx=1"));
    g_free (dir);

    // the options of the source, their defaults the values; the setup of the profile on them
    options = build_options_of_source (
        "[{\"name\": \"buildtype\", \"value\": \"debug\", \"type\": \"combo\", "
        "\"choices\": [\"plain\", \"debug\", \"release\"]},"
        " {\"name\": \"b_coverage\", \"value\": false, \"type\": \"boolean\"}]");
    g_assert_nonnull (options);
    build_profile_setup_apply (options, "-Db_coverage=true -Dnone=1");
    o = build_options_find (options, "b_coverage");
    g_assert_cmpstr (o->value, ==, "true");
    g_assert_cmpstr (o->default_value, ==, "false");
    g_assert_true (o->set_by_user);
    o = build_options_find (options, "buildtype");
    g_assert_cmpstr (o->value, ==, o->default_value);
    g_assert_false (o->set_by_user);
    g_assert_cmpstr (o->choices[2], ==, "release");
    g_ptr_array_free (options, TRUE);
    // a surrogate alone is U+FFFD, a NUL is left out
    options = build_options_of_source (
        "[{\"name\": \"s\", \"value\": \"a\\ud800b\\u0000c\", \"type\": \"string\"}]");
    g_assert_nonnull (options);
    g_assert_cmpstr (build_options_find (options, "s")->value, ==,
                     "a\xef\xbf\xbd"
                     "bc");
    g_ptr_array_free (options, TRUE);
    g_assert_null (build_options_of_source ("{}"));
    g_assert_null (build_options_of_source (NULL));

    g_assert_cmpstr (build_option_toggled ("true"), ==, "false");
    g_assert_cmpstr (build_option_toggled ("OFF"), ==, "ON");
    g_assert_cmpstr (build_option_toggled ("maybe"), ==, "true");

    {
        // a list of meson keeps its items apart, quotes and backslashes in them escaped
        static const char *const items[] = { "-O1", "-DX='a b'", "c\\d", NULL };
        char *list = build_option_meson_list (items);

        g_assert_cmpstr (list, ==, "['-O1', '-DX=\\'a b\\'', 'c\\\\d']");
        g_free (list);
        list = build_option_meson_list (NULL);
        g_assert_cmpstr (list, ==, "[]");
        g_free (list);
        // flags added to those there, once; an option not there is added as it is
        text =
            build_profile_setup_append ("-DCMAKE_CXX_FLAGS=-stdlib=libc++ -DX=1",
                                        "-DCMAKE_CXX_FLAGS=--coverage -DCMAKE_C_FLAGS=--coverage");
        g_assert_cmpstr (text, ==,
                         "-DX=1 '-DCMAKE_CXX_FLAGS=-stdlib=libc++ --coverage' "
                         "-DCMAKE_C_FLAGS=--coverage");
        setup = build_profile_setup_append (text, "-DCMAKE_C_FLAGS=--coverage");
        g_assert_cmpstr (setup, ==, text);
        g_free (setup);
        g_free (text);
        // the test command of the system follows the directory, another one stays
        text = build_profile_test_follow ("meson test -C '/src/proj/build'", BUILD_MESON,
                                          "/src/proj", "build", "/src/proj/build-2");
        g_assert_cmpstr (text, ==, "meson test -C '/src/proj/build-2'");
        g_free (text);
        text = build_profile_test_follow ("ctest --test-dir build", BUILD_CMAKE, "/src/proj",
                                          "/src/proj/build", "/b");
        g_assert_cmpstr (text, ==, "ctest --test-dir '/b'");
        g_free (text);
        g_assert_null (build_profile_test_follow ("meson test -C build --suite x", BUILD_MESON,
                                                  "/src/proj", "build", "/b"));
        g_assert_null (build_profile_test_follow ("meson test -C other", BUILD_MESON, "/src/proj",
                                                  "build", "/b"));
        text = build_profile_setup_add (NULL, "c_args", "['-O1', '-g']");
        g_assert_cmpstr (text, ==, "\"-Dc_args=['-O1', '-g']\"");
        g_free (text);
    }
}

static void
remove_tree (const char *path)
{
    GDir *dir = g_dir_open (path, 0, NULL);
    const char *name;

    if (dir != NULL)
    {
        while ((name = g_dir_read_name (dir)) != NULL)
        {
            char *child = g_build_filename (path, name, (char *) NULL);

            remove_tree (child);
            g_free (child);
        }
        g_dir_close (dir);
    }
    (void) g_remove (path);
}

int
main (int argc, char **argv)
{
    int result;

    g_test_init (&argc, &argv, NULL);
    top = g_dir_make_tmp ("build-core-XXXXXX", NULL);
    g_assert_nonnull (top);
    g_test_add_func ("/build/detect", test_detect);
    g_test_add_func ("/build/elf", test_elf);
    g_test_add_func ("/build/diagnostic", test_diagnostic);
    g_test_add_func ("/build/presets", test_presets);
    g_test_add_func ("/build/options-meson", test_options_meson);
    g_test_add_func ("/build/options-cmake", test_options_cmake);
    g_test_add_func ("/build/configure", test_configure);
    g_test_add_func ("/build/profiles", test_profiles);
    result = g_test_run ();
    remove_tree (top);
    g_free (top);
    return result;
}
