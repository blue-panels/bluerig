/*
   src/plugins/debugger - tests for GDB with its machine interface as a backend of the debugger

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

#include <sys/select.h>
#include <string.h>
#include <unistd.h>

#include <glib.h>

#include "lib/tty/key.h"
#include "src/plugins/debugger/debug-backend.h"

typedef struct
{
    int fd;
    select_fn callback;
    void *data;
} watched_t;

static watched_t watched[8];
static int watched_count;

void
add_select_channel (int fd, select_fn callback, void *data)
{
    g_assert_cmpint (watched_count, <, G_N_ELEMENTS (watched));
    watched[watched_count++] = (watched_t) { fd, callback, data };
}

void
delete_select_channel (int fd)
{
    int i;

    for (i = 0; i < watched_count; i++)
        if (watched[i].fd == fd)
        {
            watched[i] = watched[--watched_count];
            return;
        }
}

/* One round of the descriptors watched: what is ready read */
static void
run_once (void)
{
    fd_set set;
    struct timeval timeout = { 0, 100000 };
    int i, max_fd = -1, ready_count = 0;
    watched_t ready[8];

    FD_ZERO (&set);
    for (i = 0; i < watched_count; i++)
    {
        FD_SET (watched[i].fd, &set);
        max_fd = MAX (max_fd, watched[i].fd);
    }
    if (max_fd < 0 || select (max_fd + 1, &set, NULL, NULL, &timeout) <= 0)
        return;
    for (i = 0; i < watched_count; i++)
        if (FD_ISSET (watched[i].fd, &set))
            ready[ready_count++] = watched[i];
    for (i = 0; i < ready_count; i++)
        ready[i].callback (ready[i].fd, ready[i].data);
}

static void
run_until (const gboolean *done)
{
    const gint64 deadline = g_get_monotonic_time () + 5 * G_TIME_SPAN_SECOND;

    while (!*done && g_get_monotonic_time () < deadline)
        run_once ();
}

/* Until @text has @needle after its first @from bytes, or a while */
static void
run_until_text_from (const GString *text, gsize from, const char *needle)
{
    const gint64 deadline = g_get_monotonic_time () + 5 * G_TIME_SPAN_SECOND;

    while (strstr (text->str + from, needle) == NULL && g_get_monotonic_time () < deadline)
        run_once ();
    g_assert_nonnull (strstr (text->str + from, needle));
}

static void
run_until_text (const GString *text, const char *needle)
{
    run_until_text_from (text, 0, needle);
}

/* How many times @needle is in @text */
static int
count_text (const GString *text, const char *needle)
{
    const char *p = text->str;
    int n = 0;

    while ((p = strstr (p, needle)) != NULL)
    {
        n++;
        p += strlen (needle);
    }
    return n;
}

/* Until @needle is in @text @n times, or a while */
static void
run_until_count (const GString *text, const char *needle, int n)
{
    const gint64 deadline = g_get_monotonic_time () + 5 * G_TIME_SPAN_SECOND;

    while (count_text (text, needle) < n && g_get_monotonic_time () < deadline)
        run_once ();
    g_assert_cmpint (count_text (text, needle), ==, n);
}

/* --------------------------------------------------------------------------------------------- */
/* What the plugin would see */

typedef struct
{
    debug_backend_t *backend;
    gboolean ready;
    gboolean running;
    gboolean stopped;
    gboolean exited;
    gboolean ended;
    gboolean gone;
    gboolean start_failed;
    debug_stop_reason_t reason;
    gboolean has_frame;
    char *stop_func;
    char *stop_file;
    long stop_line;
    char *stop_address;
    char *exit_code;
    char *signal_name;
    char *signal_meaning;
    char *exception_name;
    char *exception_text;
    char *moved_id;
    long moved_line;
    GString *console;
    GString *program;
    GString *errors;  // what GDB writes to stderr: what the mock has got
    // the answers
    gboolean answered;
    guint request;
    gboolean ok;
    char *bp_id;
    long bp_line;
    GPtrArray *frames;
    GPtrArray *variables;
    GPtrArray *registers;
    GPtrArray *instructions;
    char *value;
    char *error;
    gboolean errored;  // an error has been told by itself
    gboolean unasked;
} seen_t;

