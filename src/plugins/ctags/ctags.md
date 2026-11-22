# Ctags <!-- help:notitle -->

**Ctags**

Goes to the definitions of names and finds the places where a name is used,
from an index that ctags builds.

|  |  |
|--|--|
| Ctrl-Enter | Definition of the name under the cursor |
| Ctrl-click | Definition of the name under the mouse |
| Ctrl-Backslash | Find a definition by its name |
| Alt-g | Find references to the name under the cursor |
| Alt-Minus | Back to the place before the jump |
| Alt-Plus | Forward |
| Alt-h | History of the jumps |
| Alt-= | Members of a class or structure |
| Alt-q | Search for a symbol of the project |
| Alt-y | Symbols of the current file |
| Shift-Tab | Complete the symbol before the cursor |

If a name has several definitions, shows a list of them. In a file that a
language server serves, Ctrl-Enter and Alt-g ask the server first, and the
server handles Ctrl-click itself. Alt-Minus and Alt-Plus count the jumps of
the language server too. The history of the jumps lasts as long as the
editor runs.

The same commands are in the Navigate menu. The Ctags... item of the Plugins
menu adds a file search, loading a tags file, reindexing and the list of
repositories.

**The index**

Builds the index of the project by itself, in the background, at the first
command in a file of the project, and at startup with *bluerig -D*. Keeps it in
*.bluerig/tags* of the project and updates it after every save of a file of
the project. A *tags* file at the root of the project is not rebuilt but
used as it is. Without a project, looks for *tags* in the directory of the
file and above it.

Needs the program ctags, preferably Universal Ctags (package universal-ctags). The
ctags options of a project are set in the Debug configuration form of the
debugger.

**Settings**

The *[ctags]* section of *~/.config/bluerig/ini* keeps the ctags command, its
options, the case sensitivity of the search and the automatic search for the
tags file. The Manage plugins dialog of the Options menu changes them.

The keys come from *ctags.keymap* in the system configuration directory of
bluerig, and *~/.config/bluerig/ctags.keymap* overrides them.
