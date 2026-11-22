/*
   Hypertext file browser of bluerig
   Markdown help pages

   Copyright (C) 2026
   Ilia Maslakov il.smind@gmail.com

   This file is part of bluerig, a text editor based on GNU Midnight Commander.

   bluerig is free software: you can redistribute it
   and/or modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of the License,
   or (at your option) any later version.

   bluerig is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see https://www.gnu.org/licenses/.
 */

/** \file help_md.c
 *  \brief Source: markdown help pages
 *
 *  The help window reads the same markdown the man pages are made from.
 *  This turns it into the text the window paints: one node per heading,
 *  named by the anchor in front of it, links between the nodes, and a
 *  contents page built from the headings.
 *
 *  The markdown it understands is what maint/check-man.pl allows.
 */

#include <config.h>

#include <string.h>

#include "lib/global.h"
#include "lib/tty/tty.h"  // mc_tty_frm_t
#include "lib/keybind.h"
#include "lib/editor-plugin.h"

#include "src/keymap.h"  // editor_map, editor_x_map

#include "help.h"
#include "help_md.h"

/*** global variables ****************************************************************************/

/*** file scope macro definitions ****************************************************************/

#define BODY_INDENT "        "
// a column of a table, but for the last, is never wider than this: the cell after a wider one
// starts the next line
#define TABLE_CELL_MAX 40

/*** file scope type declarations ****************************************************************/

typedef struct
{
    char *id;     // the name the links use
    char *title;  // what the contents page shows
    int level;    // how deep the heading was
} md_node_t;

typedef struct
{
    GString *body;     // the nodes, in the order of the page
    GString *para;     // the paragraph being collected
    GPtrArray *nodes;  // md_node_t, and NULL for a break in the contents
    char *topics;      // the heading of the contents page
    char *anchor;      // the id the next heading is to get
    gboolean skip;     // the next section stays out of the help
    gboolean skipping;
    gboolean code;     // inside a fenced block
    gboolean front;    // inside the front matter of a page
    gboolean term;     // the paragraph is the term of a definition
    gboolean notitle;  // the next heading is not to be printed
    gboolean open;     // a node has been started
} md_ctx_t;

/*** forward declarations (file scope functions) *************************************************/

/*** file scope variables ************************************************************************/

/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/** The name a heading is known by when it carries no anchor of its own. */

char *
help_md_node_id (const char *name)
{
    GString *id;
    char *folded;
    const char *p;
    gboolean dash = FALSE;

    folded = g_utf8_strdown (name, -1);
    id = g_string_sized_new (strlen (folded) + 1);

    for (p = folded; *p != '\0'; p = g_utf8_next_char (p))
    {
        gunichar c = g_utf8_get_char (p);

        if (g_unichar_isspace (c) || c == '-')
            dash = id->len != 0;
        else if (g_unichar_isalnum (c) || c == '_')
        {
            if (dash)
            {
                g_string_append_c (id, '-');
                dash = FALSE;
            }
            g_string_append_unichar (id, c);
        }
    }

    g_free (folded);

    return g_string_free (id, FALSE);
}

/* --------------------------------------------------------------------------------------------- */

static void
md_node_free (gpointer data)
{
    md_node_t *node = (md_node_t *) data;

    if (node != NULL)
    {
        g_free (node->id);
        g_free (node->title);
        g_free (node);
    }
}

/* --------------------------------------------------------------------------------------------- */
/** Text of one line, without the markdown that marks it up. */

