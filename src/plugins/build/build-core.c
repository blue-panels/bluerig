/*
   How a project is built, what it builds, and what the compiler says.

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

/** \file build-core.c
 *  \brief Source: how a project is built, what it builds, and what the compiler says
 */

#include <config.h>

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "lib/global.h"

#include "build-core.h"

/*** global variables ****************************************************************************/

/*** file scope macro definitions ****************************************************************/

/* how deep under a build directory the programs are looked for */
#define BUILD_SEARCH_DEPTH 4
#define BUILD_PROGRAMS_MAX 200
/* how much of the debug strings is read for the flags of the compiler */
#define BUILD_DEBUG_STR_MAX (8 * 1024 * 1024)

/*** file scope type declarations ****************************************************************/

typedef struct
{
    char *path;
    gint64 mtime;
} build_program_t;

/*** forward declarations (file scope functions) *************************************************/

static gboolean build_elf_read_full (const char *path, build_elf_t *elf, gboolean flags);

/*** file scope variables ************************************************************************/

/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

static char *
build_read_text (const char *path)
{
    char *text = NULL;

    if (!g_file_get_contents (path, &text, NULL, NULL))
        return NULL;
    return text;
}

/* --------------------------------------------------------------------------------------------- */

/* The string after "key": in a JSON text, as it stands: enough for the paths of meson */
static char *
build_json_string (const char *text, const char *key)
{
    char *needle = g_strdup_printf ("\"%s\": \"", key);
    const char *p = strstr (text, needle);
    GString *out;

    if (p == NULL)
    {
        g_free (needle);
        return NULL;
    }
    p += strlen (needle);
    g_free (needle);
    out = g_string_new (NULL);
    for (; *p != '\0' && *p != '"'; p++)
    {
        if (*p == '\\' && p[1] != '\0')
            p++;
        g_string_append_c (out, *p);
    }
    return g_string_free (out, FALSE);
}

/* --------------------------------------------------------------------------------------------- */

/* Whether a directory is a build directory of meson, or of cmake, for that source */
static build_system_t
build_dir_of (const char *dir, const char *root)
{
    char *path, *text, *source;
    build_system_t system = BUILD_NONE;

    path = g_build_filename (dir, "meson-info", "meson-info.json", (char *) NULL);
    text = build_read_text (path);
    g_free (path);
    if (text != NULL)
    {
        source = build_json_string (text, "source");
        if (source != NULL && strcmp (source, root) == 0)
            system = BUILD_MESON;
        g_free (source);
        g_free (text);
        return system;
    }

    path = g_build_filename (dir, "CMakeCache.txt", (char *) NULL);
    text = build_read_text (path);
    g_free (path);
    if (text != NULL)
    {
        const char *home = strstr (text, "CMAKE_HOME_DIRECTORY:INTERNAL=");

        if (home != NULL)
        {
            const char *start = home + strlen ("CMAKE_HOME_DIRECTORY:INTERNAL=");
            const char *end = strchr (start, '\n');

            source = end != NULL ? g_strndup (start, (gsize) (end - start)) : g_strdup (start);
            if (strcmp (source, root) == 0)
                system = BUILD_CMAKE;
            g_free (source);
        }
        g_free (text);
    }
    return system;
}

/* --------------------------------------------------------------------------------------------- */

/* A build directory of the project: in the root, or next to it, the newest one */
static build_system_t
build_find_dir (const char *root, char **found)
{
    char *parent = g_path_get_dirname (root);
    const char *places[] = { root, parent, NULL };
    build_system_t best_system = BUILD_NONE;
    gint64 best_time = 0;
    int i;

    *found = NULL;
    // the root itself: a build in the tree
    best_system = build_dir_of (root, root);
    if (best_system != BUILD_NONE)
    {
        *found = g_strdup (root);
        g_free (parent);
        return best_system;
    }

    for (i = 0; places[i] != NULL; i++)
    {
        GDir *dir = g_dir_open (places[i], 0, NULL);
        const char *name;

        if (dir == NULL)
            continue;
        while ((name = g_dir_read_name (dir)) != NULL)
        {
            char *candidate = g_build_filename (places[i], name, (char *) NULL);
            build_system_t system;
            struct stat st;

            if (name[0] != '.' && g_file_test (candidate, G_FILE_TEST_IS_DIR)
                && strcmp (candidate, root) != 0
                && (system = build_dir_of (candidate, root)) != BUILD_NONE
                && stat (candidate, &st) == 0 && (gint64) st.st_mtime >= best_time)
            {
                g_free (*found);
                *found = candidate;
                best_system = system;
                best_time = (gint64) st.st_mtime;
            }
            else
                g_free (candidate);
        }
        g_dir_close (dir);
    }
    g_free (parent);
    return best_system;
}

/* --------------------------------------------------------------------------------------------- */

static guint64
build_get (const guchar *p, int size, gboolean big)
{
    guint64 v = 0;
    int i;

    for (i = 0; i < size; i++)
        v |= (guint64) p[big ? size - 1 - i : i] << (8 * i);
    return v;
}

/* --------------------------------------------------------------------------------------------- */

static gboolean
build_read_at (FILE *f, guint64 offset, void *buf, gsize size)
{
    return fseeko (f, (off_t) offset, SEEK_SET) == 0 && fread (buf, 1, size, f) == size;
}

/* --------------------------------------------------------------------------------------------- */

/* Whether the flags of a compiler name an optimization: -O1 and up, -Os, -Ofast; the last -O
   counts, as for the compiler */
static gboolean
build_producer_optimized (const char *producer)
{
    const char *p;
    gboolean optimized = FALSE;

    for (p = strstr (producer, " -O"); p != NULL; p = strstr (p + 3, " -O"))
    {
        const char c = p[3];

        optimized = (c >= '1' && c <= '9') || c == 's' || c == 'z' || c == 'f' || c == ' '
            || c == '\0';  // -O alone is -O1
    }
    return optimized;
}

/* --------------------------------------------------------------------------------------------- */

