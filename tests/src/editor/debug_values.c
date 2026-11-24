/*
   src/plugins/debugger - tests for the values of the debugger: the names in a line, the
   expressions of members, the values after the lines, the stack, the registers and the watches

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

#include "src/plugins/debugger/debug-values.c"  // for its file scope functions

/* --------------------------------------------------------------------------------------------- */
/* The rest of the plugin, as much as the values ask of it */

static GString *told;  // what the plugin did that the test looks at, a line each
static int saved;

void
debug_error (debugger_t *debug, const char *message_text)
{
    (void) debug;
    g_string_append_printf (told, "error: %s\n", message_text);
}

void
debug_output_console (debugger_t *debug, const char *text_value, gboolean line)
{
    (void) debug;
    g_string_append_printf (told, "console: %s%s", text_value, line ? "\n" : "");
}

void
debug_config_save (debugger_t *debug)
{
    (void) debug;
    saved++;
}

/* the sources the test has are .c and .s files under /src, and those it writes */
gboolean
debug_source_here (const char *file)
{
    return file != NULL
        && (g_str_has_prefix (file, "/src/") || g_file_test (file, G_FILE_TEST_EXISTS));
}

gboolean
debug_source_is_asm (const char *file)
{
    return file != NULL && g_str_has_suffix (file, ".s");
}

void
debug_panel_forget_members (debugger_t *debug)
{
    (void) debug;
    g_string_append (told, "forget members\n");
}

void
debug_panel_show_registers (debugger_t *debug)
{
    (void) debug;
    g_string_append (told, "show registers\n");
}

void
debug_disasm_show (debugger_t *debug, gboolean focus)
{
    (void) debug;
    g_string_append_printf (told, "disasm show%s\n", focus ? " focused" : "");
}

void
debug_disasm_refresh (debugger_t *debug)
{
    g_string_append_printf (told, "disasm at %s\n",
                            debug->disasm_address != NULL ? debug->disasm_address : "-");
}

void
debug_clear_current (debugger_t *debug)
{
    g_clear_pointer (&debug->current_file, g_free);
}

void
debug_breakpoints_sync (debugger_t *debug)
{
    (void) debug;
}

void
debug_marks_show (debugger_t *debug, const char *file)
{
    (void) debug;
    g_string_append_printf (told, "marks %s\n", file != NULL ? file : "all");
}

void
debug_session_refresh (debugger_t *debug)
{
    (void) debug;
}

mc_ep_result_t
debug_session_show (void *data, void *edit)
{
    (void) data;
    (void) edit;
    return MC_EPR_OK;
}

gboolean
debug_require_project (debugger_t *debug, void *edit)
{
    (void) debug;
    (void) edit;
    return TRUE;
}

/* --------------------------------------------------------------------------------------------- */
/* A debugger that says what it was asked */

static guint next_token;
static gboolean refuse;  // no command can be sent

static guint
fake_evaluate (debug_backend_t *b, const char *expression, debug_reply_cb cb, void *data,
               GDestroyNotify free_data)
{
    (void) b;
    (void) data;
    (void) free_data;
    g_assert_true (cb == debug_reply_watch);
    if (refuse)
        return 0;
    g_string_append_printf (told, "evaluate %s\n", expression);
    return ++next_token;
}

static guint
fake_select_frame (debug_backend_t *b, long level)
{
    (void) b;
    g_string_append_printf (told, "frame %ld\n", level);
    return ++next_token;
}

static guint
fake_variables (debug_backend_t *b, debug_reply_cb cb, void *data, GDestroyNotify free_data)
{
    (void) b;
    (void) data;
    (void) free_data;
    g_assert_true (cb == debug_reply_variables);
    g_string_append (told, "variables\n");
    return ++next_token;
}

static const debug_backend_ops_t fake_ops = {
    .name = "fake",
    .evaluate = fake_evaluate,
    .select_frame = fake_select_frame,
    .variables = fake_variables,
};

static debug_backend_t fake_backend = { .ops = &fake_ops };

/* --------------------------------------------------------------------------------------------- */
/* The editor: the notes after the lines, the place shown */

static void
host_set_line_note (mc_editor_host_t *host, const char *file, long line, const char *text)
{
    (void) host;
    g_string_append_printf (told, "note %s:%ld %s\n", strrchr (file, '/') + 1, line, text);
}

static void
host_clear_line_notes (mc_editor_host_t *host, const char *file)
{
    (void) host;
    g_assert_null (file);
}