static void
md_inline (GString *out, const char *line)
{
    const char *p = line;

    while (*p != '\0')
    {
        if (p[0] == '\\' && p[1] != '\0')
        {
            // an escaped character stands for itself
            const char *n = g_utf8_next_char (p + 1);

            g_string_append_len (out, p + 1, n - (p + 1));
            p = n;
        }
        else if (p[0] == '<' && p[1] == '!' && strncmp (p, "<!--", 4) == 0)
        {
            const char *end = strstr (p, "-->");

            p = end == NULL ? p + strlen (p) : end + 3;
        }
        else if (p[0] == '<' && strchr (p, '>') != NULL
                 && (strncmp (p, "<http", 5) == 0 || strncmp (p, "<ftp", 4) == 0))
        {
            // an autolink prints the address it holds
            const char *end = strchr (p, '>');

            g_string_append_len (out, p + 1, end - (p + 1));
            p = end + 1;
        }
        else if (p[0] == '`')
        {
            const char *fence = p;
            size_t len;
            const char *end;

            while (*p == '`')
                p++;
            len = p - fence;
            end = strstr (p, fence);
            while (end != NULL && end[len] == '`')
                end = strstr (end + 1, fence);

            if (end == NULL)
                g_string_append_len (out, fence, len);
            else
            {
                const char *text = p;
                const char *stop = end;

                // a fence wider than the text it holds keeps a space on each side
                if (text < stop && *text == ' ' && stop[-1] == ' ')
                {
                    text++;
                    stop--;
                }

                g_string_append_c (out, CHAR_FONT_BOLD);
                g_string_append_len (out, text, stop - text);
                g_string_append_c (out, CHAR_FONT_NORMAL);
                p = end + len;
            }
        }
        else if (p[0] == '[')
        {
            const char *label = p + 1;
            const char *end = strchr (label, ']');

            if (end == NULL || end[1] != '(')
            {
                g_string_append_c (out, *p);
                p++;
            }
            else
            {
                const char *target = end + 2;
                const char *close = strchr (target, ')');

                if (close == NULL)
                {
                    g_string_append_c (out, *p);
                    p++;
                }
                else
                {
                    char *text;

                    text = g_strndup (label, end - label);

                    if (*target == '#' || g_str_has_prefix (close - 3, ".md")
                        || strstr (target, ".md#") != NULL)
                    {
                        // a node of this page, or of another help file
                        g_string_append_c (out, CHAR_LINK_START);
                        md_inline (out, text);
                        g_string_append_c (out, CHAR_LINK_POINTER);
                        g_string_append_len (out, target + (*target == '#' ? 1 : 0),
                                             close - target - (*target == '#' ? 1 : 0));
                        g_string_append_c (out, CHAR_LINK_END);
                    }
                    else
                    {
                        // nothing outside the help to jump to: name it instead
                        md_inline (out, text);
                        g_string_append_c (out, ' ');
                        g_string_append_len (out, target, close - target);
                    }

                    g_free (text);
                    p = close + 1;
                }
            }
        }
        else if (p[0] == '*')
        {
            gboolean strong = p[1] == '*';
            const char *mark = strong ? "**" : "*";
            const char *text = p + (strong ? 2 : 1);
            const char *end = strstr (text, mark);

            while (end != NULL && end > text && end[-1] == '\\')
                end = strstr (end + 1, mark);

            if (end == NULL || end == text)
            {
                g_string_append_c (out, *p);
                p++;
            }
            else
            {
                char *text_dup;

                text_dup = g_strndup (text, end - text);
                g_string_append_c (out, strong ? CHAR_FONT_BOLD : CHAR_FONT_ITALIC);
                md_inline (out, text_dup);
                g_string_append_c (out, CHAR_FONT_NORMAL);
                g_free (text_dup);
                p = end + (strong ? 2 : 1);
            }
        }
        else
        {
            const char *n = g_utf8_next_char (p);

            g_string_append_len (out, p, n - p);
            p = n;
        }
    }
}

/* --------------------------------------------------------------------------------------------- */
/** The columns the text the help window paints takes: the markers and the targets of the links
 * take none.
 */

static int
md_width (const char *text)
{
    const char *p;
    int width = 0;
    gboolean shown = TRUE;

    for (p = text; *p != '\0'; p = g_utf8_next_char (p))
    {
        const gunichar c = g_utf8_get_char (p);

        if (c == CHAR_LINK_POINTER)
            shown = FALSE;
        else if (c == CHAR_LINK_END)
            shown = TRUE;
        else if (shown && c >= ' ')
            width += g_unichar_iswide (c) ? 2 : 1;
    }

    return width;
}

/* --------------------------------------------------------------------------------------------- */
/** Whether the line is the one under the head of a table: |---|:--| and the like. */

static gboolean
md_table_rule (const char *line)
{
    const char *p;
    gboolean dash = FALSE;

    if (line == NULL || line[0] != '|')
        return FALSE;

    for (p = line; *p != '\0'; p++)
    {
        if (*p == '-')
            dash = TRUE;
        else if (*p != '|' && *p != ':' && *p != ' ')
            return FALSE;
    }

    return dash;
}

