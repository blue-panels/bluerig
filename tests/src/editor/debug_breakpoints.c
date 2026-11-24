/*
   src/plugins/debugger - tests for the breakpoints of the debugger: their marks, the answers of
   the debugger, the lines they follow, and the exceptions the program stops on

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

#include "src/plugins/debugger/debug-breakpoints.c"  // for its file scope functions

/* --------------------------------------------------------------------------------------------- */
/* The rest of the plugin, as much as the breakpoints ask of it */

static gboolean live;          // a session runs, the program loaded
static int saved;              // debug_config_save ()
static GString *console;       // what the console was told
static GString *sent;          // what the backend was asked, a line each
static GString *marks;         // the marks put in the gutter, a line each
static GHashTable *mark_rows;  // "file mark" -> GArray of long: what marker_lines () gives

void
debug_error (debugger_t *debug, const char *message_text)
{
    (void) debug;
    g_string_append_printf (console, "error: %s\n", message_text);
}

gboolean
debug_alive (const debugger_t *debug)
{
    return debug->backend != NULL && live;
}

gboolean
debug_session_live (const debugger_t *debug)
{
    return debug_alive (debug);
}

debug_launch_t *
debug_active_launch (const debugger_t *debug)
{
    return debug->active_launch < debug->launches->len
        ? g_ptr_array_index (debug->launches, debug->active_launch)
        : NULL;
}

void
debug_config_save (debugger_t *debug)
{
    (void) debug;
    saved++;
}

gboolean
debug_require_project (debugger_t *debug, void *edit)
{
    (void) debug;
    (void) edit;
    return TRUE;
}

gboolean
debug_launch_is_dap (const debug_launch_t *launch)
{
    return g_strcmp0 (launch->backend, "dap") == 0;
}

void
debug_session_refresh (debugger_t *debug)
{
    (void) debug;
}

void
debug_output_console (debugger_t *debug, const char *text_value, gboolean line)
{
    (void) debug;
    g_string_append (console, text_value);
    if (line)
        g_string_append_c (console, '\n');
}

/* --------------------------------------------------------------------------------------------- */
/* A debugger that says what it was asked */

static guint next_token;
static GPtrArray *offered;  // the kinds of exceptions it offers

static guint
fake_break_insert (debug_backend_t *b, const char *file, long line, gboolean disabled,
                   const char *condition, debug_reply_cb cb, void *data, GDestroyNotify free_data)
{
    (void) b;
    (void) data;
    g_assert_true (cb == debug_reply_breakpoint);
    g_assert_null (free_data);
    g_string_append_printf (sent, "insert %s:%ld%s%s%s\n", file, line, disabled ? " off" : "",
                            condition != NULL ? " if " : "", condition != NULL ? condition : "");
    return ++next_token;
}

static guint
fake_break_delete (debug_backend_t *b, const char *id)
{
    (void) b;
    g_string_append_printf (sent, "delete %s\n", id);
    return ++next_token;
}

static guint
fake_break_enable (debug_backend_t *b, const char *id, gboolean enable)
{
    (void) b;
    g_string_append_printf (sent, "%s %s\n", enable ? "enable" : "disable", id);
    return ++next_token;
}

static GPtrArray *
fake_exception_filters (const debug_backend_t *b)
{
    (void) b;
    return g_ptr_array_ref (offered);
}

static guint
fake_break_exceptions (debug_backend_t *b, const char *const *ids)
{
    (void) b;
    if (ids == NULL)
        g_string_append (sent, "exceptions default\n");
    else
    {
        char *joined = g_strjoinv (",", (char **) ids);

        g_string_append_printf (sent, "exceptions [%s]\n", joined);
        g_free (joined);
    }
    return ++next_token;
}

static const debug_backend_ops_t fake_ops = {
    .name = "fake",
    .break_insert = fake_break_insert,
    .break_delete = fake_break_delete,
    .break_enable = fake_break_enable,
    .exception_filters = fake_exception_filters,
    .break_exceptions = fake_break_exceptions,
};

static debug_backend_t fake_backend = { .ops = &fake_ops };