static int redraws;

static void
host_redraw (mc_editor_host_t *host)
{
    (void) host;
    redraws++;
}

static gboolean
host_show_location (mc_editor_host_t *host, const char *file, long line)
{
    (void) host;
    g_string_append_printf (told, "show %s:%ld\n", file, line);
    return TRUE;
}

static mc_editor_host_t host = {
    .redraw = host_redraw,
    .set_line_note = host_set_line_note,
    .clear_line_notes = host_clear_line_notes,
    .show_location = host_show_location,
};

/* --------------------------------------------------------------------------------------------- */

static debugger_t *debug;
static char *dir;

static void
setup (void)
{
    told = g_string_new (NULL);
    saved = 0;
    next_token = 0;
    refuse = FALSE;
    dir = g_dir_make_tmp ("debug-values-XXXXXX", NULL);
    g_assert_nonnull (dir);
    debug = g_new0 (debugger_t, 1);
    debug->host = &host;
    debug->backend = &fake_backend;
    debug->state = DEBUG_STOPPED;
    debug->frames = g_ptr_array_new_with_free_func (debug_frame_free);
    debug->locals = g_ptr_array_new_with_free_func (debug_variable_free);
    debug->registers = g_ptr_array_new_with_free_func (debug_variable_free);
    debug->watches = g_ptr_array_new_with_free_func (debug_watch_free);
}

static void
teardown (void)
{
    GDir *d = g_dir_open (dir, 0, NULL);
    const char *name;

    while ((name = g_dir_read_name (d)) != NULL)
    {
        char *path = g_build_filename (dir, name, (char *) NULL);

        (void) g_remove (path);
        g_free (path);
    }
    g_dir_close (d);
    (void) g_rmdir (dir);
    g_free (dir);
    g_ptr_array_unref (debug->frames);
    g_ptr_array_unref (debug->locals);
    g_ptr_array_unref (debug->registers);
    g_ptr_array_unref (debug->watches);
    if (debug->registers_before != NULL)
        g_hash_table_destroy (debug->registers_before);
    g_free (debug->current_file);
    g_free (debug->disasm_address);
    g_free (debug);
    g_string_free (told, TRUE);
}

static char *
source (const char *name, const char *text)
{
    char *path = g_build_filename (dir, name, (char *) NULL);

    g_assert_true (g_file_set_contents (path, text, -1, NULL));
    return path;
}

static void
value_add (GPtrArray *values, const char *name, const char *value)
{
    debug_variable_t *v = g_new0 (debug_variable_t, 1);

    v->name = g_strdup (name);
    v->value = g_strdup (value);
    g_ptr_array_add (values, v);
}

static debug_frame_t *
stack_frame_new (long level, const char *func, const char *file, long line, const char *from)
{
    debug_frame_t *frame = g_new0 (debug_frame_t, 1);

    frame->level = level;
    frame->func = g_strdup (func);
    frame->file = g_strdup (file);
    frame->line = line;
    frame->from = g_strdup (from);
    frame->address = g_strdup_printf ("0x%lx", (unsigned long) (0x1000 + level));
    return frame;
}

static const char *
label_at (guint i)
{
    g_assert_cmpuint (i, <, debug->frames->len);
    return ((debug_frame_t *) g_ptr_array_index (debug->frames, i))->label;
}

static debug_watch_t *
watch_at (guint i)
{
    g_assert_cmpuint (i, <, debug->watches->len);
    return g_ptr_array_index (debug->watches, i);
}

/* --------------------------------------------------------------------------------------------- */