/* --------------------------------------------------------------------------------------------- */
/** The cells of a line of a table, as the help window paints them.  \| is a bar inside a cell. */

static GPtrArray *
md_table_cells (const char *line)
{
    GPtrArray *cells;
    GString *cell;
    const char *p;

    cells = g_ptr_array_new_with_free_func (g_free);
    cell = g_string_sized_new (64);

    for (p = line + 1; *p != '\0'; p++)
    {
        if (*p == '\\' && p[1] == '|')
            g_string_append_c (cell, *++p);
        else if (*p != '|')
            g_string_append_c (cell, *p);
        else
        {
            GString *out = g_string_sized_new (cell->len);

            g_strstrip (cell->str);
            md_inline (out, cell->str);
            g_ptr_array_add (cells, g_string_free (out, FALSE));
            g_string_set_size (cell, 0);
        }
    }

    // text after the last bar is a cell too
    g_strstrip (cell->str);
    if (cell->str[0] != '\0')
    {
        GString *out = g_string_sized_new (cell->len);

        md_inline (out, cell->str);
        g_ptr_array_add (cells, g_string_free (out, FALSE));
    }
    g_string_free (cell, TRUE);

    return cells;
}

/* --------------------------------------------------------------------------------------------- */
/** Write out a table: a row on a line, each cell at the column of its own, and a line of a cell
 * the window breaks goes on at that column.  A head with no text in it is left out, a head with
 * text is bold.
 */

static void
md_table (md_ctx_t *ctx, GPtrArray *rows)
{
    GArray *widths;
    guint r, c;
    gboolean head = FALSE;

    widths = g_array_new (FALSE, TRUE, sizeof (int));

    for (r = 0; r < rows->len; r++)
    {
        const GPtrArray *cells = g_ptr_array_index (rows, r);

        for (c = 0; c < cells->len; c++)
        {
            const int w = md_width (g_ptr_array_index (cells, c));

            if (c >= widths->len)
                g_array_set_size (widths, c + 1);
            // a cell too wide for a column takes its line, and the column stays as it is
            if (w <= TABLE_CELL_MAX)
                g_array_index (widths, int, c) = MAX (g_array_index (widths, int, c), w);
            if (r == 0 && w != 0)
                head = TRUE;
        }
    }

    for (r = head ? 0 : 1; r < rows->len; r++)
    {
        const GPtrArray *cells = g_ptr_array_index (rows, r);
        int column = 0;

        for (c = 0; c < cells->len; c++)
        {
            if (c != 0)
            {
                column += g_array_index (widths, int, c - 1) + 2;
                g_string_append_printf (ctx->body, "%c%d%c", CHAR_COLUMN, column, CHAR_COLUMN);
            }
            if (r == 0)
                g_string_append_printf (ctx->body, "%c%s%c", CHAR_FONT_BOLD,
                                        (const char *) g_ptr_array_index (cells, c),
                                        CHAR_FONT_NORMAL);
            else
                g_string_append (ctx->body, g_ptr_array_index (cells, c));
        }
        g_string_append_c (ctx->body, '\n');
    }
    g_string_append_c (ctx->body, '\n');

    g_array_free (widths, TRUE);
}

/* --------------------------------------------------------------------------------------------- */
/** Write out the paragraph held so far.  Markdown fills it, so it goes out as one line and
 * the help window breaks it where the window ends.
 */

static void
md_flush (md_ctx_t *ctx)
{
    if (ctx->para->len == 0)
        return;

    // a break at the end of the last line of a paragraph: the flush ends it anyway
    while (ctx->para->len != 0 && ctx->para->str[ctx->para->len - 1] == '\n')
        g_string_set_size (ctx->para, ctx->para->len - 1);
    if (ctx->para->len == 0)
        return;

    if (ctx->term)
    {
        // the markdown of a term carries its own emphasis
        size_t len = ctx->body->len;

        if (len != 0
            && !(ctx->body->str[len - 1] == '\n' && len > 1 && ctx->body->str[len - 2] == '\n'))
            g_string_append_c (ctx->body, '\n');
        md_inline (ctx->body, ctx->para->str);
        g_string_append_c (ctx->body, '\n');
    }
    else
    {
        md_inline (ctx->body, ctx->para->str);
        g_string_append (ctx->body, "\n\n");
    }

    g_string_set_size (ctx->para, 0);
}