static seen_t seen;

static void
seen_clear (void)
{
    g_free (seen.stop_func);
    g_free (seen.stop_file);
    g_free (seen.stop_address);
    g_free (seen.exit_code);
    g_free (seen.signal_name);
    g_free (seen.signal_meaning);
    g_free (seen.exception_name);
    g_free (seen.exception_text);
    g_free (seen.moved_id);
    g_free (seen.bp_id);
    g_free (seen.value);
    g_free (seen.error);
    if (seen.console != NULL)
        g_string_free (seen.console, TRUE);
    if (seen.program != NULL)
        g_string_free (seen.program, TRUE);
    if (seen.errors != NULL)
        g_string_free (seen.errors, TRUE);
    g_clear_pointer (&seen.frames, g_ptr_array_unref);
    g_clear_pointer (&seen.variables, g_ptr_array_unref);
    g_clear_pointer (&seen.registers, g_ptr_array_unref);
    g_clear_pointer (&seen.instructions, g_ptr_array_unref);
    memset (&seen, 0, sizeof (seen));
    seen.console = g_string_new (NULL);
    seen.program = g_string_new (NULL);
    seen.errors = g_string_new (NULL);
}

static void
on_output (void *ui, const char *text, debug_output_t kind)
{
    (void) ui;
    g_string_append (kind == DEBUG_OUTPUT_PROGRAM     ? seen.program
                         : kind == DEBUG_OUTPUT_ERROR ? seen.errors
                                                      : seen.console,
                     text);
}

static void
on_ready (void *ui)
{
    (void) ui;
    seen.ready = TRUE;
}

static void
on_start_failed (void *ui, const char *msg)
{
    (void) ui;
    g_free (seen.error);
    seen.error = g_strdup (msg != NULL ? msg : "?");
    seen.start_failed = TRUE;
}

static void
on_running (void *ui)
{
    (void) ui;
    seen.running = TRUE;
}

static void
seen_set (char **field, const char *value)
{
    g_free (*field);
    *field = g_strdup (value);
}

static void
on_stopped (void *ui, const debug_stop_t *stop)
{
    (void) ui;
    seen.stopped = TRUE;
    seen.reason = stop->reason;
    seen.has_frame = stop->has_frame;
    seen_set (&seen.stop_func, stop->func);
    seen_set (&seen.stop_file, stop->file);
    seen.stop_line = stop->line;
    seen_set (&seen.stop_address, stop->address);
    seen_set (&seen.signal_name, stop->signal_name);
    seen_set (&seen.signal_meaning, stop->signal_meaning);
    seen_set (&seen.exception_name, stop->exception_name);
    seen_set (&seen.exception_text, stop->exception_text);
    seen_set (&seen.exit_code, stop->exit_code);
    if (stop->reason == DEBUG_STOP_EXITED)
        seen.exited = TRUE;
}

static void
on_exited (void *ui, gboolean gone)
{
    (void) ui;
    seen.ended = TRUE;
    seen.gone = gone;
}

static void
on_moved (void *ui, const char *id, long line)
{
    (void) ui;
    seen_set (&seen.moved_id, id);
    seen.moved_line = line;
}

static void
on_verified (void *ui, const char *id, long line)
{
    (void) ui;
    (void) id;
    (void) line;
    g_assert_not_reached ();
}

static void
on_error (void *ui, const char *msg, gboolean unasked)
{
    (void) ui;
    seen_set (&seen.error, msg);
    seen.errored = TRUE;
    seen.unasked = unasked;
}

static void
on_flush (void *ui)
{
    (void) ui;
}

static const debug_backend_events_t events = {
    .output = on_output,
    .ready = on_ready,
    .start_failed = on_start_failed,
    .running = on_running,
    .stopped = on_stopped,
    .exited = on_exited,
    .breakpoint_moved = on_moved,
    .breakpoint_verified = on_verified,
    .error = on_error,
    .flush = on_flush,
};

