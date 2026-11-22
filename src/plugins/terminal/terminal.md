# Terminal <!-- help:notitle -->

**Terminal**

Runs your shell in a window of the editor. This is the shell named by the
**SHELL**
variable, and it starts in the directory of the edited file. Works with bash,
zsh, fish and tcsh, and with other shells that print a prompt.

**Ctrl-o**
: Show the terminal window: the first time at the bottom of the screen, later
as it was when it was hidden. A file shown full screen becomes a window above
the terminal. Ctrl-o in the terminal hides the window again, and the file
takes the whole screen again, unless it was moved or resized in between. The
shell goes on running while the window is hidden: what it prints then is
there when the window is shown again. If the window is on the screen but
under another window, Ctrl-o brings it to the top.

The terminal window is a window of the editor like the windows of files: it
is moved, resized, shown full screen and closed with its frame or from the
Window menu, and Window > List... shows it. The title shows the directory the
shell is in and the command it runs.

Closing the window ends the shell. If a command runs in it, the editor asks
first, and asks the same when the editor ends. After the shell ends, for
example with
**exit**,
Ctrl-o starts a new one in the window.

**The keys**

Every key not listed below goes to the shell, including the function keys,
Esc, the cursor keys, Home and End, which the shell's line editor and
history use. A full-screen program, an editor or a pager, takes all
keys itself, Ctrl-o too. The menu and the button bar work with the mouse.

The terminal keeps a scrollback of all the shell printed. Shift with the
cursor keys marks it, starting where the shell waits for input.

|  |  |
|--|--|
| Ctrl-Insert, Enter, F2 | Copy the marked output to the clipboard file |
| Ctrl-Shift-u | Remove the mark |
| Alt-s | Search the output for what is typed next |
| Alt-S | Show only the rows with what is typed next |
| Ctrl-l | Clear the screen, keep the scrollback |
| Ctrl-Shift-l, Ctrl-Alt-l, F6 | Clear the screen and the scrollback |
| PgUp, PgDn | Scroll the output |
| Ctrl-Home | Oldest row of the output |
| Ctrl-End | Newest row |
| F3 | Mark the whole output or remove the mark |
| F4 | Show only the rows with the marked text or the word under the cursor |
| F5 | Turn this filter off and on again |

Alt-s and Alt-S take a pattern typed on the top row of the window, and the
output follows it as it grows. Case does not matter. Enter ends the typing
and leaves the view on what was found; Esc ends the typing and puts the view
back.

All these keys are in the
**[mcterm]**
section of the keymap file, and they can be changed there or in the Key
bindings dialog of the Options menu.

**Settings**

The
**[Terminal]**
section of the ini file holds the settings of the terminal.

*window_height*
: Height of the window when it is shown the first time, in percent of the
screen. Default is 25.

*search_direction*
: The direction in which Alt-s and Alt-S go through the output: down (the
default) or up, as in
**less**.