/* --------------------------------------------------------------------------------------------- */
/* The editor: its gutter, and the plugin ctags by its service */

static GVariant *symbols;  // what the service "ctags" answers

static void
host_set_marker (mc_editor_host_t *host, const char *file, long line, int kind, gboolean on)
{
    (void) host;
    g_assert_true (on);
    g_string_append_printf (marks, "%s:%ld %d\n", file, line, kind);
}

static void
host_clear_markers (mc_editor_host_t *host, const char *file, int kind)
{
    (void) host;
    (void) kind;
    // a file at a time, or all of them
    g_assert_true (file == NULL || *file == '/');
}

static GArray *
host_marker_lines (mc_editor_host_t *host, const char *file, int kind)
{
    char *key = g_strdup_printf ("%s %d", file, kind);
    const GArray *rows = g_hash_table_lookup (mark_rows, key);
    GArray *copy = NULL;

    (void) host;
    g_free (key);
    if (rows != NULL)
    {
        copy = g_array_new (FALSE, FALSE, sizeof (long));
        g_array_append_vals (copy, rows->data, rows->len);
    }
    return copy;
}

static GVariant *
host_service_call (mc_editor_host_t *host, const char *name, const char *method, GVariant *args,
                   GError **error)
{
    const char *query = NULL;

    (void) host;
    (void) error;
    g_variant_ref_sink (args);
    g_assert_cmpstr (name, ==, "ctags");
    g_assert_cmpstr (method, ==, "symbols");
    g_assert_true (g_variant_lookup (args, "query", "&s", &query));
    g_assert_cmpstr (query, ==, "ma");
    g_variant_unref (args);
    return symbols != NULL ? g_variant_ref (symbols) : NULL;
}

static mc_editor_host_t host = {
    .set_marker = host_set_marker,
    .clear_markers = host_clear_markers,
    .marker_lines = host_marker_lines,
    .service_call = host_service_call,
};

/* --------------------------------------------------------------------------------------------- */

static debugger_t *debug;

/* The marks are numbered by their kinds plus 100, to be told from the kinds */
#define MARK(kind) (100 + (kind))

static void
setup (void)
{
    int i;

    live = FALSE;
    saved = 0;
    next_token = 0;
    console = g_string_new (NULL);
    sent = g_string_new (NULL);
    marks = g_string_new (NULL);
    mark_rows =
        g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify) g_array_unref);
    offered = g_ptr_array_new_with_free_func (debug_exception_filter_free);
    debug = g_new0 (debugger_t, 1);
    debug->host = &host;
    debug->breakpoints = g_ptr_array_new_with_free_func (debug_breakpoint_free);
    debug->launches = g_ptr_array_new ();
    debug->dropped_tokens = g_array_new (FALSE, FALSE, sizeof (unsigned int));
    for (i = 0; i < DEBUG_MARK_COUNT; i++)
        debug->marks[i] = MARK (i);
}

static void
teardown (void)
{
    g_ptr_array_free (debug->breakpoints, TRUE);
    g_ptr_array_free (debug->launches, TRUE);
    g_array_free (debug->dropped_tokens, TRUE);
    if (debug->session_filters != NULL)
        g_ptr_array_unref (debug->session_filters);
    g_free (debug->current_file);
    g_free (debug);
    g_string_free (console, TRUE);
    g_string_free (sent, TRUE);
    g_string_free (marks, TRUE);
    g_hash_table_unref (mark_rows);
    g_ptr_array_unref (offered);
    g_clear_pointer (&symbols, g_variant_unref);
}

static debug_breakpoint_t *
bp_new (const char *file, long line)
{
    debug_breakpoint_t *bp = g_new0 (debug_breakpoint_t, 1);

    bp->file = g_strdup (file);
    bp->line = line;
    g_ptr_array_add (debug->breakpoints, bp);
    return bp;
}

static debug_breakpoint_t *
bp_at (guint i)
{
    g_assert_cmpuint (i, <, debug->breakpoints->len);
    return g_ptr_array_index (debug->breakpoints, i);
}

