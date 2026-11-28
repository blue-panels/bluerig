# Spell <!-- help:notitle -->

**Spell**

The plugin checks spelling with Aspell or Hunspell. It needs the engine's
library and a dictionary for the language.

|  |  |
|--|--|
| Ctrl-P | Check the word under the cursor |

The Spell item of the Plugins menu checks the file word by word and stops at
every word that is not in the dictionary. The check starts at the beginning of
the file; only when the cursor is on the first line does it start at the
cursor. In *-D* mode there is no Plugins menu, and the SpellCheck command has
no default key: assign one in *spell.keymap*, for example
*SpellCheck = ctrl-alt-o*.

**The check dialog**

The dialog shows the word, the replacements, the language and the engine.
The Replace button is there when there is at least one replacement.

|  |  |
|--|--|
| Enter, Replace | Replace the word with the chosen replacement |
| Skip | Leave the word as it is |
| Add word | Open a field for the word to add |
| Esc, Cancel | Stop the check |

Aspell keeps an added word in the personal dictionary. Hunspell keeps it in
memory until the engine restarts.

**Settings**

The Manage plugins dialog of the Options menu sets the engine, Aspell or
Hunspell, and the language: en, ru and other names the engine knows. The
Select... button shows the installed dictionaries; for Aspell the command
*aspell dump dicts* shows them too. The language NONE turns the check off.

The settings are kept in the *[EditorPluginSpell]* section of
*~/.config/bluerig/ini* (keys *engine* and *language*).

The plugin reads *spell.keymap* first from the bluerig data directory, then
from the system configuration directory and from
*~/.config/bluerig/spell.keymap*. The user file adds bindings. If one key is
listed several times, the first binding found wins.
