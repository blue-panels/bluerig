# Query <!-- help:notitle -->

**Query**

Runs the queries of a file in MongoDB or PostgreSQL and shows the answer at
the bottom of the screen: the messages of the database and a table of the
documents or rows. Shows the database schema, writes queries with AI and
writes rows to files. Works only in *bluerig -Q*.

|  |  |
|--|--|
| F5 | Run the selection, else the query at the cursor, else the file |
| Ctrl-F5 | Run the whole file |
| F5, Esc | While a query runs: stop it |
| F6 | Write the query with AI |
| Ctrl-F6, Alt-X | Write all the rows of the query to a file |
| Ctrl-Alt-S | Show or hide the Schema window |
| Ctrl-Alt-PgDn, Ctrl-Alt-PgUp | Next, previous answer tab |
| F11 | Query menu |

Change the keys in the *[query]* section of the keymap. The Query menu has
the commands too. QueryRollback (*ROLLBACK*) has no default key.

*MC_QUERY_KIND* sets the database: *mongo* or *postgres*. Without it,
*MC_MONGO_URI* means MongoDB, and a *.sql* file means PostgreSQL. Without a
file named, the queries are kept in *~/.local/share/bluerig/queries/*, in a
file named after the database; for PostgreSQL in *postgres/*, in a file
named after the connection and the database.

**The query file**

In MongoDB a line *// ---* starts a new query, the words after the dashes
name it, and F5 runs the part the cursor is in as one script. In
PostgreSQL F5 runs the selection, else the whole file, in one transaction
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
| // @table | Show the value the query ends with in the Table tab too |
| // @limit 20 | Limit the documents of the query; without a number, to 100 |

*@limit* adds *.limit()* to *find()* or a *$limit* stage to *aggregate()*;
the file stays as it is.

**Answer tabs**

|  |  |
|--|--|
| Output | What the database printed: query lines, time, server, errors |
| Table | Documents or rows in columns: the first 100, with the total count |
| History | Versions of the query file, one per run |

An error marks its line in the file, and the cursor goes there. In
PostgreSQL the first line of Output shows the transaction state too:
*[TX]* while it is open, and *[TX FAILED: ROLLBACK required]* when one of
its commands failed.

Each run keeps the file as a version in
*~/.local/share/bluerig/queries/.history/*; the newest 200 stay. In
History:

|  |  |
|--|--|
| Enter, double click | Bring the file back to this version (Undo takes it back) |
| F3 | Compare the version with the file (diff -u in Output) |
| F5 | Run this version's query without changing the file |
| Ctrl-Ins | Copy the marked text |

Before going to a version, the current text is kept as the line *current*;
Enter on it brings it back.

**Query menu**

F11 in a *.sql* or *.js* file opens this menu instead of the user menu:
EXPLAIN of the query, the row count of the query, the lines *// @table*
and *// @limit*, the selection in upper or lower case, editing the
menu. The menu comes from *query-menu.ini* of bluerig, and
*~/.config/bluerig/query/query-menu.ini* takes its place. An item can run
its own command: the query on its stdin, *BLUERIG_QUERY_KIND* in its
environment. EXPLAIN ANALYZE runs only SELECT, VALUES and TABLE.

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

The DDL is the definition, not a backup: it has no owner, grants, users or
data. The schema is read again by itself after a query that creates or
drops collections, tables or indexes.

The *[query]* section of the skin gives the characters and colors of the
nodes: keys such as *table-char* and *table*, *item* and *info*. By
default there are no characters and no colors.

**Export**

Ctrl-F6 writes to a file all the rows of the query F5 would run, without
*// @limit*, or all the rows of a Schema node. Each page of the dialog
is a format:

|  |  |
|--|--|
| CSV | Delimiter, header, quotes, how NULL looks |
| JSON, NDJSON | An array of objects, or an object a line |
| SQL | INSERT INTO and, if wanted, CREATE TABLE (PostgreSQL only) |
| Markdown | A table, each cell on one line |

The rows go to a temporary *.part* file, which replaces the target file once
all rows have arrived; a stopped or failed query leaves no file. For CSV and Markdown the MongoDB columns are the keys of the first
200 documents.

**A query written by AI**

F6 takes the request, the paragraph at the cursor or the selection, for
example *orders of the last week by user*, puts the AI's query under it
and makes the request a comment. The query is not run. F6 on such a
request writes the query again. When the paragraph already holds a query
of your own, AI writes the new query after the paragraph and leaves yours
as it is. F6 while AI works asks whether to stop it.

AI gets the names of the collections, fields, tables and columns with
their types, but no data.

|  |  |
|--|--|
| MC_QUERY_AI | The AI program: claude, codex or a command line |
| ai in [Query] | The same in *~/.config/bluerig/ini* |

claude and codex run without their own tools. Another command reads the
request on stdin and prints the query; it runs without *PGPASSWORD* and
*MC_MONGO_URI* in its environment.

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
file has none, *-Q* puts one at the top of the file. The query itself is
visible on the mongosh command line, so take secrets from the environment:
*process.env.NAME*.

**PostgreSQL**

The library *libpq5* runs the queries, *postgres-language-server* gives
completion. bluerig keeps one connection, so a transaction, temporary
tables and *SET* persist from one F5 to the next.

libpq reads the connection settings from the environment, as *psql* does:
*PGHOST*, *PGPORT*, *PGUSER*, *PGDATABASE*, *PGSERVICE*. Keep the password
in *~/.pgpass* rather than in *PGPASSWORD*.

|  |  |
|--|--|
| MC_QUERY_READ_ONLY=1 | Read-only transactions, *[RO]* in the header |
| MC_PG_SCHEMA | Schema that comes first in Schema and for the language server |
| MC_PG_RELATION | Table whose first 100 rows run at the start |

*[RO]* guards against a slip, but does not replace permissions: *SET* turns it
off. Run *CREATE DATABASE*, *VACUUM* and the like as a separate
selection. The commands of psql (`\d`, `\copy`) do not work.