/* The lines the gutter of @file has the marks of @kind on */
static void
gutter (const char *file, int kind, const long *lines, guint count)
{
    GArray *rows = g_array_new (FALSE, FALSE, sizeof (long));

    g_array_append_vals (rows, lines, count);
    g_hash_table_insert (mark_rows, g_strdup_printf ("%s %d", file, MARK (kind)), rows);
}

static void
start_session (void)
{
    debug->backend = &fake_backend;
    live = TRUE;
}

/* --------------------------------------------------------------------------------------------- */

/* Two names of one file: as they are, or by the real path */
static void
test_same_file (void)
{
    char *dir = g_dir_make_tmp ("debug-bp-XXXXXX", NULL);
    char *real = g_build_filename (dir, "a.c", (char *) NULL);
    char *link = g_build_filename (dir, "link.c", (char *) NULL);
    char *other = g_build_filename (dir, "b.c", (char *) NULL);

    g_assert_true (g_file_set_contents (real, "", 0, NULL));
    g_assert_true (g_file_set_contents (other, "", 0, NULL));
    g_assert_cmpint (symlink (real, link), ==, 0);
    g_assert_true (debug_same_file ("/no/such.c", "/no/such.c"));
    g_assert_true (debug_same_file (real, link));
    g_assert_false (debug_same_file (real, other));
    g_assert_false (debug_same_file ("/no/such.c", "/no/other.c"));
    g_assert_false (debug_same_file (real, NULL));
    g_assert_false (debug_same_file (NULL, NULL));
    (void) g_remove (link);
    (void) g_remove (real);
    (void) g_remove (other);
    (void) g_rmdir (dir);
    g_free (link);
    g_free (real);
    g_free (other);
    g_free (dir);
}

/* The mark of a breakpoint: disabled, waiting for the debugger, with a condition, or plain */
static void
test_marks (void)
{
    debug_breakpoint_t *a, *b, *c;

    setup ();
    a = bp_new ("/src/a.c", 3);
    b = bp_new ("/src/a.c", 7);
    b->condition = g_strdup ("x > 1");
    c = bp_new ("/src/b.c", 1);
    c->disabled = TRUE;
    g_assert_cmpint (debug_breakpoint_mark (debug, a), ==, DEBUG_MARK_BREAKPOINT);
    g_assert_cmpint (debug_breakpoint_mark (debug, b), ==, DEBUG_MARK_CONDITION);
    g_assert_cmpint (debug_breakpoint_mark (debug, c), ==, DEBUG_MARK_DISABLED);
    // a session: one the debugger has not taken, or has not put on code, waits
    start_session ();
    g_assert_cmpint (debug_breakpoint_mark (debug, a), ==, DEBUG_MARK_PENDING);
    a->gdb_number = g_strdup ("1");
    g_assert_cmpint (debug_breakpoint_mark (debug, a), ==, DEBUG_MARK_BREAKPOINT);
    a->unverified = TRUE;
    g_assert_cmpint (debug_breakpoint_mark (debug, a), ==, DEBUG_MARK_PENDING);
    g_assert_cmpint (debug_breakpoint_mark (debug, c), ==, DEBUG_MARK_DISABLED);
    live = FALSE;

    // the gutter of one file, then of all, the line of the program with them
    a->unverified = FALSE;
    debug_marks_show (debug, "/src/a.c");
    g_assert_cmpstr (marks->str, ==, "/src/a.c:3 100\n/src/a.c:7 103\n");
    g_string_truncate (marks, 0);
    debug->current_file = g_strdup ("/src/a.c");
    debug->current_line = 7;
    debug_marks_show (debug, NULL);
    g_assert_cmpstr (marks->str, ==,
                     "/src/a.c:3 100\n/src/a.c:7 103\n/src/b.c:1 102\n/src/a.c:7 105\n");
    g_string_truncate (marks, 0);
    debug->current_line = 8;
    debug_marks_show (debug, "/src/b.c");
    g_assert_cmpstr (marks->str, ==, "/src/b.c:1 102\n");
    g_string_truncate (marks, 0);
    debug_marks_show (debug, "/src/a.c");
    g_assert_cmpstr (marks->str, ==, "/src/a.c:3 100\n/src/a.c:7 103\n/src/a.c:8 104\n");

    g_assert_true (debug_breakpoint_at (debug, "/src/a.c", 7, NULL) == b);
    {
        guint index = 99;

        g_assert_true (debug_breakpoint_at (debug, "/src/b.c", 1, &index) == c);
        g_assert_cmpuint (index, ==, 2);
    }
    g_assert_null (debug_breakpoint_at (debug, "/src/b.c", 7, NULL));
    teardown ();
}

