/*
   A screen for the tests: the real tty layer of S-Lang on a pseudo-terminal of the test, read
   back from the virtual screen of S-Lang.  The widgets draw as they do on a terminal; the test
   reads the characters and the colors of the cells.

   Copyright (C) 2026
   Ilia Maslakov <il.smind@gmail.com>

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

#ifndef MC__TESTS_SCREEN_H
#define MC__TESTS_SCREEN_H

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#ifdef HAVE_PTY_H
#include <pty.h>
#endif
#ifdef HAVE_UTIL_H
#include <util.h>
#endif
#ifdef HAVE_LIBUTIL_H
#include <libutil.h>
#endif

#include <slang.h>

#include "lib/global.h"
#include "lib/skin.h"
#include "lib/strutil.h"
#include "lib/tty/color.h"
#include "lib/tty/key.h"
#include "lib/tty/tty.h"
#include "lib/widget.h"

#include "src/keymap.h"

/*** file scope variables ************************************************************************/

static int screen_master = -1;
static int screen_slave = -1;
static GThread *screen_reader = NULL;
static gint screen_reader_stop = 0;

/*** file scope functions ************************************************************************/

/* What the terminal is sent, read away all the time the screen is open: a write never waits for
   room, and the terminal modes set with TCSADRAIN by S-Lang never wait for the output to be
   read, which on macOS they do */
static gpointer
screen_read_away (gpointer data)
{
    char buf[4096];

    (void) data;
    while (g_atomic_int_get (&screen_reader_stop) == 0)
    {
        struct pollfd pfd = { screen_master, POLLIN, 0 };

        if (poll (&pfd, 1, 20) > 0 && read (screen_master, buf, sizeof (buf)) < 0 && errno != EAGAIN
            && errno != EINTR)
            break;
    }
    return NULL;
}

/* --------------------------------------------------------------------------------------------- */

/* A screen of @rows and @cols, in UTF-8, with the colors of the skin of the source tree */
static inline void
screen_open (int rows, int cols)
{
    struct winsize size = { 0 };
    char number[16];
    GError *error = NULL;

    size.ws_row = (unsigned short) rows;
    size.ws_col = (unsigned short) cols;
    if (openpty (&screen_master, &screen_slave, NULL, NULL, &size) != 0)
        abort ();
    (void) fcntl (screen_master, F_SETFL, fcntl (screen_master, F_GETFL) | O_NONBLOCK);
    g_atomic_int_set (&screen_reader_stop, 0);
    screen_reader = g_thread_new ("screen", screen_read_away, NULL);

    g_setenv ("TERM", "xterm", TRUE);
    g_snprintf (number, sizeof (number), "%d", rows);
    g_setenv ("LINES", number, TRUE);
    g_snprintf (number, sizeof (number), "%d", cols);
    g_setenv ("COLUMNS", number, TRUE);

    str_init_strings ("UTF-8");
    mc_global.tty.xterm_flag = TRUE;
    mc_global.tty.disable_x11 = TRUE;
    mc_global.utf8_display = TRUE;
    mc_global.tty.color_terminal_string = (char *) "";

    // the terminal of S-Lang is the pseudo-terminal, not the one of the test
    SLang_TT_Read_FD = screen_slave;
    SLang_TT_Write_FD = screen_slave;
    init_key ();
    tty_init (FALSE, FALSE);
    // the keys of the widgets as the program has them, not those of a file of the user
    keymap_load (FALSE);
    tty_init_colors (FALSE, FALSE);
    if (!mc_skin_init (TEST_SKIN_DIR "/default.ini", &error))
        g_clear_error (&error);
    // the colors of the widgets out of the skin, as the program takes them
    dlg_set_default_colors ();
    input_set_default_colors ();
}

/* --------------------------------------------------------------------------------------------- */

static inline void
screen_close (void)
{
    mc_skin_deinit ();
    tty_colors_done ();
    keymap_free ();
    tty_shutdown ();
    done_key ();
    g_atomic_int_set (&screen_reader_stop, 1);
    g_thread_join (screen_reader);
    screen_reader = NULL;
    close (screen_slave);
    close (screen_master);
    screen_slave = screen_master = -1;
    str_uninit_strings ();
}

/* --------------------------------------------------------------------------------------------- */

/* The cell at @row, @col of what was drawn */
static inline gboolean
screen_cell (int row, int col, SLsmg_Char_Type *cell)
{
    SLsmg_gotorc (row, col);
    return SLsmg_char_at (cell) >= 0;
}

/* --------------------------------------------------------------------------------------------- */

/* A line of a frame as ASCII: '-' along, '|' up and down, '+' where lines meet; else @c */
static inline gunichar
screen_ascii_line (gunichar c)
{
    if (c < 0x2500 || c > 0x257f)
        return c;
    switch (c)
    {
    case 0x2500:  // light
    case 0x2501:  // heavy
    case 0x2550:  // double
        return '-';
    case 0x2502:
    case 0x2503:
    case 0x2551:
        return '|';
    default:
        return '+';
    }
}

/* --------------------------------------------------------------------------------------------- */

/* The text of @row from @col for @len cells as UTF-8, a blank for an empty cell and the lines of
   frames as ASCII; to be freed */
static inline char *
screen_text (int row, int col, int len)
{
    GString *text = g_string_new (NULL);
    int c;

    for (c = col; c < col + len; c++)
    {
        SLsmg_Char_Type cell;

        if (!screen_cell (row, c, &cell) || cell.nchars == 0 || cell.wchars[0] == 0)
            g_string_append_c (text, ' ');
        else
        {
            unsigned int i;

            for (i = 0; i < cell.nchars; i++)
                g_string_append_unichar (text, screen_ascii_line ((gunichar) cell.wchars[i]));
        }
    }
    return g_string_free (text, FALSE);
}

/* --------------------------------------------------------------------------------------------- */

/* The whole of @row, its blanks at the end left out; to be freed */
static inline char *
screen_row (int row)
{
    char *text = screen_text (row, 0, COLS);

    g_strchomp (text);
    return text;
}

/* --------------------------------------------------------------------------------------------- */

/* Where @needle is on the screen: its row and column, FALSE when it is not there */
static inline gboolean
screen_find (const char *needle, int *row, int *col)
{
    int r;

    for (r = 0; r < LINES; r++)
    {
        char *text = screen_text (r, 0, COLS);
        const char *at = strstr (text, needle);

        if (at != NULL)
        {
            if (row != NULL)
                *row = r;
            if (col != NULL)
                *col = (int) g_utf8_pointer_to_offset (text, at);
            g_free (text);
            return TRUE;
        }
        g_free (text);
    }
    return FALSE;
}

/* --------------------------------------------------------------------------------------------- */

/* The color pair of the cell, as tty_setcolor () knows it */
static inline int
screen_color (int row, int col)
{
    SLsmg_Char_Type cell;

    if (!screen_cell (row, col, &cell))
        return -1;
    return (int) SLSMG_EXTRACT_COLOR (cell);
}

/* --------------------------------------------------------------------------------------------- */

#endif /* MC__TESTS_SCREEN_H */
