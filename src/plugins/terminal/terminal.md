# Terminal <!-- help:notitle -->

**Terminal**

Runs the shell from the **SHELL** variable in a window of the editor. If the
variable is not set, the user's login shell is used, and if there is none -
the first one found in */bin*, for example *bash* or *zsh*.
The shell starts in the directory of the edited file. If no file is open
or the directory is not available, the working directory of the editor is used.
Works with bash, zsh, fish, tcsh and other shells.

**Ctrl-O**
: Show the terminal window: the first time at the bottom of the screen, later
as it was when it was hidden. A file shown full screen becomes a window above
the terminal. Ctrl-O in the terminal hides the window again, and the file
takes the whole screen again, unless it was moved or resized in between. The
shell goes on running while the window is hidden: what it prints then is
there when the window is shown again. If the window is on the screen but
under another window, Ctrl-O brings it to the top.

The terminal window can be moved, resized, shown full screen and closed with
the [X] button on its frame. To go to it, use the Terminal item of the Window
menu or the window list (Window > List...).

The title shows the directory the shell is in and the command it runs.

Closing the window ends the shell. If the editor sees that the shell runs a
command, it asks for confirmation when the window closes and when the editor
exits. After the shell ends, for example with *exit*, Ctrl-O starts a new one
in the window.

**The keys**

Every key not listed below goes to the shell, including the function keys and
Esc, and also the cursor keys, Home and End, which the shell uses to edit the
command and browse its history.
A program that switched to the alternate screen of the terminal gets all
keys, Ctrl-O too. The menu and the button bar work with the mouse.

The terminal keeps up to 5000 lines of scrollback. Long lines can make this
smaller because of a limit on the number of cells. Shift with the cursor keys
marks the output, starting where the shell waits for input.

|  |  |
|--|--|
| Ctrl-Insert, Enter, F2 | Copy the marked output to the clipboard file. Without a mark Enter goes to the shell |
| Ctrl-Shift-U | Remove the mark |
| Alt-s | Search the output for what is typed next |
| Alt-S | Show only the lines with what is typed next |
| Ctrl-L | Clear the screen, keep the scrollback |
| Ctrl-Shift-L, Ctrl-Alt-L, F6 | Clear the screen and the scrollback |
| Ctrl-Up, Ctrl-Down | Scroll the output one line |
| PgUp, PgDn | Scroll the output one page |
| Ctrl-Home | To the oldest line of the output |
| Ctrl-End | To the newest line of the output |
| F3 | Mark the whole output or remove the mark |
| F4 | Show only the lines with the marked text or the word under the cursor |
| F5 | Turn this filter off and on again |

Alt-s and Alt-S open a field on the top line of the window for the text to
search or filter. The result follows as you type; case does not matter. Enter
ends the typing and leaves the result on the screen; Esc cancels the typing
and puts the old view back.

Keys can be reassigned in the *[mcterm]* section of
*~/.config/bluerig/keymap.ini* or in the Key bindings dialog of the Options menu.
Ctrl-O is the *Shell* action of the *[editor]* section.

**Settings**

The *[Terminal]* section of *~/.config/bluerig/ini* holds the settings of the
terminal.

*window_height*
: Height of the window when it is shown the first time, in percent of the
work area of the editor. From 10 to 100, default 25.

*search_direction*
: The direction in which Alt-s and Alt-S go through the output: down
(the default) or up, as in *less*.
