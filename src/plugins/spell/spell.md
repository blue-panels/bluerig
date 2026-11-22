# Spell <!-- help:notitle -->

**Spell**

Checks the spelling of the text with aspell or hunspell and offers
replacements. Needs the engine's library and a dictionary for the
language.

|  |  |
|--|--|
| Ctrl-p | Check the word under the cursor |

The Spell item of the Plugins menu checks the whole file: the check goes word
by word and stops at every word that is not in the dictionary.

**The check dialog**

Shows the word, the replacements, the language and the engine.

|  |  |
|--|--|
| Enter, Replace | Replace the word with the chosen replacement |
| Skip | Leave the word as it is |
| Add word | Add the word to the personal dictionary |
| Esc, Cancel | Stop the check |

**Settings**

The Manage plugins dialog of the Options menu sets the engine, Aspell or
Hunspell, and the language: en, ru and other names the engine knows.
Select... shows the installed dictionaries; for aspell the command
*aspell dump dicts* shows them too. The language NONE turns the check off.

The settings are kept in the *[EditorPluginSpell]* section of
*~/.config/bluerig/ini* (keys *engine* and *language*). The keys come from
*spell.keymap* in the system configuration directory of bluerig, and
*~/.config/bluerig/spell.keymap* overrides them.