/* What the debugger answers to a breakpoint: its number, a line it moved it to, a refusal */
static void
test_reply (void)
{
    debug_breakpoint_t *a, *b;
    debug_reply_t reply = { 0 };

    setup ();
    start_session ();
    a = bp_new ("/src/a.c", 3);
    a->condition = g_strdup ("n == 2");
    b = bp_new ("/src/a.c", 5);
    b->disabled = TRUE;
    g_assert_true (debug_breakpoint_install (debug, a));
    g_assert_true (debug_breakpoint_install (debug, b));
    g_assert_cmpstr (sent->str, ==, "insert /src/a.c:3 if n == 2\ninsert /src/a.c:5 off\n");
    g_assert_cmpuint (a->pending_token, ==, 1);

    // taken, on a line of its own
    reply.request = 1;
    reply.ok = TRUE;
    reply.id = "7";
    reply.line = 3;
    debug_reply_breakpoint (debug, &reply, NULL);
    g_assert_cmpstr (a->gdb_number, ==, "7");
    g_assert_cmpuint (a->pending_token, ==, 0);
    g_assert_false (a->unverified);
    g_assert_cmpint (saved, ==, 0);

    // moved to the next line with code, where the other one is: they are one, the later goes
    reply.request = 2;
    reply.id = "8";
    reply.line = 3;
    debug_reply_breakpoint (debug, &reply, NULL);
    g_assert_cmpuint (debug->breakpoints->len, ==, 1);
    g_assert_true (bp_at (0) == a);
    g_assert_cmpint (saved, ==, 1);
    g_assert_nonnull (strstr (sent->str, "delete 8\n"));

    // waiting for a library: what the debugger said is told
    b = bp_new ("/lib/x.c", 9);
    (void) debug_breakpoint_install (debug, b);
    reply.request = b->pending_token;
    reply.id = "9";
    reply.line = 0;
    reply.pending = TRUE;
    reply.msg = "No source file named /lib/x.c.";
    debug_reply_breakpoint (debug, &reply, NULL);
    g_assert_true (b->unverified);
    g_assert_cmpint (b->line, ==, 9);
    g_assert_cmpstr (console->str, ==, "No source file named /lib/x.c.\n");

    // refused: the breakpoint stays, with no number
    b = bp_new ("/src/c.c", 1);
    (void) debug_breakpoint_install (debug, b);
    reply.request = b->pending_token;
    reply.ok = FALSE;
    reply.id = NULL;
    reply.pending = FALSE;
    reply.msg = "No line 1 in file.";
    debug_reply_breakpoint (debug, &reply, NULL);
    g_assert_null (b->gdb_number);
    g_assert_cmpuint (b->pending_token, ==, 0);
    g_assert_nonnull (strstr (console->str, "No line 1 in file.\n"));

    // an answer for no breakpoint is nothing
    reply.request = 999;
    reply.ok = TRUE;
    reply.id = "99";
    debug_reply_breakpoint (debug, &reply, NULL);
    g_assert_null (strstr (sent->str, "delete 99"));
    teardown ();
}

