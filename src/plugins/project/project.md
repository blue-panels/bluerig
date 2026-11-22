# Project <!-- help:notitle -->

**Project**

Opens the files of the project by a few letters of the name, finds a text in
all its files and shows the tree of the project at the left of the screen.

|  |  |
|--|--|
| Alt-P | Open a file of the project |
| Ctrl-e | Recent files of the project |
| Alt-T | Show or hide the project tree |
| Alt-A | Switch between source and header |
| Alt-F7 | Find a text in the files of the project |

The keys are changed in the *[project]* section of the keymap file. The same
commands are in the Project menu, and Alt-A in the Navigate menu.

**Open a file of the project**

Shows the recent files of the project, and as you type, the files whose
names have the typed letters in order. The line takes other queries too:

|  |  |
|--|--|
| edwi | Files with these letters in the name: *src/editor/editwidget.c* |
| main:42 | The file *main* at line 42 |
| :42 | Line 42 of the current file |
| @divi | Functions, types and variables of the project with these letters in the name |
| calc@ | Symbols of the file *calc*; *calc@tw* only those with *tw* |

The symbols come from the Ctags plugin; without it the queries with @ do not
work.

**Find in project**

Finds the text as you type, from the second letter, and shows the lines
found, and under them the file around the chosen line. A search without
capital letters ignores case. Shows at most 5000 lines and skips
binary files.

|  |  |
|--|--|
| Up, Down, PgUp, PgDn | Choose a line |
| Enter | Go to the line |
| Esc | Close the search |

**The project tree**

Shows the directories and files of the project, and under the file of the
front window its functions, types and variables. The file of the front
window and the symbol under the cursor are in bold. At the right of a file
stands the number of its lines; large and binary files show their size.

|  |  |
|--|--|
| Enter | Open the file, open or close the directory |
| Right, Left | Open or close a directory or the symbols of a file |
| A letter | Next name that starts with this letter |
| Esc, F10 | Close the tree |

The *[project]* section of the skin sets the marks before the names, and the
file *project-marks.ini* says what each name is. The file
*~/.config/bluerig/project-marks.ini* adds to it.

**Which directory is the project**

The first file opened gives the project: the nearest directory above it with
*.bluerig* or *.git* in it, else the outermost directory with a build file
(*meson.build*, *CMakeLists.txt*, *Makefile*), but not above the home
directory. Project > Open project... chooses another project, Project > Close
project closes it. The files of the project are what git knows, else
everything but hidden directories and build directories.
