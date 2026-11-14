# Changelog

The releases of bluerig, newest first.

## 1.0.0

- bluerig is a terminal IDE, a continuation of cooledit by Paul Sheer: the
  editor that lived on in GNU Midnight Commander, developed as a program of
  its own. This is its first release.
- The editor: syntax highlighting with its rules in text files, code folding,
  the undo history browser, keyboard macros and their explorer, column blocks,
  search with a line filter, several windows, skins, key bindings changed in a
  dialog, and a preview of markdown, JSON and XML.
- Language servers: completion, documentation, definitions and diagnostics
  from clangd, pylsp, pyright, rust-analyzer, gopls, the TypeScript, bash and
  Lua servers, and those of MongoDB and PostgreSQL.
- bluerig -D opens the project of a directory: its tree with the lines of its
  files and the outline of the file in front, its files a few letters away, a
  find in all of them, and its build with meson, cmake or make, the errors
  marked in the files.
- The debugger: gdb, and debug adapters for Python, bash, Go and JavaScript;
  breakpoints, steps, the stack and the variables in a panel, the program in a
  terminal tab.
- bluerig -G opens a Git window on a work tree: the changes and the staged
  ones, a commit with a menu of trailers and a message written by AI, the log
  with its diffs, and the branches.
- bluerig -Q runs queries in PostgreSQL, by libpq, and in MongoDB, by mongosh:
  their rows as a table, the history of each query, the schema of the
  database, an export to CSV, JSON, SQL and Markdown, a query written by AI,
  and a menu of the queries.
- A terminal at the bottom of the screen, ctags navigation, spell checking
  with aspell or hunspell, and Lua scripts in the package bluerig-lua.
- Configuration, data and cache live in ~/.config/bluerig,
  ~/.local/share/bluerig and ~/.cache/bluerig.
