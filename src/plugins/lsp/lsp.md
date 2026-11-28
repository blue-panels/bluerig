# LSP <!-- help:notitle -->

**Language servers**

Connects language servers for completion, going to definitions and finding
references. Shows diagnostics, documentation and the parameters of a call.
The language servers are installed separately: clangd for C and C++, pylsp
or pyright for Python, rust-analyzer for Rust, gopls for Go and others.

|  |  |
|--|--|
| Ctrl-Space | Complete the word |
| Alt-. | Go to the definition of the name at the cursor |
| Ctrl-click | Go to the definition of the name under the mouse |
| Alt-> | Go to the prototype of a function or the declaration of a name |
| Alt-, | Documentation of the name at the cursor |
| Alt-( | Parameters of the call |

Keys can be reassigned in the *[lsp]* section of *~/.config/bluerig/keymap.ini*.
In a file that no server serves, these keys belong to the editor.
Ctrl-Enter and Alt-g of the Ctags plugin (go to definition and find
references) ask the language server first, if it can do this.

**When a server runs**

By default the server starts when a matching file is opened in the *-D* and
*-Q* modes. Start server in the LSP menu starts it by hand. The *autostart*
option sets when to start the server, for all servers or for one.

One running server serves all the files of a project. For a file inside the
project opened with *-D*, the root of that project is used. Otherwise the
plugin looks up the directories for *.bluerig*, then for the markers of
*root_markers* (for example *compile_commands.json* for clangd), then for
the root of the repository. When no root is found and *single_file* is on,
the directory of the file is used.

A server with no open files stops after a minute. A server that fails is
started again, but not more than three times a minute.

**Completion**

The items appear under the word. The letters you type are added to the word
and filter the list: *mkg* finds *make_global*. The
*completion_on_trigger* option turns on completion by itself as you type a
word and after the characters the server declared, such as a dot, *->* and
*::*.

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
the comment of the function. The box appears by itself after *(* and *,*,
when the server declared them. The *completion_on_trigger* and
*completion_on_arguments* options turn this on. When there are several signatures, Alt-Up and Alt-Down go
through them.

**Problems**

The lines with problems are marked at the left: *E* an error, *W* a warning,
*i* a note. Problems... in the LSP menu opens the list of the messages of
all servers. When the current file has messages, the first of them is
selected. Enter goes to the selected message.

**LSP menu**

|  |  |
|--|--|
| Start, Stop, Restart server | Start, stop, restart the server of the file |
| Status... | Servers, their projects and state |
| Log... | Error output and messages of the server |
| Settings... | Settings of the servers |
| Project setup... | Set the project up for clangd |

Project setup finds for clangd the headers to include in advance, so that
there are fewer false errors, and offers to leave the test directories out
of the index. It checks up to 400 headers; when there are more, it takes a
sample. Apply writes the result to the *.clangd* file at the root of the
project and starts the server again.

Settings writes to *~/.config/bluerig/lsp.json* only what differs from the
system settings. Configure in the Manage plugins dialog opens the same
dialog.

**lsp.json**

The servers are read from */etc/bluerig/lsp.json* and
*~/.config/bluerig/lsp.json*. A server of the user's file replaces the
server with the same id. The file is read when bluerig starts and after the
settings dialog writes it.

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
| completion_on_trigger | Completion by itself as you type a word and after the sequences of the server: ide (the default), always or never |
| completion_on_arguments | Completion list after ( and ,: never (the default), ide or always |
| command | Program and arguments; ${build_dir} is the build directory of the project |
| extensions | Ends of file names and their language |
| root_markers | Files that mark the root of the project |
| single_file | Serve files out of a project; true by default |
| enabled | false turns the server off |
| environment | Environment variables; null removes a variable |
| initialization_options | Options the server gets at initialization |
| settings | Settings the server asks for |
| startup | query: the server works only with -Q |
| requests | Own requests after the start: method and params |
| words | More words for the completion list |
| ascii_text | sql: replace non-ASCII characters with an underscore |

*ascii_text* is for a server that loses its place after non-ASCII
characters, such as *postgres-language-server* 0.28.

In *requests*
*${env:NAME}* is an environment variable, *${command_dir}* the directory of
the server program.

The *mongolsp* server completes MongoDB queries in *bluerig -Q*. It is the
server of the MongoDB extension for VS Code
(<https://github.com/blue-panels/mongolsp>) and must be in PATH; it gets the
connection string from *MC_MONGO_URI*.
