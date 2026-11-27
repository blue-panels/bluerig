# Coverage <!-- help:notitle -->

**Coverage**

Marks in the gutter the lines that ran when the tests or the program ran,
and the lines of code that did not run. Works in *bluerig -D*.

|  |  |
|--|--|
| Project > Show/hide coverage | Show or take off the coverage marks |
| Project > Reread coverage | Read the coverage data now |

A line that ran gets a green bar, a line that did not run a yellow one.
Lines without code, comments and empty lines get no mark. The marks move
with their lines while the file is edited, until the data is read again.
The project tree shows after each file the share of its lines of code that
ran, *83%*, and after a directory that of all its files together.

**Where the data comes from**

|  |  |
|--|--|
| gcov | the *.gcda* files in the build directory of the profile of coverage, else in the meson or cmake build directory, else in the project root |
| lcov | *coverage.info*, *lcov.info* or *coverage/lcov.info* in the project root, *meson-logs/coverage.info* in the build directory |

The profile of coverage is a build profile, see Build profiles in the help
of the plugin Build: the one the plugin made, else the active profile when
it builds for coverage, else the first profile that does. Without data the
plugin offers to make it, from the active profile with *-Db_coverage=true*
and its tests run after its build, in a directory of its own: its window
opens, and once it is saved the plugin offers to build it and run the
tests. When the profile is there, the plugin offers to build it and run its
tests.

The *.gcda* files appear when a program built with *-Db_coverage=true*
(meson) or *--coverage* (gcc, clang) exits. Without them the first lcov
file found is read: *coverage lcov* writes it for Python, *cargo llvm-cov
--lcov* for Rust, *c8* and *nyc* for JavaScript.

gcov and the reading of the files run in a process of their own, and the
marks are set in parts while the editor is idle, so the editor does not
stop even on a large project.

**When the data is read again**

The plugin reads the data again after a build of the profile of coverage,
not of another profile, after a command in the terminal window and when a
file window comes to the front, at most once in two seconds. When the
coverage files have not changed since the last reading, gcov does not run.

The keys *coverage-covered-char* and *coverage-missed-char* of the
*[widget-editor]* section of the skin set the marks, *coveragecovered* and
*coveragemissed* of the *[editor]* section their colors.
