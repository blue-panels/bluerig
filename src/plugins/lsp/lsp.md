# LSP <!-- help:notitle -->

**Language servers**

Runs language servers and asks them about the code: shows errors, completes
words, goes to definitions, shows documentation and the parameters of a
call. The servers are separate programs and must be installed: clangd for C
and C++, pylsp or pyright for Python, rust-analyzer, gopls and others.

|  |  |
|--|--|
| Ctrl-Space | Complete the word |
| Alt-. | Go to the definition of the name at the cursor |
| Ctrl-click | Go to the definition of the name under the mouse |
| Alt-> | Go to the prototype of a function or the declaration of a name |
| Alt-, | Documentation of the name at the cursor |
| Alt-( | Parameters of the call |

The keys are changed in the *[lsp]* section of the key bindings file. In a
file that no server serves, these keys belong to the editor. The ctags
commands of the Navigate menu (Ctrl-Enter, Alt-g and others) ask the server
first.

**When a server runs**

The server starts by itself when a file is opened, if bluerig was started on
a project with *-D*; else Start server in the LSP menu starts it. One server
serves all the files of a project: the directory with *.bluerig*, else the
nearest directory with a file the server names (for example
*compile_commands.json* for clangd), else the root of the repository. A
server with no open files stops after a minute. A server that fails is
started again, but not more than three times a minute.

**Completion**

The list appears under the word, and the letters you type continue the word
and narrow the list: *mkg* finds *make_global*. On a *-D* project the list
opens by itself as you type a word and after a dot, *->* and *::*.

|  |  |
|--|--|
| Up, Down, PgUp, PgDn | Choose an item |
| Enter, Tab | Insert the item |
| ., ->, :: | Insert the item and show its members |
| Esc | Close the list |

A function is inserted with its parentheses, the cursor goes inside them,
and the parameters of the call appear over the line. The server can add an
include together with the item; one Undo takes all of it back.

The letter at the left tells what the item is:

|  |  |
|--|--|
| f, m | Field, method |
| F, v | Function, variable |
| C, S | Class, struct |
| T | Type parameter |
| # | Constant |
| k | Keyword |

**Documentation and parameters**

Alt-, shows the type and the documentation of the name in a box under the
cursor, after the problems of the line. Alt-, again lets the arrows scroll
it; Esc or moving the cursor closes the box.

Alt-( shows the signature of the call in a box over the line, the current
parameter highlighted, and under it the description of the parameter from
the comment of the function. On a *-D* project the box appears by itself after
*(* and *,*. When there are several signatures, Alt-Up and Alt-Down go
through them.

**Problems**

The lines with problems are marked at the left: *E* an error, *W* a warning,
*i* a note. Problems... in the LSP menu lists them, Enter goes to the one
chosen.

**LSP menu**

|  |  |
|--|--|
| Start, Stop, Restart server | Start, stop, restart the server of the file |
| Status... | Servers, their projects and state |
| Log... | Error output and messages of the server |
| Settings... | Settings of the servers |
| Project setup... | Set the project up for clangd |

Project setup checks every header alone, as clangd reads it, finds the
headers without which it has errors, and offers to leave the test
directories out of the index. Apply writes this in a block of *.clangd* at
the root of the project and starts the server again.

Settings writes to *~/.config/bluerig/lsp.json* only what differs from the
system settings. Configure in the Manage plugins dialog opens the same
dialog.

**lsp.json**

The servers are read from */etc/bluerig/lsp.json* and
*~/.config/bluerig/lsp.json*. A server of the user's file replaces the
server with the same id.

```
{
  "autostart": "always",
  "backends": [
    {
      "id": "clangd",
      "command": ["clangd", "--background-index",
                  "--compile-commands-dir=${build_dir}"],
      "extensions": {".c": "c", ".h": "c", ".cpp": "cpp"},
      "root_markers": ["compile_commands.json"]
    }
  ]
}
```

|  |  |
|--|--|
| autostart | ide (the default, only with -D and -Q), always or never |
| completion_on_trigger | List after a dot, -> and :: by itself: ide, always or never |
| completion_on_arguments | List after ( and , by itself: never (the default), ide, always |
| command | Program and arguments; ${build_dir} is the build directory of the project |
| extensions | Ends of file names and their language |
| root_markers | Files that mark the root of the project |
| single_file | Serve files out of a project; true by default |
| enabled | false turns the server off |
| environment | Environment variables; null removes a variable |
| initialization_options | What the server gets when it starts |
| settings | Settings the server asks for |
| startup | query: the server works only with -Q |
| requests | Own requests after the start: method and params |
| words | More words for the completion list |
| ascii_text | sql: replace non-ASCII characters with an underscore |

*ascii_text* is for a server that loses its place after non-ASCII
characters, such as *postgres-language-server* 0.28. In *requests*
*${env:NAME}* is an environment variable, *${command_dir}* the directory of
the server program.

The *mongolsp* server completes MongoDB queries in *bluerig -Q*. It is the
server of the MongoDB extension for VS Code
(<https://github.com/blue-panels/mongolsp>) and must be in PATH; it gets the
connection string from *MC_MONGO_URI*.

The file is read when bluerig starts and after the settings dialog writes
it.
