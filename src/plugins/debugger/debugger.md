# Debugger <!-- help:notitle -->

**Debugger**

Runs the program of the project under GDB or the debug adapter of its
language: puts breakpoints in the gutter, steps in the source window and
shows the call stack, the variables and the watches beside it.

|  |  |
|--|--|
| Shift-F9 | Build and run, or continue |
| Ctrl-F8, Ctrl-B | Toggle breakpoint |
| Alt-F6 | Breakpoint condition |
| Alt-F5 | To the debugger panel and back |
| Ctrl-F2 | While debugging: stop the program |

These keys work in any file window. Change them in the *[debugger]*
section of the keymap, for example *DebugStepOver = alt-o*; there *f15* is
Shift-F5, *f17* is Shift-F7, *f18* is Shift-F8. The Debug menu has the
commands too.

**First start**

On the first start the debugger makes a configuration and shows the form
Debug configuration: the program, its arguments, the directory, the
environment, GDB, the options of ctags and the build before the start.
Enter accepts the form.

|  |  |
|--|--|
| Project | The one the Project plugin finds, or the one chosen in Project > Open project... |
| Program | Built by the Build plugin with debug information; of several, a list, newest first |
| Language | From *presets.ini*: GDB, the program before the first build, the adapter; rust-gdb for Rust |

For a program without debug information or built with optimization, the
debugger says so and names the command that builds it for debugging.

The project is built before the start. When the build fails, Alt-J goes to
the errors.

The arguments and the environment are split as a shell splits them, but no
shell runs them: *GREETING="hello world"*.

**Step mode**

While the program is stopped, the source windows take the debugger keys.
The keys that move, search and select stay the editor's, and a key that
changes the text beeps.

|  |  |
|--|--|
| F5, Shift-F9 | Continue |
| Shift-F6 | Pause |
| F7 | Step into |
| F8 | Step over |
| Shift-F8 | Step out |
| Shift-F7, Ctrl-F7 | Step into by one instruction |
| Shift-F4 | Step over by one instruction |
| F4, Alt-F9 | Run to the cursor line |
| F6 | Toggle breakpoint |
| Alt-F8 | Evaluate the selection or the word under the cursor |
| Alt-F10 | Back to the stop line |
| F3 | Index the symbols of the project again |
| Shift-F5, Ctrl-F2 | Stop the program |
| Esc | Back to editing until the next stop |

F2, F9 and F10 stay Save, the menu and Quit. Shift-F7 and Shift-F4 belong
to the debugger only while the program is stopped; the rest of the time
they are Search again and Replace again.

The keys come from IntelliJ IDEA where the editor leaves them free; F5 and
F4 are added because F9 and F10 are the menu and Quit.

**Debug mode**

*bluerig -D DIR* opens the project for debugging: the latest file of the
project, the project tree at the right and the debugger panel under it. In
this mode the debugger keys F3-F8 always work in the file windows, and the
text is edited as usual. Window > Debug keys in files turns this mode on
without *-D*. Before it quits, the editor asks for confirmation.

At the start the windows are arranged as in the *Debug* layout: the tree
and the panel at the right, the console at the bottom. After debugging the
windows go back to their places. Window > Layout... changes this or turns
it off.

**The program in a terminal**

With Run in a terminal window checked, the program runs in the Program tab
at the bottom: its output is there, and all keys go to it. When a step
takes longer than 0.3 s, the focus goes to the tab, and after the stop it
comes back to the file. Alt-F5 goes from the tab to the panel. Unchecked,
the output goes to the console, and Send line... of the Debug menu sends
the program a line.

**Debugger panel**

At the right: the state of the program, Locals, Registers, Watches, Call
stack and Breakpoints. Under the stop line, *ran 0.004 s* is the time from
the step to the stop, including the debugger's own time. Window >
Debugger panel and Window > Debug console reopen the panel and the console.

|  |  |
|--|--|
| Up, Down, PgUp, PgDn, Home, End | Move between rows |
| Right, Left | Open or close |
| Enter | Open, show the value, go to the frame or breakpoint |
| Space | Disable or enable the breakpoint |
| Ins | Add a watch |
| Del | Remove the watch or breakpoint |
| Alt-F6 | Breakpoint condition |
| : | Command of GDB or of the adapter console |
| F10 | Close the panel |