/* A breakpoint removed: from the debugger when it has it, else when the debugger answers */
static void
test_remove (void)
{
    debug_breakpoint_t *a, *b;
    debug_reply_t reply = { 0 };

    setup ();
    start_session ();
    a = bp_new ("/src/a.c", 3);
    a->gdb_number = g_strdup ("4");
    b = bp_new ("/src/a.c", 5);
    (void) debug_breakpoint_install (debug, b);
    g_string_truncate (sent, 0);
    debug_breakpoint_remove (debug, 1);
    g_assert_cmpuint (debug->dropped_tokens->len, ==, 1);
    debug_breakpoint_remove (debug, 0);
    g_assert_cmpstr (sent->str, ==, "delete 4\n");
    g_assert_cmpuint (debug->breakpoints->len, ==, 0);
    // the answer to the one removed while it was put: it goes from the debugger too
    reply.request = 1;
    reply.ok = TRUE;
    reply.id = "5";
    debug_reply_breakpoint (debug, &reply, NULL);
    g_assert_cmpstr (sent->str, ==, "delete 4\ndelete 5\n");
    g_assert_cmpuint (debug->dropped_tokens->len, ==, 0);

    // with no debugger, nothing is sent
    live = FALSE;
    a = bp_new ("/src/a.c", 3);
    a->gdb_number = g_strdup ("4");
    debug_breakpoint_remove (debug, 0);
    g_assert_cmpstr (sent->str, ==, "delete 4\ndelete 5\n");
    teardown ();
}

/* A breakpoint kept but not stopped on: the debugger told, the change kept */
static void
test_toggle (void)
{
    debug_breakpoint_t *a;

    setup ();
    a = bp_new ("/src/a.c", 3);
    debug_breakpoint_toggle_enabled (debug, 0);
    g_assert_true (a->disabled);
    g_assert_cmpstr (sent->str, ==, "");
    g_assert_cmpint (saved, ==, 1);
    start_session ();
    a->gdb_number = g_strdup ("2");
    debug_breakpoint_toggle_enabled (debug, 0);
    g_assert_false (a->disabled);
    debug_breakpoint_toggle_enabled (debug, 0);
    g_assert_cmpstr (sent->str, ==, "enable 2\ndisable 2\n");
    g_assert_cmpint (saved, ==, 3);
    teardown ();
}

/* The breakpoints follow their marks when the text is edited; marks not all there yet change
   nothing; two on one line are one */
static void
test_sync (void)
{
    const long moved[] = { 12, 4 };
    const long cond[] = { 30 };
    const long both[] = { 8, 8 };
    const long one[] = { 20 };

    setup ();
    bp_new ("/src/a.c", 3);
    bp_new ("/src/a.c", 10);
    bp_new ("/src/b.c", 7)->condition = g_strdup ("y");
    bp_new ("/src/c.c", 1);
    // a.c in a window, the lines sorted by the plugin; b.c by its other mark; c.c in none
    gutter ("/src/a.c", DEBUG_MARK_BREAKPOINT, moved, 2);
    gutter ("/src/b.c", DEBUG_MARK_CONDITION, cond, 1);
    // the line of the program is no breakpoint
    gutter ("/src/a.c", DEBUG_MARK_EXEC, one, 1);
    debug_breakpoints_sync (debug);
    g_assert_cmpint (bp_at (0)->line, ==, 4);
    g_assert_cmpint (bp_at (1)->line, ==, 12);
    g_assert_cmpint (bp_at (2)->line, ==, 30);
    g_assert_cmpint (bp_at (3)->line, ==, 1);

    // one mark of two: the marks are not there yet
    gutter ("/src/a.c", DEBUG_MARK_BREAKPOINT, one, 1);
    debug_breakpoints_sync (debug);
    g_assert_cmpint (bp_at (0)->line, ==, 4);
    g_assert_cmpint (bp_at (1)->line, ==, 12);

    // the lines between them deleted: they meet
    gutter ("/src/a.c", DEBUG_MARK_BREAKPOINT, both, 2);
    debug_breakpoints_sync (debug);
    g_assert_cmpuint (debug->breakpoints->len, ==, 3);
    g_assert_cmpint (bp_at (0)->line, ==, 8);
    g_assert_cmpstr (bp_at (1)->file, ==, "/src/b.c");

    // three on one line are one
    {
        const long three[] = { 2, 2, 2 };

        bp_new ("/src/d.c", 1);
        bp_new ("/src/d.c", 2);
        bp_new ("/src/d.c", 3);
        gutter ("/src/d.c", DEBUG_MARK_BREAKPOINT, three, 3);
        debug_breakpoints_sync (debug);
        g_assert_cmpuint (debug->breakpoints->len, ==, 4);
        g_assert_cmpstr (bp_at (3)->file, ==, "/src/d.c");
    }

    // no gutter to look at
    host.marker_lines = NULL;
    bp_at (0)->line = 99;
    debug_breakpoints_sync (debug);
    g_assert_cmpint (bp_at (0)->line, ==, 99);
    host.marker_lines = host_marker_lines;
    teardown ();
}

