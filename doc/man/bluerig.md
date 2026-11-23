---
date: September 2026
---

<!-- help:topics "Topics:" -->
# NAME <!-- help:skip -->

bluerig - a terminal IDE.

# SYNOPSIS <!-- help:skip -->

**bluerig**
[options] [+lineno] [file1[:lineno]] [file2[:lineno]] ...

# DESCRIPTION

**bluerig**
is a terminal IDE. It continues
**cooledit**,
the editor by Paul Sheer, whose terminal version was developed for many years
as the internal editor of GNU Midnight Commander; bluerig carries that editor on
as a program of its own.
**brg**
is a shorter name of the same program.

It edits several files at once, each in a window of its own, and it can edit
binary files. It has block copy, move, delete, cut and paste; key for key
undo and redo; pull-down menus; file insertion; macros; regular expression
search and replace; a line filter; folding; bookmarks; shift-arrow text
highlighting (where the terminal supports it); column blocks; insert and
overwrite modes; autoindent; word wrap and paragraph formatting; a tunable
tab size; syntax highlighting for many file types; a user menu that pipes the
text through shell commands; editor plugins (tags navigation, spell
checking) and Lua scripts.

Every file named on the command line is opened in a window of its own. With
no file, the editor opens an empty window. A file larger than
*editor_filesize_threshold*
(64 MB by default) is opened after a question, not refused.

# OPTIONS

*+lineno*
: Go to the line specified by number (do not put a space between the
*+*
sign and the number). Several line numbers are allowed but only the last one
is used, and it is applied to the first file only.

*file:lineno[:]*
: Open the file at that line, the way compilers and grep print a place in a
file. When a file of the whole name, colon and number included, exists, that
file is opened instead.

*-V, --version*
: Display the version of the program.

*-f, --datadir*
: Display the compiled-in search path for the data files of bluerig.

*-F, --datadir-info*
: Display extended information about the directories bluerig uses: the data
files, the user files and the plugins.

*--configure-options*
: Display the options the program was configured with.

*--no-lua*
: Start without the Lua runtime: no Lua script is loaded.

*-D, --debug=directory*
: Open the project of that directory to debug it: the latest file of the
project, the tree of the project and the panel of the debugger at
the right. The file windows take the keys of the debugger: F6 puts a
breakpoint, F5 builds the program and runs it, making its debug
configuration the first time, F7 and F8 step, F3 indexes the symbols of the
project; F2, F9 and F10 stay Save, the menu and Quit, and Alt-F5 goes
to the panel and back. It needs the project, build and debugger plugins,
and ctags for F3. The menus Drawing, Format and Plugins are not in the
menubar; their keys stay, and Options keeps Manage plugins.