A value the step has changed is yellow and marked `*`, a new one is green
and marked *+*, an error is red. Open values stay open from stop to stop.
The registers are read only while their section is open, and they open by
themselves in code without source.

The *[debugger]* section of the skin gives the colors and marks of the
panel: *title*, *changed*, *added*, *error*, *internal*, *changed-char*,
*added-char*, *arrow*.

**Values in the source**

While the program is stopped, the lines of the function up to the stop line
show, after their text, the values of the variables they use, and in
assembler the registers. *editnote* in the *[editor]* section of the skin
gives the color.

**Gutter marks**

|  |  |
|--|--|
| o | Breakpoint |
| - | Disabled breakpoint |
| c | Breakpoint with a condition |
| ? | Breakpoint the debugger has not accepted yet |
| > | Stop line |
| @ | Stop line with a breakpoint |

These are the marks without a skin. The *[widget-editor]* section of the
skin gives characters of its own, circles and a diamond in darkfar, and the
*[editor]* section the colors.

A breakpoint moves with its line. On a line without code it moves to the
next line with code.

**Breakpoint condition**

Alt-F6 sets a condition in the language of the program: *i == 7*,
*x > 2 and y*. The program stops only when the condition is true. An empty
condition makes the breakpoint a plain one. The panel shows the condition:
*loop.c:7 main if i == 7*.

**Debug menu**

|  |  |
|--|--|
| Breakpoint on function... | Breakpoint on a project function by name |
| Run to function... | Continue to a function |
| Call stack... | Choose a stack frame |
| Evaluate expression... | Evaluate an expression, as Alt-F8 |
| Add watch... | Add a watch |
| Exceptions... | Exceptions to stop on |
| Send line... | Send a line to the program |
| Stop | End the session |

The function names come from the index of the Ctags plugin. A watch shows
a value, but does not stop the program when the value changes.

Exceptions... offers the kinds the debugger knows: for GDB, C++ exceptions
when thrown or caught; for debugpy, raised, uncaught and user uncaught.
While none is chosen, the adapter stops on its own kinds.

**Instructions**

Window > Disassembly shows, under the source, the instructions of the
frame's function, each source line before its instructions. The window
opens by itself on a stop in code without source.

|  |  |
|--|--|
| Up, Down, PgUp, PgDn, Home, End | Move between instructions |
| F6 | Breakpoint on the instruction |
| Enter | To the source line |

A step into code without source (*printf*, for example) goes on until it
leaves it; a step by one instruction stays there.

**Configurations**

Project > Debug configurations... shows the configurations of the project;
a star marks the one Shift-F9 runs. With Keep in the project checked they
are kept in *.bluerig/debug.ini* of the project, else in the settings of
the user. The breakpoints and the watches are kept in the settings of the
user.

**Debug adapters**

In the Debug configuration form, choosing Debug adapter instead of GDB runs
the program through a Debug Adapter Protocol adapter:

|  |  |
|--|--|
| Adapter | Command of the adapter |
| Address | host:port of the adapter; with port 0 the port comes from the command output |
| Launch (JSON) | Members of the launch request, for example {"justMyCode": false} |

The adapter comes from *presets.ini* by the file type:

|  |  |
|--|--|
| .py | python3 -m debugpy.adapter |
| .sh, .bash | bash-dap |
| .go | dlv dap --listen=127.0.0.1:0 |
| .js, .mjs, .ts | js-debug-adapter |
| a program | gdb -i dap |

When the adapter is missing, the debugger says how to install it. A Go
program is the package with *package main* in the directory of the file,
or one chosen from a list.

**Python**

Install debugpy with the package python3-debugpy or *pip install --user
debugpy*. A project with *.venv*, *venv*, *.env* or *env* is debugged with
its own Python, and debugpy has to be installed there too. A script that
reads input needs the terminal of the program:
*{"console": "integratedTerminal"}*.

**Shell**

bash-dap debugs bash scripts and needs only Python 3: *pipx install
bash-dap*. The script runs in the terminal of the program and stops in
functions inside *$(...)* and pipes too.

The debugger runs one local program at a time.
