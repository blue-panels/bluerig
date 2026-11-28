---
date: November 2026
---

<!-- help:topics "Topics:" -->
# NAME <!-- help:skip -->

bluerig - a terminal IDE.

# SYNOPSIS <!-- help:skip -->

**bluerig**
[options] [+lineno] [file1[:lineno]] [file2[:lineno]] ...

# DESCRIPTION <a id="description"></a>

**bluerig** is a terminal IDE that grew out of mcedit (GNU Midnight Commander) and cooledit by Paul Sheer. **brg** is a shorter name of the same program.

The editor works with several files at once, each in its own window, binary files too. It marks column blocks, filters lines, folds text, records macros, passes text through shell commands from the user menu and runs Lua scripts. Plugins add projects, build and debugging, code coverage, language servers (LSP), git and code navigation, and spell checking.

Language support can be extended: syntax highlighting, build, debugging through DAP adapters and language servers are set up for each language. The default configuration includes syntax highlighting for more than 100 file types, build support for C (Meson, CMake and Make), Rust and Go, and debugging support for C, Rust, Python, Bash, Go and JavaScript.

# OPTIONS <a id="options"></a>

*+lineno*
: Go to the line specified by number (do not put a space between the
*+*
sign and the number). Several line numbers are allowed but only the last one
is used, and it is applied to the first file only.

*file:lineno[:]*
: Open the file at that line, the way compilers and grep print a place in a
file. When a file of the whole name, colon and number included, exists, that
file is opened instead.

**Application modes**

*-Q, --query*
: Open the file in the mode of queries to MongoDB or PostgreSQL. Set
*MC_QUERY_KIND=mongo* or *MC_QUERY_KIND=postgres*. If this variable is
not set, *MC_MONGO_URI* selects MongoDB, and a file with the extension
*.sql* selects PostgreSQL when *MC_MONGO_URI* is not set.
It needs the Query plugin.

*-D directory, --debug=directory*
: Open the project in debug mode. It shows the project tree and the
debugger panel. It needs the Project, Build and Debugger plugins.

*-G directory, --git=directory*
: Open the Git window for that directory. Closing the window ends
bluerig. One of the options below can follow it. Without *--git* the
option applies to the current directory.

**Git options**

*--git-log*
: Open the commit log.

*--git-show=commit*
: Show the files and the changes of that commit.

*--git-commit*
: Open the message editor to commit the staged changes.

*--git-amend*
: Change the last commit, adding the staged changes to it.
It opens the editor of the commit message.

*--git-reword=commit*
: Open the message editor of that commit. Saving the message
rewrites the Git history.

**General options**

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

*-h, -?, --help*
: Show the command line options and what they do.

*--help-all*
: Show the command line options of all the groups.

*--help-terminal, --help-color*
: Show the terminal options or the color options.

**Terminal options**

*-x, --xterm*
: Turn on xterm mode. Use it on terminals that can do what xterm does:
two screen modes and mouse sequences.

*-X, --no-x11*
: Do not use the X Window System to get the state of the modifier keys.

*-g, --oldmouse*
: Use the old mouse tracking of the terminal.

*-d, --nomouse*
: Disable mouse support.

*-t, --termcap*
: Use the termcap database instead of terminfo. The option exists only when
bluerig was built with the S-Lang library.

*-s, --slow*
: Run on a slow terminal: the screen is redrawn less often.

*-a, --stickchars*
: Draw the frames with plain characters instead of the line drawing
characters of the terminal.

