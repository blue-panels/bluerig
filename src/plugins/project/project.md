# Project <!-- help:notitle -->

**Project**

The plugin works with the files of the project: navigation and search.

|  |  |
|--|--|
| Alt-P | Open a file of the project |
| Ctrl-E | Recent files of the project |
| Alt-T | Open the project tree or go to it; in the tree - close it |
| Alt-A | Go from a source file to its header and back |
| Alt-F7 | Find a text in the files of the project |

Keys can be reassigned in the *[project]* section of *~/.config/bluerig/keymap.ini*.
The same commands are in the Project menu, and Alt-A is in the Navigate menu.

**Open a file of the project**

The dialog first shows the list of recent files of the project. If there are
none, it shows the files of the project, at most 500. Typing filters the list
by fuzzy search over all files: the paths that stay have the typed letters in
order. Enter opens the chosen file. The line also takes these queries:

|  |  |
|--|--|
| edwi | Files with these letters in order in the path: *src/editor/editwidget.c* |
| main:42 | The file *main* at line 42 |
| :42 | Line 42 of the last opened file |
| @divi | Functions, types and variables of the project with these letters in the name |
| calc@ | Symbols of the file *calc*; *calc@tw* - only those with *tw* |

The symbols come from the Ctags plugin; without it the queries with @ do not
work.

**Find in project**

The search runs as you type, from the second letter, without regular
expressions: it finds the lines that contain the typed text. If the query has
no capital letters, case is ignored. Under the list you see the part of the
file around the chosen line. The list has at most 5000 lines. The search reads
the files of the project: in a git repository - the tracked files and the
files not ignored by *.gitignore*; without git - all files except hidden ones,
build directories and *node_modules*. Binary files and the *.bluerig*
directory are always skipped.

|  |  |
|--|--|
| Up, Down, PgUp, PgDn | Choose a line |
| Enter | Go to the line |
| Esc | Close the search |

**The project tree**

Shows the directories and files of the project. The active file can show its
functions, types and variables; the Ctags plugin finds them. For files over
1 MiB the list of symbols is not built. The file of the active window and the
symbol under the cursor are in bold. At the right of a file name stands the
number of its lines; large and binary files show their size. When coverage is
on, the coverage percent follows the number of lines or the size, for
example *83%*. A directory shows the total percent of its files.

|  |  |
|--|--|
| Enter | Open the file, go to the line of the symbol, open or close the directory |
| Right, Left | Open or close a directory or the symbols of a file |
| A letter | Go to the next name that starts with this letter |
| Esc, F10 | Close the tree |

The *[project]* section of the skin sets the marks before the names, and the
file *project-marks.ini* maps names to marks; the file
*~/.config/bluerig/project-marks.ini* adds to it.

**Which directory is the project**

To find the root, the plugin goes up from the directory of the opened file.
It goes up to the home directory if it is on the path, else to the root of
the file system. The plugin takes the nearest directory with *.bluerig* or
*.git*. If there is none, it takes the outermost directory with a project
marker, for example *meson.build*, *CMakeLists.txt*, *Makefile*,
*Cargo.toml*, *go.mod* or *package.json*. If there are no markers, it uses
the directory of the file and shows only the files in it.

Project > Open project... chooses another project, Project > Close project
closes it.

In a Git project the list has the tracked files and the untracked files that
Git does not ignore. When it walks directories, the plugin skips hidden files
and directories, build directories and links to directories.