/* --------------------------------------------------------------------------------------------- */

static void
md_collect (md_ctx_t *ctx, const char *text, const char *indent)
{
    size_t len = strlen (text);
    gboolean hard;

    /* two spaces at the end of a line are a line break markdown keeps, and the help window
       breaks a line at a newline of its own, so the break is carried over */
    hard = len >= 2 && text[len - 1] == ' ' && text[len - 2] == ' ';
    while (len > 0 && text[len - 1] == ' ')
        len--;

    // after a break the next line starts where the broken one started
    if (ctx->para->len == 0 || ctx->para->str[ctx->para->len - 1] == '\n')
    {
        if (indent != NULL)
            g_string_append (ctx->para, indent);
    }
    else
        g_string_append_c (ctx->para, ' ');

    g_string_append_len (ctx->para, text, len);

    if (hard)
        g_string_append_c (ctx->para, '\n');
}

/* --------------------------------------------------------------------------------------------- */

/** Take the anchor and the markers off the end of a heading, leaving its title. */

static char *
md_heading_marks (md_ctx_t *ctx, const char *line)
{
    GString *title;
    char *p;

    title = g_string_new (line);

    while ((p = strstr (title->str, "<a id=\"")) != NULL)
    {
        char *end = strstr (p + 7, "\"></a>");

        if (end == NULL)
            break;

        g_free (ctx->anchor);
        ctx->anchor = g_strndup (p + 7, end - (p + 7));
        g_string_erase (title, p - title->str, end + 6 - p);
    }

    while ((p = strstr (title->str, "<!-- help:")) != NULL)
    {
        char *end = strstr (p, "-->");

        if (end == NULL)
            break;

        if (strncmp (p + 10, "notitle", 7) == 0)
            ctx->notitle = TRUE;
        else if (strncmp (p + 10, "skip", 4) == 0)
            ctx->skip = TRUE;

        g_string_erase (title, p - title->str, end + 3 - p);
    }

    g_strchomp (title->str);
    title->len = strlen (title->str);

    return g_string_free (title, FALSE);
}

/* --------------------------------------------------------------------------------------------- */

static void
md_heading (md_ctx_t *ctx, const char *line, int level)
{
    md_node_t *node;
    GString *title;
    char *text;

    md_flush (ctx);

    text = md_heading_marks (ctx, line);

    if (ctx->skip)
    {
        ctx->skip = FALSE;
        ctx->skipping = TRUE;
        g_free (ctx->anchor);
        ctx->anchor = NULL;
        g_free (text);
        return;
    }

    ctx->skipping = FALSE;

    title = g_string_sized_new (64);
    md_inline (title, text);
    g_free (text);

    node = g_new0 (md_node_t, 1);
    node->id = ctx->anchor != NULL ? ctx->anchor : help_md_node_id (title->str);
    node->title = g_string_free (title, FALSE);
    node->level = level;
    ctx->anchor = NULL;
    g_ptr_array_add (ctx->nodes, node);

    if (ctx->notitle)
    {
        g_string_append_printf (ctx->body, "%c[%s]\n", CHAR_NODE_END, node->id);
        ctx->notitle = FALSE;
    }
    else
        g_string_append_printf (ctx->body, "%c[%s]\n%c%s%c\n\n", CHAR_NODE_END, node->id,
                                CHAR_FONT_BOLD, node->title, CHAR_FONT_NORMAL);

    ctx->open = TRUE;
}

/* --------------------------------------------------------------------------------------------- */
/** Whether the line holds an anchor and nothing else but blanks. */

static gboolean
md_anchor_alone (const char *line)
{
    const char *p;

    p = strstr (line, "></a>");
    if (p == NULL)
        return FALSE;

    for (p += 5; *p != '\0'; p++)
        if (*p != ' ' && *p != '\t')
            return FALSE;

    return TRUE;
}

/* --------------------------------------------------------------------------------------------- */
/** The value of a marker comment, as in <!-- help:topics "Topics:" --> */

static char *
md_marker_value (const char *line)
{
    const char *start;
    const char *end;

    start = strchr (line, '"');
    if (start == NULL)
        return NULL;
    start++;
    end = strchr (start, '"');

    return end == NULL ? NULL : g_strndup (start, end - start);
}