/* Whether a string of .debug_str gives the flags of a compiler: the producer of GCC, "GNU C17
   13.2.0 -mtune=generic -g -O2" ("GNU AS 2.41" being the assembler); that of clang, "clang version
   18.1.6 /usr/bin/clang -g -O2", a vendor before it; or the command line clang keeps apart when
   it is tuned for lldb, as on FreeBSD, "/usr/bin/clang -g -O2", its producer naming no flags */
static gboolean
build_compiler_flags (const char *s)
{
    const char *space;
    char *path, *program;
    gboolean compiler;

    if (g_str_has_prefix (s, "GNU "))
        return !g_str_has_prefix (s, "GNU AS ");
    if (strstr (s, "clang version ") != NULL)
        return strstr (s, " -") != NULL;

    space = strstr (s, " -");
    if (space == NULL)
        return FALSE;
    path = g_strndup (s, (gsize) (space - s));
    program = g_path_get_basename (path);
    compiler = strstr (program, "clang") != NULL || strcmp (program, "cc") == 0
        || strcmp (program, "c++") == 0;
    g_free (program);
    g_free (path);
    return compiler;
}

/* --------------------------------------------------------------------------------------------- */

/* Whether the program was built with an optimization: every string of the flags of a compiler
   names one.  The objects the C library links in may have debug information of their own, built
   with -O2, so one -O2 in .debug_str says nothing of the program.  Clang keeps its flags only with
   -grecord-command-line: without them, no optimization is seen */
static gboolean
build_flags_optimized (const guchar *text, gsize len)
{
    gsize i = 0;
    gboolean found = FALSE;

    while (i < len)
    {
        const char *s = (const char *) text + i;
        const gsize n = strnlen (s, len - i);

        if (n < len - i && build_compiler_flags (s))
        {
            if (!build_producer_optimized (s))
                return FALSE;
            found = TRUE;
        }
        i += n + 1;
    }
    return found;
}

/* --------------------------------------------------------------------------------------------- */

static gint
build_program_compare (gconstpointer a, gconstpointer b)
{
    const build_program_t *x = (const build_program_t *) a;
    const build_program_t *y = (const build_program_t *) b;

    if (x->mtime != y->mtime)
        return x->mtime > y->mtime ? -1 : 1;
    return strcmp (x->path, y->path);
}

/* --------------------------------------------------------------------------------------------- */

static gboolean
build_dir_skipped (const char *name)
{
    static const char *const skipped[] = { "meson-private", "meson-logs", "meson-info",
                                           "CMakeFiles",    "po",         "doc",
                                           "node_modules",  NULL };
    int i;

    if (name[0] == '.' || g_str_has_suffix (name, ".p") || g_str_has_suffix (name, ".dir"))
        return TRUE;
    for (i = 0; skipped[i] != NULL; i++)
        if (strcmp (name, skipped[i]) == 0)
            return TRUE;
    return FALSE;
}

/* --------------------------------------------------------------------------------------------- */

static void
build_walk_programs (const char *dir, int depth, GArray *programs)
{
    GDir *d;
    const char *name;

    if (depth > BUILD_SEARCH_DEPTH || programs->len >= BUILD_PROGRAMS_MAX)
        return;
    d = g_dir_open (dir, 0, NULL);
    if (d == NULL)
        return;
    while ((name = g_dir_read_name (d)) != NULL && programs->len < BUILD_PROGRAMS_MAX)
    {
        char *path = g_build_filename (dir, name, (char *) NULL);
        struct stat st;

        if (lstat (path, &st) != 0 || S_ISLNK (st.st_mode))
            ;
        else if (S_ISDIR (st.st_mode))
        {
            if (!build_dir_skipped (name))
                build_walk_programs (path, depth + 1, programs);
        }
        else if (S_ISREG (st.st_mode) && (st.st_mode & 0111) != 0 && st.st_size > 64)
        {
            build_elf_t elf;

            // the programs to choose from: their flags are not read, megabytes each
            if (build_elf_read_full (path, &elf, FALSE) && elf.executable && elf.debug_info)
            {
                build_program_t p = { g_strdup (path), (gint64) st.st_mtime };

                g_array_append_val (programs, p);
            }
        }
        g_free (path);
    }
    g_dir_close (d);
}

/* --------------------------------------------------------------------------------------------- */
/* JSON, enough for what meson says of a build: a value as a GVariant, an object a{sv}, a list
   av, a string s, a number x or d, true and false b, null an empty maybe; json-glib is not
   there in every build */

#define BUILD_JSON_DEPTH 64

static GVariant *build_json_value (const char **p, int depth);

static void
build_json_space (const char **p)
{
    while (**p == ' ' || **p == '\t' || **p == '\n' || **p == '\r')
        (*p)++;
}

/* --------------------------------------------------------------------------------------------- */

static int
build_json_hex4 (const char *p)
{
    int v = 0;
    int i;

    for (i = 0; i < 4; i++)
    {
        const int d = g_ascii_xdigit_value (p[i]);

        if (d < 0)
            return -1;
        v = v * 16 + d;
    }
    return v;
}

/* --------------------------------------------------------------------------------------------- */

