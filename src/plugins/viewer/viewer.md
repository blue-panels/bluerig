# Preview <!-- help:notitle -->

**Preview**

The plugin previews files, for example Markdown, JSON and XML. Other file
formats can be added with your own Lua script in
*~/.local/share/bluerig/lua/scripts/*, following render-markdown,
render-json and render-xml. How to write scripts is described in
[Lua scripting](bluerig.md#lua-scripting).

The preview window is updated as you edit and scrolls with the cursor of the
editor.

|  |  |
|--|--|
| Ctrl-Alt-P | Show or hide the Preview window |

The key is set by the *key* entry of the *[Preview]* section of
*~/.config/bluerig/viewer.ini*.
The same item is in the Plugins menu and in the Window menu. In Git mode
(*bluerig -G*) the key and the Preview item of the Window menu are not available.

When the preview opens, the focus stays in the editor. When you go to another
editor window, the preview shows the file of that window.
The preview can be moved, resized and closed like a file window.

**Keys in the window**

When the window has the focus, it has a cursor of its own:

|  |  |
|--|--|
| Cursor keys, Home, End | Move the cursor |
| PgUp, PgDn | Page up, down |
| Ctrl-Home, Ctrl-End | To the start, to the end of the text |
| Ctrl-Left, Ctrl-Right | Eight columns left, right |
| Click | Put the cursor there |
| Mouse drag, Shift with movement keys | Mark text |
| F3 | Start or end the mark |
| Ctrl-A | Mark the whole text |
| Ctrl-Insert, Enter | Copy the marked text to the clipboard file |
| Mouse wheel | Scroll, even without focus |
| Shift-wheel | Scroll sideways |

**File types**

The preview first finds the format by the end of the file name.
If that does not work, it checks the start of the text. For example, JSON
is found by the extension, and XML also by the start `<?xml`. If the format
is not found, the preview shows the source text.

|  |  |
|--|--|
| Markdown | Headings, lists, tables, code blocks, formulas and Mermaid diagrams |
| JSON, JSON Lines | A value to a line, sizes of objects and arrays, objects of one kind as a table |
| XML | An element to a line, number of nested elements, elements of one kind as a table |

Of Mermaid diagrams, flowcharts (`flowchart` and `graph`), sequence diagrams
(`sequenceDiagram`), class diagrams (`classDiagram`) and pie charts (`pie`)
are supported.

The Manage plugins dialog sets the render-markdown settings;
they are kept in
*~/.config/bluerig/render-markdown.ini*.

JSON and XML over 16 MiB are shown as plain text.

On a parse error the preview shows its message, line and column.
The last good view stays under it. If there is none yet, the source text
is shown.

A long base64 string is cut to its first letters, and beside it you see what
it holds: the start of the text or the file type, and the size.
