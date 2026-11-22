# Build <!-- help:notitle -->

**Build**

Builds the project, shows the output of the compiler as a list of errors
and goes to the lines with errors and warnings.

|  |  |
|--|--|
| Ctrl-F9, Alt-B | Build the project |
| Alt-J | Next error |
| Alt-K | Previous error |
| Enter | In the list of errors: go to the error |

The keys are changed in the *[build]* section of the keymap file. The same
commands are in the Project menu.

Saves the modified files of the project before the build. The output of the
compiler comes in the **Build** window at the bottom of the screen; Project >
Build output shows this window again. Alt-J goes to the warnings when there
are no errors.

**The list of errors**

After the build the window shows the result, then the errors and warnings,
one line each, with their notes under them, and at the end the rest of the
output of the compiler. The cursor of the list follows Alt-J and Alt-K. When
the cursor moves in the list, the line of the error is shown in its file, if
the file is open; Enter or a double click goes to it. The cursor goes to the
column the compiler named.

Lines with errors are marked in the gutter with a cross, lines with warnings
with an exclamation mark, until the next build. The keys *build-error-char*
and *build-warning-char* of the *[widget-editor]* section of the skin set the
marks, *builderror* and *buildwarning* of the *[editor]* section their
colors.

**How the project is built**

The plugin finds by itself how to build the project:

|  |  |
|--|--|
| meson | meson compile -C with the meson build directory of the project |
| cmake | cmake --build with the cmake build directory |
| make | make, if the project has a Makefile |
| language | the command of the language from presets.ini, e.g. cargo build or go build ./... |

Project > Build command... sets a build command of the project's own, which
takes precedence over the one found. If nothing is found, the plugin asks for a command at
the first build.

**Languages and project settings**

The file *presets.ini* in the configuration directory of bluerig describes
the languages: the markers in the root of a project, the file extensions, the
build command and the way to debug. The file *~/.config/bluerig/presets.ini*
extends it, and each of its groups replaces the bluerig group of the same
name entirely.

A project opened with *bluerig -D* or Project > Open project... gets a copy of
the settings of its language in *.bluerig/project.ini*; changes in it stay
with the project. A key the copy does not have comes from *presets.ini*.
Project > Language defaults of the project... makes the copy from the
language settings again.