*-K file, --keymap=file*
: Load the key bindings from that file. See
[Redefine key bindings](#keys_redefine).

*--nokeymap*
: Do not load any keymap file: use the key bindings built into the program.

**Color options**

*-b, --nocolor*
: Black and white display.

*-c, --color*
: Color mode on terminals that do not report color support.

*-S skin, --skin=skin*
: Use the skin of that name. See
[Skins](#skins).

# Overview <a id="overview"></a>

The screen of bluerig has the
[menu bar](#menu-bar)
at the top, the editor windows under it and the labels of the function keys at
the bottom. The menu bar may be hidden: F9 or a click on the top line of the
screen brings it up.

The status line at the top of a window shows the file name, the state of the
file and the place of the cursor. The [Window menu](#window-menu) switches
between the open windows. Windows can be moved and resized.

# The bluerig editor <a id="internal-file-editor"></a>

<a id="keys"></a>
**Keys**

<!-- {{key:Action|F8}}: Action is the action name of misc/keymap.default.ini and src/keymap.c (Remove = f8); the help shows the keys bound to it now, the man page the default keys after the bar -->
[How to change the bindings](#keys_redefine)

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

A click on the text moves the cursor, and a drag marks text. The mouse opens
menus, presses buttons and runs the commands shown in the bottom line of the
screen. The mouse wheel scrolls the text.

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

The menu bar pops up when you press {{key:Menu|F9}} or click the top line of
the screen. Its menus are File, Edit, Search, Command, Navigate (when a plugin
or a script added something to it), Window, Plugins and Options. A Lua script
may add entries to any of them, or menus of its own.

## File menu <a id="file-menu"></a>

|  |  |
|--|--|
| Open file... | Opens a file in a new window |
| New | Opens an empty window |
| Close | Closes the current window |
| History... | Opens the list of the recently edited files |
| Save | Writes the current file |
| Save as... | [Writes the file under a new name](#save-file-as) |
| Insert file... | Inserts the contents of a file at the cursor |
| Copy to file... | Writes the marked block to a file |
| User menu... | [Opens the list of commands that process the text](#edit-menu-file) |
| About... | Shows the version of the program |
| Quit | Closes the editor. Offers to save unsaved changes |

## Edit menu <a id="edit-menu"></a>

Undo, Redo and the Undo history; the switch between insert and overwrite
mode; the commands that mark a block, a column block or the whole file, and
unmark it; copying, moving and deleting the block; copy to, cut to and paste
from the clipboard file; going to the start and to the end of the file.

## Search menu <a id="search-menu"></a>

The search, replace and filter commands are described in
[Search, replace and the line filter](#search-and-replace).
The same menu sets or clears a bookmark on the current line, goes to the next
or the previous bookmark, and clears all the bookmarks.

## Command menu <a id="command-menu"></a>

|  |  |
|--|--|
| Go to line... | Moves the cursor to the line with the given number |
| Toggle line state | Switches the line state column |
| Go to matching bracket | Moves the cursor to the matching bracket |
| Toggle syntax highlighting | Switches the highlighting in the editor |
| Toggle right margin | Marks the column where lines wrap |
| Toggle control characters | Shows control characters as ^M, ^L and so on |
| Encoding... | [Chooses the encoding of the file](#codepages-translation) |
| Fold / Unfold block | [Folds or unfolds a block](#folding) |
| Unfold all | Opens all the folds and removes the line filter |
| Refresh screen | Redraws the screen |
| Start/Stop record macro | [Records a macro and binds it to a key](#macros) |
| Macro explorer... | [Opens the list of the recorded macros](#macro-explorer) |
| Record/Repeat actions | Records actions and repeats them a given number of times |

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
: When you drag the corner of a window, the frames of the next windows move
with it, so the windows keep a common border. Where four windows meet in a
corner, you can move the horizontal and the vertical border at once. A border
that was on the edge of the screen at the start stays there. When the border
you move reaches another window, that window starts to resize too. Every
window has a minimum size. Esc cancels the change. A window dragged by its
title moves alone.
The setting is kept as
**editor_sticky_windows**
in the ini file.

**Next, Previous**
: Go to the next or to the previous window.

**List...**
: The open windows, to go to one of them. See
[Open files](#open-files).

## Plugins menu <a id="plugins-menu"></a>

The editor plugins that have a menu entry of their own, for example the spell
checker. A plugin that is switched off in
[Manage plugins](#manage-plugins)
is not in the menu.

## Options menu <a id="options-menu"></a>

|  |  |
|--|--|
| General... | [Sets the indents, line wrap and other editor options](#editor-options) |
| Save mode... | [Chooses how files are written](#edit-save-mode) |
| Syntax highlighting... | Chooses the highlighting rules by hand, or by the file name and the first line |
| Appearance... | [Sets the skin and the shadows](#appearance) |
| Learn keys... | [Maps keys to terminal sequences](#learn-keys) |
| Key bindings... | [Binds keys to actions](#key-bindings) |
| Key sniffer... | [Shows the terminal sequence and the bound action](#key-sniffer) |
| Manage plugins... | [Switches plugins on and off and sets them up](#manage-plugins) |
| Plugin info... | [Shows the state and the features of the plugins](#plugin-info) |
| Lua scripts... | [Manages the Lua scripts](#lua-scripts). There when built with Lua |
| Syntax file | Opens the [user syntax file](#syntax-highlighting) in the editor |
| Menu file | Opens the [user menu file](#edit-menu-file) in the editor |
| Save setup | Writes the settings to *~/.config/bluerig/ini* |

"Syntax file" opens *~/.local/share/bluerig/syntax/Syntax*. If the file does
not exist yet, the editor makes it with a short comment. The system file is
read after the user file.

With *auto_save_setup* on, the editor saves the settings on exit.

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
| Fake half tabs | In the indent the cursor moves by half a tab stop. The file gets spaces and tab characters |
| Backspace through tabs | Backspace deletes the whole indent to the left margin when no text stands between the cursor and the margin |
| Fill tabs with spaces | Inserts spaces to the next tab stop instead of a tab character |
| Tab spacing | The width of a tab character; 8 by default |

**Keyboard**

|  |  |
|--|--|
| Esc timeout, ms | How many milliseconds the editor waits for the rest of an escape sequence after Esc. 1000 by default. The setting *old_esc_mode_timeout* is in microseconds. At startup the variable **KEYBOARD_KEY_TIMEOUT_US** takes priority |

**Other options**

|  |  |
|--|--|
| Return does autoindent | Enter starts the line with the indent of the line above |
| Confirm before saving | Asks before writing the file |
| Save file position | Restores the cursor position when the file is opened |
| Visible trailing spaces | Marks the spaces at the end of lines |
| Visible tabs | Marks the tab characters |
| Show control characters | Shows control characters as ^M instead of hiding them |
| Syntax highlighting | Colors the text by the syntax rules of its type |
| Cursor after inserted block | Leaves the cursor at the end of an inserted block, not at its start |
| Persistent selection | Keeps the selection when the cursor moves |
| Cursor beyond end of line | Lets the cursor go past the end of the line |
| Group undo | Undoes a series of changes of one kind at once |
| Word wrap line length | The column where the wrap modes break the line; 72 by default |

### Edit Save Mode <a id="edit-save-mode"></a>

How a file is written, the same three modes the
*editor_option_save_mode*
setting takes:

**Quick save**
: Write the data straight into the original file. If writing fails, the file
may be left incomplete.

**Safe save**
: Write the data to a temporary file, and replace the original with it only
when writing is complete.

**Do backups with following extension**
: Safe save, and the original is kept under its name with the extension of the
input line added, "~" by default. A second save replaces the backup as well.

**Check POSIX new line**
: Ask about the missing newline at the end of the file before it is written.

### Appearance <a id="appearance"></a>

Here you choose the skin and set the shadows of the dialogs and the drop-down
menus.

**Skin**
: The button opens the list of the installed skins and the skins of the user;
the one chosen is applied at once.

**Shadows**
: Draw a shadow under every dialog and drop-down menu. Without colors there
are no shadows.

The choice is written to the
*skin*
and
*shadows*
settings of the ini file. The order in which skin files are looked for is
described in [Skins](#skins).

### Learn keys <a id="learn-keys"></a>

This dialog maps the escape sequences of the terminal to the function keys,
the arrows and the navigation keys. The title shows the terminal name from the
**TERM** variable.

Select a modifier combination (Ctrl, Alt, Shift) with the checkboxes, then
press the button of a key. Press that key on the keyboard and wait for the
capture message to disappear. The learned sequence appears next to the button.
If another key has it already, the editor reports a conflict.

**Del**
: Delete the saved sequence of the selected key.

**Save**
: Write the mappings of keys to sequences to
*~/.config/bluerig/term/\<TERM>*.

**Edit term file**
: Open that file in the editor.

### Key bindings <a id="key-bindings"></a>

The table shows the actions and the keys bound to them. The buttons at the
top choose the group, Editor or Widgets. The Editor group has the parts Main
and Ctrl-X, the Widgets group has Dialog, Menu, Input, Listbox, Radio and
Help. Back returns to the list of groups. Actions with changed bindings are
marked with \*.

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
: Open the file of the escape sequences saved in the
[Learn keys](#learn-keys)
dialog.

If the key is bound to another action already, the editor asks you to
confirm the new binding.

### Key sniffer <a id="key-sniffer"></a>

Press
**Capture key**,
then press the key you want to check. The dialog shows its data:

```
Shortcut   the symbolic name, e.g. Ctrl-F5
Action     the action of the key in the editor keymap
Raw        the escape sequence and its bytes
Keycode    the internal numeric code
```

If the editor does not recognize the sequence, set it up in the
[Learn keys](#learn-keys) dialog. If the key is recognized, check the action
bound to it in the [Key bindings](#key-bindings) dialog.

### Manage plugins <a id="manage-plugins"></a>

The table shows the kind, the name and the description of every editor plugin.
If bluerig was built with Lua, the table also has the Lua runtime, of the kind
*runtime*.
The checkbox in the row of a plugin decides whether it is loaded at the next
start. The choice is kept in *~/.config/bluerig/plugins.ini*.
Plugins that are switched off stay in the list.

**Enter, F4**
: Open the settings of the selected plugin. If the plugin has no settings, the
editor says so. For the Lua runtime it opens the
[Lua scripts](#lua-scripts) dialog.

### Plugin info <a id="plugin-info"></a>

The list of the loaded plugins shows their names, state, features and
description. The settings and the switches of the plugins are in the
[Manage plugins](#manage-plugins) dialog.

### Lua scripts <a id="lua-scripts"></a>

The table shows the state, the location (global or user), the identifier, the
workspace, the features and the description of every Lua script found.
The checkbox in the row of a script decides whether it is loaded at the next
start of bluerig.

**Settings**
: Run the script that holds the settings of the package, if there is one.

Where the packages live and how they are built is described in
[Lua scripting](#lua-scripting).

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

The filter hides the lines without a match and keeps the line numbers. The
line state column marks the hidden runs. The filter is applied when it is
switched on. New and changed lines stay shown until the filter is applied
again.

|  |  |
|--|--|
| {{key:FilterToggle|Alt-s}} | Removes the filter, or applies the last search as a filter. If there was no search yet, opens the search dialog |
| {{key:FilterWord|Alt-S}} | Filters by the marked text or by the word under the cursor |
| Command \> Unfold all | Lifts the filter and every fold |

# Folding <a id="folding"></a>

A fold shows only the first line of a block. The line state column marks
that line.
{{key:FoldToggle|Alt-F}}
(the entry Fold / Unfold block of the Command menu) folds the marked lines,
or, with nothing marked, the lines from a bracket on the current line to the
bracket that matches it. On the first line of a fold this key unfolds it.
**Unfold all**
opens all the folds of the file and removes the line filter.

# Macros <a id="macros"></a>

{{key:MacroStartStopRecord|Ctrl-r}}
starts recording a macro. To end the recording, press
{{key:MacroStartStopRecord|Ctrl-r}}
again, then press the key that will run the macro.

The macros are kept in the section
**[editor]**
of the file
**~/.local/share/bluerig/macros**.
To delete a macro, delete its line from the file or use the
[Macro Explorer](#macro-explorer).

The entry
**Record/Repeat actions**
of the Command menu records the actions that follow until it is chosen again,
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

The list of the recorded macros shows their keys and actions.

**Run**
: Run the selected macro.

**Delete**
: Delete the selected macro after a question.

**Edit file**
: Open the macro file. Its format is described in [Macros](#macros).

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

Give the file name and the line break format. You can keep the format of the
file, or choose Unix (LF), Windows and DOS (CR LF), or Macintosh (CR).
The chosen format applies to this save.

## Open files <a id="open-files"></a>

The list has one line for every open file. Enter switches the editor to the
selected file, Esc closes the list. The entry
**List...**
of the Window menu opens the same list.

## Screen selector <a id="screen-selector"></a>

Besides the editor, a Lua script may open screens of its own, full-screen
windows that stay open while you go back to the editor. There are three ways
to switch between the screens:

|  |  |
|--|--|
| Alt-} | Next screen |
| Alt-{ | Previous screen |
| Alt-\` | List of the open screens |

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

If several fit, the editor beeps. The setting
*show_all_if_ambiguous*
of the ini file, off by default, shows their list next to the cursor:
**Enter**
chooses one, typed letters narrow the list, Esc, F10, Left and Right hide it.

## Choose codepage <a id="codepages-translation"></a>

The editor loads the list of encodings from **{{pkgdatadir}}/charsets**.
Choose the encoding of the file; the editor converts the text from it for
the terminal. The entry **\<No translation>** turns the conversion off.
The list opens with {{key:SelectCodepage|Alt-e}} or with the entry Encoding...
of the Command menu.

## History <a id="history-query"></a>

**Alt-h** opens the history of the current input line. Recent entries come
first. Enter puts the selected entry into the line, Esc closes the history
without changing the line, and **F8, Del** delete the selected entry. The
history is kept in
*~/.local/share/bluerig/history*;
the setting
*num_history_items_recorded*
says how many entries each history keeps.

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

When both the user rules and the installed rules match a file, the user
rules apply. The files named by
**include**
lines are looked for in
*~/.local/share/bluerig/syntax/*
first, then among the installed ones. The user file is read again each time
a new file is opened in the editor.

A full copy of the installed file can hide the rules for file types added in
a new version. Keep only the changed entries in the user file.

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
turns on the spell check inside the context.

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
embedded rules too. Syntaxes can be nested several levels
deep. For example, the HTML rules can use the JavaScript syntax for the
contents of the SCRIPT tag:

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
| \\t, \\s | Tab and space. As they are, they separate the words of the line, so a pattern uses these forms |
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

Instead of a foreground color you can give a scope token: a word in capitals
that names the kind of text. The skin sets its color:

```
keyword  whole  if   KEYWORD_CONTROL
context  " " STRING_QUOTED_DOUBLE
    keyword  \\"   CONSTANT_CHARACTER_ESCAPE
keyword  linestart TODO  KEYWORD_OTHER_TODO black
```

The skin sets the colors of the interface and of the highlighting tokens for
all languages. See
[Skins](#skins).
The scope name is written in lower case, with dots for the underscores. For
example, KEYWORD_CONTROL_IMPORT is keyword.control.import, a kind of
keyword.control, which is a kind of keyword. These are the scope names of
Sublime Text and TextMate. A background and attributes in the skin take
priority over the values given after the token in the rule. If the skin does
not set them, the values of the rule are used, for example black above. The
other values come from the standard mcedit scheme. A token works in a define
too, and on the number, string and symbols lines of line-local rules. The
installed syntax files use tokens. The list of the tokens and their mcedit
colors is in
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

# Colors <a id="colors"></a>

bluerig will try to detect if your terminal supports color using the terminal
database and your terminal name. Sometimes it gets confused, so you may force
color mode or disable color mode using the -c and -b flag respectively.

If the program is compiled with the S-Lang screen manager instead of ncurses,
it will also check the variable
**COLORTERM**:
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

# Skins <a id="skins"></a>

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
of the skin. A key sets the color of a scope. To set the color of a scope
inside a context, give the scope of the context first, then the scope of the
text, with a space between them. In each case the longest matching scope
name is chosen. The pair with the context is checked first.

```
[syntax]
    comment = gray
    keyword = yellow
    keyword.control = brightmagenta
    string = green
    string constant.character.escape = white;red
    invalid = white;red;bold
```

Scopes without a color in the skin use the standard mcedit colors. Colors
given explicitly in the highlighting rules stay the same with any skin. See
[Syntax Highlighting](#syntax-highlighting).

# Terminal databases <a id="terminal-databases"></a>

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
key symbol sets the escape sequences for completion. Usually Alt-Tab does
it. You can give the sequence of another key.

# Lua scripting <a id="lua-scripting"></a>

When bluerig is built with Lua, scripts can add commands to the menus, bind
them to keys, handle the events of the editor, and open dialogs and
full-screen windows. Lua implements, for example, sorting the lines, formatting
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
section of the ini file turns Lua off for later starts too.

How to write a script, and the functions a script can call, is described in
**LUA_PLUGINS.md**
and
**LUA_API_REFERENCE.md**,
which are installed with the documentation of bluerig.

# General settings <a id="general-settings"></a>

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
protocol. bluerig asks the terminal for key press and release events and for
focus events. Off by default.

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

# ENVIRONMENT <a id="environment"></a>

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
: The path of the debug log of the spell check. If the variable is set, the
spell check writes its log to that file.

# FILES <a id="files"></a>

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
: The help file of the program (translations are bluerig.ru.md and so on);
the help files of the editor plugins are beside it.

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

# AVAILABILITY <a id="availability"></a>

The latest version of this program can be found at
<https://github.com/blue-panels/bluerig/releases> .

# SEE ALSO <a id="see-also"></a>

cooledit(1), aspell(1), hunspell(1), ctags(1), etags(1), gpm(1),
terminfo(5), sh(1).

# AUTHORS <a id="authors"></a>

Paul Sheer (psheer@obsidian.co.za) is the original author of cooledit, which
bluerig continues. The authors and
contributors are listed in the AUTHORS file in the source distribution.

# BUGS <a id="bugs"></a>

Bugs should be reported at
<https://github.com/blue-panels/bluerig/issues> .

Provide a detailed description of the bug, the version of the program you are
running
*(bluerig -V*
displays this information), and the operating system you are running the
program on. If the program crashes, we would appreciate a stack trace.