*-Q, --query*
: Run the queries of the file in a database, MongoDB by mongosh or
PostgreSQL by libpq; the answer comes in the tabs Output, Table and History
at the bottom. *MC_QUERY_KIND* is *mongo* or *postgres*; without it,
*MC_MONGO_URI* means MongoDB and a file *.sql* PostgreSQL. For MongoDB
*MC_MONGO_URI* and *MC_MONGO_DB* tell what to connect to, *MC_MONGOSH* the
program; F5 runs the selection, else the part between lines *// ---*
around the cursor, else the whole file. For PostgreSQL libpq takes the
connection from *PGHOST*, *PGDATABASE*, *PGSERVICE* and the like, the
password from *~/.pgpass*; F5 runs the selection, else the whole file.
Ctrl-F5 runs the whole file. Without a file named, the file is
*~/.local/share/bluerig/queries/*, with the name of the database and *.js*,
or *postgres/*, the name of the connection, and that of the database and
*.sql*. The windows the project had last time are
not shown, and the plugins of a project and of code (project, build,
debugger, ctags, etags, git, terminal, spell) are not started; the
scripts stay, and the language server of the queries when lsp.json has one.
The menus Drawing, Format and Plugins are not in the menubar, as with -D.
It needs the query plugin.

*-G, --git=directory*
: Open the window Git of the git plugin on the work tree of that
directory; closing it ends the editor. The windows the project had last
time are not shown, and the plugins of a project (project, build, debugger,
ctags, etags) are not started. For a panel of git in a file
manager, with one of these after it, or alone for the current directory:

*--git-log*
: The window Git on the log.

*--git-show=commit*
: The window Git on that commit: its files and their diffs.

*--git-commit*, *--git-amend*
: The window Git with the message of a commit of what is staged, or of the
last commit amended with it; the editor ends once it is saved and
committed, or its window closed.

*--git-reword=commit*
: Another message for that commit, the same way, the files of the commit
under it.

*-h, -?, --help*
: Show the options and what they do.

*--help-terminal, --help-color*
: Show the terminal options or the color options below.

The terminal options:

*-x, --xterm*
: Force xterm mode. Used when running on xterm-capable terminals (two
screen modes, and able to send mouse escape sequences).

*-X, --no-x11*
: Do not use the X Window System to get the state of the modifier keys.

*-g, --oldmouse*
: Use the old highlight mouse tracking of the terminal.

*-d, --nomouse*
: Disable mouse support.

*-t, --termcap*
: Use the termcap database instead of terminfo. The option exists only when
bluerig was built with the S-Lang library.

*-s, --slow*
: Run on a slow terminal: the screen is drawn with fewer updates.

*-a, --stickchars*
: Draw the frames with plain characters instead of the line drawing
characters of the terminal.

*-K file, --keymap=file*
: Load the key bindings from that file. See
[Redefine key bindings](#keys_redefine).

*--nokeymap*
: Do not load any keymap file: use the key bindings built into the program.

The color options:

*-b, --nocolor*
: Force black and white display.

*-c, --color*
: Force color mode on terminals that don't seem to have color support.

*-S skin, --skin=skin*
: Use the named skin. See
[Skins](#skins).

# Overview

The screen of bluerig has the
[menu bar](#menu-bar)
at the top, the editor windows under it and the labels of the function keys at
the bottom. The menu bar may be hidden: F9 or a click on the top line of the
screen brings it up.

Every window shows one file: its name, the state of the file and the place of
the cursor stand in the status line at its top. Several windows may be open
at a time; the
[Window menu](#window-menu)
moves between them, and each of them can be moved and resized.

# The bluerig editor <a id="internal-file-editor"></a>

<a id="keys"></a>
**Keys**

<!-- {{key:Action|F8}}: Action is the action name of misc/keymap.default.ini and src/keymap.c (Remove = f8); the help shows the keys bound to it now, the man page the default keys after the bar -->
These are the default keys. Any of them can be changed, see
[Redefine key bindings](#keys_redefine).

<a id="moving-around"></a>
**Cursor movement**

|  |  |
|--|--|
| {{key:Left|Left}} | Character left |
| {{key:Right|Right}} | Character right |
| {{key:Up|Up}} | Line up |
| {{key:Down|Down}} | Line down |
| {{key:WordLeft|Ctrl-Left, Ctrl-z}} | Word left |
| {{key:WordRight|Ctrl-Right, Ctrl-x}} | Word right |
| {{key:ScrollUp|Ctrl-Up}} | Scroll the screen up |
| {{key:ScrollDown|Ctrl-Down}} | Scroll the screen down |
| {{key:PageUp|PgUp}} | Page up |
| {{key:PageDown|PgDn}} | Page down |
| {{key:Home|Home}} | Start of the line |
| {{key:End|End}} | End of the line |
| {{key:Top|Ctrl-Home, Alt-\<}} | Start of the file |
| {{key:Bottom|Ctrl-End, Alt-\>}} | End of the file |
| {{key:TopOnScreen|Ctrl-PgUp}} | Top of the screen |
| {{key:BottomOnScreen|Ctrl-PgDn}} | Bottom of the screen |

**Deletion**

|  |  |
|--|--|
| {{key:Delete|Delete, Ctrl-d}} | Delete the character |
| {{key:BackSpace|Backspace, Ctrl-h}} | Delete the character to the left |
| {{key:DeleteLine|Ctrl-y}} | Delete the line |
| {{key:DeleteToEnd|Ctrl-k}} | Delete to the end of the line |
| {{key:DeleteToWordBegin|Alt-Backspace}} | Delete to the start of the word |
| {{key:DeleteToWordEnd|Alt-d}} | Delete to the end of the word |

<a id="blocks"></a>
**Blocks**

|  |  |
|--|--|
| Shift+cursor keys | Mark a block |
| Alt+cursor keys | Mark a column block |
| {{key:Mark|F3}} | Start or end marking a block |
| {{key:MarkColumn|F13}} | Start a column block |
| {{key:MarkAll|Ctrl-a}} | Mark the whole text |
| {{key:Copy|F5}} | Copy the block to the cursor |
| {{key:Move|F6}} | Move the block to the cursor |
| {{key:Remove|F8}} | Delete the block |
| {{key:Store|Ctrl-Ins}} | Copy the block to the clipboard file |
| {{key:Cut|Shift-Del}} | Move the block to the clipboard file |
| {{key:Paste|Shift-Ins}} | Paste the block from the clipboard file |
| {{key:BlockSave|Ctrl-f}} | Write the block to a file |

<a id="search-keys"></a>
**Search and replace**

|  |  |
|--|--|
| {{key:Search|F7}} | [Search](#search-and-replace) |
| {{key:SearchContinue|F17}} | Search again |
| {{key:Replace|F4}} | Replace |
| {{key:ReplaceContinue|F14}} | Replace again |
| {{key:FilterToggle|Alt-s}} | Remove the line filter, or apply the last search as a filter |
| {{key:FilterWord|Alt-S}} | Filter by the marked text or the word under the cursor |

**Jumps and bookmarks**

|  |  |
|--|--|
| {{key:Goto|Alt-l}} | Go to a line by its number |
| {{key:MatchBracket|Alt-b}} | To the matching bracket |
| {{key:Bookmark|Alt-k}} | Set or clear a bookmark |
| {{key:BookmarkNext|Alt-j}}, {{key:BookmarkPrev|Alt-i}} | To the next or the previous bookmark |
| {{key:BookmarkFlush|Alt-o}} | Clear all the bookmarks |
| {{key:Find|Alt-Enter}} | Find the definition of the name under the cursor |
| {{key:FilePrev|Alt-Minus}}, {{key:FileNext|Alt-Plus}} | Back and forward through the places the definition lookup went to |

<a id="files-and-windows"></a>
**Files**

|  |  |
|--|--|
| {{key:Save|F2}} | Save the file |
| {{key:SaveAs|F12, Ctrl-F2}} | [Save the file under another name](#save-file-as) |
| {{key:EditNew|Ctrl-n}} | New empty window |
| {{key:InsertFile|F15}} | Insert a file at the cursor |
| {{key:History|Alt-E}} | History of the edited files |
| {{key:SelectCodepage|Alt-e}} | [Encoding of the text](#codepages-translation) |
| {{key:Quit|F10, Esc}} | Close the window; closing the last one quits the editor |

<a id="editing"></a>
<a id="macros-and-plugins"></a>
**Other commands**

|  |  |
|--|--|
| {{key:InsertOverwrite|Insert}} | Insert or overwrite mode |
| {{key:Undo|Ctrl-u}} | Undo |
| {{key:Redo|Alt-r}} | Redo |
| {{key:UndoHistory|Ctrl-t, Alt-U}} | Undo history |
| {{key:Complete|Alt-Tab}} | [Complete the word](#completion) from the words of the open files |
| {{key:FoldToggle|Alt-F}} | Fold or unfold the block between matching brackets |
| {{key:Sort|Alt-t}} | Sort the marked lines |
| {{key:ExternalCommand|Alt-u}} | Insert the output of a shell command |
| {{key:ParagraphFormat|Alt-p}} | Format the paragraph |
| {{key:InsertLiteral|Ctrl-q}} | Insert the next key, or a code U+XXXX, as it is |
| Ctrl-p | Check the spelling of the word under the cursor |
| {{key:MacroStartStopRecord|Ctrl-r}} | Start or stop recording a [macro](#macros) |
| {{key:MacroExplorer|Alt-M}} | [Macro Explorer](#macro-explorer) |
| {{key:ShowNumbers|Alt-n}} | Show or hide the line numbers |
| {{key:ShowTabTws|Alt-\_}} | Show or hide the tabs and the trailing spaces |
| {{key:SyntaxOnOff|Ctrl-s}} | Switch the syntax highlighting on or off |
| {{key:Help|F1}} | Help |
| {{key:Menu|F9}} | Menu bar |
| {{key:UserMenu|F11}} | [User menu](#edit-menu-file) |
| {{key:Shell|Ctrl-o}} | Show or hide the terminal |
| Ctrl-Alt-p | Show or hide the Preview |
| {{key:Refresh|Ctrl-l}} | Redraw the screen |

|  |  |
|--|--|
| See also: | [Menus](#menu-bar) |
|  | [Search, replace and the line filter](#search-and-replace) |
|  | [Folding](#folding) |
|  | [Macros](#macros) |
|  | [User menu](#edit-menu-file) |
|  | [Plugins](#plugins) |
|  | [Syntax highlighting](#syntax-highlighting) |
|  | [Editor options](#editor-options) |
|  | [Settings in the ini file](#internal-file-editor-options) |

## Redefine key bindings <a id="keys_redefine"></a>

|  |  |
|--|--|
| Options \> [Key bindings](#key-bindings) | Give an action keys of your own; they are kept in **~/.config/bluerig/keymap.ini** |
| Options \> [Learn keys](#learn-keys) | Give keys their escape sequences |
| Options \> [Key sniffer](#key-sniffer) | Show what the terminal sends for a key and which action the key is bound to |

At start bluerig takes the built-in keymap and loads over it
**{{pkgdatadir}}/keymap.ini**,
then
**{{sysconfdir}}/bluerig/keymap.ini**.
An action named in a file gets the keys of the file, and its built-in keys
stop working.

The package installs the keymaps
**keymap.default.ini**,
**keymap.emacs.ini**
and
**keymap.vim.ini**
in
**{{sysconfdir}}/bluerig**
and makes
**keymap.ini**
a link to
**keymap.default.ini**.

The option
**--nokeymap**
turns the loading of the files off, and only the built-in keymap works.

The user's keymap file is the first one found of:

```
1) command line option -K <keymap>, --keymap=<keymap>
2) environment variable BLUERIG_KEYMAP
3) the keymap setting in the [bluerig] section of the ini file
4) file ~/.config/bluerig/keymap.ini
```

The first three take a name or an absolute path. A name that does not end in
**.keymap**
gets that extension, and the file is looked for in these directories, the
first match wins:

```
1) ~/.config/bluerig/
2) {{pkgdatadir}}/
```

The option
**-K**,
the variable
**BLUERIG_KEYMAP**
and the setting
**keymap**
add the extension
**.keymap**
to a name that has none, so the keymaps of the package, which end in
**.ini**,
cannot be chosen this way. To take one of them, make
**~/.config/bluerig/keymap.ini**
a link to it: this file is loaded when none of the three is given.

```
ln -s {{sysconfdir}}/bluerig/keymap.vim.ini ~/.config/bluerig/keymap.ini
```

The sections of a keymap file:

|  |  |
|--|--|
| [editor] | Editor |
| [editor:xmap] | Keys after the prefix, action ExtendedKeyMap |
| [dialog] | Dialogs |
| [menu] | Menus |
| [input] | Input lines |
| [listbox] | Lists |
| [radio] | Radio buttons |
| [help] | Help window |
| [mcterm] | Terminal |

A line of a section names an action and its keys, separated by semicolons:

```
[editor]
Save = f2
SaveAs = f12; ctrl-f2
```

## Input line keys <a id="input-line-keys"></a>

The input lines of the dialogs accept these keys:

|  |  |
|--|--|
| Ctrl-a, Home | Start of the line |
| Ctrl-e, End | End of the line |
| Ctrl-b, Left | Character left |
| Ctrl-f, Right | Character right |
| Alt-f, Ctrl-Right | Word forward |
| Alt-b, Ctrl-Left | Word back |
| Ctrl-h, Backspace | Delete the character before the cursor |
| Ctrl-d, Delete | Delete the character under the cursor |
| Ctrl-@ | Set the mark |
| Ctrl-w | Cut the text from the mark to the cursor to the buffer |
| Alt-w | Copy the text from the mark to the cursor to the buffer |
| Ctrl-y | Paste the buffer |
| Ctrl-k | Delete to the end of the line |
| Ctrl-Insert | Copy the selection, or the whole line without one, to the clipboard file |
| Shift-Delete | Cut the selection to the clipboard file |
| Shift-Insert | Paste the clipboard file as one line: line breaks become spaces |
| Alt-p, Alt-n | Previous or next entry of the history of the line |
| Alt-h | [History](#history-query) of the line as a list |
| Alt-Backspace | Delete the word before the cursor |
| Alt-Tab | [Complete](#completion) a file name, a variable or a user name |

## General movement keys <a id="general-movement-keys"></a>

The help window and the lists share the same movement code, so they accept
the same keys:

|  |  |
|--|--|
| Up, Ctrl-p | Line back |
| Down, Ctrl-n | Line forward |
| Page Up, Alt-v | Page up |
| Page Down, Ctrl-v | Page down |
| Home, A1 | To the start |
| End, C1 | To the end |

The help window also accepts these keys:

|  |  |
|--|--|
| b, Backspace | Page up |
| Space, f | Page down |
| u, d | Half a page up or down |
| g, G | To the start or to the end |

# Mouse support <a id="mouse-support"></a>

bluerig supports the mouse. It works in an
**xterm(1)**
terminal (even over a telnet or ssh connection from the xterm) and on the
Linux console with the
**gpm**
mouse server running.

A click puts the cursor where you click, a drag highlights text, and a click on a
menu, a button or a function key label chooses it. The wheel scrolls the
text.

The default auto repeat rate for the mouse buttons is 400 milliseconds. This
may be changed with the
*mouse_repeat_rate*
setting of the ini file, and the time within which two clicks are a double
click with
*double_click_speed*.

If you are running bluerig with the mouse support, you can get the default mouse
behavior of the terminal (cutting and pasting text) by holding down the Shift
key.

# Menu bar <a id="menu-bar"></a>

The menu bar pops up when you press F9 or click the mouse on the top row of
the screen. Its menus are File, Edit, Search, Command, Navigate (only when a
plugin or a script adds something to it), Window, Plugins and Options. A Lua
script may add entries to any of them, or menus of its own.

## File menu <a id="file-menu"></a>

**Open file...**
: Open a file in a new window.

**New**
: Open an empty window.

**Close**
: Close the window.

**History...**
: The files that were edited before, to open one of them again.

**Save, Save as...**
: Write the file, under its name or another one. See
[Save As](#save-file-as).

**Insert file..., Copy to file...**
: Insert a file at the cursor, or write the highlighted block to a file.

**User menu...**
: The [user menu](#edit-menu-file).

**About...**
: The version of the program.

**Quit**
: Leave the editor, asking about the files that are not saved.

## Edit menu <a id="edit-menu"></a>

Undo, Redo and the Undo history; the switch between insert and overwrite
mode; the commands that highlight a block, a column block or the whole file,
and remove the highlighting; copying, moving and deleting the block; copy to, cut
to and paste from the clipboard file; and going to the start and the end of
the file.

## Search menu <a id="search-menu"></a>

Search, Search again and Replace; the line filter (Toggle line filter, Filter
by word; see [Search, replace and the line filter](#search-and-replace)); and
the bookmarks: toggle a bookmark on the current line, go to the next or to
the previous one, and clear them all.

## Command menu <a id="command-menu"></a>

**Go to line...**
: Go to a line, given by its number.

**Toggle line state**
: Show or hide the column of line numbers.

**Go to matching bracket**
: Go to the bracket that matches the one under the cursor.

**Toggle syntax highlighting, Toggle right margin, Toggle control characters**
: Switch these on and off.

**Encoding...**
: The codepage the text is written in. See
[Choose codepage](#codepages-translation).

**Fold / Unfold block, Unfold all**
: See [Folding](#folding).

**Refresh screen**
: Redraw the screen.

**Start/Stop record macro, Macro explorer..., Record/Repeat actions**
: See [Macros](#macros).

## Navigate menu <a id="navigate-menu"></a>

The commands of the plugins: go to the declaration, find the references, the
symbols of the file and others. See
[Plugins](#plugins).

## Window menu <a id="window-menu"></a>

**Move, Resize**
: Move or resize the window with the arrow keys; Enter ends it.

**Toggle fullscreen**
: Switch the window between full screen and its own size.

**Toggle sticky windows**
: With sticky windows, windows whose frames stand next to each other resize
together: the corner of a window moves the edge it is on with all the windows
along it, on both sides, and no gap opens between them. Where four windows
meet, the corner moves both edges. A side that is on the edge of the screen
when the resize begins stays there, a window keeps its smallest size, and an
edge stops at a window in its way, and that window then resizes with the others. Esc puts
every window back. A window dragged by its title moves alone. The setting is kept as
**editor_sticky_windows**
in the ini file.

**Next, Previous**
: Go to the next or to the previous window.

**List...**
: The windows that are open, to go to one of them. See
[Open files](#open-files).

Under them come the windows of the plugins: the tree of the project, the
output of the build, the panel and the console of the debugger, the
terminal, the terminal of the program the debugger runs, the Preview. An entry opens its window, brings it to the front
when it is behind another, and closes it when it is in front already; an
open one is marked with *\**. A plugin that is switched off has no entry.

The windows of the plugins stand in docks. The tree, the panel of the
debugger and the Preview go in the column at the right, one above the other,
all the height of the screen. The console, the output of the build and the
terminal go in the row at the bottom, under the windows of the files: they
share the row as tabs: one is shown, and its title lists their names. A click on a name, or
**Ctrl-Alt-PgDn** and **Ctrl-Alt-PgUp** (*WindowTabNext* and *WindowTabPrev*
in the keymap), shows another. A fullscreen file window fills the rest of
the screen, and the whole screen again when the docks are empty. Resizing a
docked window resizes the dock; moving it takes it out of the dock.

**Layout...**
: The layouts of the windows: which windows of the plugins are open, in
which dock, and how big the docks are. *Edit* has the files alone,
*Project* the tree of the project, *Debug* the tree and the panel of the
debugger. **Use** puts the windows so; **Save as...** keeps them as they are
under a name, the name of one of these too; **Delete** forgets a layout
kept, the one of the editor of that name coming back. With **Debug layout
while debugging** the start of the debugger puts the windows as *Debug* has
them, and its stop puts them back, the tabs of the bottom staying to be
read.

The windows of a project are kept when the editor ends, and come back when
it starts with a file of the project.
*bluerig --debug* starts with the layout *Debug*. The layouts are kept in
*layouts.ini* in the settings directory.

## Plugins menu <a id="plugins-menu"></a>

The editor plugins that carry an entry of their own, the spell checker for
one; their windows are in the [Window menu](#window-menu). A plugin that is switched off in
[Manage plugins](#manage-plugins)
is not in the menu.

## Options menu <a id="options-menu"></a>

**General...**
: The [editor options](#editor-options).

**Save mode...**
: How a file is written. See [Edit Save Mode](#edit-save-mode).

**Syntax highlighting...**
: Choose the syntax rules for the file by hand, or leave them to the name and
the first line of the file.

**Appearance...**
: The skin and the shadows. See [Appearance](#appearance).

**Learn keys...**
: See [Learn keys](#learn-keys).

**Key bindings...**
: See [Key bindings](#key-bindings).

**Key sniffer...**
: See [Key sniffer](#key-sniffer).

**Manage plugins...**
: See [Manage plugins](#manage-plugins).

**Plugin info...**
: See [Plugin info](#plugin-info).

**Lua scripts...**
: See [Lua scripts](#lua-scripts). The entry is there only where bluerig was
built with Lua.

**Syntax file**
: Open the syntax file of the user,
*~/.local/share/bluerig/syntax/Syntax*,
in the editor. Where it does not exist, it is made with a short comment on what
it holds, not as a copy of the system file, which is still read after it. See
[Syntax Highlighting](#syntax-highlighting).

**Menu file**
: Open the [user menu](#edit-menu-file) file in the editor.

**Save setup**
: Write the settings to
*~/.config/bluerig/ini*.
With the
*auto_save_setup*
setting on, they are written whenever the editor ends.

### Editor options <a id="editor-options"></a>

The entry
**General...**
of the Options menu opens the dialog of the editor options. Each option is
written to the ini file, see
[Settings in the ini file](#internal-file-editor-options).

**Wrap mode**

|  |  |
|--|--|
| None | Lines are not wrapped |
| Dynamic paragraphing | Formats the paragraph under the cursor while typing |
| Type writer wrap | Breaks the line at the wrap length while typing |

**Tabulation**

|  |  |
|--|--|
| Fake half tabs | Between the left margin and the text a tab steps half its width, and the file gets spaces and tabs |
| Backspace through tabs | Backspace deletes the whole indent to the left margin when no text stands between the cursor and the margin |
| Fill tabs with spaces | Inserts spaces to the next tab stop instead of a tab character |
| Tab spacing | The width of a tab character; 8 by default |

**Keyboard**

|  |  |
|--|--|
| Esc timeout, ms | How many milliseconds Esc waits for the rest of a key before it counts as Esc; 1000 by default. In the ini file it is *old_esc_mode_timeout* in microseconds, and the variable **KEYBOARD_KEY_TIMEOUT_US** overrides it at startup |

**Other options**

|  |  |
|--|--|
| Return does autoindent | Enter starts the line with the indent of the line above |
| Confirm before saving | Asks before writing the file |
| Save file position | Opens the file where it was left |
| Visible trailing spaces | Marks the spaces at the end of lines |
| Visible tabs | Marks the tab characters |
| Show control characters | Shows control characters as ^M instead of hiding them |
| Syntax highlighting | Colors the text by the syntax rules of its type |
| Cursor after inserted block | Leaves the cursor at the end of an inserted block, not at its start |
| Persistent selection | Keeps the selection when the cursor moves |
| Cursor beyond end of line | Lets the cursor go past the last character of the line |
| Group undo | Undoes a series of changes of one kind at once |
| Word wrap line length | The column where the wrap modes break the line; 72 by default |

### Edit Save Mode <a id="edit-save-mode"></a>

How a file is written, the same three modes the
*editor_option_save_mode*
setting takes:

**Quick save**
: Write over the file at once. Fast, and a failure in the middle leaves the
file half written.

**Safe save**
: Write a temporary file first and rename it over the original when it is
whole, so a failure leaves the original untouched.

**Do backups with following extension**
: Safe save, and the original is kept under its name with the extension of the
input line added, "~" by default. Saving twice replaces the backup as well.

**Check POSIX new line**
: Ask about the missing newline at the end of the file before it is written.

### Appearance

The skin the program is drawn with, and whether the dialogs and the drop-down
menus have a shadow.

**Skin**
: The button opens the list of the skins that are installed and those of the
user; the one chosen is applied at once.

**Shadows**
: Draw a shadow under every dialog and drop-down menu. Without colors there
are no shadows.

The choice is written to the
*skin*
and
*shadows*
settings of the ini file. See the
[Skins](#skins)
section for the skin files themselves.

### Learn keys

This dialog teaches bluerig the escape sequences your terminal sends for the
function keys, the arrows and the navigation keys. Its title names the
terminal, from the
**TERM**
variable.

Select a modifier combination (Ctrl, Alt, Shift) using the checkboxes, then
press the button of a key. Press the key on the keyboard and wait for the
capture message to disappear. The learned sequence appears next to the button.
A sequence that another key has already is reported as a conflict.

**Del**
: Clear the learned key the cursor is on.

**Save**
: Write the learned keys to
*~/.config/bluerig/term/\<TERM>*.

**Edit term file**
: Open that file in the editor.

### Key bindings <a id="key-bindings"></a>

The actions of the program with the keys they answer to, in a table. The
buttons at the top choose the group, Editor or Widgets, and then the part of
it: Main and Ctrl-X for the editor; Dialog, Menu, Input, Listbox, Radio and
Help for the widgets. Back returns to the groups. Actions marked with \* have
keys that differ from the defaults.

**Enter**
: Replace the key of the action: press the key to assign.

**F5**
: Add another key to the action.

**F8, Del**
: Remove the key.

**Save**
: Write the changes to
*~/.config/bluerig/keymap.ini*.

**Edit keymap file**
: Open
*keymap.ini*
in the editor.

**Edit term file**
: Open the key sequences of the terminal, the file
[Learn keys](#learn-keys)
writes.

A key that is taken by another action is reported, and the question is
whether to take it from there.

### Key sniffer <a id="key-sniffer"></a>

Press
**Capture key**,
then any key. The dialog shows what arrived:

```
Shortcut   the symbolic name, e.g. Ctrl-F5
Action     the action of the key in the editor keymap
Raw        the escape sequence and its bytes in hex
Keycode    the internal numeric code
```

It is the place to look when a key does nothing or the wrong thing: a key
that arrives as a sequence the program does not recognize is to be taught in
[Learn keys](#learn-keys),
and one that arrives but has no action is to be bound in
[Key bindings](#key-bindings).

### Manage plugins <a id="manage-plugins"></a>

The plugins the program has loaded, in a table: the kind, the name and what the
plugin says about itself. The editor plugins are listed here, and, where bluerig
was built with Lua, the Lua engine as a plugin of the kind
*runtime*.
A plugin is switched off and on with the checkbox of its row, and what is
switched off is not loaded the next time either; the choice is kept in
*~/.config/bluerig/plugins.ini*.
A plugin that is switched off stays in the list, so that it can be switched
on again.

**Enter, F4**
: Open the settings of the plugin the cursor is on. A plugin that has none says
so. On the row of the Lua engine it opens the
[Lua scripts](#lua-scripts).

**IDE: project, build, debugger**
: Switch the three plugins that make an IDE of the editor on together, or off
together when all of them are on: the project and its files, the build, and
the debugger. Each of them works without the others too.

### Plugin info <a id="plugin-info"></a>

The plugins the editor has loaded: the name, whether it is on, what it
provides and what it does. It is a list to look at; a plugin is switched off
and its settings are opened in the
[Manage plugins](#manage-plugins)
dialog.

### Lua scripts <a id="lua-scripts"></a>

The Lua scripts that were found, the system ones and those of the user, in a
table: whether it is on, where the script lives (global or user), its
identifier, its workspace, what it provides and what it does. A script is
switched off and on with the checkbox of its row; the change takes effect the
next time bluerig starts.

**Settings**
: Run the script that carries the settings of the package, where it has one.

See [Lua scripting](#lua-scripting) for the scripts themselves.

# Search, replace and the line filter <a id="search-and-replace"></a>

{{key:Search|F7}} opens the search dialog.

|  |  |
|--|--|
| Normal | Searches for the string as it is |
| Regular expression | Searches by a regular expression |
| Hexadecimal | Searches for bytes written as hexadecimal numbers |
| Wildcard search | Searches by a shell pattern with \* and ? |
| Case sensitive | Tells lowercase from uppercase letters |
| Backwards | Searches from the cursor to the start of the file |
| In selection | Searches only in the marked block |
| Whole words | Matches only a whole word |
| All charsets | Searches in every encoding the file may be written in |
| Find all | Sets a bookmark on every line with a match |
| Filter | Hides the lines without a match |

{{key:Replace|F4}} opens the replace dialog: the same fields and the line
"Enter replacement string". In a regular expression replacement \\1 to
\\9 insert the matched groups.

|  |  |
|--|--|
| Replace | Replaces this match and searches for the next one |
| All | Replaces every match without asking |
| Skip | Leaves this match and searches for the next one |

**Filter**

Hides the lines without a match; the line numbers do not change, and the
line state column marks every hidden run. The lines are hidden once, when the
button is pressed: a line changed or typed afterwards stays shown even if it
does not match.

|  |  |
|--|--|
| {{key:FilterToggle|Alt-s}} | Removes the filter, or applies the last search as a filter; with no search, opens the search dialog |
| {{key:FilterWord|Alt-S}} | Filters by the marked text or by the word under the cursor |
| Command \> Unfold all | Lifts the filter and every fold |

# Folding <a id="folding"></a>

A fold hides a run of lines behind its first line, which stays on the screen
with a mark in the line state gutter.
**Alt-F**
(Fold / Unfold block in the Command menu) folds the highlighted lines, or,
with nothing highlighted, the lines from a bracket on the current line to the
bracket that matches it. On the first line of a fold it unfolds it.
**Unfold all**
opens every fold of the file, and lifts the line filter.

# Macros <a id="macros"></a>

{{key:MacroStartStopRecord|Ctrl-r}}
starts recording a macro: the keys pressed after it are recorded. Pressing
**Ctrl-r**
again ends the recording and waits for the key the macro is bound to; that
key then runs it.

The macros are kept in the section
**[editor]**
of the file
**~/.local/share/bluerig/macros**,
and a macro is deleted by deleting its line there, or in the
[Macro Explorer](#macro-explorer).

**Record/Repeat actions**
in the Command menu records the actions that follow until it is chosen again,
and then asks how many times to repeat them.

A shell script can be assigned to a key as well, by a line of the
**macros**
file like this:

```
[editor]
Ctrl-W=ExecuteScript:25;
```

This means that Ctrl-W runs the
*ExecuteScript(25)*
action, which runs the shell script
**~/.local/share/bluerig/macros.d/macro.25.sh**.
The scripts are kept in the directory
**~/.local/share/bluerig/macros.d/**
and must be named
**macro.XXXX.sh**,
where
**XXXX**
is a number from 0 to 9999. Examples are installed in
**{{pkgdatadir}}/examples/macros.d/**.
A script has the format of the entries of the
[user menu](#edit-menu-file)
and takes the same substitutions. Its first line may be

*#silent*
: The script is run without asking for its parameters.

Here is a sample script:

```
l       comment selection
	TMPFILE=`mktemp ${BLUERIG_TMPDIR:-/tmp}/up.XXXXXX` || exit 1
	echo '#if 0' > $TMPFILE
	cat %b >> $TMPFILE
	echo '#endif' >> $TMPFILE
	cat $TMPFILE > %b
	rm -f $TMPFILE
```

## Macro Explorer <a id="macro-explorer"></a>

The macros that are recorded, with the key each one answers to and what it
does. The buttons are

**Run**
: Play the macro the cursor is on.

**Delete**
: Remove it, after a question.

**Edit file**
: Open the file the macros live in, which the
[Macros](#macros)
section describes.

# User menu <a id="edit-menu-file"></a>

{{key:UserMenu|F11}}
opens the user menu: each of its entries is a shell script run on the text of
the editor. The menu is read from the first file found of:

```
1) ./.bluerig.menu in the current directory
2) ~/.config/bluerig/menu
3) ~/bluerig.menu
4) {{sysconfdir}}/bluerig/bluerig.menu
5) {{pkgdatadir}}/bluerig.menu
```

The file of the current directory is read only where it belongs to the user or
to root and nobody else can write it: its entries run commands. The
**Menu file**
entry of the Options menu opens the menu file in the editor.

A line that starts with anything but a space or a tab is a menu entry, its
first character the hot key of the entry. The lines that start with a space
or a tab are the commands of the entry. A line that starts with '#' is a
comment.

When an entry is chosen its commands are written to a temporary file, the
substitutions in them are replaced, and the shell runs that file:

|  |  |
|--|--|
| %f, %p, %s | The name of the current file |
| %n | The name of the current file without its extension |
| %x | The extension of the name of the current file |
| %d | The current directory |
| %c | The position of the cursor in the file |
| %i | An indent of spaces as wide as the cursor column |
| %y | The syntax type of the current file |
| %b | The block file: the marked text is written to **~/.cache/bluerig/block** before the run, and what the commands leave there replaces the block |
| %m | The name of the menu file |
| %% | The % character |
| %{text} | An input line with the prompt "text"; what is typed goes in, Esc or F10 cancel the entry |
| %view | Opens the standard output of the commands in a new editor window; words in braces after it, as in %view{ascii}, are skipped |

Substitution letters are case-insensitive. A number
after % turns quoting on (the default) or off: %f and %1f quote the inserted
value, %0f does not.

A sample menu file:

```
s       Sort the block
	sort %b > %b.tmp && mv -f %b.tmp %b

w       Count the words of the file
	%view wc %f

m       Run make in the directory of the file
	make
```

**Default entry**

A line that starts with '=' in the first column is a condition of the entry
under it: when the condition is true, the entry becomes the default one.

```
Condition syntax:   = <sub-cond>
  or:               = <sub-cond> | <sub-cond> ...
  or:               = <sub-cond> & <sub-cond> ...

Sub-condition is one of following:

  y <pattern>   syntax of current file matching pattern?
  f <pattern>   current file matching pattern?
  d <pattern>   current directory matching pattern?
  t <type>      current file of type?
  x <filename>  is it executable filename?
  ! <sub-cond>  negate the result of sub-condition
```

Pattern is a shell pattern or a regular expression, according to the
*shell_patterns*
setting; a line "shell_patterns=0" or "shell_patterns=1" as the first line of
the menu file sets it for that file.

Type is one or more characters:

```
  n   not a directory
  r   regular file
  d   directory
  l   link
  c   character device
  b   block device
  f   FIFO (pipe)
  s   socket
  x   executable file
```

For example 'rlf' is a regular file, a link or a FIFO.

A condition that starts with '=?' instead of '=' shows a debug trace each
time it is calculated.

The conditions are calculated from left to right; '&' does not bind tighter
than '|':

```
	= f *.c | f *.h & y C
```

is calculated as

```
	( (f *.c) | (f *.h) ) & (y C)
```

**Addition conditions**

A condition that starts with '+' (or '+?') puts the entry in the menu only
when it is true. A condition with '+=' or '=+' ('+=?' and '=+?' with a trace)
works both as an addition and as a default condition. Two condition lines
before an entry, one with '+' and one with '=', set them apart.

# Plugins <a id="plugins"></a>

{{plugins}}
A plugin is switched on and off in the Options menu, entry
[Manage plugins](#manage-plugins).

# Dialogs <a id="dialogs"></a>

## Save As <a id="save-file-as"></a>

The name to write the file under, and the line breaks to write it with: as the
file has them, Unix (LF), Windows and DOS (CR LF), or Macintosh (CR). The
choice holds for that save; the file keeps what it is given.

## Open files <a id="open-files"></a>

The files the editor has open, one to a line. Enter goes to the file the
cursor is on, Esc leaves the file that is shown where it is. The same list is
the
**List...**
item of the Window menu.

## Screen selector

Besides the editor, a Lua script may open screens of its own, full-screen
views that stay open while you go back to the editor. There are three ways to
switch between the screens:

**Alt-}**
: switch to the next screen;

**Alt-{**
: switch to the previous screen;

**Alt-\`**
: open a dialog window with the list of the screens that are open, to choose
one.

## Completion <a id="completion"></a>

{{key:Complete|Alt-Tab}}
in the editor completes the word before the cursor from the words of the open
files, and shows a list when several words fit. The setting
*editor_wordcompletion_collect_all_files*
of the ini file, on by default, takes words from all the open files; when off,
from the current file only.

**Alt-Tab**
in an input line of a dialog completes the text:

|  |  |
|--|--|
| $name | An environment variable |
| ~name | A user name |
| Else | A file name |

Where several fit, the program beeps. The setting
*show_all_if_ambiguous*
of the ini file, off by default, shows their list next to the cursor:
**Enter**
chooses one, typed letters narrow the list, Esc, F10, Left and Right hide it.

## Choose codepage <a id="codepages-translation"></a>

The list of the codepages the program knows, from
**{{pkgdatadir}}/charsets**.
Choosing one tells the editor in which codepage the text of the file is
written, and the text is recoded from it into the codepage of the terminal
for the screen;
**\<No translation>**
leaves it as bytes. The list is opened by
**Alt-e**
and by the Encoding item of the Command menu.

## History <a id="history-query"></a>

The list of what was typed into an input line before, newest first, which
**Alt-h**
opens for the line the cursor is in. Enter takes the entry the cursor is on
into the line, Esc leaves the line as it was, and
**F8, Del**
removes the entry the cursor is on from the history. The histories are kept in
*~/.local/share/bluerig/history*;
*num_history_items_recorded*
says how many entries each of them keeps.

# Syntax highlighting <a id="syntax-highlighting"></a>

{{key:SyntaxOnOff|Ctrl-s}}
switches the syntax highlighting on and off. The rules of the highlighting
are in the syntax files: keywords and contexts, such as comments and strings,
get their colors there.

|  |  |
|--|--|
| [Syntax files](#syntax-files) | Where the rules are and in which order they are read |
| [The file section](#syntax-file) | Which files a section of rules highlights |
| [Contexts](#syntax-contexts) | Regions of the text with colors of their own |
| [Embedding and layers](#syntax-embed) | Rules of another syntax inside a context and over the text |
| [Keywords](#syntax-keywords) | Words and patterns colored inside a context |
| [Colors and attributes](#syntax-colors) | Colors, scope tokens of the skin, attributes |
| [Other lines](#syntax-other) | Comments, caseinsensitive, line-local |

## Syntax files <a id="syntax-files"></a>

|  |  |
|--|--|
| **~/.local/share/bluerig/syntax/Syntax** | The rules of the user: what the user added or wants otherwise |
| **{{pkgdatadir}}/syntax/Syntax** | The installed rules, for the file types the user file does not name |

A user entry overrides an installed one for the files it
matches. The files named by
**include**
lines are looked for in
*~/.local/share/bluerig/syntax/*
first, then among the installed ones. The user file is read again each time
a new file is opened in the editor.

A full copy of the installed file, as older versions made, hides every file
type installed after it, and their highlighting stops working: keep only the
changed entries in the copy.

## The file section <a id="syntax-file"></a>

A syntax file consists of sections. Each one starts with a
**file**
line and is usually put in a file of its own with an
**include**
line:

```
file <file name> <description> [<first line>]
```

The
*file name*
and the
*first line*
are regular expressions: the section applies to a file when its name or the
first line of its text matches. The
*description*
is the name of the syntax, with \s for a space: the
**embed**
and
**overlay**
lines use it. A section ends where the next one starts and is divided into
contexts with keywords.

A trivial section for a C program:

```
file .\*\\.c C\sProgram\sFile (#include|/\\\*)

wholechars abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_

# default colors
define  comment   brown
context default
  keyword  whole  if       yellow
  keyword  whole  else     yellow
  keyword  whole  for      yellow
  keyword  whole  while    yellow
  keyword  whole  do       yellow
  keyword  whole  switch   yellow
  keyword  whole  case     yellow
  keyword  whole  static   yellow
  keyword  whole  extern   yellow
  keyword         {        brightcyan
  keyword         }        brightcyan
  keyword         '*'      green

# C comments
context /\* \*/ comment