/* A name as a word of its own; a register by the names of its parts; a label */
static void
test_words (void)
{
    g_assert_true (debug_line_names ("int x = y;", "x"));
    g_assert_true (debug_line_names ("x", "x"));
    g_assert_false (debug_line_names ("int xy = 1;", "x"));
    g_assert_false (debug_line_names ("a_x + x1", "x"));
    g_assert_true (debug_line_names ("a_x + x1 + (x)", "x"));
    g_assert_false (debug_line_names ("", "x"));
    g_assert_false (debug_line_names ("x_y = 1;", "x"));

    g_assert_true (debug_line_names_register ("mov %rax, %rbx", "rax"));
    g_assert_true (debug_line_names_register ("mov %eax, 1", "rax"));
    g_assert_true (debug_line_names_register ("mov ax, 1", "rax"));
    g_assert_true (debug_line_names_register ("mov al, 1", "rax"));
    g_assert_true (debug_line_names_register ("mov ah, 1", "rax"));
    g_assert_true (debug_line_names_register ("mov sil, 1", "rsi"));
    g_assert_false (debug_line_names_register ("mov sh, 1", "rsi"));
    g_assert_true (debug_line_names_register ("mov r8d, 1", "r8"));
    g_assert_true (debug_line_names_register ("mov r8b, 1", "r8"));
    g_assert_false (debug_line_names_register ("mov r80, 1", "r8"));
    // what a comment says does not count
    g_assert_false (debug_line_names_register ("nop # rax", "rax"));
    g_assert_false (debug_line_names_register ("nop ; eax", "rax"));
    g_assert_false (debug_line_names_register ("mov %eaxx, 1", "rax"));

    g_assert_true (debug_line_is_label ("main:"));
    g_assert_true (debug_line_is_label ("  .L2:  # loop"));
    g_assert_true (debug_line_is_label ("_start$1:"));
    g_assert_false (debug_line_is_label ("    mov %eax, 1"));
    g_assert_false (debug_line_is_label ("1:"));
    g_assert_false (debug_line_is_label ("a b:"));
    g_assert_false (debug_line_is_label (""));
}

/* The expression of a member as C writes it */
static void
test_member_expression (void)
{
    char *e;

    e = debug_member_expression ("p", "struct pt *", "x");
    g_assert_cmpstr (e, ==, "p->x");
    g_free (e);
    e = debug_member_expression ("p", "struct pt*  ", "x");
    g_assert_cmpstr (e, ==, "p->x");
    g_free (e);
    e = debug_member_expression ("s", "struct pt", "x");
    g_assert_cmpstr (e, ==, "s.x");
    g_free (e);
    e = debug_member_expression ("a", "int [3]", "2");
    g_assert_cmpstr (e, ==, "a[2]");
    g_free (e);
    e = debug_member_expression ("a->b[1]", NULL, "c");
    g_assert_cmpstr (e, ==, "a->b[1].c");
    g_free (e);
    // what is not a name gets its parentheses
    e = debug_member_expression ("*p", "struct pt", "x");
    g_assert_cmpstr (e, ==, "(*p).x");
    g_free (e);
    e = debug_member_expression ("f()", "int *", "0");
    g_assert_cmpstr (e, ==, "(f())[0]");
    g_free (e);
    e = debug_member_expression ("a-b", "struct q", "n");
    g_assert_cmpstr (e, ==, "(a-b).n");
    g_free (e);
    e = debug_member_expression ("a > b", "struct q *", "n");
    g_assert_cmpstr (e, ==, "(a > b)->n");
    g_free (e);
    // a name of digits and letters is a member, not an index
    e = debug_member_expression ("s", "struct u", "1st");
    g_assert_cmpstr (e, ==, "s.1st");
    g_free (e);
    e = debug_member_expression ("s", "struct u", "");
    g_assert_cmpstr (e, ==, "s.");
    g_free (e);

    {
        debug_variable_t root_var = { .name = (char *) "list", .type = (char *) "struct l *" };
        debug_variable_t next_var = { .name = (char *) "next", .type = (char *) "struct l *" };
        debug_variable_t item_var = { .name = (char *) "3", .type = (char *) "int" };
        debug_variable_t given = { .name = (char *) "y", .expression = (char *) "q.y" };
        debug_node_t *root = debug_node_new (&root_var, NULL);
        debug_node_t *next = debug_node_new (&next_var, root);
        debug_node_t *item = debug_node_new (&item_var, next);
        debug_node_t *own = debug_node_new (&given, next);

        g_assert_cmpint (item->depth, ==, 2);
        e = debug_node_expression (item);
        g_assert_cmpstr (e, ==, "list->next[3]");
        g_free (e);
        // the debugger's own, when it gives one
        e = debug_node_expression (own);
        g_assert_cmpstr (e, ==, "q.y");
        g_free (e);
        debug_node_free (own);
        debug_node_free (item);
        debug_node_free (next);
        debug_node_free (root);
    }
}

/* The values of the variables after the lines that name them, from the start of the function to
   the line of the stop */