static void
on_reply (void *ui, const debug_reply_t *reply, void *data)
{
    (void) ui;
    g_assert_cmpstr ((const char *) data, ==, "mine");
    seen.answered = TRUE;
    seen.request = reply->request;
    seen.ok = reply->ok;
    if (!reply->ok)
        seen_set (&seen.error, reply->msg != NULL ? reply->msg : "?");
    seen_set (&seen.bp_id, reply->id);
    seen.bp_line = reply->line;
    seen_set (&seen.value, reply->value);
    g_clear_pointer (&seen.frames, g_ptr_array_unref);
    g_clear_pointer (&seen.variables, g_ptr_array_unref);
    g_clear_pointer (&seen.registers, g_ptr_array_unref);
    g_clear_pointer (&seen.instructions, g_ptr_array_unref);
    if (reply->frames != NULL)
        seen.frames = g_ptr_array_ref (reply->frames);
    if (reply->variables != NULL)
        seen.variables = g_ptr_array_ref (reply->variables);
    if (reply->registers != NULL)
        seen.registers = g_ptr_array_ref (reply->registers);
    if (reply->instructions != NULL)
        seen.instructions = g_ptr_array_ref (reply->instructions);
}

/* An answer to wait for */
static void
wait_answer (void)
{
    run_until (&seen.answered);
    g_assert_true (seen.answered);
    seen.answered = FALSE;
}

/* A stop to wait for, after a command that runs the program */
static void
wait_stop (void)
{
    run_until (&seen.stopped);
    g_assert_true (seen.stopped);
    g_assert_true (seen.running);
    seen.stopped = seen.running = FALSE;
}

#define MINE (g_strdup ("mine")), g_free

/* The mock started with @program, ready for the commands */
static void
start_mock (const char *program)
{
    char *argv[] = { (char *) "one arg", (char *) "plain", NULL };
    char *environment[] = { (char *) "GREETING=hi", NULL };
    debug_start_t spec = { 0 };
    GError *error = NULL;

    seen_clear ();
    spec.program = program;
    spec.argv = argv;
    spec.directory = "/tmp";
    spec.environment = environment;
    spec.tty = "/dev/null";
    spec.debugger = MOCK_GDB;
    seen.backend = debug_gdb_mi_new (&events, NULL);
    g_assert_cmpstr (seen.backend->ops->name, ==, "gdb-mi");
    g_assert_true (seen.backend->ops->start (seen.backend, &spec, &error));
    g_assert_no_error (error);
    g_assert_true (seen.backend->ops->alive (seen.backend));
}

static void
stop_mock (void)
{
    seen.backend->ops->stop (seen.backend);
    g_assert_false (seen.backend->ops->alive (seen.backend));
    seen.backend->ops->free (seen.backend);
    g_assert_cmpint (watched_count, ==, 0);
    seen_clear ();
}

/* --------------------------------------------------------------------------------------------- */

/* The commands that load the program go one after the other, each after the answer to the one
   before; then the program is ready */
static void
test_startup (void)
{
    start_mock ("/bin/true");
    run_until (&seen.ready);
    g_assert_true (seen.ready);
    g_assert_false (seen.start_failed);
    run_until_text (seen.errors, "mock: -gdb-set mi-async on;");
    run_until_text (seen.errors, "mock: -enable-pretty-printing;");
    run_until_text (seen.errors, "mock: -file-exec-and-symbols \"/bin/true\";");
    run_until_text (seen.errors, "mock: -environment-cd \"/tmp\";");
    run_until_text (seen.errors, "mock: -gdb-set environment GREETING=hi;");
    run_until_text (seen.errors, "mock: -inferior-tty-set \"/dev/null\";");
    // the arguments as sh reads them: the one with a space stays one
    run_until_text (seen.errors, "mock: -exec-arguments 'one arg' 'plain';");
    // the program comes before the directory it runs in
    g_assert_true (strstr (seen.errors->str, "-file-exec-and-symbols")
                   < strstr (seen.errors->str, "-environment-cd"));
    stop_mock ();
}

/* GDB refuses the program: the start fails with what GDB says, and goes no further */
static void
test_startup_refused (void)
{
    start_mock ("/missing");
    run_until (&seen.start_failed);
    g_assert_true (seen.start_failed);
    g_assert_false (seen.ready);
    g_assert_cmpstr (seen.error, ==, "/missing: No such file or directory.");
    run_until_text (seen.errors, "mock: -file-exec-and-symbols \"/missing\";");
    g_assert_null (strstr (seen.errors->str, "-environment-cd"));
    stop_mock ();
}