# C preprocessor directives
context linestart # \n red
  keyword  \\\n  brightred

# C string constants
context " " green
  keyword  %d    brightgreen
  keyword  %s    brightgreen
  keyword  %c    brightgreen
  keyword  \\"   brightgreen
```

## Contexts <a id="syntax-contexts"></a>

A context is a region of the text with its own color and its own keywords,
such as a C comment between /\* and \*/. It starts with the line:

```
context [exclusive] [oneline] [whole|wholeright|wholeleft] [linestart] <delim>
        [linestart] <delim> [<foreground>] [<background>] [<attributes>]
```

The first context of a section is
**context default**
[*foreground*] [*background*] [*attributes*];
without it the section gives an error.

|  |  |
|--|--|
| linestart | The delimiter stands at the start of a line |
| whole | The delimiter is a whole word |
| wholeleft | The delimiter is on a word boundary at the left |
| wholeright | The delimiter is on a word boundary at the right |
| exclusive | Colors the text between the delimiters, not the delimiters |
| oneline | After exclusive: ends the context at the end of the line when the right delimiter is not on it |

A line
**wholechars**
[**left**|**right**] *characters*
anywhere in the file sets the characters of a whole word; left and right
set the two sides separately. A line
**spellcheck**
after
**context**
gives the words of the context to the spell check.

## Embedding and layers <a id="syntax-embed"></a>

A line

**embed**
[**soft**]
*type*

after
**context**
gives the text between the delimiters to the rules of another syntax;
*type*
is the description of its file line, with \s for a space. The type is looked
up in the Syntax file the rules are read from, then in the one installed with
bluerig. A type found in neither leaves the context in its own colors. The
delimiters keep the colors of the context. The right delimiter ends the
embedded text wherever it stands, inside a string or a comment of the
embedded rules too. An embedded syntax can embed another one, a few levels
deep. For example, HTML gives scripts to the rules of JavaScript:

```
  context <SCRIPT*> </SCRIPT> brightcyan
      embed JavaScript\sProgram