/* The string at @p, after its opening quote; NULL when it does not end */
static char *
build_json_string_at (const char **p)
{
    GString *out = g_string_new (NULL);
    const char *s = *p;

    while (*s != '"')
    {
        if (*s == '\0')
        {
            g_string_free (out, TRUE);
            return NULL;
        }
        if (*s != '\\')
        {
            g_string_append_c (out, *s++);
            continue;
        }
        s++;
        switch (*s)
        {
        case 'b':
            g_string_append_c (out, '\b');
            break;
        case 'f':
            g_string_append_c (out, '\f');
            break;
        case 'n':
            g_string_append_c (out, '\n');
            break;
        case 'r':
            g_string_append_c (out, '\r');
            break;
        case 't':
            g_string_append_c (out, '\t');
            break;
        case 'u':
        {
            int c = build_json_hex4 (s + 1);

            if (c < 0)
            {
                g_string_free (out, TRUE);
                return NULL;
            }
            s += 4;
            // a pair of surrogates is one character
            if (c >= 0xd800 && c < 0xdc00 && s[1] == '\\' && s[2] == 'u')
            {
                const int low = build_json_hex4 (s + 3);

                if (low >= 0xdc00 && low < 0xe000)
                {
                    c = 0x10000 + ((c - 0xd800) << 10) + (low - 0xdc00);
                    s += 6;
                }
            }
            // a surrogate alone is no character, a NUL would end the string early
            if (c >= 0xd800 && c < 0xe000)
                c = 0xfffd;
            if (c != 0)
                g_string_append_unichar (out, (gunichar) c);
            break;
        }
        case '\0':
            g_string_free (out, TRUE);
            return NULL;
        default:
            // \" \\ \/
            g_string_append_c (out, *s);
            break;
        }
        s++;
    }
    *p = s + 1;
    return g_string_free (out, FALSE);
}

/* --------------------------------------------------------------------------------------------- */

static GVariant *
build_json_object (const char **p, int depth)
{
    GVariantBuilder b;

    g_variant_builder_init (&b, G_VARIANT_TYPE ("a{sv}"));
    build_json_space (p);
    if (**p == '}')
    {
        (*p)++;
        return g_variant_builder_end (&b);
    }
    while (TRUE)
    {
        char *key;
        GVariant *value;

        build_json_space (p);
        if (**p != '"')
            break;
        (*p)++;
        key = build_json_string_at (p);
        if (key == NULL)
            break;
        build_json_space (p);
        if (**p != ':')
        {
            g_free (key);
            break;
        }
        (*p)++;
        value = build_json_value (p, depth + 1);
        if (value == NULL)
        {
            g_free (key);
            break;
        }
        g_variant_builder_add (&b, "{sv}", key, value);
        g_free (key);
        build_json_space (p);
        if (**p == ',')
        {
            (*p)++;
            continue;
        }
        if (**p == '}')
        {
            (*p)++;
            return g_variant_builder_end (&b);
        }
        break;
    }
    g_variant_builder_clear (&b);
    return NULL;
}

/* --------------------------------------------------------------------------------------------- */

static GVariant *
build_json_array (const char **p, int depth)
{
    GVariantBuilder b;

    g_variant_builder_init (&b, G_VARIANT_TYPE ("av"));
    build_json_space (p);
    if (**p == ']')
    {
        (*p)++;
        return g_variant_builder_end (&b);
    }
    while (TRUE)
    {
        GVariant *value = build_json_value (p, depth + 1);

        if (value == NULL)
            break;
        g_variant_builder_add (&b, "v", value);
        build_json_space (p);
        if (**p == ',')
        {
            (*p)++;
            continue;
        }
        if (**p == ']')
        {
            (*p)++;
            return g_variant_builder_end (&b);
        }
        break;
    }
    g_variant_builder_clear (&b);
    return NULL;
}

/* --------------------------------------------------------------------------------------------- */

static GVariant *
build_json_value (const char **p, int depth)
{
    if (depth > BUILD_JSON_DEPTH)
        return NULL;
    build_json_space (p);
    switch (**p)
    {
    case '{':
        (*p)++;
        return build_json_object (p, depth);
    case '[':
        (*p)++;
        return build_json_array (p, depth);
    case '"':
    {
        char *s;

        (*p)++;
        s = build_json_string_at (p);
        return s == NULL ? NULL : g_variant_new_take_string (s);
    }
    case 't':
        if (strncmp (*p, "true", 4) != 0)
            return NULL;
        *p += 4;
        return g_variant_new_boolean (TRUE);
    case 'f':
        if (strncmp (*p, "false", 5) != 0)
            return NULL;
        *p += 5;
        return g_variant_new_boolean (FALSE);
    case 'n':
        if (strncmp (*p, "null", 4) != 0)
            return NULL;
        *p += 4;
        return g_variant_new_maybe (G_VARIANT_TYPE_VARIANT, NULL);
    default:
    {
        const char *start = *p;
        char *end;

        if (**p != '-' && !g_ascii_isdigit (**p))
            return NULL;
        while (**p == '-' || **p == '+' || g_ascii_isdigit (**p))
            (*p)++;
        if (**p == '.' || **p == 'e' || **p == 'E')
        {
            const double d = g_ascii_strtod (start, &end);

            *p = end;
            return g_variant_new_double (d);
        }
        return g_variant_new_int64 (g_ascii_strtoll (start, &end, 10));
    }
    }
}

/* --------------------------------------------------------------------------------------------- */

/* The JSON @text as a GVariant, NULL when it is none */
static GVariant *
build_json_parse (const char *text)
{
    const char *p = text;
    GVariant *value = build_json_value (&p, 0);

    if (value == NULL)
        return NULL;
    g_variant_ref_sink (value);
    build_json_space (&p);
    if (*p != '\0')
    {
        g_variant_unref (value);
        return NULL;
    }
    return value;
}

/* --------------------------------------------------------------------------------------------- */
/* The options of a build directory */

static void
build_option_free (gpointer p)
{
    build_option_t *o = (build_option_t *) p;

    g_free (o->name);
    g_free (o->section);
    g_free (o->value);
    g_free (o->default_value);
    g_strfreev (o->choices);
    g_strfreev (o->items);
    g_free (o->description);
    g_free (o);
}

/* --------------------------------------------------------------------------------------------- */