/* A GDB that is not there */
static void
test_missing_gdb (void)
{
    debug_start_t spec = { 0 };
    GError *error = NULL;
    debug_backend_t *backend = debug_gdb_mi_new (&events, NULL);

    spec.program = "/bin/true";
    spec.directory = "/";
    spec.debugger = "/nonexistent/gdb";
    g_assert_false (backend->ops->start (backend, &spec, &error));
    g_assert_nonnull (error);
    g_clear_error (&error);
    g_assert_false (backend->ops->alive (backend));
    backend->ops->free (backend);
    g_assert_cmpint (watched_count, ==, 0);
}

/* Breakpoints: the number GDB gives, the line it puts it on, a pending one moved later */
static void
test_breakpoints (void)
{
    guint token;

    start_mock ("/bin/true");
    run_until (&seen.ready);

    token = seen.backend->ops->break_insert (seen.backend, "/src/a.c", 7, FALSE, "x > 1", on_reply,
                                             MINE);
    g_assert_cmpuint (token, >, 0);
    wait_answer ();
    g_assert_cmpuint (seen.request, ==, token);
    g_assert_true (seen.ok);
    g_assert_cmpstr (seen.bp_id, ==, "1");
    g_assert_cmpint (seen.bp_line, ==, 7);

    // in a library not loaded yet: taken, on no line
    (void) seen.backend->ops->break_insert (seen.backend, "/src/lib.c", 9, TRUE, "", on_reply,
                                            MINE);
    wait_answer ();
    g_assert_true (seen.ok);
    g_assert_cmpstr (seen.bp_id, ==, "2");
    g_assert_cmpint (seen.bp_line, ==, 0);

    // refused, with why
    (void) seen.backend->ops->break_insert (seen.backend, "/src/none.c", 1, FALSE, NULL, on_reply,
                                            MINE);
    wait_answer ();
    g_assert_false (seen.ok);
    g_assert_null (seen.bp_id);
    g_assert_cmpstr (seen.error, ==, "No source file named /src/none.c.");
    g_clear_pointer (&seen.error, g_free);

    (void) seen.backend->ops->break_address (seen.backend, "0x1001", on_reply, MINE);
    wait_answer ();
    g_assert_cmpstr (seen.bp_id, ==, "5");

    g_assert_cmpuint (seen.backend->ops->break_function (seen.backend, "main", TRUE), >, 0);
    run_until_text (seen.errors, "mock: -break-insert -t \"main\";");
    (void) seen.backend->ops->break_enable (seen.backend, "1", FALSE);
    run_until_text (seen.errors, "mock: -break-disable 1;");
    (void) seen.backend->ops->break_enable (seen.backend, "1", TRUE);
    run_until_text (seen.errors, "mock: -break-enable 1;");
    // the condition goes as it is, none taking it off
    (void) seen.backend->ops->break_condition (seen.backend, "1", "y == \"a b\"");
    run_until_text (seen.errors, "mock: -break-condition 1 y == \"a b\";");
    (void) seen.backend->ops->break_condition (seen.backend, "1", NULL);
    run_until_text (seen.errors, "mock: -break-condition 1;");
    (void) seen.backend->ops->break_delete (seen.backend, "5");
    run_until_text (seen.errors, "mock: -break-delete 5;");

    // the run: the pending one is on code now, and the program stops on the first
    g_assert_cmpuint (seen.backend->ops->exec (seen.backend, DEBUG_EXEC_RUN, NULL, NULL, NULL), >,
                      0);
    wait_stop ();
    g_assert_cmpstr (seen.moved_id, ==, "2");
    g_assert_cmpint (seen.moved_line, ==, 12);
    g_assert_cmpint (seen.reason, ==, DEBUG_STOP_BREAKPOINT);
    g_assert_true (seen.has_frame);
    g_assert_cmpstr (seen.stop_func, ==, "main");
    g_assert_cmpstr (seen.stop_file, ==, "/src/a.c");
    g_assert_cmpint (seen.stop_line, ==, 7);
    g_assert_cmpstr (seen.stop_address, ==, "0x1000");
    // what the program writes is its own, what GDB says is the console's
    g_assert_cmpstr (seen.program->str, ==, "program says hello\n");
    g_assert_nonnull (strstr (seen.console->str, "Starting program: /bin/true\n"));
    // no command has been refused
    g_assert_null (seen.error);
    stop_mock ();
}