```

With
**soft**
the right delimiter ends the embedded text only outside the strings,
comments and keywords of the embedded rules, so that in {{ "}}" }} of a
Jinja2 template the first }} stays part of the string, and in ${x/\\}/y} of
a shell script the first } stays part of the escape.

A line

**overlay**
*type*

among the rules makes the whole syntax a layer over
*type*:
its contexts color what they hold wherever it stands, and the rest of the
text is read by the rules of
*type*,
which see the contents of the contexts as blanks. This way the tags of a template
language are colored in the text, inside an HTML tag, in a string of an
attribute and in a script alike, and HTML colors the rest. The default
context of a layer colors nothing: a keyword there only keeps a context from
starting. A type found in no Syntax file leaves the layer over plain text.
For example, Smarty over HTML:

```
  overlay HTML\sFile
  context default
      keyword {\s
  context {\* \*} brown
  context { } brightcyan
      embed soft Smarty\sTag
```

## Keywords <a id="syntax-keywords"></a>

A keyword of a context is given by the line:

```
keyword [whole|wholeright|wholeleft] [linestart] <string> [<foreground>] [<background>] [<attributes>]
```

The strings of keywords and of context delimiters are read so:

|  |  |
|--|--|
| \\t, \\s | Tab and space: blanks separate the words of the line and cannot be used as they are |
| \\n, \\r | Newline and carriage return |
| `\\` | Backslash |
| \\\* | Asterisk |
| \* | Any characters, any number of them |
| + | Any number of word characters, including none |
| \\[abc\\] | Any number of characters from the set |
| \\{abc\\} | One character of the set |

A pattern cannot be the first or the last character of a context delimiter.
For example,

```
  keyword         '*'      green
```

colors the C single character constants green, and

```
  keyword         "*"      green
```

colors the string constants, as long as they do not run across a newline.

The line

```
  keyword  \\\n  brightgreen
```

gives a keyword of a backslash and a newline. Keywords are matched before
the context delimiters, so it keeps the context from ending at the end of a
line with a backslash, and a C preprocessor directive goes on across several
lines.

A line
**define**
*name words...*
gives a name that stands for these words in the rules, such as
**define comment brown**.

## Colors and attributes <a id="syntax-colors"></a>

The colors: black, gray, red, brightred, green, brightgreen, brown, yellow,
blue, brightblue, magenta, brightmagenta, cyan, brightcyan, lightgray and
white. "default" is the default color of the terminal, "base" the main
colors of the editor; it sets attributes without changing the background
color. With 256 colors there are also color16 to color255, rgb000 to rgb555
and gray0 to gray23.

The foreground may be a scope token instead of a color: a word in capitals
that says what the text is rather than how it looks:

```
keyword  whole  if   KEYWORD_CONTROL
context  " " STRING_QUOTED_DOUBLE
    keyword  \\"   CONSTANT_CHARACTER_ESCAPE
keyword  linestart TODO  KEYWORD_OTHER_TODO black
```

The color of a token comes from the skin, so one skin colors the program and
the text, and every language in the same way; see
[Skins](#skins).
A token is read as a scope: in lower case, with dots for the underscores.
KEYWORD_CONTROL_IMPORT is keyword.control.import, a kind of keyword.control,
which is a kind of keyword. These are the scope names of Sublime Text and
TextMate. What the skin does not color keeps the colors mcedit gave that
text. A background or attributes after a token stand over the colors of
mcedit, as black does above, but not over a background or attributes the
skin gives the token itself. A token works in a define too, and on the
number, string and symbols lines of line-local rules. The installed syntax
files use tokens only; the list of them with the colors of mcedit is in
**{{pkgdatadir}}/syntax/README.txt**.

A syntax file shared with
**cooledit**
gives the colors for both with a slash, and bluerig takes the color before
it:

```
keyword  #include  red/Orange
```

The attributes are bold, italic, underline, reverse and blink, several
joined with a plus sign.

## Other lines <a id="syntax-other"></a>

|  |  |
|--|--|
| # ... | A comment, on a line of its own |
| caseinsensitive | At the start of the file: the case of letters does not matter |
| line-local | The mode for plain text: resets at every newline and at the screen edge and reads only the text shown |
| number *max* *color* | In line-local: decimal numbers of up to *max* digits |
| string *quote* *color* | In line-local: strings in single or double quotes |
| symbols *characters* *color* | In line-local: punctuation characters colored one by one |

# Colors

bluerig will try to detect if your terminal supports color using the terminal
database and your terminal name. Sometimes it gets confused, so you may force
color mode or disable color mode using the -c and -b flag respectively.

If the program is compiled with the S-Lang screen manager instead of ncurses,
it will also check the variable
**COLORTERM,**
if it is set, it has the same effect as the -c flag.

You may specify terminals that always force color mode by adding the
*color_terminals*
variable to the Colors section of the initialization file. This will prevent
bluerig from trying to detect if your terminal supports color. Example:

```
[Colors]
color_terminals=linux,xterm
color_terminals=terminal-name1,terminal-name2...
```

The program can be compiled with both ncurses and S-Lang, ncurses does not
provide a way to force color mode: ncurses uses just the information in the
terminal database.

# Skins

The colors of the program and the lines of its frames are taken from a skin
file. The
[Appearance](#appearance)
dialog chooses one from a list.

If your skin contains any true-color definitions, you should define the
'truecolors' key set to TRUE value in [skin] section. If true-color is not
used but 256-color is, you should define '256colors' instead.

The skin is the first one named by:

```
1) command line option -S <skin>, --skin=<skin>
2) environment variable BLUERIG_SKIN
3) setting skin of the [bluerig] section of the ini file
4) the skin called default
```

A name may be the absolute path of a skin file (with the extension .ini or
without it). Otherwise the skin file is looked for in (to the first one
found):

```
1) ~/.local/share/bluerig/skins/
2) {{sysconfdir}}/bluerig/skins/
3) {{pkgdatadir}}/skins/
```

The format of skin files is described in
**{{pkgdatadir}}/skins/README.txt**.
The colors of the syntax highlighting come from the section
**[syntax]**
of the skin: there a scope names the colors of a kind of text, and a scope
and the scope of the context it stands in, the two with a blank between
them, name the colors of that kind of text in that context. The longest
scope the skin names wins:

```
[syntax]
    comment = gray
    keyword = yellow
    keyword.control = brightmagenta
    string = green
    string constant.character.escape = white;red
    invalid = white;red;bold