/* --------------------------------------------------------------------------------------------- */

static void
md_parse (md_ctx_t *ctx, const char *text)
{
    char **lines;
    int i;

    lines = g_strsplit (text, "\n", -1);

    for (i = 0; lines[i] != NULL; i++)
    {
        char *line = lines[i];
        int level;

        if (ctx->code)
        {
            if (strncmp (line, "```", 3) == 0)
            {
                g_string_append_c (ctx->body, '\n');
                ctx->code = FALSE;
            }
            else if (!ctx->skipping)
            {
                g_string_append (ctx->body, line);
                g_string_append_c (ctx->body, '\n');
            }
            continue;
        }

        // the front matter of a man page carries its date, nothing the help
        // window shows
        if (i == 0 && strcmp (line, "---") == 0)
        {
            ctx->front = TRUE;
            continue;
        }
        if (ctx->front)
        {
            if (strcmp (line, "---") == 0)
                ctx->front = FALSE;
            continue;
        }

        if (strncmp (line, "<!--", 4) == 0)
        {
            char *value;

            if (strstr (line, "help:break") != NULL)
            {
                md_flush (ctx);
                g_ptr_array_add (ctx->nodes, NULL);
            }
            else if (strstr (line, "help:topics") != NULL)
            {
                value = md_marker_value (line);
                if (value != NULL)
                {
                    g_free (ctx->topics);
                    ctx->topics = value;
                }
            }
            continue;
        }

        // an anchor of its own marks a place inside the node; one with text
        // after it belongs to a heading, and is taken there. It takes no room:
        // the text after it starts on its line, not under an empty one
        if (strncmp (line, "<a id=\"", 7) == 0 && md_anchor_alone (line))
        {
            const char *end = strchr (line + 7, '"');

            if (end != NULL && !ctx->skipping)
            {
                md_flush (ctx);
                g_string_append_c (ctx->body, CHAR_ANCHOR);
                g_string_append_len (ctx->body, line + 7, end - (line + 7));
                g_string_append_c (ctx->body, CHAR_ANCHOR);
            }
            continue;
        }

        for (level = 0; line[level] == '#'; level++)
            ;
        if (level > 0 && line[level] == ' ')
        {
            md_heading (ctx, line + level + 1, level);
            continue;
        }

        if (ctx->skipping)
            continue;

        if (strncmp (line, "```", 3) == 0)
        {
            md_flush (ctx);
            ctx->code = TRUE;
            continue;
        }

        if (line[0] == '\0')
        {
            md_flush (ctx);
            ctx->term = FALSE;
            continue;
        }

        // a table: the head, the rule under it, and the rows up to a line that is not one
        if (line[0] == '|' && ctx->para->len == 0 && md_table_rule (lines[i + 1]))
        {
            GPtrArray *rows = g_ptr_array_new_with_free_func ((GDestroyNotify) g_ptr_array_unref);

            g_ptr_array_add (rows, md_table_cells (line));
            for (i += 2; lines[i] != NULL && lines[i][0] == '|'; i++)
                g_ptr_array_add (rows, md_table_cells (lines[i]));
            i--;

            md_table (ctx, rows);
            g_ptr_array_free (rows, TRUE);
            continue;
        }

        if (line[0] == ':' && line[1] == ' ')
        {
            // the body of the definition the line before opened
            md_flush (ctx);
            ctx->term = FALSE;
            md_collect (ctx, line + 2, BODY_INDENT);
            continue;
        }

        if (line[0] == '>')
        {
            const char *text_start = line[1] == ' ' ? line + 2 : line + 1;

            md_collect (ctx, text_start, BODY_INDENT);
            continue;
        }

        // a term is a line of its own that the next line describes
        if (ctx->para->len == 0 && lines[i + 1] != NULL && lines[i + 1][0] == ':'
            && lines[i + 1][1] == ' ')
            ctx->term = TRUE;

        md_collect (ctx, line, NULL);
    }

    md_flush (ctx);
    g_strfreev (lines);
}

/* --------------------------------------------------------------------------------------------- */
/** A picture of lines on a terminal that knows no UTF-8: its line drawing characters become those
 * of the frames of the skin, which the locale has, KOI8-R double lines too, or else the terminal
 * draws.  The charset of the terminal would have no room for most of them.
 */