/* The reasons of the stops: a step, a step out, a signal, run to a line, the end */
static void
test_stops (void)
{
    start_mock ("/bin/true");
    run_until (&seen.ready);
    (void) seen.backend->ops->exec (seen.backend, DEBUG_EXEC_RUN, NULL, NULL, NULL);
    wait_stop ();

    (void) seen.backend->ops->exec (seen.backend, DEBUG_EXEC_NEXT, NULL, NULL, NULL);
    wait_stop ();
    g_assert_cmpint (seen.reason, ==, DEBUG_STOP_STEP);
    g_assert_cmpint (seen.stop_line, ==, 8);

    (void) seen.backend->ops->exec (seen.backend, DEBUG_EXEC_FINISH, NULL, NULL, NULL);
    wait_stop ();
    g_assert_cmpint (seen.reason, ==, DEBUG_STOP_FINISH);
    g_assert_cmpstr (seen.stop_func, ==, "caller");
    g_assert_cmpstr (seen.stop_file, ==, "/src/b.c");

    (void) seen.backend->ops->exec (seen.backend, DEBUG_EXEC_STEP, NULL, NULL, NULL);
    wait_stop ();
    g_assert_cmpint (seen.reason, ==, DEBUG_STOP_SIGNAL);
    g_assert_cmpstr (seen.signal_name, ==, "SIGSEGV");
    g_assert_cmpstr (seen.signal_meaning, ==, "Segmentation fault");

    g_assert_cmpuint (seen.backend->ops->run_to (seen.backend, "/src/a.c", 10), >, 0);
    wait_stop ();
    run_until_text (seen.errors, "mock: -exec-until \"/src/a.c:10\";");
    g_assert_cmpint (seen.reason, ==, DEBUG_STOP_OTHER);
    g_assert_cmpint (seen.stop_line, ==, 10);

    g_assert_cmpuint (seen.backend->ops->select_frame (seen.backend, 1), >, 0);
    run_until_text (seen.errors, "mock: -stack-select-frame 1;");

    // the mock stops the first going on at its catchpoint 0, and there is none: a breakpoint
    (void) seen.backend->ops->exec (seen.backend, DEBUG_EXEC_CONTINUE, NULL, NULL, NULL);
    wait_stop ();
    g_assert_cmpint (seen.reason, ==, DEBUG_STOP_BREAKPOINT);
    g_assert_null (seen.exception_name);
    // then the end: the program is gone, GDB is not
    (void) seen.backend->ops->exec (seen.backend, DEBUG_EXEC_CONTINUE, NULL, NULL, NULL);
    run_until (&seen.ended);
    g_assert_true (seen.exited);
    g_assert_false (seen.has_frame);
    g_assert_cmpstr (seen.exit_code, ==, "03");
    g_assert_false (seen.gone);
    g_assert_true (seen.backend->ops->alive (seen.backend));
    stop_mock ();
}