/* A value of meson as text: a list joined by ',' */
static char *
build_option_text (GVariant *v)
{
    if (g_variant_is_of_type (v, G_VARIANT_TYPE_VARIANT))
    {
        GVariant *inner = g_variant_get_variant (v);
        char *text = build_option_text (inner);

        g_variant_unref (inner);
        return text;
    }
    if (g_variant_is_of_type (v, G_VARIANT_TYPE_BOOLEAN))
        return g_strdup (g_variant_get_boolean (v) ? "true" : "false");
    if (g_variant_is_of_type (v, G_VARIANT_TYPE_INT64))
        return g_strdup_printf ("%" G_GINT64_FORMAT, g_variant_get_int64 (v));
    if (g_variant_is_of_type (v, G_VARIANT_TYPE_DOUBLE))
    {
        char buf[G_ASCII_DTOSTR_BUF_SIZE];

        return g_strdup (g_ascii_dtostr (buf, sizeof (buf), g_variant_get_double (v)));
    }
    if (g_variant_is_of_type (v, G_VARIANT_TYPE_STRING))
        return g_variant_dup_string (v, NULL);
    if (g_variant_is_of_type (v, G_VARIANT_TYPE ("av")))
    {
        GString *out = g_string_new (NULL);
        gsize i;

        for (i = 0; i < g_variant_n_children (v); i++)
        {
            GVariant *item = g_variant_get_child_value (v, i);
            char *text = build_option_text (item);

            if (i > 0)
                g_string_append_c (out, ',');
            g_string_append (out, text);
            g_free (text);
            g_variant_unref (item);
        }
        return g_string_free (out, FALSE);
    }
    return g_strdup ("");
}

/* --------------------------------------------------------------------------------------------- */

static build_option_type_t
build_option_type_of_meson (const char *type)
{
    static const struct
    {
        const char *name;
        build_option_type_t type;
    } types[] = {
        { "boolean", BUILD_OPTION_BOOLEAN }, { "combo", BUILD_OPTION_COMBO },
        { "integer", BUILD_OPTION_INTEGER }, { "array", BUILD_OPTION_ARRAY },
        { "feature", BUILD_OPTION_FEATURE },
    };
    size_t i;

    for (i = 0; type != NULL && i < G_N_ELEMENTS (types); i++)
        if (strcmp (type, types[i].name) == 0)
            return types[i].type;
    return BUILD_OPTION_STRING;
}

/* --------------------------------------------------------------------------------------------- */

/* A string of the object @o of meson, NULL when it has none */
static char *
build_meson_string (GVariant *o, const char *key)
{
    const char *s = NULL;

    return g_variant_lookup (o, key, "&s", &s) ? g_strdup (s) : NULL;
}

/* --------------------------------------------------------------------------------------------- */

/* The options in @json, what meson-info/intro-buildoptions.json has; NULL when it has none */
static GPtrArray *
build_options_of_meson_json (const char *json)
{
    GVariant *list = build_json_parse (json);
    GPtrArray *options;
    gsize i;

    if (list == NULL || !g_variant_is_of_type (list, G_VARIANT_TYPE ("av")))
    {
        if (list != NULL)
            g_variant_unref (list);
        return NULL;
    }
    options = g_ptr_array_new_with_free_func (build_option_free);
    for (i = 0; i < g_variant_n_children (list); i++)
    {
        GVariant *item = g_variant_get_child_value (list, i);
        GVariant *o = g_variant_get_variant (item);
        GVariant *value, *choices;
        build_option_t *opt;
        char *type;

        g_variant_unref (item);
        if (!g_variant_is_of_type (o, G_VARIANT_TYPE ("a{sv}")))
        {
            g_variant_unref (o);
            continue;
        }
        opt = g_new0 (build_option_t, 1);
        opt->name = build_meson_string (o, "name");
        if (opt->name == NULL)
        {
            g_free (opt);
            g_variant_unref (o);
            continue;
        }
        opt->section = build_meson_string (o, "section");
        opt->description = build_meson_string (o, "description");
        type = build_meson_string (o, "type");
        opt->type = build_option_type_of_meson (type);
        g_free (type);
        value = g_variant_lookup_value (o, "value", NULL);
        opt->value = value != NULL ? build_option_text (value) : g_strdup ("");
        // a list keeps its items: one may have a ',' in it
        if (value != NULL)
        {
            GVariant *inner = g_variant_is_of_type (value, G_VARIANT_TYPE_VARIANT)
                ? g_variant_get_variant (value)
                : g_variant_ref (value);

            if (g_variant_is_of_type (inner, G_VARIANT_TYPE ("av")))
            {
                const gsize n = g_variant_n_children (inner);
                gsize j;

                opt->items = g_new0 (char *, n + 1);
                for (j = 0; j < n; j++)
                {
                    GVariant *child = g_variant_get_child_value (inner, j);

                    opt->items[j] = build_option_text (child);
                    g_variant_unref (child);
                }
            }
            g_variant_unref (inner);
            g_variant_unref (value);
        }
        choices = g_variant_lookup_value (o, "choices", G_VARIANT_TYPE ("av"));
        if (choices != NULL)
        {
            gsize n = g_variant_n_children (choices), j;

            opt->choices = g_new0 (char *, n + 1);
            for (j = 0; j < n; j++)
            {
                GVariant *c = g_variant_get_child_value (choices, j);

                opt->choices[j] = build_option_text (c);
                g_variant_unref (c);
            }
            g_variant_unref (choices);
        }
        g_ptr_array_add (options, opt);
        g_variant_unref (o);
    }
    g_variant_unref (list);
    return options;
}

/* --------------------------------------------------------------------------------------------- */

static GPtrArray *
build_options_read_meson (const char *dir)
{
    char *path = g_build_filename (dir, "meson-info", "intro-buildoptions.json", (char *) NULL);
    char *json = build_read_text (path);
    GPtrArray *options;
    GKeyFile *given;

    g_free (path);
    if (json == NULL)
        return NULL;
    options = build_options_of_meson_json (json);
    g_free (json);
    if (options == NULL)
        return NULL;

    // what was given to setup and configure: meson keeps it for the next setup
    path = g_build_filename (dir, "meson-private", "cmd_line.txt", (char *) NULL);
    given = g_key_file_new ();
    if (g_key_file_load_from_file (given, path, G_KEY_FILE_NONE, NULL))
    {
        guint i;

        for (i = 0; i < options->len; i++)
        {
            build_option_t *o = (build_option_t *) g_ptr_array_index (options, i);

            o->set_by_user = g_key_file_has_key (given, "options", o->name, NULL);
        }
    }
    g_key_file_free (given);
    g_free (path);
    return options;
}