/* A breakpoint added: by the real path, an empty condition none; sent by itself once the
   breakpoints of the start are */
static void
test_add (void)
{
    setup ();
    debug_breakpoint_add (debug, "/no/such/file.c", 5, "");
    g_assert_cmpstr (bp_at (0)->file, ==, "/no/such/file.c");
    g_assert_null (bp_at (0)->condition);
    {
        char *cwd = g_get_current_dir ();
        char *dotted = g_build_filename (cwd, ".", (char *) NULL);
        char *real = realpath (cwd, NULL);

        debug_breakpoint_add (debug, dotted, 6, "z");
        g_assert_cmpstr (bp_at (1)->file, ==, real);
        g_assert_cmpstr (bp_at (1)->condition, ==, "z");
        free (real);
        g_free (dotted);
        g_free (cwd);
    }
    g_assert_cmpstr (sent->str, ==, "");
    start_session ();
    debug_breakpoint_add (debug, "/no/such/b.c", 7, NULL);
    g_assert_cmpstr (sent->str, ==, "");
    debug->breakpoints_installed = TRUE;
    debug_breakpoint_add (debug, "/no/such/c.c", 8, NULL);
    g_assert_cmpstr (sent->str, ==, "insert /no/such/c.c:8\n");
    g_assert_cmpuint (bp_at (3)->pending_token, ==, 1);
    teardown ();
}

/* The functions of the project, from the index of the plugin ctags */
static void
test_functions (void)
{
    GVariantBuilder list;
    GVariantDict dict;
    GPtrArray *found;
    const debug_function_t *f;

    setup ();
    g_assert_null (debug_functions (debug, "/p", NULL, "ma"));
    g_variant_builder_init (&list, G_VARIANT_TYPE ("a(sssi)"));
    g_variant_builder_add (&list, "(sssi)", "main", "func", "/p/main.c", 10);
    g_variant_builder_add (&list, "(sssi)", "max_len", "var", "/p/x.c", 3);
    g_variant_builder_add (&list, "(sssi)", "make", "func", "/p/b.c", 40);
    g_variant_dict_init (&dict, NULL);
    g_variant_dict_insert_value (&dict, "symbols", g_variant_builder_end (&list));
    symbols = g_variant_ref_sink (g_variant_dict_end (&dict));
    found = debug_functions (debug, "/p", NULL, "ma");
    g_assert_cmpuint (found->len, ==, 2);
    f = g_ptr_array_index (found, 1);
    g_assert_cmpstr (f->name, ==, "make");
    g_assert_cmpstr (f->file, ==, "/p/b.c");
    g_assert_cmpint (f->line, ==, 40);
    g_ptr_array_free (found, TRUE);
    teardown ();
}

/* The kinds of exceptions: those of GDB, those an adapter offered, those of the sessions that
   run; the ones chosen, else those the debugger stops on by itself */