/* The call stack, the variables, an expression, a variable object and its members */
static void
test_values (void)
{
    const debug_frame_t *frame;
    const debug_variable_t *variable;

    start_mock ("/bin/true");
    run_until (&seen.ready);
    (void) seen.backend->ops->exec (seen.backend, DEBUG_EXEC_RUN, NULL, NULL, NULL);
    wait_stop ();

    (void) seen.backend->ops->stack (seen.backend, on_reply, MINE);
    wait_answer ();
    g_assert_cmpuint (seen.frames->len, ==, 2);
    frame = g_ptr_array_index (seen.frames, 0);
    g_assert_cmpint (frame->level, ==, 0);
    g_assert_cmpstr (frame->func, ==, "main");
    g_assert_cmpstr (frame->file, ==, "/src/a.c");
    g_assert_cmpint (frame->line, ==, 7);
    g_assert_cmpstr (frame->address, ==, "0x1000");
    g_assert_null (frame->from);
    // a frame with no source: its library
    frame = g_ptr_array_index (seen.frames, 1);
    g_assert_cmpint (frame->level, ==, 1);
    g_assert_null (frame->file);
    g_assert_cmpint (frame->line, ==, 0);
    g_assert_cmpstr (frame->from, ==, "/lib/libc.so.6");

    // a struct has its type for a value, one with neither is unavailable, one with no name none
    (void) seen.backend->ops->variables (seen.backend, on_reply, MINE);
    wait_answer ();
    g_assert_cmpuint (seen.variables->len, ==, 3);
    variable = g_ptr_array_index (seen.variables, 0);
    g_assert_cmpstr (variable->name, ==, "x");
    g_assert_cmpstr (variable->value, ==, "42");
    g_assert_cmpstr (variable->type, ==, "int");
    variable = g_ptr_array_index (seen.variables, 1);
    g_assert_cmpstr (variable->name, ==, "p");
    g_assert_cmpstr (variable->value, ==, "{struct point}");
    variable = g_ptr_array_index (seen.variables, 2);
    g_assert_cmpstr (variable->name, ==, "q");
    g_assert_cmpstr (variable->value, ==, "<unavailable>");
    g_assert_null (variable->type);

    (void) seen.backend->ops->evaluate (seen.backend, "x+1", on_reply, MINE);
    wait_answer ();
    g_assert_true (seen.ok);
    g_assert_cmpstr (seen.value, ==, "43");
    (void) seen.backend->ops->evaluate (seen.backend, "nope", on_reply, MINE);
    wait_answer ();
    g_assert_false (seen.ok);
    g_assert_null (seen.value);
    g_assert_cmpstr (seen.error, ==, "No symbol \"nope\" in current context.");

    // a value with members: its reference, the members by it
    (void) seen.backend->ops->inspect (seen.backend, "p", on_reply, MINE);
    wait_answer ();
    g_assert_cmpuint (seen.variables->len, ==, 1);
    variable = g_ptr_array_index (seen.variables, 0);
    g_assert_cmpstr (variable->value, ==, "{...}");
    g_assert_cmpstr (variable->ref, ==, "var1");
    g_assert_cmpstr (variable->type, ==, "struct point");
    (void) seen.backend->ops->children (seen.backend, "var1", on_reply, MINE);
    wait_answer ();
    g_assert_cmpuint (seen.variables->len, ==, 2);
    variable = g_ptr_array_index (seen.variables, 1);
    g_assert_cmpstr (variable->name, ==, "y");
    g_assert_cmpstr (variable->value, ==, "2");
    g_assert_null (variable->ref);
    seen.backend->ops->release (seen.backend, "var1");
    run_until_text (seen.errors, "mock: -var-delete \"var1\";");

    // with no members it is of no use: it goes at once
    (void) seen.backend->ops->inspect (seen.backend, "x", on_reply, MINE);
    wait_answer ();
    variable = g_ptr_array_index (seen.variables, 0);
    g_assert_cmpstr (variable->value, ==, "42");
    g_assert_null (variable->ref);
    run_until_text (seen.errors, "mock: -var-delete \"var2\";");

    // a command of GDB itself: its words on the console
    (void) seen.backend->ops->console (seen.backend, "info frame", on_reply, MINE);
    wait_answer ();
    g_assert_true (seen.ok);
    run_until_text (seen.errors, "mock: -interpreter-exec console \"info frame\";");
    g_assert_nonnull (strstr (seen.console->str, "words of the console\n"));
    stop_mock ();
}