/* --------------------------------------------------------------------------------------------- */

static build_option_t *
build_options_find_rw (GPtrArray *options, const char *name)
{
    guint i;

    for (i = 0; i < options->len; i++)
    {
        build_option_t *o = (build_option_t *) g_ptr_array_index (options, i);

        if (strcmp (o->name, name) == 0)
            return o;
    }
    return NULL;
}

/* --------------------------------------------------------------------------------------------- */

/* CMakeCache.txt: "NAME:TYPE=VALUE" with the lines "//" of its help before it; the INTERNAL
   entries NAME-ADVANCED and NAME-STRINGS say which are advanced and what a STRING takes */
static GPtrArray *
build_options_read_cmake (const char *dir)
{
    static const char *const cmake_given[] = {
        "CMAKE_BUILD_TYPE",
        "CMAKE_C_FLAGS",
        "CMAKE_CXX_FLAGS",
        "CMAKE_EXE_LINKER_FLAGS",
        "CMAKE_SHARED_LINKER_FLAGS",
        "CMAKE_INSTALL_PREFIX",
        NULL,
    };
    char *path = g_build_filename (dir, "CMakeCache.txt", (char *) NULL);
    char *text = build_read_text (path);
    GPtrArray *options;
    GHashTable *advanced, *strings;
    GString *help;
    char **lines, **l;
    guint i;

    g_free (path);
    if (text == NULL)
        return NULL;
    options = g_ptr_array_new_with_free_func (build_option_free);
    advanced = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
    strings = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
    help = g_string_new (NULL);
    lines = g_strsplit (text, "\n", -1);
    g_free (text);
    for (l = lines; *l != NULL; l++)
    {
        char *line = g_strchomp (*l);
        char *name, *type, *value, *colon, *equal;

        if (g_str_has_prefix (line, "//"))
        {
            if (help->len > 0)
                g_string_append_c (help, ' ');
            g_string_append (help, g_strstrip (line + 2));
            continue;
        }
        if (line[0] == '\0' || line[0] == '#')
        {
            g_string_truncate (help, 0);
            continue;
        }
        // a name with odd characters is quoted
        if (line[0] == '"')
        {
            char *close = strchr (line + 1, '"');

            if (close == NULL)
                continue;
            name = g_strndup (line + 1, (gsize) (close - line - 1));
            colon = close + 1;
            if (*colon != ':')
            {
                g_free (name);
                continue;
            }
        }
        else
        {
            colon = strchr (line, ':');
            if (colon == NULL)
                continue;
            name = g_strndup (line, (gsize) (colon - line));
        }
        equal = strchr (colon, '=');
        if (equal == NULL)
        {
            g_free (name);
            continue;
        }
        type = g_strndup (colon + 1, (gsize) (equal - colon - 1));
        value = equal + 1;

        if (strcmp (type, "INTERNAL") == 0)
        {
            if (g_str_has_suffix (name, "-ADVANCED"))
            {
                if (strcmp (value, "1") == 0)
                    g_hash_table_add (advanced,
                                      g_strndup (name, strlen (name) - strlen ("-ADVANCED")));
            }
            else if (g_str_has_suffix (name, "-STRINGS"))
                g_hash_table_replace (strings,
                                      g_strndup (name, strlen (name) - strlen ("-STRINGS")),
                                      g_strdup (value));
        }
        else if (strcmp (type, "STATIC") != 0)
        {
            build_option_t *o = g_new0 (build_option_t, 1);

            o->name = name;
            name = NULL;
            o->section = g_strdup (g_str_has_prefix (o->name, "CMAKE_") ? "cmake" : "user");
            o->type = strcmp (type, "BOOL") == 0 ? BUILD_OPTION_BOOLEAN
                : strcmp (type, "PATH") == 0 || strcmp (type, "FILEPATH") == 0
                ? BUILD_OPTION_PATH
                : BUILD_OPTION_STRING;
            o->value = g_strdup (value);
            o->description = help->len > 0 ? g_strdup (help->str) : NULL;
            g_ptr_array_add (options, o);
        }
        g_string_truncate (help, 0);
        g_free (name);
        g_free (type);
    }
    g_strfreev (lines);
    g_string_free (help, TRUE);

    for (i = 0; i < options->len; i++)
    {
        build_option_t *o = (build_option_t *) g_ptr_array_index (options, i);
        const char *choices = (const char *) g_hash_table_lookup (strings, o->name);

        o->advanced = g_hash_table_contains (advanced, o->name);
        // an empty list of choices is none: any value goes
        if (choices != NULL && *choices != '\0' && o->type == BUILD_OPTION_STRING)
        {
            o->type = BUILD_OPTION_COMBO;
            o->choices = g_strsplit (choices, ";", -1);
        }
        /* the cache says not what was given: what sets another directory up the same is the
           build type and the flags given, and the options of the project, not the paths CMake
           found */
        if (g_str_has_prefix (o->name, "CMAKE_"))
            o->set_by_user = *o->value != '\0' && g_strv_contains (cmake_given, o->name);
        else
            o->set_by_user = !o->advanced && o->type != BUILD_OPTION_PATH;
    }
    g_hash_table_destroy (advanced);
    g_hash_table_destroy (strings);
    return options;
}

/* --------------------------------------------------------------------------------------------- */

