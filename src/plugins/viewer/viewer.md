# Preview <!-- help:notitle -->

**Preview**

The viewer plugin shows the file the way it is meant to read, whatever the
file is, in a window named Preview at the right of it.
**Ctrl-Alt-p**,
or Preview in the Window menu, shows and hides it. The file keeps the
focus; the Preview is drawn again as the text changes, follows the cursor,
and shows the file window that comes to the front. The window is moved,
resized and closed like the window of a file. Once it has the focus, it has
a cursor of its own, which the keys of the editor move as in a file: the
arrows, Home and End, the pages, the top and the bottom of the text; the keys
of the words, Ctrl-Left and Ctrl-Right, move it eight columns. The view
follows it, sideways too for what is wider than it. A click
on the text puts the cursor there.
Text is marked as in a file: the mouse drawn over it, a word by a double
click, a line by a triple one, Shift and the arrows, F3 at the start and
again at the end, Ctrl-A all of it. Ctrl-Ins, or Enter, copies what is
marked to the clipboard, without the colors. The windows of the build and
of the query plugin are such windows too; in History, a list of queries,
Enter and F3 are its own, and Ctrl-Ins copies.
The mouse wheel scrolls it without the focus, sideways when it is tilted or
turned with Shift. Its frame has a scrollbar on the right and one at the
bottom: an arrow scrolls a step, the bar beside the thumb a page, and the
thumb is dragged. The key is set in
**viewer.ini**,
section
**[Preview]**,
entry
**key**.

The renderers tell the viewer what files they draw, by the ends of their names
or how their texts start, and the viewer has the file drawn by the renderer of
its type: a markdown file comes out with its headings, lists,
tables with rules, blocks of code in the colors of the editor, formulas as
symbols and mermaid diagrams as drawings; that renderer is the Lua script
render-markdown, lua-markdown of Midnight Commander, whose settings, from
Manage plugins, are kept in
**~/.config/bluerig/render-markdown.ini**.
A JSON file, and JSON Lines, comes out one value to a line in the colors the
editor gives JSON, the size of every object and array beside it, a long array
packed to the width or cut short, and an array of objects of one kind as a
table; comments and a comma before a closing bracket are taken. While it does
not read, halfway through a change, the error and its place are shown above
the last view that read. That renderer is the Lua script render-json.
An XML file, by the end of its name or by a text that starts with <?xml, comes
out an element to a line in the colors the editor gives XML: a short text on
the line of its element, a long one wrapped under it, the attributes one to a
line when they do not fit, the number of the elements an element holds beside
it, a long run of elements of one name cut short, and a run of elements of one
name that hold attributes and plain text as a table. That renderer is the Lua
script render-xml, and it shows its errors the way render-json does.
A string of base64 in JSON or XML, a text, a value or a data: URI, is cut to
its first letters, and what it holds is shown beside it in parentheses: the
start of the text, or the kind of file its bytes start like, and the size.
Only the start of it is decoded, so a string of megabytes costs no more than a
short one; a string of the letters of base64 that holds neither text nor a
known kind of file, a hash for one, is left as it is.
A file of a type no renderer knows is shown as its text.
