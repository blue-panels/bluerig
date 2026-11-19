# bluerig

bluerig is a terminal IDE: an editor that works with language servers, with
the tree of a project, its build and its debugger (gdb and debug adapters), a
Git window, and the queries of PostgreSQL and MongoDB. It runs full screen in a
terminal, from the Linux console to tmux and a remote session over ssh.

bluerig continues cooledit, the editor written by Paul Sheer, whose terminal
version lived on for many years as the internal editor of GNU Midnight
Commander.

Site: <https://blue-panels.github.io/bluerig/> - wiki:
<https://github.com/blue-panels/bluerig/wiki>

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

`brg` is a shorter name of the same program. `bluerig --help` lists all the
options.

## With mcommander

bluerig has no file manager of its own: it works next to
[mcommander](https://github.com/blue-panels/mcommander), the file manager that
continues GNU Midnight Commander and that bluerig's editor came from.

- Its panels open archives as directories, and connections to other machines,
  through panel plugins.
- Its Git, MongoDB and PostgreSQL panels run bluerig: the Git window, a commit
  message, a commit to reword, the editor of queries.
- With its Sessions plugin it outlives its terminal, as screen and tmux do:
  when the terminal is closed or the ssh connection drops, the panels, the
  shell, the editors and the copy jobs go on, and mcommander started again
  takes them over.
- F4 opens files in bluerig when "Use internal editor" is off and EDITOR is
  bluerig.

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
scripts. Each [release](https://github.com/blue-panels/bluerig/releases)
carries them for Debian 13, Ubuntu 26.04, Fedora and Arch, on amd64 and arm64
where the distribution has it, and a package for macOS on Apple Silicon.

- Ubuntu 24.04 and 26.04: `sudo add-apt-repository ppa:bluepanels/bluerig`
- Homebrew, macOS and Linux: `brew install blue-panels/tap/bluerig`
- Nix: `nix run github:blue-panels/bluerig`, or
  `nix profile install github:blue-panels/bluerig`

The recipes for these and for Gentoo, Alpine and Nix are in
[`packaging/`](packaging/README.md).

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