/* The words of the flags of cmake that make coverage or a sanitizer */
static void
build_summary_of_flags (build_summary_t *s, const char *flags, GPtrArray *sanitizers)
{
    char **words;
    char **w;

    if (flags == NULL)
        return;
    words = g_strsplit_set (flags, " \t", -1);
    for (w = words; *w != NULL; w++)
    {
        if (strcmp (*w, "--coverage") == 0 || strcmp (*w, "-fprofile-arcs") == 0
            || strcmp (*w, "-fprofile-instr-generate") == 0)
            s->coverage = TRUE;
        else if (g_str_has_prefix (*w, "-fsanitize="))
        {
            char **names = g_strsplit (*w + strlen ("-fsanitize="), ",", -1);
            char **n;

            for (n = names; *n != NULL; n++)
            {
                guint i;
                gboolean known = FALSE;

                for (i = 0; i < sanitizers->len && !known; i++)
                    known = strcmp (g_ptr_array_index (sanitizers, i), *n) == 0;
                if (**n != '\0' && !known)
                    g_ptr_array_add (sanitizers, g_strdup (*n));
            }
            g_strfreev (names);
        }
    }
    g_strfreev (words);
}

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

build_info_t *
build_detect (const char *root)
{
    build_info_t *info;
    char *dir = NULL;
    build_system_t system;
    char *makefile;

    if (root == NULL)
        return NULL;
    system = build_find_dir (root, &dir);
    info = g_new0 (build_info_t, 1);
    info->system = system;
    switch (system)
    {
    case BUILD_MESON:
    {
        char *quoted = g_shell_quote (dir);

        info->build_dir = dir;
        info->command = g_strdup_printf ("meson compile -C %s", quoted);
        g_free (quoted);
        return info;
    }
    case BUILD_CMAKE:
    {
        char *quoted = g_shell_quote (dir);

        info->build_dir = dir;
        info->command = g_strdup_printf ("cmake --build %s", quoted);
        g_free (quoted);
        return info;
    }
    default:
        break;
    }
    g_free (dir);

    makefile = g_build_filename (root, "Makefile", (char *) NULL);
    if (g_file_test (makefile, G_FILE_TEST_IS_REGULAR))
    {
        info->system = BUILD_MAKE;
        info->build_dir = g_strdup (root);
        info->command = g_strdup ("make");
        g_free (makefile);
        return info;
    }
    g_free (makefile);
    g_free (info);
    return NULL;
}

/* --------------------------------------------------------------------------------------------- */

void
build_info_free (build_info_t *info)
{
    if (info == NULL)
        return;
    g_free (info->build_dir);
    g_free (info->command);
    g_free (info);
}

/* --------------------------------------------------------------------------------------------- */

/* The ELF @path; with @flags, the flags of the compiler too, which takes reading its strings */
static gboolean
build_elf_read_full (const char *path, build_elf_t *elf, gboolean flags)
{
    guchar head[64];
    size_t got;
    FILE *f;
    gboolean wide, big;
    guint64 phoff, shoff;
    guint phentsize, phnum, shentsize, shnum, shstrndx, type, i;
    guchar *names = NULL;
    guint64 names_off = 0, names_size = 0;
    gboolean ok = FALSE;

    memset (elf, 0, sizeof (*elf));
    f = fopen (path, "rb");
    if (f == NULL)
        return FALSE;
    got = fread (head, 1, sizeof (head), f);
    if (got < 52 || memcmp (head, "\177ELF", 4) != 0)
        goto out;

    wide = head[4] == 2;
    // the header of 64 bits is 64 bytes long
    if (wide && got < 64)
        goto out;
    big = head[5] == 2;
    type = (guint) build_get (head + 16, 2, big);
    if (wide)
    {
        phoff = build_get (head + 32, 8, big);
        shoff = build_get (head + 40, 8, big);
        phentsize = (guint) build_get (head + 54, 2, big);
        phnum = (guint) build_get (head + 56, 2, big);
        shentsize = (guint) build_get (head + 58, 2, big);
        shnum = (guint) build_get (head + 60, 2, big);
        shstrndx = (guint) build_get (head + 62, 2, big);
    }
    else
    {
        phoff = build_get (head + 28, 4, big);
        shoff = build_get (head + 32, 4, big);
        phentsize = (guint) build_get (head + 42, 2, big);
        phnum = (guint) build_get (head + 44, 2, big);
        shentsize = (guint) build_get (head + 46, 2, big);
        shnum = (guint) build_get (head + 48, 2, big);
        shstrndx = (guint) build_get (head + 50, 2, big);
    }
    ok = TRUE;

    // a program: ET_EXEC, or ET_DYN with an interpreter (a PIE, not a library)
    elf->executable = type == 2;
    if (type == 3)
        for (i = 0; i < phnum && i < 256; i++)
        {
            guchar ph[4];

            if (build_read_at (f, phoff + (guint64) i * phentsize, ph, sizeof (ph))
                && build_get (ph, 4, big) == 3)
                elf->executable = TRUE;
        }

    // the names of the sections
    if (shstrndx < shnum && shentsize >= (wide ? 40u : 40u))
    {
        guchar sh[64];

        if (build_read_at (f, shoff + (guint64) shstrndx * shentsize, sh, MIN (shentsize, 64)))
        {
            names_off = wide ? build_get (sh + 24, 8, big) : build_get (sh + 16, 4, big);
            names_size = wide ? build_get (sh + 32, 8, big) : build_get (sh + 20, 4, big);
            if (names_size > 0 && names_size < 1024 * 1024)
            {
                names = g_malloc (names_size + 1);
                if (!build_read_at (f, names_off, names, names_size))
                    g_clear_pointer (&names, g_free);
                else
                    names[names_size] = '\0';
            }
        }
    }

    for (i = 0; names != NULL && i < shnum && i < 4096; i++)
    {
        guchar sh[64];
        guint64 name, offset, size;
        const char *section;

        if (!build_read_at (f, shoff + (guint64) i * shentsize, sh, MIN (shentsize, 64)))
            break;
        name = build_get (sh, 4, big);
        if (name >= names_size)
            continue;
        section = (const char *) names + name;
        offset = wide ? build_get (sh + 24, 8, big) : build_get (sh + 16, 4, big);
        size = wide ? build_get (sh + 32, 8, big) : build_get (sh + 20, 4, big);
        if (strcmp (section, ".debug_info") == 0)
            elf->debug_info = TRUE;
        else if (flags && strcmp (section, ".debug_str") == 0 && size > 0)
        {
            // the producer strings name the flags of the compiler
            const gsize len = (gsize) MIN (size, BUILD_DEBUG_STR_MAX);
            guchar *text = g_malloc (len);

            if (build_read_at (f, offset, text, len))
                elf->optimized = build_flags_optimized (text, len);
            g_free (text);
        }
    }

out:
    g_free (names);
    fclose (f);
    return ok;
}

