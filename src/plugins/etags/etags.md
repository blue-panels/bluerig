# Definitions <!-- help:notitle --><a id="definitions"></a>

**The list of definitions**

Shows the definitions of a name found in the tags file. Each one shows the
tag name or a part of the *TAGS* entry, the file name and the line number.

|  |  |
|--|--|
| Enter | Go to the definition |
| Esc | Close the list |

The list opens even when only one definition is found. Press Enter to go
to it.

|  |  |
|--|--|
| Alt-Enter | Find the definitions of the name the cursor is at the end of |
| Alt-Minus | Back through the places of the jumps |
| Alt-Plus | Forward |

The plugin looks for a *TAGS* file in Emacs format, starting in the current
working directory and going up through the parent directories. It uses the
first file found, even if it has no matches. etags or ctags builds the file:

```
etags -R .
ctags -e -R .
```
