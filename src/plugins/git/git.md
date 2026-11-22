# Git <!-- help:notitle -->

**Git**

Shows the changes of the git work tree and the commits of the branch, stages
files, commits, amends and rewords, runs push, pull and fetch. The commit
message is written in an editor window.

|  |  |
|--|--|
| Alt-C | Git window on the Status tab |
| Alt-L | Git window on the Log tab |

The keys are changed in the *[git]* section of the key bindings file. The
GitCommit command, which commits what is staged from any window, has no
default key; it is in the Command menu.

The work tree is that of the file in front, else that of the project, else
that of the current directory. The plugin runs the system *git*, so hooks,
configuration and user identity apply.

**Git window**

```
 ┌─[Commit message]───────────────────┐┌─[Diff]────────────────┐
 │ the message of                     ││ the diff of the file  │
 │ the next commit                    ││ or of the commit      │
 └────────────────────────────────────┘│                       │
 ╔═[ 1 Status ] [ 2 Log ] [ 3 Graph ]═╗│                       │
 ║ changes not staged                 ║│                       │
 ╚════════════════════════════════════╝│                       │
 ┌─[Staged changes]───────────────────┐│                       │
 │ staged changes                     ││                       │
 └────────────────────────────────────┘└───────────────────────┘
```

The windows stand frame to frame in place of the file windows. The list
title shows the branch and how far it is ahead of and behind its upstream. A
closed window has its item in the Window menu to bring it back; Esc or F10
closes the whole Git window, and the commit message stays for the next time.

|  |  |
|--|--|
| 1, 2, 3 | Status, Log and Graph tabs |
| Tab | Next window: changes, staged, diff |
| Right, Left | To the diff and back |
| F5, Ctrl-R | Read everything again |
| F6 | git push |
| F7 | git pull |
| Shift-F5 | git fetch |
| Shift-F7, b | Branches |
| F1 | This help |
| Esc | Leave the diff or the commit files, else close the Git window |
| F10 | Close the Git window |

The status is read again when the window comes to the front. In the diff Up,
Down, PgUp, PgDn, Home, End and Space scroll it, Right and Left scroll it
sideways.

Push, pull and fetch run in the terminal window, so that git can ask for a
password there; after them everything is read again. Without the Terminal
plugin git runs on the terminal of the editor.

**Status tab**

Unstaged changes, including the files git does not track (*?*), and staged
changes. A file with a merge conflict is marked *U*.

|  |  |
|--|--|
| Enter, Space | Stage or unstage the file |
| a | Stage all or unstage all |
| F2, c | To the commit message; F2 there commits |
| F3, A | Amend the last commit |
| F4, e, r | Open the file in an editor window |
| F8, d, Delete | Discard the changes of the file |

F8 asks first: a tracked file goes back to the state of the index, an
untracked one is deleted.

**Log tab**

The commits of the branch, newest first, with branches and tags; at the
right the author, dates, message and files of the commit. Commits are read
2000 at a time as you scroll.

|  |  |
|--|--|
| Enter | Commit files; Enter on a file shows its diff |
| e | Open the work tree file |
| F4, r | Change the commit message (reword) |
| Left, Esc | Back to the commits |

**Graph tab**

Shows the commits of the branches as a graph: each branch in its own column,
at the left *main* or *master*, *develop*, *release*, *hotfix*, then the
others. A merge is a hollow node. Enter, e, F4 and r work as on Log; reword
works only for commits of the current branch.

|  |  |
|--|--|
| v | Choose which branches to read |

v offers *Current* (HEAD only), *Local* (local branches and tags),
*Local+remote* (the default, remote branches too) and *All* (every ref,
the stash and *refs/pr/...* among them). The choice is kept.

The graph lines are the frame lines of the skin. The *[git-graph]* section
of the skin sets its own lines (the keys of *[Lines]*), the characters
*commit*, *merge*, *arrow-left*, *arrow-right*, *more* and *rail*, and the
colors *main*, *develop*, *release*, *hotfix* and *others* (separated by
*;*):

```
[git-graph]
    commit = *
    merge = o
    rail = (
    others = magenta;cyan;brightmagenta;brightcyan
```

*rail* puts a commit beside the line of its branch, after this character.

**Branches**

Local and remote branches and, when checked, tags; the current one first,
then by the date of the last commit, newest first. Typed words filter the list by name,
author and subject of the last commit. *Extended* also searches the messages
of the commits unique to that branch.