/* --------------------------------------------------------------------------------------------- */

gboolean
build_elf_read (const char *path, build_elf_t *elf)
{
    return build_elf_read_full (path, elf, TRUE);
}

/* --------------------------------------------------------------------------------------------- */

GPtrArray *
build_find_programs (const char *dir)
{
    GArray *programs = g_array_new (FALSE, FALSE, sizeof (build_program_t));
    GPtrArray *result = g_ptr_array_new_with_free_func (g_free);
    guint i;

    if (dir != NULL)
        build_walk_programs (dir, 0, programs);
    g_array_sort (programs, build_program_compare);
    for (i = 0; i < programs->len; i++)
        g_ptr_array_add (result, g_array_index (programs, build_program_t, i).path);
    g_array_free (programs, TRUE);
    return result;
}

/* --------------------------------------------------------------------------------------------- */

build_diagnostic_t *
build_parse_diagnostic (const char *line, const char *dir)
{
    static GRegex *re = NULL, *bare = NULL;
    GMatchInfo *match = NULL;
    build_diagnostic_t *d = NULL;
    char *file = NULL, *line_text = NULL, *column = NULL, *kind = NULL, *message = NULL;

    if (line == NULL)
        return NULL;
    if (re == NULL)
        re = g_regex_new ("^([^:\\s][^:]*):(\\d+):(?:(\\d+):)?\\s*(fatal error|error|warning|note)"
                          // the code of rustc: error[E0425]
                          "(?:\\[\\w+\\])?:\\s*(.*)$",
                          G_REGEX_OPTIMIZE, 0, NULL);
    // go: "./main.go:12:2: undefined: x", with no kind; the context of gcc, "a.cc:5:10:   required
    // from here", has more than a space before its text
    if (bare == NULL)
        bare = g_regex_new ("^([^:\\s][^:]*):(\\d+):(\\d+): (\\S.*)$", G_REGEX_OPTIMIZE, 0, NULL);
    if (re != NULL && g_regex_match (re, line, 0, &match))
    {
        file = g_match_info_fetch (match, 1);
        line_text = g_match_info_fetch (match, 2);
        column = g_match_info_fetch (match, 3);
        kind = g_match_info_fetch (match, 4);
        message = g_match_info_fetch (match, 5);
    }
    else if (bare != NULL)
    {
        g_match_info_free (match);
        match = NULL;
        if (g_regex_match (bare, line, 0, &match))
        {
            file = g_match_info_fetch (match, 1);
            line_text = g_match_info_fetch (match, 2);
            column = g_match_info_fetch (match, 3);
            message = g_match_info_fetch (match, 4);
        }
    }
    g_match_info_free (match);
    if (file != NULL)
    {
        char *path = g_path_is_absolute (file) || dir == NULL ? g_canonicalize_filename (file, NULL)
                                                              : g_canonicalize_filename (file, dir);

        // a line of no kind is an error only of a file there, not any text with colons
        if (kind != NULL || g_file_test (path, G_FILE_TEST_IS_REGULAR))
        {
            d = g_new0 (build_diagnostic_t, 1);
            d->file = path;
            d->line = atol (line_text);
            d->column = column != NULL && *column != '\0' ? atol (column) : 0;
            d->error = kind == NULL || strstr (kind, "error") != NULL;
            d->message = message;
            path = NULL;
            message = NULL;
        }
        g_free (path);
    }
    g_free (file);
    g_free (line_text);
    g_free (column);
    g_free (kind);
    g_free (message);
    return d;
}

/* --------------------------------------------------------------------------------------------- */

void
build_diagnostic_free (build_diagnostic_t *d)
{
    if (d == NULL)
        return;
    g_free (d->file);
    g_free (d->message);
    g_free (d);
}

/* --------------------------------------------------------------------------------------------- */

GPtrArray *
build_options_read (const char *dir, build_system_t system)
{
    if (dir == NULL)
        return NULL;
    switch (system)
    {
    case BUILD_MESON:
        return build_options_read_meson (dir);
    case BUILD_CMAKE:
        return build_options_read_cmake (dir);
    default:
        return NULL;
    }
}

/* --------------------------------------------------------------------------------------------- */

gboolean
build_options_set_defaults (GPtrArray *options, const char *json)
{
    GPtrArray *defaults;
    guint i;

    if (options == NULL || json == NULL)
        return FALSE;
    defaults = build_options_of_meson_json (json);
    if (defaults == NULL)
        return FALSE;
    for (i = 0; i < defaults->len; i++)
    {
        const build_option_t *d = (const build_option_t *) g_ptr_array_index (defaults, i);
        build_option_t *o = build_options_find_rw (options, d->name);

        if (o != NULL)
        {
            g_free (o->default_value);
            o->default_value = g_strdup (d->value);
        }
    }
    g_ptr_array_free (defaults, TRUE);
    return TRUE;
}

/* --------------------------------------------------------------------------------------------- */

GPtrArray *
build_options_of_source (const char *json)
{
    GPtrArray *options = json != NULL ? build_options_of_meson_json (json) : NULL;
    guint i;

    for (i = 0; options != NULL && i < options->len; i++)
    {
        build_option_t *o = (build_option_t *) g_ptr_array_index (options, i);

        o->default_value = g_strdup (o->value);
    }
    return options;
}

/* --------------------------------------------------------------------------------------------- */

const char *
build_option_toggled (const char *value)
{
    static const char *const pairs[][2] = {
        { "true", "false" }, { "ON", "OFF" }, { "on", "off" }, { "TRUE", "FALSE" },
        { "YES", "NO" },     { "yes", "no" }, { "1", "0" },
    };
    gsize i;

    for (i = 0; i < G_N_ELEMENTS (pairs); i++)
    {
        if (strcmp (value, pairs[i][0]) == 0)
            return pairs[i][1];
        if (strcmp (value, pairs[i][1]) == 0)
            return pairs[i][0];
    }
    return "true";
}

