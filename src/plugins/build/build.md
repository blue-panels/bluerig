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

A build directory and the way it is built make a build profile, see
[Build profiles](#build-profiles). Ctrl-F9 builds the active one. The profile
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
Ctrl-F9 builds it and the debugger runs the programs it builds. A profile
of make builds in the tree of the sources: one made from it builds there
too, and the list says it shares the tree with the other.

**Space**
: Make the profile of the cursor the active one. Enter does the same and
closes the window.

**Ins**
: A new profile made from the one of the cursor: its options and commands,
in a directory of its own next to its directory, opened in its window. The
first build sets the new directory up.

**F4**
: The window of the profile, see [Build profile](#build-profile).

**F5**
: Build the profile, the active one stays.

**Del**
: Delete the profile. Its build directory stays.

The profiles are kept in the configuration of the user, not in the project:
*~/.config/bluerig/build/*, one file for each project.

# Build profile

The window of a profile: its fields on top, the options of its build
directory under them. Tab goes between the fields and the options; the line
under the options says what the field or the option of the cursor is.

**Name**
: The name of the profile, without *[* and *]*.

**Directory**
: The build directory, from the root of the project (*../coole-build*,
*build/debug*) or absolute. A directory not set up yet is set up by F2 or by
the first build, with the options of the profile.

**Build command**
: The command that builds the profile, run in the root of the project.
Empty is the command of the build system: *meson compile -C* or *cmake
--build* with the directory, set up first when it is not yet. Leave it empty
unless the profile is built another way.

**Test command**
: The command that runs the tests, run in the root of the project: *meson
test -C* or *ctest --test-dir* with the directory. Empty: no tests.

**Run the tests after the build**
: Ctrl-F9 runs the test command after the build. The coverage of the tests
comes from a build with *b_coverage* and its tests run.

The options of the directory, meson or cmake, show a value and a default; a
value that is not its default has another color. The frame says what the
directory is: not set up yet, changed, applied.

**Del**
: Undo the change of the option of the cursor.

**F2**
: Save the profile and apply the changes: *meson configure* with them, or
*meson setup* with all the options of the profile when the directory is
not set up yet; cmake likewise. The output comes in the Build window, the
window of the profile stays. The options are then read again from the
directory.

**F3**
: Show the main options, those off their default, or all of them, in turn.
The main ones are those set by the user, the options of the project and
those that say what the build is for.

**F4**
: Change the option of the cursor: a boolean takes its other value, an
option with choices takes one from a list, another one is typed. A change
not applied is marked with *\** before the name.

**Enter**
: Save the profile and apply the changes of the options, then close the
window, in the fields and in the options alike.

**Esc**
: Close the window; what is not saved is asked about.

Letters typed in the options filter their names.