/* The registers: the names once, the general ones only, then the values at every asking */
static void
test_registers (void)
{
    const debug_variable_t *reg;

    start_mock ("/bin/true");
    run_until (&seen.ready);
    (void) seen.backend->ops->exec (seen.backend, DEBUG_EXEC_RUN, NULL, NULL, NULL);
    wait_stop ();

    g_assert_cmpuint (seen.backend->ops->registers (seen.backend, on_reply, MINE), >, 0);
    wait_answer ();
    // up to the first of the floating point ones, those with no name left out
    run_until_text (seen.errors, "mock: -data-list-register-values --skip-unavailable x 0 1 3 4;");
    // a number GDB has no name for is left out
    g_assert_cmpuint (seen.registers->len, ==, 4);
    reg = g_ptr_array_index (seen.registers, 2);
    g_assert_cmpstr (reg->name, ==, "rip");
    g_assert_cmpstr (reg->value, ==, "0x1000");
    reg = g_ptr_array_index (seen.registers, 3);
    g_assert_cmpstr (reg->name, ==, "eflags");

    (void) seen.backend->ops->registers (seen.backend, on_reply, MINE);
    wait_answer ();
    g_assert_cmpuint (seen.registers->len, ==, 4);
    run_until_count (seen.errors, "mock: -data-list-register-values", 2);
    g_assert_cmpint (count_text (seen.errors, "mock: -data-list-register-names;"), ==, 1);
    stop_mock ();
}

/* The instructions of a function with their source lines; with no function there, those after
   the address; an address that is not one is not sent */
static void
test_disassemble (void)
{
    const debug_instruction_t *insn;

    start_mock ("/bin/true");
    run_until (&seen.ready);

    (void) seen.backend->ops->disassemble (seen.backend, "0x1000", on_reply, MINE);
    wait_answer ();
    g_assert_true (seen.ok);
    g_assert_cmpuint (seen.instructions->len, ==, 3);
    insn = g_ptr_array_index (seen.instructions, 1);
    g_assert_cmpstr (insn->address, ==, "0x1001");
    g_assert_cmpstr (insn->func, ==, "main");
    g_assert_cmpint (insn->offset, ==, 1);
    g_assert_cmpstr (insn->text, ==, "mov %rsp,%rbp");
    g_assert_cmpstr (insn->file, ==, "/src/a.c");
    g_assert_cmpint (insn->line, ==, 7);
    insn = g_ptr_array_index (seen.instructions, 2);
    g_assert_cmpint (insn->line, ==, 8);

    (void) seen.backend->ops->disassemble (seen.backend, "0x2000", on_reply, MINE);
    wait_answer ();
    g_assert_true (seen.ok);
    run_until_text (seen.errors, "mock: -data-disassemble -s 0x2000 -e 0x2000+256 -- 4;");
    g_assert_cmpuint (seen.instructions->len, ==, 2);
    insn = g_ptr_array_index (seen.instructions, 1);
    g_assert_cmpstr (insn->text, ==, "ret");
    g_assert_null (insn->func);
    g_assert_null (insn->file);

    g_assert_cmpuint (
        seen.backend->ops->disassemble (seen.backend, "0x1; -gdb-exit", on_reply, MINE), ==, 0);
    g_assert_cmpuint (seen.backend->ops->disassemble (seen.backend, NULL, on_reply, MINE), ==, 0);
    g_assert_true (seen.backend->ops->alive (seen.backend));
    stop_mock ();
}

/* The exceptions of C++: catchpoints, a stop on one told as an exception, taken off */
static void
test_exceptions (void)
{
    const char *const throw_only[] = { "throw", NULL };
    GPtrArray *filters;
    const debug_exception_filter_t *filter;

    start_mock ("/bin/true");
    run_until (&seen.ready);

    filters = seen.backend->ops->exception_filters (seen.backend);
    g_assert_cmpuint (filters->len, ==, 2);
    filter = g_ptr_array_index (filters, 0);
    g_assert_cmpstr (filter->id, ==, "throw");
    g_assert_false (filter->on);
    filter = g_ptr_array_index (filters, 1);
    g_assert_cmpstr (filter->id, ==, "catch");
    g_ptr_array_unref (filters);

    /* asked twice before GDB answers: the first one taken is taken off again, the second one
       stays */
    g_assert_cmpuint (seen.backend->ops->break_exceptions (seen.backend, throw_only), >, 0);
    g_assert_cmpuint (seen.backend->ops->break_exceptions (seen.backend, throw_only), >, 0);
    run_until_text (seen.errors, "mock: -break-delete 10;");
    run_until_count (seen.errors, "mock: -catch-throw;", 2);
    g_assert_cmpint (count_text (seen.errors, "mock: -catch-catch;"), ==, 0);

    (void) seen.backend->ops->exec (seen.backend, DEBUG_EXEC_RUN, NULL, NULL, NULL);
    wait_stop ();
    // the mock stops the first going on at its last catchpoint, number 20
    (void) seen.backend->ops->exec (seen.backend, DEBUG_EXEC_CONTINUE, NULL, NULL, NULL);
    wait_stop ();
    g_assert_cmpint (seen.reason, ==, DEBUG_STOP_EXCEPTION);
    g_assert_cmpstr (seen.exception_name, ==, "C++ exception");
    g_assert_cmpstr (seen.exception_text, ==, "thrown");

    // none any more: the one there goes
    (void) seen.backend->ops->break_exceptions (seen.backend, NULL);
    run_until_text (seen.errors, "mock: -break-delete 20;");
    g_assert_null (seen.error);
    stop_mock ();
}