/* --------------------------------------------------------------------------------------------- */

const build_option_t *
build_options_find (const GPtrArray *options, const char *name)
{
    return options == NULL || name == NULL ? NULL
                                           : build_options_find_rw ((GPtrArray *) options, name);
}

/* --------------------------------------------------------------------------------------------- */

char *
build_option_meson_list (const char *const *items)
{
    GString *out = g_string_new ("[");
    const char *const *i;

    for (i = items; i != NULL && *i != NULL; i++)
    {
        const char *c;

        if (i != items)
            g_string_append (out, ", ");
        g_string_append_c (out, '\'');
        for (c = *i; *c != '\0'; c++)
        {
            if (*c == '\'' || *c == '\\')
                g_string_append_c (out, '\\');
            g_string_append_c (out, *c);
        }
        g_string_append_c (out, '\'');
    }
    g_string_append_c (out, ']');
    return g_string_free (out, FALSE);
}

/* --------------------------------------------------------------------------------------------- */

char *
build_option_arg (const build_option_t *o)
{
    if (o->type == BUILD_OPTION_ARRAY && o->items != NULL)
        return build_option_meson_list ((const char *const *) o->items);
    return g_strdup (o->value);
}

/* --------------------------------------------------------------------------------------------- */

build_summary_t *
build_summary_of (const GPtrArray *options, build_system_t system)
{
    build_summary_t *s = g_new0 (build_summary_t, 1);
    const build_option_t *o;

    if (system == BUILD_MESON)
    {
        o = build_options_find (options, "buildtype");
        if (o != NULL && o->value[0] != '\0')
            s->buildtype = g_strdup (o->value);
        o = build_options_find (options, "b_coverage");
        s->coverage = o != NULL && strcmp (o->value, "true") == 0;
        // a word until meson 1.7, a list from 1.8: "address,undefined" either way
        o = build_options_find (options, "b_sanitize");
        if (o != NULL && o->value[0] != '\0' && strcmp (o->value, "none") != 0)
            s->sanitize = g_strdup (o->value);
    }
    else if (system == BUILD_CMAKE)
    {
        static const char *const flags[] = {
            "CMAKE_C_FLAGS",
            "CMAKE_CXX_FLAGS",
            "CMAKE_EXE_LINKER_FLAGS",
            "CMAKE_SHARED_LINKER_FLAGS",
        };
        GPtrArray *sanitizers = g_ptr_array_new_with_free_func (g_free);
        size_t i;

        o = build_options_find (options, "CMAKE_BUILD_TYPE");
        if (o != NULL && o->value[0] != '\0')
            s->buildtype = g_ascii_strdown (o->value, -1);
        for (i = 0; i < G_N_ELEMENTS (flags); i++)
        {
            o = build_options_find (options, flags[i]);
            build_summary_of_flags (s, o != NULL ? o->value : NULL, sanitizers);
            // the flags of the build type too: CMAKE_C_FLAGS_DEBUG
            if (s->buildtype != NULL)
            {
                char *upper = g_ascii_strup (s->buildtype, -1);
                char *name = g_strconcat (flags[i], "_", upper, (char *) NULL);

                o = build_options_find (options, name);
                build_summary_of_flags (s, o != NULL ? o->value : NULL, sanitizers);
                g_free (name);
                g_free (upper);
            }
        }
        if (sanitizers->len > 0)
        {
            g_ptr_array_add (sanitizers, NULL);
            s->sanitize = g_strjoinv (",", (char **) sanitizers->pdata);
        }
        g_ptr_array_free (sanitizers, TRUE);
    }
    return s;
}

/* --------------------------------------------------------------------------------------------- */

/* Whether @dir or a directory under it, @depth levels down, has a .gcda file */
static gboolean
build_coverage_data_in (const char *dir, int depth)
{
    GDir *d = g_dir_open (dir, 0, NULL);
    const char *name;
    gboolean found = FALSE;

    if (d == NULL)
        return FALSE;
    while (!found && (name = g_dir_read_name (d)) != NULL)
    {
        if (g_str_has_suffix (name, ".gcda"))
            found = TRUE;
        else if (depth > 0 && name[0] != '.')
        {
            char *path = g_build_filename (dir, name, (char *) NULL);

            if (g_file_test (path, G_FILE_TEST_IS_DIR)
                && !g_file_test (path, G_FILE_TEST_IS_SYMLINK))
                found = build_coverage_data_in (path, depth - 1);
            g_free (path);
        }
    }
    g_dir_close (d);
    return found;
}

/* --------------------------------------------------------------------------------------------- */

gboolean
build_coverage_data (const char *dir)
{
    // the objects of meson are in <target>.p/ under the directories of the sources
    return dir != NULL && build_coverage_data_in (dir, 6);
}

/* --------------------------------------------------------------------------------------------- */

char *
build_summary_text (const build_summary_t *summary)
{
    GString *out = g_string_new (NULL);

    if (summary == NULL)
        return g_string_free (out, FALSE);
    if (summary->buildtype != NULL)
        g_string_append (out, summary->buildtype);
    if (summary->coverage)
        g_string_append_printf (out, "%s%s", out->len > 0 ? ", " : "",
                                summary->coverage_missing ? _ ("coverage, no data") : "coverage");
    if (summary->sanitize != NULL)
        g_string_append_printf (out, "%s%s", out->len > 0 ? ", " : "", summary->sanitize);
    return g_string_free (out, FALSE);
}

/* --------------------------------------------------------------------------------------------- */

void
build_summary_free (build_summary_t *summary)
{
    if (summary == NULL)
        return;
    g_free (summary->buildtype);
    g_free (summary->sanitize);
    g_free (summary);
}

/* --------------------------------------------------------------------------------------------- */