static void
md_lines (GString *text)
{
    static const struct
    {
        gunichar c;
        mc_tty_frm_t frm;
    } line_char[] = {
        { 0x2500, MC_TTY_FRM_HORIZ },         { 0x2502, MC_TTY_FRM_VERT },
        { 0x250C, MC_TTY_FRM_LEFTTOP },       { 0x2510, MC_TTY_FRM_RIGHTTOP },
        { 0x2514, MC_TTY_FRM_LEFTBOTTOM },    { 0x2518, MC_TTY_FRM_RIGHTBOTTOM },
        { 0x252C, MC_TTY_FRM_TOPMIDDLE },     { 0x2534, MC_TTY_FRM_BOTTOMMIDDLE },
        { 0x251C, MC_TTY_FRM_LEFTMIDDLE },    { 0x2524, MC_TTY_FRM_RIGHTMIDDLE },
        { 0x253C, MC_TTY_FRM_CROSS },         { 0x2550, MC_TTY_FRM_DHORIZ },
        { 0x2551, MC_TTY_FRM_DVERT },         { 0x2554, MC_TTY_FRM_DLEFTTOP },
        { 0x2557, MC_TTY_FRM_DRIGHTTOP },     { 0x255A, MC_TTY_FRM_DLEFTBOTTOM },
        { 0x255D, MC_TTY_FRM_DRIGHTBOTTOM },  { 0x2564, MC_TTY_FRM_DTOPMIDDLE },
        { 0x2567, MC_TTY_FRM_DBOTTOMMIDDLE }, { 0x255F, MC_TTY_FRM_DLEFTMIDDLE },
        { 0x2562, MC_TTY_FRM_DRIGHTMIDDLE },
    };

    GString *out;
    const char *p;

    if (mc_global.utf8_display)
        return;

    out = g_string_sized_new (text->len);

    for (p = text->str; *p != '\0'; p = g_utf8_next_char (p))
    {
        const gunichar c = g_utf8_get_char (p);
        size_t j;

        for (j = 0; j < G_N_ELEMENTS (line_char) && line_char[j].c != c; j++)
            ;

        if (j < G_N_ELEMENTS (line_char))
        {
            g_string_append_c (out, CHAR_LINE);
            g_string_append_c (out, (char) ('A' + line_char[j].frm));
        }
        else
            g_string_append_len (out, p, g_utf8_next_char (p) - p);
    }

    g_string_assign (text, out->str);
    g_string_free (out, TRUE);
}

/* --------------------------------------------------------------------------------------------- */
/** What the build put in the paths the help text names, and the version of the program. */

static void
md_substitute (GString *text)
{
    static const struct
    {
        const char *name;
        const char *value;
    } paths[] = {
        { "{{pkgdatadir}}", MC_PKGDATADIR },
        { "{{sysconfdir}}", MC_SYSCONFDIR },
        { "{{pkglibexecdir}}", MC_PKGLIBEXECDIR },
        { "{{plugins_dir}}", MC_PLUGINS_DIR },
        { "{{bindir}}", MC_BINDIR },
    };

    size_t i;

    for (i = 0; i < G_N_ELEMENTS (paths); i++)
        g_string_replace (text, paths[i].name, paths[i].value, 0);

    // The version stands in a field of its own, and the field says how wide
    // it is: %MC_VERSION:32% takes 32 columns of the markdown, the same 32 the
    // window gives it, so the picture around it stands where it is written.
    // The width goes into the text between two markers for the window to read.
    {
        const char *token = "{{MC_VERSION";
        const size_t len = strlen (token);
        char *p;

        while ((p = strstr (text->str, token)) != NULL)
        {
            size_t start = p - text->str;
            size_t end = len;
            int width = 0;
            char *marker;

            if (p[end] == ':')
                for (end++; g_ascii_isdigit (p[end]); end++)
                    width = width * 10 + (p[end] - '0');

            if (p[end] != '}' || p[end + 1] != '}')
                break;  // not a field of ours, and the next pass would loop
            end += 2;

            // the spaces that make the field as wide as it says it is
            while ((int) end < width && p[end] == ' ')
                end++;

            marker = g_strdup_printf ("%c%d%c", CHAR_VERSION, width, CHAR_VERSION);
            g_string_erase (text, start, end);
            g_string_insert (text, start, marker);
            g_free (marker);
        }
    }
}