/* Answers no request waits for: a request forgotten, a command sent with no one to answer */
static void
test_cancel (void)
{
    start_mock ("/bin/true");
    run_until (&seen.ready);

    // the error of a forgotten request is told by itself
    (void) seen.backend->ops->evaluate (seen.backend, "nope", on_reply, MINE);
    seen.backend->ops->cancel (seen.backend);
    run_until (&seen.unasked);
    g_assert_true (seen.unasked);
    g_assert_false (seen.answered);
    g_assert_cmpstr (seen.error, ==, "No symbol \"nope\" in current context.");

    // with no one to answer, a refusal is an error of the backend, not unasked
    (void) seen.backend->ops->break_delete (seen.backend, "99");
    seen.errored = FALSE;
    run_until_text (seen.errors, "mock: -break-delete 99;");
    run_until (&seen.errored);
    g_assert_false (seen.answered);
    g_assert_false (seen.unasked);
    g_assert_cmpstr (seen.error, ==, "No breakpoint number 99.");
    stop_mock ();
}

/* GDB goes away by itself: told as gone, the answer owed forgotten, nothing sent any more */
static void
test_gdb_exits (void)
{
    start_mock ("/bin/true");
    run_until (&seen.ready);
    (void) seen.backend->ops->console (seen.backend, "quit", on_reply, MINE);
    run_until (&seen.ended);
    g_assert_true (seen.ended);
    g_assert_true (seen.gone);
    g_assert_false (seen.answered);
    g_assert_false (seen.backend->ops->alive (seen.backend));
    g_assert_cmpint (watched_count, ==, 0);

    g_assert_cmpuint (seen.backend->ops->stack (seen.backend, on_reply, MINE), ==, 0);
    g_assert_cmpstr (seen.error, ==, "Could not send a command to GDB.");
    g_assert_false (seen.unasked);
    // nothing to take off: a variable object went with GDB
    seen.backend->ops->release (seen.backend, "var1");
    seen.backend->ops->free (seen.backend);
    seen_clear ();
}

/* --------------------------------------------------------------------------------------------- */

int
main (int argc, char **argv)
{
    g_test_init (&argc, &argv, NULL);
    g_test_add_func ("/debugger/gdb-mi-backend/startup", test_startup);
    g_test_add_func ("/debugger/gdb-mi-backend/startup-refused", test_startup_refused);
    g_test_add_func ("/debugger/gdb-mi-backend/missing", test_missing_gdb);
    g_test_add_func ("/debugger/gdb-mi-backend/breakpoints", test_breakpoints);
    g_test_add_func ("/debugger/gdb-mi-backend/stops", test_stops);
    g_test_add_func ("/debugger/gdb-mi-backend/values", test_values);
    g_test_add_func ("/debugger/gdb-mi-backend/registers", test_registers);
    g_test_add_func ("/debugger/gdb-mi-backend/disassemble", test_disassemble);
    g_test_add_func ("/debugger/gdb-mi-backend/exceptions", test_exceptions);
    g_test_add_func ("/debugger/gdb-mi-backend/cancel", test_cancel);
    g_test_add_func ("/debugger/gdb-mi-backend/gdb-exits", test_gdb_exits);
    return g_test_run ();
}
