# Build <!-- help:notitle -->

**Build**

Builds the project and shows the errors and warnings of the compiler as a
list. From the list you can go to the line of an error.

|  |  |
|--|--|
| Ctrl-F9, Alt-B | Build the project |
| Alt-J | Next error |
| Alt-K | Previous error |
| Enter | In the list of errors: go to the error |

Keys can be reassigned in the *[build]* section of *~/.config/bluerig/keymap.ini*. The same
commands are in the Project menu.

Saves the modified files of the project before the build. The output of the
compiler comes in the **Build** window at the bottom of the screen; Project >
Build output shows this window again. Alt-J and Alt-K go to the warnings
when there are no errors.

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

A profile sets the build directory, the build and test commands and the
options of the build system. Ctrl-F9 builds the active
[profile](#build-profiles). The profile
found first is *Default*; a command of its own set in it takes precedence over
the one found. If nothing is found, the plugin asks for a command at the first
build.

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

# Build profiles

Project > Build profiles... lists the profiles of the project: a name, a
build directory, the build system and what the directory is built for
(debug, coverage, address,undefined). The active profile is marked with *\**:
Ctrl-F9 builds it and the debugger runs the programs it builds. Make profiles made from one
profile build in the same source tree. The list marks such profiles as
sharing a directory.

|  |  |
|--|--|
| Space | Make the selected profile active |
| Enter | Make the selected profile active and close the list |
| Ins | Create a profile with the options and commands of the selected one and open it for editing |
| F4 | Open the [profile for editing](#build-profile) |
| F5 | Build the selected profile; the active one stays |
| Del, F8 | Delete the profile. The build directory stays |

A new profile gets a directory next to the old one, if the build system
supports separate directories. The first build sets it up.

The profiles are kept in the configuration of the user, not in the project:
*~/.config/bluerig/build/*, one file for each project.

# Build profile

The fields of the profile are at the top of the window, the options of the
build directory under them. Tab moves the focus between the fields and the
options. The line under the list describes the selected field or option.

**Name**
: The name of the profile, without *[* and *]*.

**Directory**
: The build directory, from the root of the project (*../coole-build*,
*build/debug*) or absolute. A directory not set up yet is set up by F2 or by
the first build, with the options of the profile.

**Build command**
: The command that builds the profile, run in the root of the project. Empty
means the command of the build system: *meson compile -C* or *cmake --build*
with the build directory, or *make*. For Meson and CMake the plugin first sets
up a new directory. It also does this before your own command, if the command
names the build directory and does not run the setup itself.

**Test command**
: The command that runs the tests, run in the root of the project: *meson
test -C* or *ctest --test-dir* with the directory. Empty means no tests.

**Run the tests after the build**
: Runs the test command after a successful build, also for a build by F5
from the list of profiles. Test coverage comes from a build with
*b_coverage* and the tests run: when *b_coverage* becomes *true*, the box is
checked and an empty test command is filled in.

The options of the directory, meson or cmake, show a value and a default; a
value that differs from its default has another color. The frame shows the
state of the directory: not set up yet, changed, applied.

|  |  |
|--|--|
| Del | Undo the change of the selected option |
| F2 | Save the profile and apply the options; the window stays open |
| F3 | Switch the list of options: main, different from the default, all |
| F4 | Change the selected option |
| Enter | Save the profile. If options changed, apply them and close the window after a successful setup. If only the fields of the profile changed, close the window without setting up the directory |
| Esc | Close the window. If there are unsaved changes, it asks first |

F2 applies the options with *meson configure* or sets up a new directory
with *meson setup*. For CMake it runs the matching setup command. The output
comes in the Build window. After a successful setup the options are read
again from the directory.

The main options are those set by the user, the options of the project and
the options that say what the build is for.

F4 toggles a boolean option. If an option has a list of allowed values,
pick one of them. For other options, type the value. A change not applied
yet is marked with *\** before the name.

Letters typed in the options filter their names.