static void
test_notes (void)
{
    setup ();
    debug->current_file = source ("f.c",
                                  "int g;\n"
                                  "int f (int a)\n"
                                  "{\n"
                                  "    int b = a + 1;\n"
                                  "    b++;\n"
                                  "    return b * a + g;\n"
                                  "}\n");
    debug->current_line = 6;
    value_add (debug->locals, "a", "2");
    value_add (debug->locals, "b", "0123456789012345678901234567890123456789XYZ");
    debug_notes_show (debug);
    g_assert_cmpstr (told->str, ==,
                     "note f.c:4 a = 2, b = 0123456789012345678901234567890123456789\n"
                     "note f.c:5 b = 0123456789012345678901234567890123456789\n"
                     "note f.c:6 a = 2, b = 0123456789012345678901234567890123456789\n");

    // no values: the notes taken off and nothing more drawn
    {
        GPtrArray *some = debug->locals;

        debug->locals = g_ptr_array_new ();
        g_string_truncate (told, 0);
        redraws = 0;
        debug_notes_show (debug);
        g_assert_cmpstr (told->str, ==, "");
        g_assert_cmpint (redraws, ==, 1);
        g_ptr_array_unref (debug->locals);
        debug->locals = some;
    }

    // the program is not stopped, or the line is past the end of the file: none
    g_string_truncate (told, 0);
    debug->state = DEBUG_RUNNING;
    debug_notes_show (debug);
    debug->state = DEBUG_STOPPED;
    debug->current_line = 50;
    debug_notes_show (debug);
    g_assert_cmpstr (told->str, ==, "");

    // in assembler the registers, from the label before
    g_free (debug->current_file);
    debug->current_file = source ("f.s",
                                  "f:\n"
                                  "    mov %eax, %ebx\n"
                                  "    add $1, %rax # rbx\n"
                                  "loop:\n"
                                  "    dec %rcx\n");
    value_add (debug->registers, "rax", "0x1");
    value_add (debug->registers, "rbx", "0x2");
    value_add (debug->registers, "rcx", "0x3");
    debug->current_line = 3;
    debug_notes_show (debug);
    g_assert_cmpstr (told->str, ==, "note f.s:2 rax = 0x1, rbx = 0x2\nnote f.s:3 rax = 0x1\n");
    g_string_truncate (told, 0);
    debug->current_line = 5;
    debug_notes_show (debug);
    g_assert_cmpstr (told->str, ==, "note f.s:5 rcx = 0x3\n");
    teardown ();
}

/* The call stack: the frames GDB makes up under _start left out, a label for each */
static void
test_stack (void)
{
    debug_reply_t reply = { 0 };

    setup ();
    reply.ok = TRUE;
    reply.frames = g_ptr_array_new_with_free_func (debug_frame_free);
    g_ptr_array_add (reply.frames, stack_frame_new (0, "main", "/src/a.c", 7, NULL));
    g_ptr_array_add (reply.frames, stack_frame_new (1, NULL, NULL, 0, "/lib/libc.so.6"));
    g_ptr_array_add (reply.frames, stack_frame_new (2, NULL, NULL, 0, NULL));
    g_ptr_array_add (reply.frames, stack_frame_new (3, "??", NULL, 0, NULL));
    debug_reply_stack (debug, &reply, NULL);
    g_assert_cmpuint (debug->frames->len, ==, 2);
    g_assert_cmpstr (label_at (0), ==, "#0 main  /src/a.c:7");
    g_assert_cmpstr (label_at (1), ==, "#1 ?  /lib/libc.so.6");
    g_ptr_array_unref (reply.frames);

    // the first one stays whatever it is; one with no library either
    reply.frames = g_ptr_array_new_with_free_func (debug_frame_free);
    g_ptr_array_add (reply.frames, stack_frame_new (0, "??", NULL, 0, NULL));
    g_ptr_array_add (reply.frames, stack_frame_new (1, "start", NULL, 0, NULL));
    g_ptr_array_add (reply.frames, stack_frame_new (2, "??", NULL, 0, NULL));
    debug_reply_stack (debug, &reply, NULL);
    g_assert_cmpuint (debug->frames->len, ==, 2);
    g_assert_cmpstr (label_at (0), ==, "#0 ??  <no source>");
    g_assert_cmpstr (label_at (1), ==, "#1 start  <no source>");
    g_ptr_array_unref (reply.frames);

    // no frames at all
    reply.frames = NULL;
    debug_reply_stack (debug, &reply, NULL);
    g_assert_cmpuint (debug->frames->len, ==, 0);
    g_assert_cmpstr (told->str, ==, "");
    teardown ();
}

