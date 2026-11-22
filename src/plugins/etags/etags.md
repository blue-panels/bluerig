# Definitions <!-- help:notitle --><a id="definitions"></a>

**The list of definitions**

Shows the definitions of the name under the cursor found in the tags file:
the file, the line number and the text of the line of each.

|  |  |
|--|--|
| Enter | Go to the definition |
| Esc | Close the list |

The list appears only when there are several definitions; the editor goes to
a single one at once.

|  |  |
|--|--|
| Alt-Enter | Find the definitions of the name the cursor is at the end of |
| Alt-Minus | Back through the places of the jumps |
| Alt-Plus | Forward |

The definitions come from the *TAGS* file in Emacs format, which is
looked for in the current directory and above. etags or ctags builds it:

```
etags -R .
ctags -e -R .
```