/* --------------------------------------------------------------------------------------------- */
/** Append text with a backslash before each character markdown reads. */

static void
md_escaped (GString *out, const char *text)
{
    for (; *text != '\0'; text++)
    {
        if (strchr ("\\*_[]<>|`", *text) != NULL)
            g_string_append_c (out, '\\');
        g_string_append_c (out, *text);
    }
}

/* --------------------------------------------------------------------------------------------- */
/** Append the keys of keymap that call action, each of them once and only where the editor finds
 * that action for it: a key bound twice calls the first.  prefix is written before each key of a
 * keymap that a prefix key opens.
 */

static void
md_keys_of (GString *keys, const global_keymap_t *keymap, long action, const char *prefix)
{
    size_t i;

    if (keymap == NULL)
        return;

    for (i = 0; keymap[i].key != 0; i++)
    {
        GString *key;

        if (keymap[i].command != action || keymap[i].caption[0] == '\0'
            || keybind_lookup_keymap_command (keymap, keymap[i].key) != action)
            continue;

        // the caption may hold a character markdown reads, as Alt-_ does
        key = g_string_new (prefix != NULL ? prefix : "");
        if (prefix != NULL)
            g_string_append_c (key, ' ');
        md_escaped (key, keymap[i].caption);

        // a key bound twice is written once; the list is compared item by item
        {
            char *list = g_strconcat (", ", keys->str, ", ", (char *) NULL);
            char *item = g_strconcat (", ", key->str, ", ", (char *) NULL);

            if (strstr (list, item) == NULL)
            {
                if (keys->len != 0)
                    g_string_append (keys, ", ");
                g_string_append (keys, key->str);
            }
            g_free (item);
            g_free (list);
        }
        g_string_free (key, TRUE);
    }
}

/* --------------------------------------------------------------------------------------------- */
/** The name of a plugin as the index shows it: its label without the hotkey mark and the dots
 * of a dialog.
 */

static char *
md_plugin_title (const mc_editor_plugin_t *plugin)
{
    GString *title;
    const char *p;

    title = g_string_new ("");
    for (p = plugin->display_name != NULL ? _ (plugin->display_name) : plugin->name; *p != '\0';
         p++)
        if (*p != '&')
            g_string_append_c (title, *p);
    while (title->len != 0 && title->str[title->len - 1] == '.')
        g_string_truncate (title, title->len - 1);

    return g_string_free (title, FALSE);
}

/* --------------------------------------------------------------------------------------------- */

static int
md_plugin_compare (gconstpointer a, gconstpointer b)
{
    return g_utf8_collate (*(const char *const *) a, *(const char *const *) b);
}

/* --------------------------------------------------------------------------------------------- */
/** The index of the plugins: a row for each plugin loaded with a help of its own, its name a link
 * to that help and what it does beside it, by name.
 */

static void
md_plugins (GString *out)
{
    GPtrArray *rows;
    const GSList *l;
    guint i;

    rows = g_ptr_array_new_with_free_func (g_free);

    for (l = mc_editor_plugin_list (); l != NULL; l = g_slist_next (l))
    {
        const mc_editor_plugin_t *plugin = (const mc_editor_plugin_t *) l->data;
        GString *row;
        char *title;

        if (plugin->help_file == NULL)
            continue;

        // the title first: the rows are sorted by it
        title = md_plugin_title (plugin);
        row = g_string_new ("");
        md_escaped (row, title);
        g_string_append (row, "\t| [");
        md_escaped (row, title);
        g_string_append_printf (row, "](%s#%s) | ", plugin->help_file,
                                plugin->help_node != NULL ? plugin->help_node : "");
        if (plugin->summary != NULL)
            md_escaped (row, _ (plugin->summary));
        g_string_append (row, " |\n");
        g_ptr_array_add (rows, g_string_free (row, FALSE));
        g_free (title);
    }

    if (rows->len != 0)
    {
        g_ptr_array_sort (rows, md_plugin_compare);
        g_string_append (out, "|  |  |\n|--|--|\n");
        for (i = 0; i < rows->len; i++)
            g_string_append (out, strchr (g_ptr_array_index (rows, i), '\t') + 1);
    }

    g_ptr_array_free (rows, TRUE);
}