/* Stopped in code with no source: the first frame of the stack that has one is chosen, else the
   registers and the instructions shown; only for the stop the stack was asked at */
static void
test_stack_source_frame (void)
{
    debug_reply_t reply = { 0 };

    setup ();
    debug->stop_serial = 4;
    debug->source_frame_wanted = TRUE;
    reply.ok = TRUE;
    reply.frames = g_ptr_array_new_with_free_func (debug_frame_free);
    g_ptr_array_add (reply.frames, stack_frame_new (0, "raise", NULL, 0, "/lib/libc.so.6"));
    g_ptr_array_add (reply.frames, stack_frame_new (1, "abort", "/build/glibc/abort.c", 79, NULL));
    g_ptr_array_add (reply.frames, stack_frame_new (2, "main", "/src/a.c", 12, NULL));
    // an answer of another stop
    debug_reply_stack (debug, &reply, GUINT_TO_POINTER (3));
    g_assert_true (debug->source_frame_wanted);
    g_assert_cmpstr (told->str, ==, "");
    g_ptr_array_unref (reply.frames);

    reply.frames = g_ptr_array_new_with_free_func (debug_frame_free);
    g_ptr_array_add (reply.frames, stack_frame_new (0, "raise", NULL, 0, "/lib/libc.so.6"));
    g_ptr_array_add (reply.frames, stack_frame_new (1, "abort", "/build/glibc/abort.c", 79, NULL));
    // a source with no line is none to show
    g_ptr_array_add (reply.frames, stack_frame_new (2, "inlined", "/src/b.c", 0, NULL));
    g_ptr_array_add (reply.frames, stack_frame_new (3, "main", "/src/a.c", 12, NULL));
    debug_reply_stack (debug, &reply, GUINT_TO_POINTER (4));
    g_assert_false (debug->source_frame_wanted);
    g_assert_cmpstr (told->str, ==,
                     "show /src/a.c:12\nframe 3\nvariables\ndisasm at 0x1003\n"
                     "marks /src/a.c\n");
    g_assert_cmpstr (debug->current_file, ==, "/src/a.c");
    g_assert_cmpint (debug->current_line, ==, 12);
    g_assert_cmpint (debug->panel_frame, ==, 3);
    g_ptr_array_unref (reply.frames);

    // no frame with its source
    g_string_truncate (told, 0);
    debug->source_frame_wanted = TRUE;
    reply.frames = g_ptr_array_new_with_free_func (debug_frame_free);
    g_ptr_array_add (reply.frames, stack_frame_new (0, "raise", NULL, 0, "/lib/libc.so.6"));
    debug_reply_stack (debug, &reply, GUINT_TO_POINTER (4));
    g_assert_cmpstr (told->str, ==, "show registers\ndisasm show focused\n");
    g_ptr_array_unref (reply.frames);
    teardown ();
}

/* The registers: those of the stop before kept to tell which have changed */
static void
test_registers (void)
{
    debug_reply_t reply = { 0 };

    setup ();
    value_add (debug->registers, "rax", "0x1");
    value_add (debug->registers, "rip", "0x1000");
    reply.ok = TRUE;
    reply.registers = g_ptr_array_new_with_free_func (debug_variable_free);
    value_add (reply.registers, "rax", "0x2");
    debug_reply_registers (debug, &reply, NULL);
    g_assert_cmpuint (debug->registers->len, ==, 1);
    g_assert_true (debug->registers == reply.registers);
    g_assert_cmpstr (g_hash_table_lookup (debug->registers_before, "rax"), ==, "0x1");
    g_assert_cmpstr (g_hash_table_lookup (debug->registers_before, "rip"), ==, "0x1000");
    g_ptr_array_unref (reply.registers);

    reply.ok = FALSE;
    reply.registers = NULL;
    reply.msg = "No registers.";
    debug_reply_registers (debug, &reply, NULL);
    g_assert_cmpuint (debug->registers->len, ==, 0);
    g_assert_cmpstr (g_hash_table_lookup (debug->registers_before, "rax"), ==, "0x2");
    g_assert_null (g_hash_table_lookup (debug->registers_before, "rip"));
    g_assert_cmpstr (told->str, ==, "console: No registers.\n");
    teardown ();
}

