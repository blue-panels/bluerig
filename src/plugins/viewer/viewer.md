# Preview <!-- help:notitle -->

**Preview**

Shows the file in a window to its right, the way it is meant to be read:
markdown with headings and tables, JSON and XML with indents and colors.
Draws the view again as you edit and follows the cursor.

|  |  |
|--|--|
| Ctrl-Alt-p | Show or hide the Preview window |

The key is set by the *key* entry of the *[Preview]* section of
*viewer.ini*. The same item is in the Plugins menu and in the Window menu.

The focus stays in the file, and the window shows the file that comes to the
front. The window is moved, resized and closed like a file window.

**Keys in the window**

When the window has the focus, it has a cursor of its own:

|  |  |
|--|--|
| Arrow keys, Home, End | Move the cursor |
| PgUp, PgDn | Page up, down |
| Ctrl-Home, Ctrl-End | To the start, to the end of the text |
| Ctrl-Left, Ctrl-Right | Eight columns left, right |
| Click | Put the cursor there |
| Mouse wheel | Scroll, even without focus |
| Shift-wheel | Scroll sideways |

**File types**

A renderer for the file type draws the file; the viewer picks the renderer by
the end of the file name or by the start of the text. A file no renderer knows is shown
as text.

|  |  |
|--|--|
| Markdown | Headings, lists, tables, code blocks, formulas and mermaid diagrams |
| JSON, JSON Lines | A value to a line, sizes of objects and arrays, objects of one kind as a table |
| XML | An element to a line, number of nested elements, elements of one kind as a table |

The renderers are the Lua scripts render-markdown, render-json and
render-xml. The Manage plugins dialog sets the render-markdown settings;
they are kept in *~/.config/bluerig/render-markdown.ini*.

While JSON or XML cannot be parsed, in the middle of an edit for example,
the error and its place are shown at the top, and the last good view under it.
A long base64 string is cut to its first letters, and what it holds is shown
beside it: the start of the text or the file type, and the size.
