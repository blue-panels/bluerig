# Query <!-- help:notitle -->

**Query**

A plugin that runs queries in MongoDB and PostgreSQL. It works only in the
query mode (*bluerig -Q*).

|  |  |
|--|--|
| F5 | Run the marked text. When nothing is marked, in MongoDB the section at the cursor runs (from a *// ---* line to the next one), if there is one. Otherwise the whole file runs |
| Ctrl-F5 | Run the whole file |
| F5, Esc | While a query runs: offer to stop it |
| F6 | Write the query with AI |
| Ctrl-F6, Alt-X | Export the query result to a file |
| Ctrl-Alt-S | Show or hide the Schema window |
| Ctrl-Alt-PgDn, Ctrl-Alt-PgUp | Next, previous answer tab |
| F11 | Query menu |

Keys can be reassigned in the *[query]* section of *~/.config/bluerig/keymap.ini*.
The commands are in the Query menu too. The **Roll back** item of the Query
menu runs *ROLLBACK*; a key for it can be set there too.

The database is chosen by the *MC_QUERY_KIND* variable, *mongo* or
*postgres*. When it is not set, *MC_MONGO_URI* chooses MongoDB. When that
variable is not set either, a *.sql* file chooses PostgreSQL. When no file
is named, the queries are kept in *~/.local/share/bluerig/queries/*, in a
file named after the database; for PostgreSQL in *postgres/*, in a file
named after the connection and the database.

**The query file**

In MongoDB a line *// ---* starts a new query, and the words after the
dashes are its name. In PostgreSQL the queries run in one transaction,
unless the file has its own *BEGIN*.

```
// --- the last orders
// @table
db.orders.find({ status: 1 }).sort({ ts: -1 })

// --- how many
db.orders.countDocuments({ status: 1 })
```

|  |  |
|--|--|
| // @table | Show the result of the last expression in the Table tab |
| // @limit 20 | Return at most 20 documents; without a number, at most 100 |

*@limit* adds *.limit()* to *find()* or a *$limit* stage to *aggregate()*;
the file stays as it is.

**Answer tabs**

|  |  |
|--|--|
| Output | What the database printed: query lines, time, server, errors |
| Table | Documents or rows in columns: the first 100, all of them counted |
| History | Versions of the query file |

An error marks its line in the file, and the cursor goes there. In
PostgreSQL the first line of Output shows the transaction state too:
*[TX]* while it is open, and *[TX FAILED: ROLLBACK required]* when one of
its commands failed.

On a run the plugin saves a new version of the file if the text has changed
since the last saved version. *~/.local/share/bluerig/queries/.history/*
keeps up to 200 versions per file. In History:

|  |  |
|--|--|
| Enter, double click | Bring the file back to this version (Undo takes it back) |
| F3 | Compare the version with the file (diff -u in Output) |
| F5 | Run this version's query without changing the file |
| Ctrl-Ins | Copy the marked text |

Before going to a version, the current text is kept in History as the line
*current*;
Enter on it brings it back.

**Query menu**

F11 opens the query menu in an unnamed window and in a file with the
extension of the current database: *.js* for MongoDB, *.sql* for
PostgreSQL. The menu has EXPLAIN of the query, the row count of the query,
the lines *// @table* and *// @limit*, the marked text in upper or lower
case, and editing the menu.

The plugin loads the menu from *~/.config/bluerig/query/query-menu.ini*.
When it is not there, it looks for *query-menu.ini* first in the system
configuration directory (usually */etc/bluerig/query*), then in the bluerig
data directory. An item can run its own command: the command gets the query
on standard input and the database kind in *BLUERIG_QUERY_KIND*. In
PostgreSQL the EXPLAIN item adds *EXPLAIN (ANALYZE, BUFFERS)* when the first
word of the query is *SELECT*, *VALUES* or *TABLE*. Otherwise it adds
*EXPLAIN*.

**Schema**

Ctrl-Alt-S shows the database tree at the right. For MongoDB: the
databases, the collections with their document counts, the views, the time
series, and in an open collection the fields from a sample of 100
documents, the indexes and the validator. For PostgreSQL: the schemas,
tables, views, sequences and functions, and in an open relation the
columns, keys, foreign keys, indexes, references and triggers.

|  |  |
|--|--|
| Enter, Right, Left | Open or close the node |
| F5 | First 100 rows in the Table tab |
| F3 | Put the query of the rows in the query file |
| F4 | Put the DDL in the query file: DROP (commented) and CREATE |
| Insert | Put the name of the node at the cursor |
| Ctrl-F6, Alt-X | Write all the rows to a file |
| F2 | Connect again and read the schema again |
| F10, Esc | Close the window |

F4 puts the DDL of the object: a commented *DROP* and *CREATE*, without
owner, grants and data. The schema is read again by itself after a query that creates or
drops collections, tables or indexes.

The *[query]* section of the skin gives the characters and colors of the
nodes: keys such as *table-char* and *table*, *item* and *info*. By
default there are no characters and no colors.

**Export**

Ctrl-F6 exports the result of the query F5 would run, or all the rows of a
Schema node. An export from MongoDB ignores the *// @limit* directive. In
PostgreSQL only the first table result is exported, even when it has no
rows. The other results can be seen in the Table tab. The pages of the
dialog are the formats:

|  |  |
|--|--|
| CSV | Delimiter, header, quotes, how NULL looks |
| JSON, NDJSON | An array of objects, or an object a line |
| SQL | INSERT INTO and, if wanted, CREATE TABLE (PostgreSQL only) |
| Markdown | A table, each cell on one line |

The plugin writes the rows to a temporary *.part* file and renames it to the
chosen file when the export ends well. On cancel or error the temporary file
is deleted. For CSV and Markdown the MongoDB columns are the keys of the
first 200 documents.

**A query written by AI**

F6 takes the prompt from the paragraph at the cursor or from the marked
text, for example *orders of the last week by user*. The query written by
AI is put under it, and the prompt becomes a comment. The query is not run.
F6 on such a prompt writes the query again. When the paragraph has a query
of the user, AI writes the new query after the paragraph and does not touch
it. F6 while AI works asks whether to stop it.

AI gets the names of the collections, fields, tables and columns with
their types, but no data.

|  |  |
|--|--|
| MC_QUERY_AI | The AI program: claude, codex or a command line |
| ai in [Query] | Command from *~/.config/bluerig/ini*, when *MC_QUERY_AI* is not set or empty |

claude and codex run without their own tools. Another command reads the
prompt on stdin and prints the query. *PGPASSWORD*, *MC_MONGO_URI* and
*BLUERIG_PG_URI* are removed from the environment of the command.

The scripts the plugin uses to read the schema and write the DDL are in
*/usr/share/bluerig/query*; a file of the same name in
*~/.config/bluerig/query* or */etc/bluerig/query* runs in its place.

**MongoDB**

*mongosh* runs the queries, the language server *mongolsp*
(<https://github.com/blue-panels/mongolsp/releases>) gives completion. The
first lines of Output say whether they are found.

|  |  |
|--|--|
| MC_MONGO_URI | Connection string, *mongodb://user:password@host:port/* |
| MC_MONGO_DB | Database; by default the one of the connection string |
| MC_MONGOSH | The mongosh program; by default the one in PATH |

A line *use("name")* runs the queries after it in that database. When the
file has none, the plugin adds one at the top of the file. The query itself is
visible on the mongosh command line, so take secrets from the environment:
*process.env.NAME*.

**PostgreSQL**

The library *libpq* runs the queries, *postgres-language-server* gives
completion. bluerig keeps one connection, so a transaction, temporary
tables and *SET* persist from one F5 to the next.

libpq reads the connection settings from the environment, as *psql* does:
*PGHOST*, *PGPORT*, *PGUSER*, *PGDATABASE*, *PGSERVICE*. Keep the password
in *~/.pgpass* rather than in *PGPASSWORD*.

|  |  |
|--|--|
| MC_QUERY_READ_ONLY=1 | Read-only transactions, *[RO]* in the header |
| MC_PG_SCHEMA | Schema that comes first in Schema and for the language server |
| MC_PG_RELATION | Table whose first 100 rows are shown at the start |

*[RO]* guards against a slip, but does not replace permissions: *SET* turns it
off. *CREATE DATABASE*, *VACUUM* and the like run separately: mark only
them and press F5.
The commands of psql (`\d`, `\copy`) do not work.