/* The local variables of a frame, the members the panel read forgotten */
static void
test_variables (void)
{
    debug_reply_t reply = { 0 };

    setup ();
    reply.ok = TRUE;
    reply.variables = g_ptr_array_new_with_free_func (debug_variable_free);
    value_add (reply.variables, "x", "1");
    debug_reply_variables (debug, &reply, NULL);
    g_assert_true (debug->locals == reply.variables);
    g_assert_cmpstr (told->str, ==, "forget members\n");
    g_ptr_array_unref (reply.variables);
    reply.variables = NULL;
    debug_reply_variables (debug, &reply, NULL);
    g_assert_cmpuint (debug->locals->len, ==, 0);
    teardown ();
}

/* The watches: once each, asked for at a stop, their values or what the debugger said */
static void
test_watches (void)
{
    debug_reply_t reply = { 0 };

    setup ();
    debug->state = DEBUG_RUNNING;
    g_assert_cmpint (debug_watch_add (debug, "p->x"), ==, MC_EPR_OK);
    g_assert_cmpint (debug_watch_add (debug, "n"), ==, MC_EPR_OK);
    g_assert_cmpint (debug_watch_add (debug, "n"), ==, MC_EPR_FAILED);
    g_assert_cmpstr (told->str, ==, "error: This expression is already watched.\n");
    g_assert_cmpint (saved, ==, 2);
    g_assert_cmpuint (debug->watches->len, ==, 2);
    // running: not asked for
    debug_watches_refresh (debug);
    g_assert_null (watch_at (0)->value);

    g_string_truncate (told, 0);
    debug->state = DEBUG_STOPPED;
    debug_watches_refresh (debug);
    g_assert_cmpstr (told->str, ==, "evaluate p->x\nevaluate n\n");
    g_assert_cmpstr (watch_at (0)->value, ==, "<evaluating>");
    g_assert_cmpuint (watch_at (1)->pending_token, ==, 2);

    reply.request = 2;
    reply.ok = TRUE;
    reply.value = "5";
    debug_reply_watch (debug, &reply, NULL);
    g_assert_cmpstr (watch_at (1)->value, ==, "5");
    g_assert_false (watch_at (1)->failed);
    g_assert_cmpuint (watch_at (1)->pending_token, ==, 0);
    reply.request = 1;
    reply.ok = FALSE;
    reply.msg = "Cannot access memory at address 0x0";
    debug_reply_watch (debug, &reply, NULL);
    g_assert_cmpstr (watch_at (0)->value, ==, "Cannot access memory at address 0x0");
    g_assert_true (watch_at (0)->failed);
    // an answer of none, and one with nothing in it
    reply.request = 1;
    reply.ok = TRUE;
    reply.value = "9";
    debug_reply_watch (debug, &reply, NULL);
    g_assert_cmpstr (watch_at (0)->value, ==, "Cannot access memory at address 0x0");
    watch_at (0)->pending_token = 7;
    reply.request = 7;
    reply.value = NULL;
    debug_reply_watch (debug, &reply, NULL);
    g_assert_cmpstr (watch_at (0)->value, ==, "<unavailable>");

    // the debugger gone: said so
    refuse = TRUE;
    debug_watches_refresh (debug);
    g_assert_cmpstr (watch_at (1)->value, ==, "<GDB unavailable>");
    g_assert_true (watch_at (1)->failed);

    debug_watches_clear_values (debug);
    g_assert_null (watch_at (0)->value);
    g_assert_cmpuint (watch_at (0)->pending_token, ==, 0);

    // a watch added at a stop is asked for at once
    refuse = FALSE;
    g_string_truncate (told, 0);
    g_assert_cmpint (debug_watch_add (debug, "m"), ==, MC_EPR_OK);
    g_assert_nonnull (strstr (told->str, "evaluate m\n"));
    teardown ();
}

/* --------------------------------------------------------------------------------------------- */

int
main (int argc, char **argv)
{
    g_test_init (&argc, &argv, NULL);
    g_test_add_func ("/debugger/values/words", test_words);
    g_test_add_func ("/debugger/values/member-expression", test_member_expression);
    g_test_add_func ("/debugger/values/notes", test_notes);
    g_test_add_func ("/debugger/values/stack", test_stack);
    g_test_add_func ("/debugger/values/stack-source-frame", test_stack_source_frame);
    g_test_add_func ("/debugger/values/registers", test_registers);
    g_test_add_func ("/debugger/values/variables", test_variables);
    g_test_add_func ("/debugger/values/watches", test_watches);
    return g_test_run ();
}