static void
test_exceptions (void)
{
    debug_launch_t gdb = { 0 }, dap = { 0 };
    GPtrArray *filters;
    char **ids;
    const char *const chosen[] = { "raised", NULL };

    setup ();
    dap.backend = (char *) "dap";
    // GDB has its own whether it runs or not
    filters = debug_exception_filters (debug, &gdb);
    g_assert_cmpuint (filters->len, ==, 2);
    g_assert_cmpstr (((debug_exception_filter_t *) g_ptr_array_index (filters, 0))->id, ==,
                     "throw");
    g_ptr_array_unref (filters);
    // an adapter that has not run: not known
    g_assert_null (debug_exception_filters (debug, &dap));

    // it runs: its own, kept with the configuration, once
    g_ptr_array_add (offered, debug_exception_filter_new ("raised", "Raised", FALSE));
    g_ptr_array_add (offered, debug_exception_filter_new ("uncaught", "Uncaught", TRUE));
    start_session ();
    debug->session_launch = &dap;
    debug_exceptions_cache (debug);
    g_assert_cmpint (saved, ==, 1);
    g_assert_cmpuint (dap.exception_filters->len, ==, 2);
    debug_exceptions_cache (debug);
    g_assert_cmpint (saved, ==, 1);
    g_assert_cmpuint (debug->session_filters->len, ==, 2);
    filters = debug_exception_filters (debug, &dap);
    g_assert_true (filters == debug->session_filters);
    g_ptr_array_unref (filters);
    // another configuration of an adapter is not the one that runs
    {
        debug_launch_t other = { 0 };

        other.backend = (char *) "dap";
        g_assert_null (debug_exception_filters (debug, &other));
    }
    // another session adds its own
    g_ptr_array_set_size (offered, 0);
    g_ptr_array_add (offered, debug_exception_filter_new ("uncaught", "Uncaught", TRUE));
    g_ptr_array_add (offered, debug_exception_filter_new ("userUnhandled", "User", FALSE));
    debug_exceptions_cache (debug);
    g_assert_cmpint (saved, ==, 2);
    g_assert_cmpuint (dap.exception_filters->len, ==, 3);

    // none chosen: those it stops on by itself
    ids = debug_exceptions_chosen (&dap, dap.exception_filters);
    g_assert_cmpuint (g_strv_length (ids), ==, 1);
    g_assert_cmpstr (ids[0], ==, "uncaught");
    g_strfreev (ids);
    ids = debug_exceptions_chosen (&dap, NULL);
    g_assert_null (ids[0]);
    g_strfreev (ids);
    dap.exceptions = g_strdupv ((char **) chosen);
    ids = debug_exceptions_chosen (&dap, dap.exception_filters);
    g_assert_cmpuint (g_strv_length (ids), ==, 1);
    g_assert_cmpstr (ids[0], ==, "raised");
    g_strfreev (ids);

    // the program loaded: the debugger is told those chosen
    debug_exceptions_install (debug);
    g_assert_cmpstr (sent->str, ==, "exceptions [raised]\n");
    g_strfreev (dap.exceptions);
    dap.exceptions = NULL;
    debug_exceptions_install (debug);
    g_assert_cmpstr (sent->str, ==, "exceptions [raised]\nexceptions default\n");
    // another configuration chosen since is not the one that runs
    debug->session_launch = &gdb;
    live = FALSE;
    debug_exceptions_install (debug);
    g_assert_cmpstr (sent->str, ==, "exceptions [raised]\nexceptions default\n");
    filters = debug_exception_filters (debug, &dap);
    g_assert_true (filters == dap.exception_filters);
    g_ptr_array_unref (filters);

    g_assert_true (debug_exception_filters_equal (NULL, NULL));
    g_assert_false (debug_exception_filters_equal (dap.exception_filters, NULL));
    g_assert_true (debug_exception_filters_equal (dap.exception_filters, debug->session_filters));
    ((debug_exception_filter_t *) g_ptr_array_index (debug->session_filters, 0))->on = TRUE;
    g_assert_false (debug_exception_filters_equal (dap.exception_filters, debug->session_filters));
    g_ptr_array_unref (dap.exception_filters);
    teardown ();
}

/* --------------------------------------------------------------------------------------------- */

int
main (int argc, char **argv)
{
    g_test_init (&argc, &argv, NULL);
    g_test_add_func ("/debugger/breakpoints/same-file", test_same_file);
    g_test_add_func ("/debugger/breakpoints/marks", test_marks);
    g_test_add_func ("/debugger/breakpoints/reply", test_reply);
    g_test_add_func ("/debugger/breakpoints/remove", test_remove);
    g_test_add_func ("/debugger/breakpoints/toggle", test_toggle);
    g_test_add_func ("/debugger/breakpoints/sync", test_sync);
    g_test_add_func ("/debugger/breakpoints/add", test_add);
    g_test_add_func ("/debugger/breakpoints/functions", test_functions);
    g_test_add_func ("/debugger/breakpoints/exceptions", test_exceptions);
    return g_test_run ();
}