/* --------------------------------------------------------------------------------------------- */
/** The page with each {{key:Action|default}} made the keys of the editor that call the action
 * now; the default, as the man page has it, where none of them does.
 */

static char *
md_keys (const char *text)
{
    GString *out;
    const char *p, *start;

    out = g_string_sized_new (strlen (text) + 1024);

    for (p = text; (start = strstr (p, "{{")) != NULL; p = start)
    {
        const char *name = start + 6;

        // {{plugins}} on a line of its own: the index of the plugins
        if (strncmp (start, "{{plugins}}", 11) == 0)
        {
            g_string_append_len (out, p, start - p);
            md_plugins (out);
            start += 11;
            if (*start == '\n')
                start++;
            continue;
        }
        if (strncmp (start, "{{key:", 6) != 0)
        {
            g_string_append_len (out, p, start + 2 - p);
            start += 2;
            continue;
        }
        const char *bar = strchr (name, '|');
        const char *end = strstr (name, "}}");
        GString *keys;
        char *action_name;
        long action;

        g_string_append_len (out, p, start - p);

        if (bar == NULL || end == NULL || bar > end)
        {
            // not a marker: the text goes as it is
            g_string_append_len (out, start, 6);
            start = name;
            continue;
        }

        keys = g_string_new ("");
        action_name = g_strndup (name, bar - name);
        action = keybind_lookup_action (action_name);
        if (action > 0)
        {
            md_keys_of (keys, editor_map, action, NULL);
            md_keys_of (keys, editor_x_map, action,
                        keybind_lookup_keymap_shortcut (editor_map, CK_ExtendedKeyMap));
        }
        g_free (action_name);

        if (keys->len != 0)
            g_string_append (out, keys->str);
        else
            g_string_append_len (out, bar + 1, end - (bar + 1));
        g_string_free (keys, TRUE);

        start = end + 2;
    }
    g_string_append (out, p);

    return g_string_free (out, FALSE);
}

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/** Turn the markdown of a help page, and of the template that goes with it, into the text the
 * help window paints.  The contents page comes first, as the window looks for [Contents] there.
 */

char *
help_md_convert (const char *page, const char *tmpl)
{
    md_ctx_t ctx;
    GString *out;
    guint i;

    memset (&ctx, 0, sizeof (ctx));
    ctx.body = g_string_sized_new (128 * 1024);
    ctx.para = g_string_sized_new (1024);
    ctx.nodes = g_ptr_array_new_with_free_func (md_node_free);

    // the keys go in first: a marker holds a bar, and a table would take it for one of its own
    if (page != NULL)
    {
        char *text = md_keys (page);

        md_parse (&ctx, text);
        g_free (text);
    }
    if (tmpl != NULL)
    {
        char *text = md_keys (tmpl);

        md_parse (&ctx, text);
        g_free (text);
    }

    out = g_string_sized_new (ctx.body->len + 4096);
    g_string_append_printf (out, "%c[Contents]\n", CHAR_NODE_END);
    if (ctx.topics != NULL)
        g_string_append_printf (out, "%s\n\n", ctx.topics);

    for (i = 0; i < ctx.nodes->len; i++)
    {
        const md_node_t *node = (const md_node_t *) g_ptr_array_index (ctx.nodes, i);

        if (node == NULL)
            g_string_append_c (out, '\n');
        else if (strcmp (node->id, "main") == 0)
            ;  // the screen the help opens on is not a topic of its own
        else
            g_string_append_printf (out, "  %*s%c%s%c%s%c\n", 2 * (node->level - 1), "",
                                    CHAR_LINK_START, node->title, CHAR_LINK_POINTER, node->id,
                                    CHAR_LINK_END);
    }

    g_string_append (out, ctx.body->str);
    g_string_append_c (out, CHAR_NODE_END);

    md_substitute (out);
    md_lines (out);

    g_string_free (ctx.body, TRUE);
    g_string_free (ctx.para, TRUE);
    g_ptr_array_free (ctx.nodes, TRUE);
    g_free (ctx.topics);
    g_free (ctx.anchor);

    return g_string_free (out, FALSE);
}

/* --------------------------------------------------------------------------------------------- */