|  |  |
|--|--|
| Enter | Switch to the branch (git switch) |
| F7 | New branch from the selected one |
| F8 | Delete the local branch |
| F3 | Compare with the current branch |
| Esc | Close |

A remote branch is switched to through the local branch of the same name,
which is created when there is none. A tag is switched to with a detached
HEAD. F8 asks first, and asks again for a branch that is not merged.

**Push**

F6 shows the branch, the remote (space chooses another one) and three
options:

|  |  |
|--|--|
| Force with lease | --force-with-lease; checked after an amend or reword of a pushed branch |
| Set upstream | -u; checked for a branch with no upstream |
| Include tags | --follow-tags |

A force push is never made without a lease. When the branch has diverged from
its upstream for another reason, Force with lease is not checked, and the
first line of the dialog says so.

**Commit message**

It is written in an editor window, in the file *BLUERIG_EDITMSG* in the git
directory (*BLUERIG_AMEND_EDITMSG* for an amend, *BLUERIG_REWORD_EDITMSG*
for a reword). Saving (F2) commits. When nothing is staged, the plugin first
offers to stage everything. Lines starting with *#* are left out; below them
is what is being committed.

The message of the next commit is kept when the Git window is closed and
when the editor ends. An amend or a reword takes its place temporarily.
When git refuses (a *commit-msg* hook, for example), the window stays, to be
saved again.

Reword re-creates the commit and the commits after it, as *git rebase*
does, without touching the index and the work tree and without running
hooks; the re-created commits lose their signatures. A commit with a merge
after it cannot be reworded.

**Message menu**

F11 in the message window (and in *COMMIT_EDITMSG*, when bluerig is the
editor of *git commit*) opens this menu instead of the user menu. Ctrl-U
undoes what an item did.

|  |  |
|--|--|
| s | Signed-off-by with the name and email from git config |
| c | Co-authored-by with an author from the history |
| f | Fixes with a chosen commit |
| r | Restore the current commit message (amend, reword) |
| d | Diff of the commit in a window at the right |
| l | Check the message: subject length, empty second line, lines over 72 |
| g | Generate the message with AI |
| p | Edit the prompt of the AI |
| m | Edit this menu |

**git-menu.ini**

The menu is read from *~/.config/bluerig/git-menu.ini*, else from
*git-menu.ini* in */etc/bluerig*. An item is a group named by its key:

```
[x]
label = Reviewed-by: a reviewer
command = echo 'Reviewed-by: A Reviewer <a@example.org>'
output = trailer
```

|  |  |
|--|--|
| label | Text in the menu |
| action | Plugin action: signoff, coauthor, fixes, restore, diff, check, generate, prompt, menu |
| command | sh command in the work tree; gets the message on standard input |
| output | trailer, replace, insert (the default), show or none |
| modes | commit, amend, reword separated by ;, all by default |

A repository cannot supply a menu: its commands would run code the
repository's author wrote.

**ai.ini**

The command that writes messages is set in the *[commit-message]* group of
*~/.config/bluerig/ai.ini*:

```
[commit-message]
command = claude -p --tools "" --no-session-persistence
input = diff
```

|  |  |
|--|--|
| command | sh command: reads the prompt and diff on standard input, prints the message |
| prompt | The prompt; by default the bluerig prompt |
| `prompt-<kind>` | The prompt for commit, amend or reword |
| input | diff (the default) or none |

The command gets *BLUERIG_GIT_MODE* (commit, amend, reword) and
*BLUERIG_GIT_COMMIT* (the commit of an amend or a reword) in its
environment. The values are read as GLib key files: a backslash starts an
escape sequence.

The *.bluerig/ai.ini* file of a project sets its own prompts and input, but
not the command. The access key of the service is kept in the environment or
in a file the command reads, never in *ai.ini*. The diff goes to the service
the command calls.

**From a file manager**

|  |  |
|--|--|
| bluerig -G dir | Git window; F10 ends the editor |
| bluerig -G dir --git-log | The same, on the Log tab |
| bluerig -G dir --git-show commit | The same, on the commit files |
| bluerig -G dir --git-commit | Commit message; the editor ends after the commit |
| bluerig -G dir --git-amend | The same for an amend |
| bluerig -G dir --git-reword commit | The same for a reword |

With *-G* the editor does not open the project windows and does not start
the Project, Build, Debugger, Ctags and Etags plugins.
