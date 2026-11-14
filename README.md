# bluerig

bluerig is a terminal IDE: an editor that works with language servers, with
the tree of a project, its build and its debugger (gdb and debug adapters), a
Git window, and the queries of PostgreSQL and MongoDB. It runs full screen in a
terminal, from the Linux console to tmux and a remote session over ssh.

bluerig continues cooledit, the editor written by Paul Sheer, whose terminal
version lived on for many years as the internal editor of GNU Midnight
Commander.

![bluerig: a C file with the tree of its project, the README with its Preview, and the shell at the bottom](https://raw.githubusercontent.com/wiki/blue-panels/bluerig/assets/overview.gif)

## What it has

- Editor: syntax highlighting, folding, undo history, macros, column blocks,
  search with a line filter, several windows, skins, key bindings to change.
- Language servers: completion, documentation, definitions, diagnostics
  ([lsp.md](src/plugins/lsp/lsp.md)).
- Project: its files, its tree, find in its files
  ([project.md](src/plugins/project/project.md)).
- Build with meson, cmake or make, the errors marked in the file
  ([build.md](src/plugins/build/build.md)).
- Debugger: gdb, and debug adapters for Python, bash, Go and JavaScript
  ([debugger.md](src/plugins/debugger/debugger.md)).
- Git: status, commit, log, branches
  ([git.md](src/plugins/git/git.md)).
- Queries of PostgreSQL and MongoDB, their rows as a table
  ([query.md](src/plugins/query/query.md)).
- Terminal, preview of markdown, JSON and XML, ctags, spell checking, Lua
  scripts ([terminal.md](src/plugins/terminal/terminal.md),
  [ctags.md](src/plugins/ctags/ctags.md), [spell.md](src/plugins/spell/spell.md),
  [LUA_PLUGINS.md](doc/LUA_PLUGINS.md)).

The same help is built into the program.

## Usage

```sh
bluerig file.c:42          # files, at a line
bluerig -D ~/src/project   # the project of a directory: tree, build, debugger
bluerig -Q queries.sql     # queries of a database
bluerig -G ~/src/project   # the Git window of a work tree
```

`bluerig --help` lists all the options.

## Building

bluerig is built with [meson](https://mesonbuild.com) 0.61 or newer and
ninja. See [`INSTALL`](INSTALL) for all the options.

Required: a C compiler, meson, ninja, pkg-config, gettext, GLib 2.58 or newer,
and S-Lang or ncurses. Optional: json-glib (language servers, debug adapters,
queries), Lua 5.3 or newer (scripts), gpm, the X11 headers, and the check
library for the tests. The queries need libpq for PostgreSQL, loaded when they
run, and mongosh for MongoDB.

Debian and Ubuntu:

```sh
sudo apt install build-essential meson ninja-build pkg-config gettext \
    libglib2.0-dev libjson-glib-dev libslang2-dev libgpm-dev libx11-dev \
    liblua5.4-dev check
```

Fedora:

```sh
sudo dnf install gcc meson pkgconf-pkg-config gettext glib2-devel \
    json-glib-devel slang-devel gpm-devel libX11-devel lua-devel check-devel
```

Arch Linux:

```sh
sudo pacman -S base-devel meson pkgconf gettext glib2 json-glib slang gpm \
    libx11 lua check
```

Then:

```sh
meson setup build --prefix=/usr --sysconfdir=/etc
meson compile -C build
meson test -C build
sudo meson install -C build
```

`sudo ninja -C build uninstall` removes what the last install put in place.

## Packages

The packages are **`bluerig`** and **`bluerig-lua`**, the Lua runtime and the
scripts. There are no releases yet; the recipes for Debian and Ubuntu, Fedora,
Arch and Gentoo are in [`packaging/`](packaging/README.md).

## Translations

Translations are merged into `po/` by a maintainer, so please do not open pull
requests that edit `po/*.po` by hand.

## Reporting problems

Open an issue: <https://github.com/blue-panels/bluerig/issues>, with
`bluerig --version`, your OS and distribution, and for a crash a `gdb`
backtrace.

## License

GNU General Public License, version 3 or any later version. See
[`COPYING`](COPYING). The authors of cooledit and of the code bluerig took
over are listed in [`AUTHORS`](AUTHORS).
