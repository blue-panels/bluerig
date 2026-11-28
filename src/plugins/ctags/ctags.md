# Ctags <!-- help:notitle -->

**Ctags**

Goes to definitions using the ctags index. To find references, checks that
the name is in the index, then searches for it with grep in the indexed
files.

|  |  |
|--|--|
| Ctrl-Enter | Definition of the name under the cursor |
| Ctrl-click | Definition of the name under the pointer |
| Ctrl-Backslash | Find a definition by its name |
| Alt-g | Find references to the name under the cursor |
| Alt-Minus | Back to the place before the jump |
| Alt-Plus | Forward |
| Alt-h | History of the jumps |
| Alt-= | Members of a class or structure |
| Alt-q | Search for a symbol of the project |
| Alt-y | Symbols of the current file |
| Shift-Tab | Complete the symbol before the cursor |

If a name has several definitions, shows a list of them. Ctrl-Enter,
Ctrl-click and Alt-g ask the language server first, if it supports the
request. If the LSP plugin does not take the request, Ctags is used.
Alt-Minus and Alt-Plus count the jumps of the language server too. The
history of the jumps is kept until the editor quits.

The same commands are in the Navigate menu. The Ctags menu in the Plugins
menu adds a file search, loading a tags file, reindexing and the list of
repositories.

**The index**

If the automatic search is on, the plugin loads the index at the first
command that needs it. With *bluerig -D* it loads it when a file opens. If
the root of the project has a *tags* file, the plugin uses it. Otherwise it
builds an index of its own in the background and keeps it in
*.bluerig/tags*. Its own index is updated when a file of the project is
saved or renamed. A manual reindex can overwrite the *tags* file at the
root. Without a project, the plugin looks for *tags* in the directory of the
file and above it.

Needs the program ctags, preferably Universal Ctags (package universal-ctags). The
ctags options of a project are set in the Debug configuration form of the
debugger.

**Settings**

The *[ctags]* section of *~/.config/bluerig/ini* keeps the ctags command, its
options, the case sensitivity of the search and the automatic search for the
tags file. Change them in Plugins > Ctags > Configuration.

The plugin reads *ctags.keymap* from the data directory, then from the
system configuration directory. The user file
*~/.config/bluerig/ctags.keymap* is loaded last and overrides the bindings.