```

A kind of text the skin does not name keeps the colors mcedit gave it, so a
skin may name a few kinds and leave the rest. The rules that name colors
instead of scope tokens keep their colors whatever the skin; see
[Syntax Highlighting](#syntax-highlighting).

# Terminal databases

bluerig provides a way to fix your system terminal database without requiring
root privileges. The
[Learn keys](#learn-keys)
dialog writes what it learns to
**~/.config/bluerig/term/\<TERM>**,
one line for every key in its
**[keys]**
section. bluerig also searches the system file
**defaults.ini**
(in
**{{sysconfdir}}/bluerig**
or
**{{pkgdatadir}}**)
and the
**~/.config/bluerig/ini**
file for the section "terminal:your-terminal-name" and then for the section
"terminal:general". Each line of the section contains a key symbol that you
want to define, followed by an equal sign and the definition for the key. You
can use the special \\e form to represent the escape character and the ^x to
represent the control-x character. A line
*copy=other*
takes every key of the section "terminal:other" as well.

The possible key symbols are:

```
f0 to f20     Function keys f0-f20
bs            backspace
home          home key
end           end key
up            up arrow key
down          down arrow key
left          left arrow key
right         right arrow key
pgdn          page down key
pgup          page up key
insert        the insert character
delete        the delete character
complete      to do completion
```

For example, to define the key insert to be the Escape + [ + O + p, you set
this in the ini file:

```
insert=\e[Op
```

A key symbol may take modifiers:

```
    ctrl-alt-right=\e[[1;6C
    ctrl-alt-left=\e[[1;6D
```

This means that ctrl+alt+left sends a \\e[[1;6D escape sequence and therefore
bluerig interprets "\\e[[1;6D" as Ctrl-Alt-Left.

The
*complete*
key symbol represents the escape sequences used to invoke the completion
process, this is invoked with Alt-tab, but you can define other keys to do the
same work (on those keyboard with tons of nice and unused keys everywhere).

# Lua scripting

Where bluerig was built with Lua, it runs Lua scripts that extend the editor:
actions bound to keys and to menus, reactions to the events of the editor
(a file is opened, saved, a key is pressed), dialogs and full-screen views.
Several commands of the editor are such scripts: sorting the lines, formatting
the paragraph, inserting the output of a command, inserting a character by its
code point, inserting the date, decoding base64 and drawing tables.

The system scripts are in
**{{pkgdatadir}}/lua/scripts/**,
and the scripts of the user in
**~/.local/share/bluerig/lua/scripts/**,
one directory for every script, with its
**lua.ini**
and its
**init.lua**.
The
[Lua scripts](#lua-scripts)
dialog switches them on and off.

The option
**--no-lua**,
or the variable
**BLUERIG_NO_LUA**
set to 1, starts the editor without any Lua script, and
*enabled=false*
in the
**[Lua]**
section of the ini file does so for good.

How to write a script, and the functions a script can call, is described in
**LUA_PLUGINS.md**
and
**LUA_API_REFERENCE.md**,
which are installed with the documentation of bluerig.

# General settings

These settings of the
**[bluerig]**
section of the ini file concern the program rather than the editor. Unless
said otherwise, 1 turns a setting on and 0 turns it off.

*auto_save_setup*
: Write the settings to the ini file whenever the editor ends.

*skin*
: The name of the skin. See [Skins](#skins).

*shadows*
: Draw a shadow under the dialogs and the drop-down menus.

*keymap*
: The keymap file to use. See [Redefine key bindings](#keys_redefine).

*drop_menus*
: Open the first menu at once when F9 is pressed, instead of only the menu
bar.

*mouse_close_dialog*
: A click outside a dialog closes it.

*mouse_repeat_rate, double_click_speed*
: The auto repeat rate of the mouse buttons, and the time within which two
clicks make a double click, in milliseconds.

*old_esc_mode, old_esc_mode_timeout*
: With old_esc_mode on, a single Esc is taken as Esc after
old_esc_mode_timeout microseconds, instead of waiting for the next key.

*show_all_if_ambiguous*
: Show the list of the possible completions at once where the completion is
ambiguous. See [Completion](#completion).

*shell_patterns*
: The patterns of the user menu conditions are shell patterns (1) or regular
expressions (0).

*num_history_items_recorded*
: How many entries every history of an input line keeps.

*confirm_history_cleanup*
: Ask before a history is cleared.

*clear_before_exec*
: Clear the screen before a command of the user menu runs.

*pause_after_run*
: Wait for a key after a command of the user menu ran: never (0), on dumb
terminals only (1, the default) or always (2).

*alternate_plus_minus*
: Keep the plus and the minus keys of the numeric keypad as plus and minus.

*keybar_modifiers*
: If true, the key bar shows the commands of Shift, Ctrl or Alt with the F keys
while that modifier is held. This needs a terminal with the kitty keyboard
protocol: bluerig then asks it for every key with its release, and for focus
events. Other terminals are not affected. False by default.

*verbose*
: Show the progress of long operations.

*keybar_visible*
: In the
**[Layout]**
section: show the labels of the function keys at the bottom of the screen.

# Settings in the ini file <a id="internal-file-editor-options"></a>

The dialogs of the
**Options**
menu make most settings, and
**Save setup**
writes them to the
**[bluerig]**
section of
**~/.config/bluerig/ini**.
These are the editor settings of that file: 1 turns a setting on, 0 off, and
the default comes last.

|  |  |
|--|--|
| editor_tab_spacing | The width of a tab; 8 by default. editor_fake_half_tabs gives half tabs |
| editor_fill_tabs_with_spaces | Inserts spaces instead of a tab character; 0 |
| editor_return_does_auto_indent | Enter indents like the nearest line with text above; 1 |
| editor_backspace_through_tabs | Backspace deletes all the blank space to the left margin when no text stands between the cursor and the margin; 0 |
| editor_fake_half_tabs | A tab between the left margin and the text moves 4 columns, and the file gets spaces and 8-column tabs; 1 |
| editor_option_save_mode | How the file is written, see [Edit Save Mode](#edit-save-mode): 0 - straight into the file, and a failure while writing leaves it cut short; 1 - through a temporary file; 2 - with a backup copy; 0 |
| editor_word_wrap_line_length | The line length for wrapping; 72 |
| editor_backup_extension | The suffix of a backup file name; ~ |
| editor_line_state | Shows the line state column: numbers, folds, hidden runs; 0. Alt-n switches it |
| editor_visible_spaces | Shows the trailing spaces as dots; 1 |
| editor_visible_tabs | Shows tabs as \<----\>; 1 |
| editor_show_control_chars | Shows control characters as ^M, ^L, else as a blank cell; 1. An entry of the Command menu switches it |
| editor_persistent_selections | Keeps the selection after the cursor moves; 1 |
| editor_drop_selection_on_copy | Drops the selection after a copy to the clipboard file; 1 |
| editor_cursor_beyond_eol | Lets the cursor go past the end of the line; 0 |
| editor_cursor_after_inserted_block | Puts the cursor after an inserted block; 0 |
| editor_syntax_highlighting | Switches the syntax highlighting on; 1 |
| editor_edit_confirm_save | Asks before writing; 1 |
| editor_option_typewriter_wrap | Breaks the line at editor_word_wrap_line_length while typing; 0 |
| editor_option_auto_para_formatting | Formats the paragraph under the cursor while typing; 0 |
| editor_option_save_position | Keeps the position in the file on exit; 1 |
| editor_group_undo | Undoes several actions of one kind in a row at once; 0 |
| editor_wordcompletion_collect_entire_file | Takes the words for completion from the whole file, not only up to the cursor; 0 |
| editor_wordcompletion_collect_all_files | Takes the words for completion from all the open files; 1 |
| editor_stop_format_chars | Paragraph formatting leaves alone a line that starts with one of these characters, and the lines after it; -+\*\\,.;:&\> |
| editor_state_full_filename | Shows the full path in the status line, not the file name; 0 |
| editor_filesize_threshold | The size over which a file is opened after a question, with a suffix K or M; 64M |
| editor_show_right_margin | Marks the column editor_word_wrap_line_length as the right margin; 0. An entry of the Command menu switches it |
| editor_simple_statusbar | Shows a short status line, without the offset and the character under the cursor; 0 |
| editor_check_new_line | Asks about a missing newline at the end of the file when saving; 0. A check box of the save mode dialog switches it |
| editor_sticky_windows | Resizes windows whose frames touch as one, see [Window menu](#window-menu); 0 |

# ENVIRONMENT

The variables below are the ones bluerig reads. Variables such as **TERM**,
**SHELL**, **HOME** or **PATH** are not listed here: the program reads them
to find out where it runs, not to be configured by them.

**BLUERIG_DATADIR**
: The directory the data files are taken from, in place of
{{pkgdatadir}}. See [FILES](#files).

**BLUERIG_PROFILE_ROOT**
: The root of the user files, as an absolute path, in place of the home
directory. See [FILES](#files).

**BLUERIG_SKIN**
: The skin to use, by name or by path. See [Skins](#skins).

**BLUERIG_KEYMAP**
: The keymap file to use. See [Redefine key bindings](#keys_redefine).

**BLUERIG_TMPDIR**, **TMPDIR**
: The directory for the temporary files of the program, the first of the two
that is set; /tmp without either.

**BLUERIG_NO_LUA**
: Set to 1 to start without the Lua runtime. No Lua script is loaded.

**KEYBOARD_KEY_TIMEOUT_US**
: How long to wait for the rest of an escape sequence, in microseconds.

**COLORTERM**
: Read when the colors are chosen. See [Colors](#colors).

**BLUERIG_LOG_ENABLE**, **BLUERIG_LOG_FILE**
: A debug log is written when BLUERIG_LOG_ENABLE is 1. Without **BLUERIG_LOG_FILE** the
file named by *logfile* in the *[Logging]* section of the *ini* file is used,
and without that entry *~/.cache/bluerig/bluerig.log*.

**BLUERIG_SPELL_LOG**
: The file the spell checker writes its debug log to. It has no switch of its
own: the log is written when the variable names a file.

# FILES

Full paths below may vary between installations. The data files are also
affected by the
**BLUERIG_DATADIR**
environment variable: if it's set, its value is used instead of
{{pkgdatadir}} in the paths below. The user files follow the XDG base
directories:
*~/.config*,
*~/.local/share*
and
*~/.cache*
stand for
**XDG_CONFIG_HOME**,
**XDG_DATA_HOME**
and
**XDG_CACHE_HOME**
where these are set.

*{{pkgdatadir}}/help/bluerig.md*
: The help file of the program, and the help files of the editor plugins
beside it.

*{{sysconfdir}}/bluerig/bluerig.ini*, *{{pkgdatadir}}/bluerig.ini*
: System-wide setup files, used only if the user doesn't have his own
**~/.config/bluerig/ini**
file. If {{sysconfdir}}/bluerig/bluerig.ini exists, {{pkgdatadir}}/bluerig.ini isn't
used.

*{{sysconfdir}}/bluerig/defaults.ini*, *{{pkgdatadir}}/defaults.ini*
: Global settings, which affect all users, whether they have
~/.config/bluerig/ini or not. Currently, only
[terminal settings](#terminal-databases)
are loaded from it.

*{{sysconfdir}}/bluerig/keymap.ini*, *{{pkgdatadir}}/keymap.ini*
: The system-wide key bindings. See
[Redefine key bindings](#keys_redefine).

*{{sysconfdir}}/bluerig/bluerig.menu*, *{{pkgdatadir}}/bluerig.menu*
: The system-wide [user menu](#edit-menu-file).

*{{sysconfdir}}/bluerig/spell.keymap, ctags.keymap*
: The keys of the spell and the ctags plugins; a file of the same name in
~/.config/bluerig overrides them.

*{{pkgdatadir}}/syntax/\**
: The system-wide syntax files, used only if the corresponding user's own
file in
**~/.local/share/bluerig/syntax/**
is missing.

*{{pkgdatadir}}/skins/\**
: The skins that come with the program.

*{{pkgdatadir}}/charsets*
: The list of the codepages.

*{{pkgdatadir}}/lua/scripts/\**
: The Lua scripts that come with the program.

*{{pkgdatadir}}/examples/macros.d/\**
: Example scripts for the macros.

*{{plugins_dir}}/*
: The editor plugins that are loaded at run time.

*~/.config/bluerig/ini*
: User's own setup. If this file is present then the setup is loaded
from here instead of the system-wide setup file.

*~/.config/bluerig/keymap.ini*
: User's own key bindings.

*~/.config/bluerig/menu*
: User's own [user menu](#edit-menu-file).

*~/.config/bluerig/plugins.ini*
: The plugins and the Lua scripts that are switched off.

*~/.config/bluerig/term/\<TERM>*
: The keys taught in [Learn keys](#learn-keys).

*./.bluerig.menu*
: The user menu of the current directory. If this file is present in the
current directory, it is used instead of the home or system-wide menu.

*~/.local/share/bluerig/history*
: The histories of the input lines.

*~/.local/share/bluerig/filepos*
: The places the files were left at, for
*editor_option_save_position*.

*~/.local/share/bluerig/clipboard*
: The clipboard file of the editor.

*~/.local/share/bluerig/macros*
: The recorded [macros](#macros).

*~/.local/share/bluerig/macros.d/macro.N.sh*
: The shell scripts of the macros.

*~/.local/share/bluerig/syntax/Syntax*
: User's own syntax file.

*~/.local/share/bluerig/skins/*
: User's own skins.

*~/.local/share/bluerig/lua/scripts/*
: User's own Lua scripts.

*~/.cache/bluerig/block*
: The block file the user menu and the macro scripts work on.

*~/.cache/bluerig/temp*
: A temporary file of the editor.

To change the root directory of the user files, you can use the
**BLUERIG_PROFILE_ROOT**
environment variable. The value of BLUERIG_PROFILE_ROOT must be an absolute path.
If BLUERIG_PROFILE_ROOT is unset or empty, the HOME variable is used. If HOME is
unset or empty, the directories are taken from the GLib library.

# LICENSE <!-- help:skip -->

This program is distributed under the terms of the GNU General Public
License as published by the Free Software Foundation. See the built-in
help for details on the License and the lack of warranty.

# AVAILABILITY

The latest version of this program can be found at
<https://github.com/blue-panels/bluerig/releases> .

# SEE ALSO

cooledit(1), aspell(1), hunspell(1), ctags(1), etags(1), gpm(1),
terminfo(5), sh(1).

# AUTHORS

Paul Sheer (psheer@obsidian.co.za) is the original author of cooledit, which
bluerig continues. The authors and
contributors are listed in the AUTHORS file in the source distribution.

# BUGS

Bugs should be reported at
<https://github.com/blue-panels/bluerig/issues> .

Provide a detailed description of the bug, the version of the program you are
running
*(bluerig -V*
displays this information), and the operating system you are running the
program on. If the program crashes, we would appreciate a stack trace.
