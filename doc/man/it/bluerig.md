---
date: novembre 2026
---

<!-- help:topics "Indice degli argomenti:" -->
# NOME <!-- help:skip -->

bluerig - un IDE per il terminale.

# USO <!-- help:skip -->

**bluerig**
[opzioni] [+riga] [file1[:riga]] [file2[:riga]] ...

# DESCRIZIONE <a id="description"></a>

**bluerig**
è un IDE per il terminale. Continua
**cooledit**,
l'editor di Paul Sheer, la cui versione per terminale è stata sviluppata per
molti anni come editor interno di GNU Midnight Commander; bluerig porta avanti
quell'editor come programma a sé.
**brg**
è un nome più corto dello stesso programma.

Modifica più file insieme, ciascuno nella sua finestra, e può modificare
file binari. Offre copia, spostamento, cancellazione, taglio e incolla di
blocchi; annullamento e ripristino tasto per tasto; menu a discesa;
inserimento di file; macro; ricerca e sostituzione con espressioni regolari;
un filtro delle righe; il ripiegamento; i segnalibri; la selezione del testo
con Shift e le frecce (dove il terminale lo permette); i blocchi a colonne;
le modalità di inserimento e sovrascrittura; il rientro automatico; l'a capo
automatico e la formattazione dei paragrafi; l'ampiezza regolabile delle
tabulazioni;
l'evidenziazione della sintassi per molti tipi di file; un menu utente che
passa il testo a comandi di shell; plugin dell'editor (navigazione per tag,
controllo ortografico) e script Lua.

Ogni file indicato sulla riga di comando viene aperto nella sua finestra.
Senza file, l'editor apre una finestra vuota. Un file più grande di
*editor_filesize_threshold*
(64 MB in modo predefinito) viene aperto dopo una domanda, non rifiutato.

# OPZIONI <a id="options"></a>

*+riga*
: Va alla riga con quel numero (senza spazio tra il segno
*+*
e il numero). Sono ammessi più numeri, ma viene usato solo l'ultimo, e solo
per il primo file.

*file:riga[:]*
: Apre il file a quella riga, come compilatori e grep indicano un punto di un
file. Se esiste un file con il nome intero, due punti e numero compresi,
viene aperto quel file.

*-V, --version*
: Mostra la versione del programma.

*-f, --datadir*
: Mostra il percorso di ricerca compilato per i file di dati di bluerig.

*-F, --datadir-info*
: Mostra informazioni estese sulle directory che bluerig usa: i file di dati,
i file dell'utente e i plugin.

*--configure-options*
: Mostra le opzioni con cui il programma è stato configurato.

*--no-lua*
: Avvia senza l'ambiente Lua: non viene caricato nessuno script Lua.

*-D, --debug=directory*
: Apre il progetto di quella directory per farne il debug: l'ultimo file del
progetto, l'albero del progetto e il pannello del debugger a destra. Le
finestre dei file prendono i tasti del debugger: F6 mette un punto di
interruzione, F5 compila il programma e lo avvia, creando la sua
configurazione di debug la prima volta, F7 e F8 avanzano passo passo, F3
indicizza i simboli del progetto; F2, F9 e F10 restano salvataggio, menu e
uscita, e Alt-F5 va al pannello e torna indietro. Servono i plugin project,
build e debugger, e ctags per F3. I menu Drawing, Format e Plugins non sono
nella barra dei menu; i loro tasti restano, e Opzioni conserva Manage
plugins.

*-Q, --query*
: Esegue le query del file in un database, MongoDB tramite mongosh o
PostgreSQL tramite libpq; la risposta arriva nelle schede Output, Table e
Cronologia in basso. *MC_QUERY_KIND* vale *mongo* o *postgres*; senza di
essa, *MC_MONGO_URI* indica MongoDB e un file *.sql* PostgreSQL. Per MongoDB
*MC_MONGO_URI* e *MC_MONGO_DB* dicono a cosa connettersi, *MC_MONGOSH* il
programma; F5 esegue la selezione, altrimenti la parte tra le righe *// ---*
intorno al cursore, altrimenti tutto il file. Per PostgreSQL libpq prende la
connessione da *PGHOST*, *PGDATABASE*, *PGSERVICE* e simili, la password da
*~/.pgpass*; F5 esegue la selezione, altrimenti tutto il file. Ctrl-F5
esegue tutto il file. Senza un file indicato, il file è
*~/.local/share/bluerig/queries/*, con il nome del database e *.js*, oppure
*postgres/*, il nome della connessione, quello del database e *.sql*. Le
finestre che il progetto aveva l'ultima volta non vengono mostrate, e i
plugin di progetto e di codice (project, build, debugger, ctags, etags, git,
terminal, spell) non vengono avviati; restano gli script, e il language
server delle query quando lsp.json ne ha uno. I menu Drawing, Format e
Plugins non sono nella barra dei menu, come con -D. Serve il plugin query.

*-G, --git=directory*
: Apre la finestra Git del plugin git sull'albero di lavoro di quella
directory; chiuderla termina l'editor. Le finestre che il progetto aveva
l'ultima volta non vengono mostrate, e i plugin di progetto (project, build,
debugger, ctags, etags) non vengono avviati. Per un pannello di git in un
file manager, con una di queste dopo, oppure da sola per la directory
corrente:

*--git-log*
: La finestra Git sul log.

*--git-show=commit*
: La finestra Git su quel commit: i suoi file e le loro differenze.

*--git-commit*, *--git-amend*
: La finestra Git con il messaggio di un commit di ciò che è nell'area di
stage, o dell'ultimo commit corretto con esso; l'editor termina quando il
messaggio è salvato e il commit fatto, o quando la sua finestra viene
chiusa.

*--git-reword=commit*
: Un altro messaggio per quel commit, allo stesso modo, con i file del commit
sotto.

*-h, -?, --help*
: Mostra le opzioni e cosa fanno.

*--help-terminal, --help-color*
: Mostra le opzioni del terminale o le opzioni dei colori qui sotto.

Le opzioni del terminale:

*-x, --xterm*
: Forza la modalità xterm. Si usa con i terminali compatibili con xterm (due
modalità dello schermo, e capaci di inviare sequenze di escape del mouse).

*-X, --no-x11*
: Non usa il sistema X Window per conoscere lo stato dei tasti modificatori.

*-g, --oldmouse*
: Usa il vecchio tracciamento del mouse con evidenziazione del terminale.

*-d, --nomouse*
: Disattiva il mouse.

*-t, --termcap*
: Usa il database termcap invece di terminfo. L'opzione esiste solo quando
bluerig è stato compilato con la libreria S-Lang.

*-s, --slow*
: Funziona su un terminale lento: lo schermo viene disegnato con meno
aggiornamenti.

*-a, --stickchars*
: Disegna le cornici con caratteri semplici invece dei caratteri grafici
del terminale.

*-K file, --keymap=file*
: Carica le associazioni dei tasti da quel file. Vedere
[Ridefinire i tasti](#keys_redefine).

*--nokeymap*
: Non carica nessun file dei tasti: usa le associazioni interne del
programma.

Le opzioni dei colori:

*-b, --nocolor*
: Forza la visualizzazione in bianco e nero.

*-c, --color*
: Forza la modalità a colori sui terminali che non sembrano supportare i
colori.

*-S tema, --skin=tema*
: Usa il tema con quel nome. Vedere
[Temi](#skins).

# Panoramica <a id="overview"></a>

Lo schermo di bluerig ha la
[barra dei menu](#menu-bar)
in alto, le finestre dell'editor sotto e le etichette dei tasti funzione in
basso. La barra dei menu può essere nascosta: F9 o un clic sulla prima riga
dello schermo la fanno comparire.

Ogni finestra mostra un file: il nome, lo stato del file e la posizione del
cursore stanno nella riga di stato in alto. Si possono aprire più finestre
insieme; con il
[menu Finestra](#window-menu)
si passa dall'una all'altra, e ciascuna si può spostare e ridimensionare.

# L'editor bluerig <a id="internal-file-editor"></a>

<a id="keys"></a>
**Tasti**

<!-- {{key:Action|F8}}: Action is the action name of misc/keymap.default.ini and src/keymap.c (Remove = f8); the help shows the keys bound to it now, the man page the default keys after the bar -->
Questi sono i tasti predefiniti. Ognuno si può cambiare, vedere
[Ridefinire i tasti](#keys_redefine).

<a id="moving-around"></a>
**Movimento del cursore**

|  |  |
|--|--|
| {{key:Left|Left}} | Un carattere a sinistra |
| {{key:Right|Right}} | Un carattere a destra |
| {{key:Up|Up}} | Una riga in su |
| {{key:Down|Down}} | Una riga in giù |
| {{key:WordLeft|Ctrl-Left, Ctrl-z}} | Una parola a sinistra |
| {{key:WordRight|Ctrl-Right, Ctrl-x}} | Una parola a destra |
| {{key:ScrollUp|Ctrl-Up}} | Scorre lo schermo in su |
| {{key:ScrollDown|Ctrl-Down}} | Scorre lo schermo in giù |
| {{key:PageUp|PgUp}} | Pagina su |
| {{key:PageDown|PgDn}} | Pagina giù |
| {{key:Home|Home}} | Inizio della riga |
| {{key:End|End}} | Fine della riga |
| {{key:Top|Ctrl-Home, Alt-\<}} | Inizio del file |
| {{key:Bottom|Ctrl-End, Alt-\>}} | Fine del file |
| {{key:TopOnScreen|Ctrl-PgUp}} | Cima dello schermo |
| {{key:BottomOnScreen|Ctrl-PgDn}} | Fondo dello schermo |

**Cancellazione**

|  |  |
|--|--|
| {{key:Delete|Delete, Ctrl-d}} | Cancella il carattere |
| {{key:BackSpace|Backspace, Ctrl-h}} | Cancella il carattere a sinistra |
| {{key:DeleteLine|Ctrl-y}} | Cancella la riga |
| {{key:DeleteToEnd|Ctrl-k}} | Cancella fino alla fine della riga |
| {{key:DeleteToWordBegin|Alt-Backspace}} | Cancella fino all'inizio della parola |
| {{key:DeleteToWordEnd|Alt-d}} | Cancella fino alla fine della parola |

<a id="blocks"></a>
**Blocchi**

|  |  |
|--|--|
| Shift+tasti cursore | Seleziona un blocco |
| Alt+tasti cursore | Seleziona un blocco a colonne |
| {{key:Mark|F3}} | Inizia o termina la selezione di un blocco |
| {{key:MarkColumn|F13}} | Inizia un blocco a colonne |
| {{key:MarkAll|Ctrl-a}} | Seleziona tutto il testo |
| {{key:Copy|F5}} | Copia il blocco al cursore |
| {{key:Move|F6}} | Sposta il blocco al cursore |
| {{key:Remove|F8}} | Cancella il blocco |
| {{key:Store|Ctrl-Ins}} | Copia il blocco nel file degli appunti |
| {{key:Cut|Shift-Del}} | Sposta il blocco nel file degli appunti |
| {{key:Paste|Shift-Ins}} | Incolla il blocco dal file degli appunti |
| {{key:BlockSave|Ctrl-f}} | Scrive il blocco in un file |

<a id="search-keys"></a>
**Ricerca e sostituzione**

|  |  |
|--|--|
| {{key:Search|F7}} | [Ricerca](#search-and-replace) |
| {{key:SearchContinue|F17}} | Ripete la ricerca |
| {{key:Replace|F4}} | Sostituzione |
| {{key:ReplaceContinue|F14}} | Ripete la sostituzione |
| {{key:FilterToggle|Alt-s}} | Toglie il filtro delle righe, o applica l'ultima ricerca come filtro |
| {{key:FilterWord|Alt-S}} | Filtra per il testo selezionato o per la parola sotto il cursore |

**Salti e segnalibri**

|  |  |
|--|--|
| {{key:Goto|Alt-l}} | Va a una riga per numero |
| {{key:MatchBracket|Alt-b}} | Alla parentesi corrispondente |
| {{key:Bookmark|Alt-k}} | Mette o toglie un segnalibro |
| {{key:BookmarkNext|Alt-j}}, {{key:BookmarkPrev|Alt-i}} | Al segnalibro successivo o precedente |
| {{key:BookmarkFlush|Alt-o}} | Toglie tutti i segnalibri |
| {{key:Find|Alt-Enter}} | Trova la definizione del nome sotto il cursore |
| {{key:FilePrev|Alt-Minus}}, {{key:FileNext|Alt-Plus}} | Indietro e avanti tra i punti in cui è andata la ricerca delle definizioni |

<a id="files-and-windows"></a>
**File**

|  |  |
|--|--|
| {{key:Save|F2}} | Salva il file |
| {{key:SaveAs|F12, Ctrl-F2}} | [Salva il file con un altro nome](#save-file-as) |
| {{key:EditNew|Ctrl-n}} | Nuova finestra vuota |
| {{key:InsertFile|F15}} | Inserisce un file al cursore |
| {{key:History|Alt-E}} | Cronologia dei file modificati |
| {{key:SelectCodepage|Alt-e}} | [Codifica del testo](#codepages-translation) |
| {{key:Quit|F10, Esc}} | Chiude la finestra; chiudere l'ultima fa uscire dall'editor |

<a id="editing"></a>
<a id="macros-and-plugins"></a>
**Altri comandi**

|  |  |
|--|--|
| {{key:InsertOverwrite|Insert}} | Modalità inserimento o sovrascrittura |
| {{key:Undo|Ctrl-u}} | Annulla |
| {{key:Redo|Alt-r}} | Ripristina |
| {{key:UndoHistory|Ctrl-t, Alt-U}} | Cronologia degli annullamenti |
| {{key:Complete|Alt-Tab}} | [Completa la parola](#completion) con le parole dei file aperti |
| {{key:FoldToggle|Alt-F}} | Ripiega o riapre il blocco tra parentesi corrispondenti |
| {{key:Sort|Alt-t}} | Ordina le righe selezionate |
| {{key:ExternalCommand|Alt-u}} | Inserisce l'output di un comando di shell |
| {{key:ParagraphFormat|Alt-p}} | Formatta il paragrafo |
| {{key:InsertLiteral|Ctrl-q}} | Inserisce il tasto successivo, o un codice U+XXXX, così com'è |
| Ctrl-p | Controlla l'ortografia della parola sotto il cursore |
| {{key:MacroStartStopRecord|Ctrl-r}} | Avvia o ferma la registrazione di una [macro](#macros) |
| {{key:MacroExplorer|Alt-M}} | [Macro Explorer](#macro-explorer) |
| {{key:ShowNumbers|Alt-n}} | Mostra o nasconde i numeri di riga |
| {{key:ShowTabTws|Alt-\_}} | Mostra o nasconde le tabulazioni e gli spazi finali |
| {{key:SyntaxOnOff|Ctrl-s}} | Attiva o disattiva l'evidenziazione della sintassi |
| {{key:Help|F1}} | Aiuto |
| {{key:Menu|F9}} | Barra dei menu |
| {{key:UserMenu|F11}} | [Menu utente](#edit-menu-file) |
| {{key:Shell|Ctrl-o}} | Mostra o nasconde il terminale |
| Ctrl-Alt-p | Mostra o nasconde la Preview |
| {{key:Refresh|Ctrl-l}} | Ridisegna lo schermo |

|  |  |
|--|--|
| Vedere anche: | [Menu](#menu-bar) |
|  | [Ricerca, sostituzione e filtro delle righe](#search-and-replace) |
|  | [Ripiegamento](#folding) |
|  | [Macro](#macros) |
|  | [Menu utente](#edit-menu-file) |
|  | [Plugin](#plugins) |
|  | [Evidenziazione della sintassi](#syntax-highlighting) |
|  | [Opzioni editor](#editor-options) |
|  | [Impostazioni nel file ini](#internal-file-editor-options) |

## Ridefinire i tasti <a id="keys_redefine"></a>

|  |  |
|--|--|
| Opzioni \> [Key bindings](#key-bindings) | Dà a un'azione tasti propri; vengono salvati in **~/.config/bluerig/keymap.ini** |
| Opzioni \> [Impara tasti](#learn-keys) | Dà ai tasti le loro sequenze di escape |
| Opzioni \> [Key sniffer](#key-sniffer) | Mostra cosa invia il terminale per un tasto e a quale azione il tasto è associato |

All'avvio bluerig prende la mappa dei tasti interna e vi carica sopra
**{{pkgdatadir}}/keymap.ini**,
poi
**{{sysconfdir}}/bluerig/keymap.ini**.
Un'azione nominata in un file prende i tasti del file, e i suoi tasti interni
smettono di funzionare.

Il pacchetto installa le mappe dei tasti
**keymap.default.ini**,
**keymap.emacs.ini**
e
**keymap.vim.ini**
in
**{{sysconfdir}}/bluerig**
e fa di
**keymap.ini**
un collegamento a
**keymap.default.ini**.

L'opzione
**--nokeymap**
disattiva il caricamento dei file, e funziona solo la mappa interna.

Il file dei tasti dell'utente è il primo trovato tra:

```
1) command line option -K <keymap>, --keymap=<keymap>
2) environment variable BLUERIG_KEYMAP
3) the keymap setting in the [bluerig] section of the ini file
4) file ~/.config/bluerig/keymap.ini
```

I primi tre accettano un nome o un percorso assoluto. Un nome che non finisce
in
**.keymap**
prende quell'estensione, e il file viene cercato in queste directory, vince
la prima corrispondenza:

```
1) ~/.config/bluerig/
2) {{pkgdatadir}}/
```

L'opzione
**-K**,
la variabile
**BLUERIG_KEYMAP**
e l'impostazione
**keymap**
aggiungono l'estensione
**.keymap**
a un nome che non ne ha, quindi le mappe del pacchetto, che finiscono in
**.ini**,
non si possono scegliere in questo modo. Per usarne una, fare di
**~/.config/bluerig/keymap.ini**
un collegamento a essa: questo file viene caricato quando non è dato nessuno
dei tre.

```
ln -s {{sysconfdir}}/bluerig/keymap.vim.ini ~/.config/bluerig/keymap.ini
```

Le sezioni di un file dei tasti:

|  |  |
|--|--|
| [editor] | Editor |
| [editor:xmap] | Tasti dopo il prefisso, azione ExtendedKeyMap |
| [dialog] | Finestre di dialogo |
| [menu] | Menu |
| [input] | Righe di immissione |
| [listbox] | Elenchi |
| [radio] | Pulsanti di opzione |
| [help] | Finestra dell'aiuto |
| [mcterm] | Terminale |

Una riga di una sezione nomina un'azione e i suoi tasti, separati da punti e
virgola:

```
[editor]
Save = f2
SaveAs = f12; ctrl-f2
```

## Tasti delle righe di immissione <a id="input-line-keys"></a>

Le righe di immissione delle finestre di dialogo accettano questi tasti:

|  |  |
|--|--|
| Ctrl-a, Home | Inizio della riga |
| Ctrl-e, End | Fine della riga |
| Ctrl-b, Left | Un carattere a sinistra |
| Ctrl-f, Right | Un carattere a destra |
| Alt-f, Ctrl-Right | Una parola avanti |
| Alt-b, Ctrl-Left | Una parola indietro |
| Ctrl-h, Backspace | Cancella il carattere prima del cursore |
| Ctrl-d, Delete | Cancella il carattere sotto il cursore |
| Ctrl-@ | Mette il segno |
| Ctrl-w | Taglia nel buffer il testo dal segno al cursore |
| Alt-w | Copia nel buffer il testo dal segno al cursore |
| Ctrl-y | Incolla il buffer |
| Ctrl-k | Cancella fino alla fine della riga |
| Ctrl-Insert | Copia la selezione, o senza selezione tutta la riga, nel file degli appunti |
| Shift-Delete | Taglia la selezione nel file degli appunti |
| Shift-Insert | Incolla il file degli appunti come una sola riga: gli a capo diventano spazi |
| Alt-p, Alt-n | Voce precedente o successiva della cronologia della riga |
| Alt-h | [Cronologia](#history-query) della riga come elenco |
| Alt-Backspace | Cancella la parola prima del cursore |
| Alt-Tab | [Completa](#completion) un nome di file, una variabile o un nome utente |

## Tasti di movimento generali <a id="general-movement-keys"></a>

La finestra dell'aiuto e gli elenchi usano lo stesso codice di movimento,
quindi accettano gli stessi tasti:

|  |  |
|--|--|
| Up, Ctrl-p | Una riga indietro |
| Down, Ctrl-n | Una riga avanti |
| Page Up, Alt-v | Pagina su |
| Page Down, Ctrl-v | Pagina giù |
| Home, A1 | All'inizio |
| End, C1 | Alla fine |

La finestra dell'aiuto accetta anche questi tasti:

|  |  |
|--|--|
| b, Backspace | Pagina su |
| Space, f | Pagina giù |
| u, d | Mezza pagina su o giù |
| g, G | All'inizio o alla fine |

# Supporto del mouse <a id="mouse-support"></a>

bluerig supporta il mouse. Funziona in un terminale
**xterm(1)**
(anche attraverso una connessione telnet o ssh dall'xterm) e sulla console di
Linux con il server del mouse
**gpm**
in esecuzione.

Un clic mette il cursore dove si fa clic, un trascinamento evidenzia il
testo, e un clic su un menu, un pulsante o un'etichetta dei tasti funzione lo
sceglie. La rotella fa scorrere il testo.

La ripetizione automatica dei pulsanti del mouse è di 400 millisecondi in
modo predefinito. Si cambia con l'impostazione
*mouse_repeat_rate*
del file ini, e il tempo entro cui due clic sono un doppio clic con
*double_click_speed*.

Se bluerig funziona con il supporto del mouse, si ottiene il comportamento
normale del mouse del terminale (tagliare e incollare il testo) tenendo
premuto il tasto Shift.

# Barra dei menu <a id="menu-bar"></a>

La barra dei menu compare quando si preme F9 o si fa clic sulla riga in alto
dello schermo. I suoi menu sono File, Modifica, Cerca, Comandi, Navigate
(solo quando un plugin o uno script vi aggiunge qualcosa), Finestra, Plugins
e Opzioni. Uno script Lua può aggiungere voci a ognuno di essi, o menu
propri.

## Menu File <a id="file-menu"></a>

**Apri file...**
: Apre un file in una nuova finestra.

**Nuovo**
: Apre una finestra vuota.

**Chiudi**
: Chiude la finestra.

**Cronologia...**
: I file modificati in precedenza, per riaprirne uno.

**Salva, Salva come...**
: Scrive il file, con il suo nome o con un altro. Vedere
[Salva come](#save-file-as).

**Inserisci file..., Copia su file...**
: Inserisce un file al cursore, o scrive il blocco evidenziato in un file.

**Menu utente...**
: Il [menu utente](#edit-menu-file).

**Informazioni...**
: La versione del programma.

**Uscita**
: Esce dall'editor, chiedendo dei file non salvati.

## Menu Modifica <a id="edit-menu"></a>

Annulla, Rifà e Undo History; il passaggio tra modalità di inserimento e di
sovrascrittura; i comandi che evidenziano un blocco, un blocco a colonne o
tutto il file, e tolgono l'evidenziazione; copia, spostamento e
cancellazione del blocco; copia nel file degli appunti, taglio nel file e
incolla da esso; e il salto all'inizio e alla fine del file.

## Menu Cerca <a id="search-menu"></a>

Cerca, Ripeti cerca e Rimpiazza; il filtro delle righe (Toggle line filter,
Filter by word; vedere
[Ricerca, sostituzione e filtro delle righe](#search-and-replace)); e i
segnalibri: mettere o togliere un segnalibro sulla riga corrente, andare al
successivo o al precedente, e toglierli tutti.

## Menu Comandi <a id="command-menu"></a>

**Vai alla riga...**
: Va a una riga, indicata dal suo numero.

**Commuta la riga di stato**
: Mostra o nasconde la colonna dei numeri di riga.

**Vai alla parentesi relativa**
: Va alla parentesi che corrisponde a quella sotto il cursore.

**Commuta l'evidenziazione della sintassi, Commuta margine destro, Toggle control characters**
: Li attiva e disattiva.

**Codifica...**
: La codifica in cui è scritto il testo. Vedere
[Scegli la codepage](#codepages-translation).

**Fold / Unfold block, Unfold all**
: Vedere [Ripiegamento](#folding).

**Aggiorna lo schermo**
: Ridisegna lo schermo.

**Avvio/stop registrazione macro, Macro explorer..., Registra/ripeti le azioni**
: Vedere [Macro](#macros).

## Menu Navigate <a id="navigate-menu"></a>

I comandi dei plugin: andare alla dichiarazione, trovare i riferimenti, i
simboli del file e altri. Vedere
[Plugin](#plugins).

## Menu Finestra <a id="window-menu"></a>

**Sposta, Ridimensiona**
: Sposta o ridimensiona la finestra con i tasti freccia; Enter termina.

**Commuta a pieno schermo**
: Passa la finestra tra lo schermo intero e la sua dimensione.

**Toggle sticky windows**
: Con le finestre adesive, le finestre le cui cornici stanno una accanto
all'altra si ridimensionano insieme: l'angolo di una finestra sposta il bordo
su cui si trova con tutte le finestre lungo di esso, da entrambi i lati, e
tra di esse non si apre nessuno spazio. Dove si incontrano quattro finestre,
l'angolo sposta entrambi i bordi. Un lato che è sul bordo dello schermo
quando il ridimensionamento comincia resta lì, una finestra conserva la sua
dimensione minima, e un bordo si ferma a una finestra che trova sulla sua
strada, e quella finestra allora si ridimensiona con le altre. Esc rimette
ogni finestra com'era. Una finestra trascinata per il titolo si sposta da
sola. L'impostazione è salvata come
**editor_sticky_windows**
nel file ini.

**Successiva, Precedente**
: Va alla finestra successiva o precedente.

**Elenco...**
: Le finestre aperte, per andare a una di esse. Vedere
[Apri i file](#open-files).

Sotto vengono le finestre dei plugin: l'albero del progetto, l'output della
compilazione, il pannello e la console del debugger, il terminale, il
terminale del programma che il debugger esegue, la Preview. Una voce apre la
sua finestra, la porta in primo piano quando è dietro un'altra, e la chiude
quando è già in primo piano; una finestra aperta è segnata con *\**. Un
plugin disattivato non ha voce.

Le finestre dei plugin stanno in aree di aggancio. L'albero, il pannello del
debugger e la Preview vanno nella colonna a destra, una sopra l'altra, per
tutta l'altezza dello schermo. La console, l'output della compilazione e il
terminale vanno nella fila in basso, sotto le finestre dei file, e se la
dividono come schede: se ne vede una, e il suo titolo elenca i nomi di tutte. Un
clic su un nome, oppure
**Ctrl-Alt-PgDn** e **Ctrl-Alt-PgUp** (*WindowTabNext* e *WindowTabPrev*
nella mappa dei tasti), ne mostra un'altra. Una finestra di file a pieno
schermo riempie il resto dello schermo, e di nuovo tutto lo schermo quando
le aree di aggancio sono vuote. Ridimensionare una finestra agganciata
ridimensiona l'area; spostarla la toglie dall'area.

**Layout...**
: Le disposizioni delle finestre: quali finestre dei plugin sono aperte, in
quale area di aggancio, e quanto sono grandi le aree. *Edit* ha solo i file,
*Project* l'albero del progetto, *Debug* l'albero e il pannello del
debugger. **Use** mette le finestre così; **Save as...** le salva come sono
con un nome, anche con il nome di una di queste; **Delete** dimentica una
disposizione salvata, e torna quella dell'editor con quel nome. Con **Debug
layout while debugging** l'avvio del debugger mette le finestre come le ha
*Debug*, e il suo arresto le rimette com'erano, lasciando le schede in basso
da leggere.

Le finestre di un progetto vengono salvate quando l'editor termina, e
tornano quando l'editor si avvia con un file del progetto.
*bluerig --debug* parte con la disposizione *Debug*. Le disposizioni sono
salvate in *layouts.ini* nella directory delle impostazioni.

## Menu Plugins <a id="plugins-menu"></a>

I plugin dell'editor che hanno una voce propria, per esempio il controllo
ortografico; le loro finestre sono nel [menu Finestra](#window-menu). Un
plugin disattivato in
[Manage plugins](#manage-plugins)
non è nel menu.

## Menu Opzioni <a id="options-menu"></a>

**Generale...**
: Le [opzioni dell'editor](#editor-options).

**Modalità salvataggio...**
: Come viene scritto un file. Vedere
[Modifica modalità salvataggio](#edit-save-mode).

**Sintassi evidenziata...**
: Sceglie a mano le regole di sintassi per il file, o le lascia al nome e
alla prima riga del file.

**Aspetto...**
: Il tema e le ombre. Vedere [Aspetto](#appearance).

**Impara tasti...**
: Vedere [Impara tasti](#learn-keys).

**Key bindings...**
: Vedere [Key bindings](#key-bindings).

**Key sniffer...**
: Vedere [Key sniffer](#key-sniffer).

**Manage plugins...**
: Vedere [Manage plugins](#manage-plugins).

**Plugin info...**
: Vedere [Plugin info](#plugin-info).

**Lua scripts...**
: Vedere [Lua scripts](#lua-scripts). La voce c'è solo dove bluerig è stato
compilato con Lua.

**File sintassi**
: Apre nell'editor il file di sintassi dell'utente,
*~/.local/share/bluerig/syntax/Syntax*.
Dove non esiste, viene creato con un breve commento su ciò che contiene, non
come copia del file di sistema, che viene letto comunque dopo di esso.
Vedere
[Evidenziazione della sintassi](#syntax-highlighting).

**Menu file**
: Apre nell'editor il file del [menu utente](#edit-menu-file).

**Salva configurazione**
: Scrive le impostazioni in
*~/.config/bluerig/ini*.
Con l'impostazione
*auto_save_setup*
attiva, vengono scritte ogni volta che l'editor termina.

### Opzioni editor <a id="editor-options"></a>

La voce
**Generale...**
del menu Opzioni apre la finestra delle opzioni dell'editor. Ogni opzione
viene scritta nel file ini, vedere
[Impostazioni nel file ini](#internal-file-editor-options).

**Modalità a capo**

|  |  |
|--|--|
| Niente | Le righe non vanno a capo |
| Paragrafi dinamici | Formatta il paragrafo sotto il cursore mentre si scrive |
| A capo automatico | Spezza la riga alla lunghezza di a capo mentre si scrive |

**Tabulazione**

|  |  |
|--|--|
| Imbroglia i mezzi tab | Tra il margine sinistro e il testo una tabulazione avanza di metà della sua ampiezza, e il file riceve spazi e tabulazioni |
| Backspace attraverso i tab | Backspace cancella tutto il rientro fino al margine sinistro quando tra il cursore e il margine non c'è testo |
| Riempie i tabulatori di spazi | Inserisce spazi fino alla tabulazione successiva invece di un carattere di tabulazione |
| Spaziatura tab | L'ampiezza di un carattere di tabulazione; 8 in modo predefinito |

**Keyboard**

|  |  |
|--|--|
| Esc timeout, ms | Quanti millisecondi Esc aspetta il resto di un tasto prima di valere come Esc; 1000 in modo predefinito. Nel file ini è *old_esc_mode_timeout* in microsecondi, e la variabile **KEYBOARD_KEY_TIMEOUT_US** lo sostituisce all'avvio |

**Altre opzioni**

|  |  |
|--|--|
| Invio autoindenta | Enter comincia la riga con il rientro della riga sopra |
| Conferma prima di salvare | Chiede prima di scrivere il file |
| Salva posizione file | Apre il file dove è stato lasciato |
| Spazi finali visibili | Segna gli spazi a fine riga |
| Tab visibili | Segna i caratteri di tabulazione |
| Show control characters | Mostra i caratteri di controllo come ^M invece di nasconderli |
| Sintassi evidenziata | Colora il testo con le regole di sintassi del suo tipo |
| Cursore dopo il blocco inserito | Lascia il cursore alla fine di un blocco inserito, non all'inizio |
| Selezione persistente | Mantiene la selezione quando il cursore si muove |
| Cursore oltre la fine riga | Lascia andare il cursore oltre l'ultimo carattere della riga |
| Annullamento di gruppo | Annulla in una volta una serie di modifiche dello stesso tipo |
| Auto a capo colonna | La colonna in cui le modalità di a capo spezzano la riga; 72 in modo predefinito |

### Modifica modalità salvataggio <a id="edit-save-mode"></a>

Come viene scritto un file, le stesse tre modalità che accetta
l'impostazione
*editor_option_save_mode*:

**Salva veloce**
: Scrive subito sopra il file. È veloce, ma un errore a metà lascia il file
scritto a metà.

**Salva sicuro**
: Scrive prima un file temporaneo e lo rinomina sopra l'originale quando è
completo, così un errore lascia intatto l'originale.

**Esegue i backup con le seguenti estensioni**
: Salvataggio sicuro, e l'originale resta con il suo nome più l'estensione
della riga di immissione, "~" in modo predefinito. Un secondo salvataggio
sostituisce anche la copia di sicurezza.

**Controllo nuova linea POSIX**
: Prima di scrivere il file chiede cosa fare se manca il fine riga finale.

### Aspetto <a id="appearance"></a>

Il tema con cui viene disegnato il programma, e se le finestre di dialogo e
i menu a discesa hanno un'ombra.

**Tema**
: Il pulsante apre l'elenco dei temi installati e di quelli dell'utente;
quello scelto viene applicato subito.

**Ombre**
: Disegna un'ombra sotto ogni finestra di dialogo e menu a discesa. Senza
colori non ci sono ombre.

La scelta viene scritta nelle impostazioni
*skin*
e
*shadows*
del file ini. Vedere la sezione
[Temi](#skins)
per i file dei temi.

### Impara tasti <a id="learn-keys"></a>

Questa finestra insegna a bluerig le sequenze di escape che il terminale
invia per i tasti funzione, le frecce e i tasti di navigazione. Il titolo
riporta il nome del terminale, preso dalla variabile
**TERM**.

Scegliere una combinazione di modificatori (Ctrl, Alt, Shift) con le
caselle, poi premere il pulsante di un tasto. Premere il tasto sulla
tastiera e aspettare che il messaggio di cattura scompaia. La sequenza
appresa compare accanto al pulsante. Una sequenza che ha già un altro tasto
viene segnalata come conflitto.

**Del**
: Cancella il tasto appreso su cui è il cursore.

**Salva**
: Scrive i tasti appresi in
*~/.config/bluerig/term/\<TERM>*.

**Edit term file**
: Apre quel file nell'editor.

### Key bindings <a id="key-bindings"></a>

Le azioni del programma con i tasti a cui rispondono, in una tabella. I
pulsanti in alto scelgono il gruppo, Editor o Widgets, e poi la sua parte:
Main e Ctrl-X per l'editor; Dialog, Menu, Input, Listbox, Radio e Help per i
widget. Back torna ai gruppi. Le azioni segnate con \* hanno tasti diversi
da quelli predefiniti.

**Enter**
: Sostituisce il tasto dell'azione: premere il tasto da assegnare.

**F5**
: Aggiunge un altro tasto all'azione.

**F8, Del**
: Toglie il tasto.

**Salva**
: Scrive le modifiche in
*~/.config/bluerig/keymap.ini*.

**Edit keymap file**
: Apre
*keymap.ini*
nell'editor.

**Edit term file**
: Apre le sequenze dei tasti del terminale, il file che scrive
[Impara tasti](#learn-keys).

Un tasto già preso da un'altra azione viene segnalato, con la domanda se
toglierlo da lì.

### Key sniffer <a id="key-sniffer"></a>

Premere
**Capture key**,
poi un tasto qualsiasi. La finestra mostra cosa è arrivato:

```
Shortcut   the symbolic name, e.g. Ctrl-F5
Action     the action of the key in the editor keymap
Raw        the escape sequence and its bytes in hex
Keycode    the internal numeric code
```

È il posto dove guardare quando un tasto non fa niente o fa la cosa
sbagliata: un tasto che arriva come una sequenza che il programma non
riconosce va insegnato in
[Impara tasti](#learn-keys),
e uno che arriva ma non ha un'azione va associato in
[Key bindings](#key-bindings).

### Manage plugins <a id="manage-plugins"></a>

I plugin che il programma ha caricato, in una tabella: il tipo, il nome e
ciò che il plugin dice di sé. Qui sono elencati i plugin dell'editor e, dove
bluerig è stato compilato con Lua, il motore Lua come plugin di tipo
*runtime*.
Un plugin si disattiva e si attiva con la casella della sua riga, e ciò che
è disattivato non viene caricato nemmeno la volta successiva; la scelta è
salvata in
*~/.config/bluerig/plugins.ini*.
Un plugin disattivato resta nell'elenco, così che si possa riattivare.

**Enter, F4**
: Apre le impostazioni del plugin su cui è il cursore. Un plugin che non ne
ha lo dice. Sulla riga del motore Lua apre i
[Lua scripts](#lua-scripts).

**IDE: project, build, debugger**
: Attiva insieme i tre plugin che fanno dell'editor un IDE, o li disattiva
insieme quando sono tutti attivi: il progetto e i suoi file, la
compilazione e il debugger. Ognuno funziona anche senza gli altri.

### Plugin info <a id="plugin-info"></a>

I plugin che l'editor ha caricato: il nome, se è attivo, cosa offre e cosa
fa. È un elenco da guardare; un plugin si disattiva e se ne aprono le
impostazioni nella finestra
[Manage plugins](#manage-plugins).

### Lua scripts <a id="lua-scripts"></a>

Gli script Lua trovati, quelli di sistema e quelli dell'utente, in una
tabella: se è attivo, dove si trova lo script (globale o dell'utente), il
suo identificatore, il suo spazio di lavoro, cosa offre e cosa fa. Uno
script si disattiva e si attiva con la casella della sua riga; la modifica
ha effetto al prossimo avvio di bluerig.

**Settings**
: Esegue lo script con le impostazioni del pacchetto, se il pacchetto ne ha
uno.

Vedere [Script Lua](#lua-scripting) per gli script stessi.

# Ricerca, sostituzione e filtro delle righe <a id="search-and-replace"></a>

{{key:Search|F7}} apre la finestra di ricerca.

|  |  |
|--|--|
| Normale | Cerca la stringa così com'è |
| Espressione regolare | Cerca con un'espressione regolare |
| Esadecimale | Cerca byte scritti come numeri esadecimali |
| Ricerca tipo jolly | Cerca con un modello della shell con \* e ? |
| Distingui le maiuscole | Distingue le lettere minuscole dalle maiuscole |
| Indietro | Cerca dal cursore verso l'inizio del file |
| In selezione | Cerca solo nel blocco selezionato |
| Parole intere | Trova solo parole intere |
| Tutti i caratteri | Cerca in ogni codifica in cui il file può essere scritto |
| Trova tutti | Mette un segnalibro su ogni riga con una corrispondenza |
| Filter | Nasconde le righe senza corrispondenza |

{{key:Replace|F4}} apre la finestra di sostituzione: gli stessi campi e la
riga "Inserire stringa di sostituzione". In una sostituzione con espressione
regolare da \\1 a \\9 inseriscono i gruppi trovati.

|  |  |
|--|--|
| Rimpiazza | Sostituisce questa corrispondenza e cerca la successiva |
| Tutti | Sostituisce ogni corrispondenza senza chiedere |
| Salta | Lascia questa corrispondenza e cerca la successiva |

**Filter**

Nasconde le righe senza corrispondenza; i numeri di riga non cambiano, e la
colonna di stato delle righe segna ogni tratto nascosto. Le righe vengono
nascoste una volta sola, quando si preme il pulsante: una riga modificata o
scritta dopo resta visibile anche se non corrisponde.

|  |  |
|--|--|
| {{key:FilterToggle|Alt-s}} | Toglie il filtro, o applica l'ultima ricerca come filtro; senza ricerca, apre la finestra di ricerca |
| {{key:FilterWord|Alt-S}} | Filtra per il testo selezionato o per la parola sotto il cursore |
| Comandi \> Unfold all | Toglie il filtro e ogni ripiegamento |

# Ripiegamento <a id="folding"></a>

Un ripiegamento nasconde una serie di righe dietro la sua prima riga, che
resta sullo schermo con un segno nella colonna di stato delle righe.
**Alt-F**
(Fold / Unfold block nel menu Comandi) ripiega le righe evidenziate o, se
non c'è niente di evidenziato, le righe da una parentesi sulla riga corrente
alla parentesi che le corrisponde. Sulla prima riga di un ripiegamento lo
riapre.
**Unfold all**
riapre ogni ripiegamento del file, e toglie il filtro delle righe.

# Macro <a id="macros"></a>

{{key:MacroStartStopRecord|Ctrl-r}}
avvia la registrazione di una macro: i tasti premuti dopo vengono
registrati. Un secondo
**Ctrl-r**
termina la registrazione e aspetta il tasto a cui associare la macro; quel
tasto poi la esegue.

Le macro sono salvate nella sezione
**[editor]**
del file
**~/.local/share/bluerig/macros**,
e una macro si cancella cancellando la sua riga in quel file, o nel
[Macro Explorer](#macro-explorer).

**Registra/ripeti le azioni**
nel menu Comandi registra le azioni che seguono finché non viene scelto di
nuovo, e poi chiede quante volte ripeterle.

Anche uno script di shell si può assegnare a un tasto, con una riga del file
**macros**
come questa:

```
[editor]
Ctrl-W=ExecuteScript:25;
```

Questo significa che Ctrl-W esegue l'azione
*ExecuteScript(25)*,
che esegue lo script di shell
**~/.local/share/bluerig/macros.d/macro.25.sh**.
Gli script sono salvati nella directory
**~/.local/share/bluerig/macros.d/**
e devono chiamarsi
**macro.XXXX.sh**,
dove
**XXXX**
è un numero da 0 a 9999. Gli esempi sono installati in
**{{pkgdatadir}}/examples/macros.d/**.
Uno script ha il formato delle voci del
[menu utente](#edit-menu-file)
e accetta le stesse sostituzioni. La sua prima riga può essere

*#silent*
: Lo script viene eseguito senza chiedere i suoi parametri.

Ecco uno script di esempio:

```
l       comment selection
	TMPFILE=`mktemp ${BLUERIG_TMPDIR:-/tmp}/up.XXXXXX` || exit 1
	echo '#if 0' > $TMPFILE
	cat %b >> $TMPFILE
	echo '#endif' >> $TMPFILE
	cat $TMPFILE > %b
	rm -f $TMPFILE
```

## Macro Explorer <a id="macro-explorer"></a>

Le macro registrate, con il tasto a cui ciascuna risponde e ciò che fa. I
pulsanti sono

**Run**
: Esegue la macro su cui è il cursore.

**Elimina**
: La toglie, dopo una domanda.

**Modifica file**
: Apre il file in cui stanno le macro, descritto nella sezione
[Macro](#macros).

# Menu utente <a id="edit-menu-file"></a>

{{key:UserMenu|F11}}
apre il menu utente: ognuna delle sue voci è uno script di shell eseguito sul
testo dell'editor. Il menu viene letto dal primo file trovato tra:

```
1) ./.bluerig.menu in the current directory
2) ~/.config/bluerig/menu
3) ~/bluerig.menu
4) {{sysconfdir}}/bluerig/bluerig.menu
5) {{pkgdatadir}}/bluerig.menu
```

Il file della directory corrente viene letto solo se appartiene all'utente o
a root e nessun altro può scriverlo: le sue voci eseguono comandi. La voce
**Menu file**
del menu Opzioni apre il file del menu nell'editor.

Una riga che comincia con qualsiasi carattere tranne uno spazio o una
tabulazione è una voce del menu, e il suo primo carattere è il tasto rapido
della voce. Le righe che cominciano con uno spazio o una tabulazione sono i
comandi della voce. Una riga che comincia con '#' è un commento.

Quando si sceglie una voce, i suoi comandi vengono scritti in un file
temporaneo, vi si applicano le sostituzioni, e la shell esegue quel file:

|  |  |
|--|--|
| %f, %p, %s | Il nome del file corrente |
| %n | Il nome del file corrente senza estensione |
| %x | L'estensione del nome del file corrente |
| %d | La directory corrente |
| %c | La posizione del cursore nel file |
| %i | Un rientro di spazi largo quanto la colonna del cursore |
| %y | Il tipo di sintassi del file corrente |
| %b | Il file del blocco: il testo selezionato viene scritto in **~/.cache/bluerig/block** prima dell'esecuzione, e ciò che i comandi vi lasciano sostituisce il blocco |
| %m | Il nome del file del menu |
| %% | Il carattere % |
| %{text} | Una riga di immissione con la richiesta "text"; ciò che si scrive viene inserito, Esc o F10 annullano la voce |
| %view | Apre l'output standard dei comandi in una nuova finestra dell'editor; le parole tra graffe dopo di esso, come in %view{ascii}, vengono saltate |

Le lettere delle sostituzioni non distinguono maiuscole e minuscole. Un
numero dopo % attiva le virgolette (il comportamento predefinito) o le
disattiva: %f e %1f mettono tra virgolette il valore inserito, %0f no.

Un file di menu di esempio:

```
s       Sort the block
	sort %b > %b.tmp && mv -f %b.tmp %b

w       Count the words of the file
	%view wc %f

m       Run make in the directory of the file
	make
```

**Voce predefinita**

Una riga che comincia con '=' nella prima colonna è una condizione della
voce sotto di essa: quando la condizione è vera, la voce diventa quella
predefinita.

```
Condition syntax:   = <sub-cond>
  or:               = <sub-cond> | <sub-cond> ...
  or:               = <sub-cond> & <sub-cond> ...

Sub-condition is one of following:

  y <pattern>   syntax of current file matching pattern?
  f <pattern>   current file matching pattern?
  d <pattern>   current directory matching pattern?
  t <type>      current file of type?
  x <filename>  is it executable filename?
  ! <sub-cond>  negate the result of sub-condition
```

Il modello è un modello della shell o un'espressione regolare, secondo
l'impostazione
*shell_patterns*;
una riga "shell_patterns=0" o "shell_patterns=1" come prima riga del file
del menu la fissa per quel file.

Il tipo è uno o più caratteri:

```
  n   not a directory
  r   regular file
  d   directory
  l   link
  c   character device
  b   block device
  f   FIFO (pipe)
  s   socket
  x   executable file
```

Per esempio 'rlf' è un file normale, un collegamento o una FIFO.

Una condizione che comincia con '=?' invece di '=' mostra una traccia di
debug ogni volta che viene calcolata.

Le condizioni vengono calcolate da sinistra a destra; '&' non lega più
strettamente di '|':

```
	= f *.c | f *.h & y C
```

viene calcolata come

```
	( (f *.c) | (f *.h) ) & (y C)
```

**Condizioni di aggiunta**

Una condizione che comincia con '+' (o '+?') mette la voce nel menu solo
quando è vera. Una condizione con '+=' o '=+' ('+=?' e '=+?' con traccia)
vale sia come condizione di aggiunta sia come condizione predefinita. Due
righe di condizione prima di una voce, una con '+' e una con '=', tengono
separate le due condizioni.

# Plugin <a id="plugins"></a>

{{plugins}}
Un plugin si attiva e si disattiva nel menu Opzioni, voce
[Manage plugins](#manage-plugins).

# Finestre di dialogo <a id="dialogs"></a>

## Salva come <a id="save-file-as"></a>

Il nome con cui scrivere il file, e i fine riga con cui scriverlo: come li ha
il file, Unix (LF), Windows e DOS (CR LF), oppure Macintosh (CR). La scelta
vale per quel salvataggio; il file conserva i fine riga che riceve.

## Apri i file <a id="open-files"></a>

I file che l'editor ha aperti, uno per riga. Enter va al file su cui è il
cursore, Esc lascia dov'è il file mostrato. Lo stesso elenco è la voce
**Elenco...**
del menu Finestra.

## Scelta dello schermo <a id="screen-selector"></a>

Oltre all'editor, uno script Lua può aprire schermi propri, viste a pieno
schermo che restano aperte mentre si torna all'editor. Ci sono tre modi per
passare da uno schermo all'altro:

**Alt-}**
: passa allo schermo successivo;

**Alt-{**
: passa allo schermo precedente;

**Alt-\`**
: apre una finestra di dialogo con l'elenco degli schermi aperti, per
sceglierne uno.

## Completamento <a id="completion"></a>

{{key:Complete|Alt-Tab}}
nell'editor completa la parola prima del cursore con le parole dei file
aperti, e mostra un elenco quando più parole vanno bene. L'impostazione
*editor_wordcompletion_collect_all_files*
del file ini, attiva in modo predefinito, prende le parole da tutti i file
aperti; quando è disattivata, solo dal file corrente.

**Alt-Tab**
in una riga di immissione di una finestra di dialogo completa il testo:

|  |  |
|--|--|
| $nome | Una variabile d'ambiente |
| ~nome | Un nome utente |
| Altrimenti | Un nome di file |

Quando ne vanno bene più d'uno, il programma emette un segnale acustico.
L'impostazione
*show_all_if_ambiguous*
del file ini, disattivata in modo predefinito, mostra il loro elenco accanto
al cursore:
**Enter**
ne sceglie uno, le lettere scritte restringono l'elenco, Esc, F10, Left e
Right lo nascondono.

## Scegli la codepage <a id="codepages-translation"></a>

L'elenco delle codifiche che il programma conosce, da
**{{pkgdatadir}}/charsets**.
Sceglierne una dice all'editor in quale codifica è scritto il testo del
file, e il testo viene ricodificato da essa nella codifica del terminale
per lo schermo;
**\<Non tradotto>**
lo lascia come byte. L'elenco si apre con
**Alt-e**
e con la voce Codifica del menu Comandi.

## Cronologia <a id="history-query"></a>

L'elenco di ciò che è stato scritto prima in una riga di immissione, dal più
recente, che
**Alt-h**
apre per la riga in cui è il cursore. Enter porta nella riga la voce su cui
è il cursore, Esc lascia la riga com'era, e
**F8, Del**
toglie dalla cronologia la voce su cui è il cursore. Le cronologie sono
salvate in
*~/.local/share/bluerig/history*;
*num_history_items_recorded*
dice quante voci conserva ognuna di esse.

# Evidenziazione della sintassi <a id="syntax-highlighting"></a>

{{key:SyntaxOnOff|Ctrl-s}}
attiva e disattiva l'evidenziazione della sintassi. Le regole
dell'evidenziazione sono nei file di sintassi: parole chiave e contesti,
come commenti e stringhe, vi ricevono i loro colori.

|  |  |
|--|--|
| [File di sintassi](#syntax-files) | Dove sono le regole e in quale ordine vengono lette |
| [La sezione file](#syntax-file) | Quali file evidenzia una sezione di regole |
| [Contesti](#syntax-contexts) | Zone del testo con colori propri |
| [Incorporamento e livelli](#syntax-embed) | Regole di un'altra sintassi dentro un contesto e sopra il testo |
| [Parole chiave](#syntax-keywords) | Parole e modelli colorati dentro un contesto |
| [Colori e attributi](#syntax-colors) | Colori, token di scope del tema, attributi |
| [Altre righe](#syntax-other) | Commenti, caseinsensitive, line-local |

## File di sintassi <a id="syntax-files"></a>

|  |  |
|--|--|
| **~/.local/share/bluerig/syntax/Syntax** | Le regole dell'utente: ciò che l'utente ha aggiunto o vuole diverso |
| **{{pkgdatadir}}/syntax/Syntax** | Le regole installate, per i tipi di file che il file dell'utente non nomina |

Una voce dell'utente sostituisce una voce installata per i file a cui
corrisponde. I file nominati dalle righe
**include**
vengono cercati prima in
*~/.local/share/bluerig/syntax/*,
poi tra quelli installati. Il file dell'utente viene riletto ogni volta che
si apre un nuovo file nell'editor.

Una copia completa del file installato, come facevano le versioni
precedenti, nasconde ogni tipo di file installato dopo di essa, e la loro
evidenziazione smette di funzionare: nella copia tenere solo le voci
cambiate.

## La sezione file <a id="syntax-file"></a>

Un file di sintassi è fatto di sezioni. Ognuna comincia con una riga
**file**
e di solito sta in un file a sé con una riga
**include**:

```
file <file name> <description> [<first line>]
```

Il
*file name*
e la
*first line*
sono espressioni regolari: la sezione si applica a un file quando il suo
nome o la prima riga del suo testo corrispondono. La
*description*
è il nome della sintassi, con \s per uno spazio: le righe
**embed**
e
**overlay**
lo usano. Una sezione finisce dove comincia la successiva ed è divisa in
contesti con parole chiave.

Una sezione minima per un programma C:

```
file .\*\\.c C\sProgram\sFile (#include|/\\\*)

wholechars abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_

# default colors
define  comment   brown
context default
  keyword  whole  if       yellow
  keyword  whole  else     yellow
  keyword  whole  for      yellow
  keyword  whole  while    yellow
  keyword  whole  do       yellow
  keyword  whole  switch   yellow
  keyword  whole  case     yellow
  keyword  whole  static   yellow
  keyword  whole  extern   yellow
  keyword         {        brightcyan
  keyword         }        brightcyan
  keyword         '*'      green

# C comments
context /\* \*/ comment

# C preprocessor directives
context linestart # \n red
  keyword  \\\n  brightred

# C string constants
context " " green
  keyword  %d    brightgreen
  keyword  %s    brightgreen
  keyword  %c    brightgreen
  keyword  \\"   brightgreen
```

## Contesti <a id="syntax-contexts"></a>

Un contesto è una zona del testo con un colore proprio e parole chiave
proprie, come un commento C tra /\* e \*/. Comincia con la riga:

```
context [exclusive] [oneline] [whole|wholeright|wholeleft] [linestart] <delim>
        [linestart] <delim> [<foreground>] [<background>] [<attributes>]
```

Il primo contesto di una sezione è
**context default**
[*foreground*] [*background*] [*attributes*];
senza di esso la sezione dà un errore.

|  |  |
|--|--|
| linestart | Il delimitatore sta all'inizio di una riga |
| whole | Il delimitatore è una parola intera |
| wholeleft | Il delimitatore è su un confine di parola a sinistra |
| wholeright | Il delimitatore è su un confine di parola a destra |
| exclusive | Colora il testo tra i delimitatori, non i delimitatori |
| oneline | Dopo exclusive: termina il contesto alla fine della riga quando il delimitatore destro non è su di essa |

Una riga
**wholechars**
[**left**|**right**] *characters*
in qualsiasi punto del file fissa i caratteri di una parola intera; left e
right fissano i due lati separatamente. Una riga
**spellcheck**
dopo
**context**
passa le parole del contesto al controllo ortografico.

## Incorporamento e livelli <a id="syntax-embed"></a>

Una riga

**embed**
[**soft**]
*type*

dopo
**context**
passa il testo tra i delimitatori alle regole di un'altra sintassi;
*type*
è la descrizione della sua riga file, con \s per uno spazio. Il tipo viene
cercato nel file Syntax da cui si leggono le regole, poi in quello
installato con bluerig. Un tipo che non si trova in nessuno dei due lascia
il contesto nei suoi colori. I delimitatori mantengono i colori del
contesto. Il delimitatore destro termina il testo incorporato ovunque si
trovi, anche dentro una stringa o un commento delle regole incorporate. Una
sintassi incorporata può incorporarne un'altra, fino a pochi livelli. Per
esempio, HTML passa gli script alle regole di JavaScript:

```
  context <SCRIPT*> </SCRIPT> brightcyan
      embed JavaScript\sProgram
```

Con
**soft**
il delimitatore destro termina il testo incorporato solo fuori dalle
stringhe, dai commenti e dalle parole chiave delle regole incorporate, così
che in {{ "}}" }} di un template Jinja2 il primo }} resta parte della
stringa, e in ${x/\\}/y} di uno script di shell il primo } resta parte
dell'escape.

Una riga

**overlay**
*type*

tra le regole fa di tutta la sintassi un livello sopra
*type*:
i suoi contesti colorano ciò che contengono ovunque si trovi, e il resto del
testo viene letto dalle regole di
*type*,
che vedono il contenuto dei contesti come spazi vuoti. In questo modo i tag
di un linguaggio di template vengono colorati allo stesso modo nel testo,
dentro un tag HTML, in una stringa di un attributo e in uno script, e HTML
colora il resto. Il contesto predefinito di un livello non colora niente:
una parola chiave lì impedisce solo a un contesto di cominciare. Un tipo che
non si trova in nessun file Syntax lascia il livello sopra il testo
semplice. Per esempio, Smarty sopra HTML:

```
  overlay HTML\sFile
  context default
      keyword {\s
  context {\* \*} brown
  context { } brightcyan
      embed soft Smarty\sTag
```

## Parole chiave <a id="syntax-keywords"></a>

Una parola chiave di un contesto è data dalla riga:

```
keyword [whole|wholeright|wholeleft] [linestart] <string> [<foreground>] [<background>] [<attributes>]
```

Le stringhe delle parole chiave e dei delimitatori dei contesti si leggono
così:

|  |  |
|--|--|
| \\t, \\s | Tabulazione e spazio: gli spazi vuoti separano le parole della riga e non si possono usare così come sono |
| \\n, \\r | A capo e ritorno carrello |
| `\\` | Barra rovesciata |
| \\\* | Asterisco |
| \* | Qualsiasi carattere, in qualsiasi numero |
| + | Qualsiasi numero di caratteri di parola, anche nessuno |
| \\[abc\\] | Qualsiasi numero di caratteri dell'insieme |
| \\{abc\\} | Un carattere dell'insieme |

Un modello non può essere il primo o l'ultimo carattere di un delimitatore
di contesto. Per esempio,

```
  keyword         '*'      green
```

colora di verde le costanti C di un solo carattere, e

```
  keyword         "*"      green
```

colora le costanti stringa, purché non attraversino un a capo.

La riga

```
  keyword  \\\n  brightgreen
```

dà una parola chiave fatta di una barra rovesciata e un a capo. Le parole
chiave vengono confrontate prima dei delimitatori dei contesti, quindi questa
impedisce al contesto di finire alla fine di una riga con una barra
rovesciata, e una direttiva del preprocessore C continua su più righe.

Una riga
**define**
*name words...*
dà un nome che sta per queste parole nelle regole, come
**define comment brown**.

## Colori e attributi <a id="syntax-colors"></a>

I colori: black, gray, red, brightred, green, brightgreen, brown, yellow,
blue, brightblue, magenta, brightmagenta, cyan, brightcyan, lightgray e
white. "default" è il colore predefinito del terminale, "base" i colori
principali dell'editor; imposta gli attributi senza cambiare il colore di
sfondo. Con 256 colori ci sono anche da color16 a color255, da rgb000 a
rgb555 e da gray0 a gray23.

Il colore del testo può essere un token di scope invece di un colore: una
parola in maiuscolo che dice cos'è il testo invece di come appare:

```
keyword  whole  if   KEYWORD_CONTROL
context  " " STRING_QUOTED_DOUBLE
    keyword  \\"   CONSTANT_CHARACTER_ESCAPE
keyword  linestart TODO  KEYWORD_OTHER_TODO black
```

Il colore di un token viene dal tema, così un solo tema colora il programma
e il testo, e ogni linguaggio allo stesso modo; vedere
[Temi](#skins).
Un token si legge come uno scope: in minuscolo, con punti al posto dei
trattini bassi. KEYWORD_CONTROL_IMPORT è keyword.control.import, un tipo di
keyword.control, che è un tipo di keyword. Sono i nomi di scope di Sublime
Text e TextMate. Ciò che il tema non colora mantiene i colori che mcedit
dava a quel testo. Uno sfondo o attributi dopo un token prevalgono sui
colori di mcedit, come black qui sopra, ma non su uno sfondo o attributi che
il tema dà al token stesso. Un token funziona anche in un define, e sulle
righe number, string e symbols delle regole line-local. I file di sintassi
installati usano solo token; il loro elenco con i colori di mcedit è in
**{{pkgdatadir}}/syntax/README.txt**.

Un file di sintassi condiviso con
**cooledit**
dà i colori per entrambi con una barra, e bluerig prende il colore prima di
essa:

```
keyword  #include  red/Orange
```

Gli attributi sono bold, italic, underline, reverse e blink, più d'uno uniti
con un segno più.

## Altre righe <a id="syntax-other"></a>

|  |  |
|--|--|
| # ... | Un commento, su una riga a sé |
| caseinsensitive | All'inizio del file: maiuscole e minuscole non contano |
| line-local | La modalità per il testo semplice: si azzera a ogni a capo e al bordo dello schermo e legge solo il testo mostrato |
| number *max* *color* | In line-local: numeri decimali fino a *max* cifre |
| string *quote* *color* | In line-local: stringhe tra virgolette singole o doppie |
| symbols *characters* *color* | In line-local: segni di punteggiatura colorati uno per uno |

# Colori <a id="colors"></a>

bluerig cerca di capire se il terminale supporta i colori usando il database
dei terminali e il nome del terminale. A volte si sbaglia, quindi si può
forzare la modalità a colori o disattivarla con le opzioni -c e -b
rispettivamente.

Se il programma è compilato con il gestore dello schermo S-Lang invece di
ncurses, controlla anche la variabile
**COLORTERM,**
che, se è impostata, ha lo stesso effetto dell'opzione -c.

Si possono indicare terminali che forzano sempre la modalità a colori
aggiungendo la variabile
*color_terminals*
alla sezione Colors del file di inizializzazione. Questo impedisce a bluerig
di cercare di capire se il terminale supporta i colori. Esempio:

```
[Colors]
color_terminals=linux,xterm
color_terminals=terminal-name1,terminal-name2...
```

Il programma si può compilare sia con ncurses sia con S-Lang; ncurses non
offre un modo per forzare la modalità a colori: ncurses usa solo le
informazioni del database dei terminali.

# Temi <a id="skins"></a>

I colori del programma e le linee delle sue cornici vengono presi da un file
di tema. La finestra
[Aspetto](#appearance)
ne sceglie uno da un elenco.

Se il tema contiene definizioni true-color, si deve definire la chiave
'truecolors' con il valore TRUE nella sezione [skin]. Se non si usa il
true-color ma i 256 colori, si deve definire invece '256colors'.

Il tema è il primo indicato da:

```
1) command line option -S <skin>, --skin=<skin>
2) environment variable BLUERIG_SKIN
3) setting skin of the [bluerig] section of the ini file
4) the skin called default
```

Un nome può essere il percorso assoluto di un file di tema (con
l'estensione .ini o senza). Altrimenti il file del tema viene cercato
in (fino al primo trovato):

```
1) ~/.local/share/bluerig/skins/
2) {{sysconfdir}}/bluerig/skins/
3) {{pkgdatadir}}/skins/
```

Il formato dei file di tema è descritto in
**{{pkgdatadir}}/skins/README.txt**.
I colori dell'evidenziazione della sintassi vengono dalla sezione
**[syntax]**
del tema: lì uno scope nomina i colori di un tipo di testo, e uno scope e lo
scope del contesto in cui si trova, i due separati da uno spazio, nominano i
colori di quel tipo di testo in quel contesto. Vince lo scope più lungo che
il tema nomina:

```
[syntax]
    comment = gray
    keyword = yellow
    keyword.control = brightmagenta
    string = green
    string constant.character.escape = white;red
    invalid = white;red;bold
```

Un tipo di testo che il tema non nomina mantiene i colori che mcedit gli
dava, quindi un tema può nominare pochi tipi e lasciare il resto. Le regole
che nominano colori invece di token di scope mantengono i loro colori
qualunque sia il tema; vedere
[Evidenziazione della sintassi](#syntax-highlighting).

# Database dei terminali <a id="terminal-databases"></a>

bluerig offre un modo per correggere il database dei terminali del sistema
senza bisogno dei privilegi di root. La finestra
[Impara tasti](#learn-keys)
scrive ciò che apprende in
**~/.config/bluerig/term/\<TERM>**,
una riga per ogni tasto nella sua sezione
**[keys]**.
bluerig cerca anche nel file di sistema
**defaults.ini**
(in
**{{sysconfdir}}/bluerig**
o
**{{pkgdatadir}}**)
e nel file
**~/.config/bluerig/ini**
la sezione "terminal:nome-del-terminale" e poi la sezione
"terminal:general". Ogni riga della sezione contiene un simbolo di tasto da
definire, seguito da un segno di uguale e dalla definizione del tasto. Si
può usare la forma speciale \\e per rappresentare il carattere di escape e
^x per rappresentare il carattere control-x. Una riga
*copy=other*
prende anche ogni tasto della sezione "terminal:other".

I simboli di tasto possibili sono:

```
f0 to f20     Function keys f0-f20
bs            backspace
home          home key
end           end key
up            up arrow key
down          down arrow key
left          left arrow key
right         right arrow key
pgdn          page down key
pgup          page up key
insert        the insert character
delete        the delete character
complete      to do completion
```

Per esempio, per definire il tasto insert come Escape + [ + O + p, si
scrive questo nel file ini:

```
insert=\e[Op
```

Un simbolo di tasto può avere modificatori:

```
    ctrl-alt-right=\e[[1;6C
    ctrl-alt-left=\e[[1;6D
```

Questo significa che ctrl+alt+left invia una sequenza di escape \\e[[1;6D e
quindi bluerig interpreta "\\e[[1;6D" come Ctrl-Alt-Left.

Il simbolo di tasto
*complete*
rappresenta le sequenze di escape usate per avviare il completamento, che si
avvia con Alt-tab, ma si possono definire altri tasti per fare lo stesso
lavoro (su quelle tastiere piene di bei tasti inutilizzati).

# Script Lua <a id="lua-scripting"></a>

Dove bluerig è stato compilato con Lua, esegue script Lua che estendono
l'editor: azioni associate a tasti e a menu, reazioni agli eventi
dell'editor (un file viene aperto, salvato, un tasto viene premuto),
finestre di dialogo e viste a pieno schermo. Diversi comandi dell'editor
sono script di questo tipo: l'ordinamento delle righe, la formattazione del
paragrafo, l'inserimento dell'output di un comando, l'inserimento di un
carattere per il suo codice, l'inserimento della data, la decodifica base64
e il disegno di tabelle.

Gli script di sistema sono in
**{{pkgdatadir}}/lua/scripts/**,
e gli script dell'utente in
**~/.local/share/bluerig/lua/scripts/**,
una directory per ogni script, con il suo
**lua.ini**
e il suo
**init.lua**.
La finestra
[Lua scripts](#lua-scripts)
li attiva e li disattiva.

L'opzione
**--no-lua**,
o la variabile
**BLUERIG_NO_LUA**
impostata a 1, avvia l'editor senza nessuno script Lua, e
*enabled=false*
nella sezione
**[Lua]**
del file ini lo fa in modo permanente.

Come scrivere uno script, e le funzioni che uno script può chiamare, è
descritto in
**LUA_PLUGINS.md**
e
**LUA_API_REFERENCE.md**,
installati con la documentazione di bluerig.

# Impostazioni generali <a id="general-settings"></a>

Queste impostazioni della sezione
**[bluerig]**
del file ini riguardano il programma più che l'editor. Salvo dove detto
altrimenti, 1 attiva un'impostazione e 0 la disattiva.

*auto_save_setup*
: Scrive le impostazioni nel file ini ogni volta che l'editor termina.

*skin*
: Il nome del tema. Vedere [Temi](#skins).

*shadows*
: Disegna un'ombra sotto le finestre di dialogo e i menu a discesa.

*keymap*
: Il file dei tasti da usare. Vedere [Ridefinire i tasti](#keys_redefine).

*drop_menus*
: Apre subito il primo menu quando si preme F9, invece della sola barra dei
menu.

*mouse_close_dialog*
: Un clic fuori da una finestra di dialogo la chiude.

*mouse_repeat_rate, double_click_speed*
: La ripetizione automatica dei pulsanti del mouse, e il tempo entro cui due
clic fanno un doppio clic, in millisecondi.

*old_esc_mode, old_esc_mode_timeout*
: Con old_esc_mode attivo, un singolo Esc vale come Esc dopo
old_esc_mode_timeout microsecondi, invece di aspettare il tasto successivo.

*show_all_if_ambiguous*
: Mostra subito l'elenco dei completamenti possibili quando il completamento
è ambiguo. Vedere [Completamento](#completion).

*shell_patterns*
: I modelli delle condizioni del menu utente sono modelli della shell (1) o
espressioni regolari (0).

*num_history_items_recorded*
: Quante voci conserva ogni cronologia di una riga di immissione.

*confirm_history_cleanup*
: Chiede prima di svuotare una cronologia.

*clear_before_exec*
: Pulisce lo schermo prima che venga eseguito un comando del menu utente.

*pause_after_run*
: Aspetta un tasto dopo l'esecuzione di un comando del menu utente: mai (0),
solo sui terminali semplici (1, il predefinito) o sempre (2).

*alternate_plus_minus*
: Mantiene i tasti più e meno del tastierino numerico come più e meno.

*keybar_modifiers*
: Se vero, la barra dei tasti mostra i comandi di Shift, Ctrl o Alt con i
tasti F mentre si tiene premuto quel modificatore. Serve un terminale con il
protocollo di tastiera kitty: bluerig gli chiede allora ogni tasto con il
suo rilascio, e gli eventi di focus. Sugli altri terminali non ha effetto.
Falso in modo predefinito.

*verbose*
: Mostra l'avanzamento delle operazioni lunghe.

*keybar_visible*
: Nella sezione
**[Layout]**:
mostra le etichette dei tasti funzione in basso sullo schermo.

# Impostazioni nel file ini <a id="internal-file-editor-options"></a>

Con le finestre del menu
**Opzioni**
si fa la maggior parte delle impostazioni, e
**Salva configurazione**
le scrive nella sezione
**[bluerig]**
di
**~/.config/bluerig/ini**.
Queste sono le impostazioni dell'editor di quel file: 1 attiva
un'impostazione, 0 la disattiva, e il valore predefinito viene per ultimo.

|  |  |
|--|--|
| editor_tab_spacing | L'ampiezza di una tabulazione; 8 in modo predefinito. editor_fake_half_tabs dà le mezze tabulazioni |
| editor_fill_tabs_with_spaces | Inserisce spazi invece di un carattere di tabulazione; 0 |
| editor_return_does_auto_indent | Enter rientra come la riga non vuota più vicina sopra; 1 |
| editor_backspace_through_tabs | Backspace cancella tutto lo spazio vuoto fino al margine sinistro quando tra il cursore e il margine non c'è testo; 0 |
| editor_fake_half_tabs | Una tabulazione tra il margine sinistro e il testo avanza di 4 colonne, e il file riceve spazi e tabulazioni di 8 colonne; 1 |
| editor_option_save_mode | Come viene scritto il file, vedere [Modifica modalità salvataggio](#edit-save-mode): 0 - direttamente nel file, e un errore durante la scrittura lo lascia troncato; 1 - attraverso un file temporaneo; 2 - con una copia di sicurezza; 0 |
| editor_word_wrap_line_length | La lunghezza della riga per l'a capo; 72 |
| editor_backup_extension | Il suffisso del nome di una copia di sicurezza; ~ |
| editor_line_state | Mostra la colonna di stato delle righe: numeri, ripiegamenti, tratti nascosti; 0. Alt-n la commuta |
| editor_visible_spaces | Mostra gli spazi finali come punti; 1 |
| editor_visible_tabs | Mostra le tabulazioni come \<----\>; 1 |
| editor_show_control_chars | Mostra i caratteri di controllo come ^M, ^L, altrimenti come una cella vuota; 1. Una voce del menu Comandi la commuta |
| editor_persistent_selections | Mantiene la selezione dopo che il cursore si è mosso; 1 |
| editor_drop_selection_on_copy | Toglie la selezione dopo una copia nel file degli appunti; 1 |
| editor_cursor_beyond_eol | Lascia andare il cursore oltre la fine della riga; 0 |
| editor_cursor_after_inserted_block | Mette il cursore dopo un blocco inserito; 0 |
| editor_syntax_highlighting | Attiva l'evidenziazione della sintassi; 1 |
| editor_edit_confirm_save | Chiede prima di scrivere; 1 |
| editor_option_typewriter_wrap | Spezza la riga a editor_word_wrap_line_length mentre si scrive; 0 |
| editor_option_auto_para_formatting | Formatta il paragrafo sotto il cursore mentre si scrive; 0 |
| editor_option_save_position | Conserva la posizione nel file all'uscita; 1 |
| editor_group_undo | Annulla in una volta sola più azioni consecutive dello stesso tipo; 0 |
| editor_wordcompletion_collect_entire_file | Prende le parole per il completamento da tutto il file, non solo fino al cursore; 0 |
| editor_wordcompletion_collect_all_files | Prende le parole per il completamento da tutti i file aperti; 1 |
| editor_stop_format_chars | La formattazione dei paragrafi non tocca una riga che comincia con uno di questi caratteri, e le righe dopo di essa; -+\*\\,.;:&\> |
| editor_state_full_filename | Mostra nella riga di stato il percorso completo, non il nome del file; 0 |
| editor_filesize_threshold | La dimensione oltre la quale un file viene aperto dopo una domanda, con un suffisso K o M; 64M |
| editor_show_right_margin | Segna la colonna editor_word_wrap_line_length come margine destro; 0. Una voce del menu Comandi la commuta |
| editor_simple_statusbar | Mostra una riga di stato corta, senza la posizione in byte e il carattere sotto il cursore; 0 |
| editor_check_new_line | Al salvataggio chiede cosa fare se manca il fine riga finale; 0. Una casella della finestra della modalità di salvataggio la commuta |
| editor_sticky_windows | Ridimensiona come una sola le finestre le cui cornici si toccano, vedere [menu Finestra](#window-menu); 0 |

# AMBIENTE <a id="environment"></a>

Le variabili qui sotto sono quelle che bluerig legge. Variabili come
**TERM**, **SHELL**, **HOME** o **PATH** non sono elencate qui: il programma
le legge per sapere dove funziona, non per essere configurato da esse.

**BLUERIG_DATADIR**
: La directory da cui vengono presi i file di dati, al posto di
{{pkgdatadir}}. Vedere [FILE](#files).

**BLUERIG_PROFILE_ROOT**
: La radice dei file dell'utente, come percorso assoluto, al posto della
directory personale. Vedere [FILE](#files).

**BLUERIG_SKIN**
: Il tema da usare, per nome o per percorso. Vedere [Temi](#skins).

**BLUERIG_KEYMAP**
: Il file dei tasti da usare. Vedere [Ridefinire i tasti](#keys_redefine).

**BLUERIG_TMPDIR**, **TMPDIR**
: La directory dei file temporanei del programma, la prima delle due che è
impostata; /tmp se nessuna delle due lo è.

**BLUERIG_NO_LUA**
: Con il valore 1 avvia senza l'ambiente Lua. Non viene caricato nessuno
script Lua.

**KEYBOARD_KEY_TIMEOUT_US**
: Quanto aspettare il resto di una sequenza di escape, in microsecondi.

**COLORTERM**
: Letta quando vengono scelti i colori. Vedere [Colori](#colors).

**BLUERIG_LOG_ENABLE**, **BLUERIG_LOG_FILE**
: Un registro di debug viene scritto quando BLUERIG_LOG_ENABLE vale 1. Senza
**BLUERIG_LOG_FILE** si usa il file indicato da *logfile* nella sezione
*[Logging]* del file *ini*, e senza quella voce
*~/.cache/bluerig/bluerig.log*.

**BLUERIG_SPELL_LOG**
: Il file in cui il controllo ortografico scrive il suo registro di debug.
Non ha un interruttore proprio: il registro viene scritto quando la
variabile nomina un file.

# FILE <a id="files"></a>

I percorsi completi qui sotto possono variare da un'installazione all'altra.
Sui file di dati influisce anche la variabile d'ambiente
**BLUERIG_DATADIR**:
se è impostata, il suo valore viene usato al posto di {{pkgdatadir}} nei
percorsi qui sotto. I file dell'utente seguono le directory di base XDG:
*~/.config*,
*~/.local/share*
e
*~/.cache*
stanno per
**XDG_CONFIG_HOME**,
**XDG_DATA_HOME**
e
**XDG_CACHE_HOME**
dove queste sono impostate.

*{{pkgdatadir}}/help/bluerig.md*
: Il file dell'aiuto del programma, e accanto i file dell'aiuto dei plugin
dell'editor.

*{{sysconfdir}}/bluerig/bluerig.ini*, *{{pkgdatadir}}/bluerig.ini*
: I file di configurazione di sistema, usati solo se l'utente non ha un suo
file
**~/.config/bluerig/ini**.
Se {{sysconfdir}}/bluerig/bluerig.ini esiste, {{pkgdatadir}}/bluerig.ini non
viene usato.

*{{sysconfdir}}/bluerig/defaults.ini*, *{{pkgdatadir}}/defaults.ini*
: Le impostazioni globali, che valgono per tutti gli utenti, che abbiano o
no ~/.config/bluerig/ini. Per ora da esso vengono caricate solo le
[impostazioni del terminale](#terminal-databases).

*{{sysconfdir}}/bluerig/keymap.ini*, *{{pkgdatadir}}/keymap.ini*
: Le associazioni dei tasti di sistema. Vedere
[Ridefinire i tasti](#keys_redefine).

*{{sysconfdir}}/bluerig/bluerig.menu*, *{{pkgdatadir}}/bluerig.menu*
: Il [menu utente](#edit-menu-file) di sistema.

*{{sysconfdir}}/bluerig/spell.keymap, ctags.keymap*
: I tasti dei plugin spell e ctags; un file con lo stesso nome in
~/.config/bluerig li sostituisce.

*{{pkgdatadir}}/syntax/\**
: I file di sintassi di sistema, usati solo se manca il corrispondente file
dell'utente in
**~/.local/share/bluerig/syntax/**.

*{{pkgdatadir}}/skins/\**
: I temi forniti con il programma.

*{{pkgdatadir}}/charsets*
: L'elenco delle codifiche.

*{{pkgdatadir}}/lua/scripts/\**
: Gli script Lua forniti con il programma.

*{{pkgdatadir}}/examples/macros.d/\**
: Script di esempio per le macro.

*{{plugins_dir}}/*
: I plugin dell'editor che vengono caricati durante l'esecuzione.

*~/.config/bluerig/ini*
: La configurazione dell'utente. Se questo file esiste, la configurazione
viene caricata da qui invece che dal file di configurazione di sistema.

*~/.config/bluerig/keymap.ini*
: Le associazioni dei tasti dell'utente.

*~/.config/bluerig/menu*
: Il [menu utente](#edit-menu-file) personale.

*~/.config/bluerig/plugins.ini*
: I plugin e gli script Lua disattivati.

*~/.config/bluerig/term/\<TERM>*
: I tasti insegnati in [Impara tasti](#learn-keys).

*./.bluerig.menu*
: Il menu utente della directory corrente. Se questo file è presente nella
directory corrente, viene usato al posto del menu personale o di sistema.

*~/.local/share/bluerig/history*
: Le cronologie delle righe di immissione.

*~/.local/share/bluerig/filepos*
: I punti in cui i file sono stati lasciati, per
*editor_option_save_position*.

*~/.local/share/bluerig/clipboard*
: Il file degli appunti dell'editor.

*~/.local/share/bluerig/macros*
: Le [macro](#macros) registrate.

*~/.local/share/bluerig/macros.d/macro.N.sh*
: Gli script di shell delle macro.

*~/.local/share/bluerig/syntax/Syntax*
: Il file di sintassi dell'utente.

*~/.local/share/bluerig/skins/*
: I temi dell'utente.

*~/.local/share/bluerig/lua/scripts/*
: Gli script Lua dell'utente.

*~/.cache/bluerig/block*
: Il file del blocco su cui lavorano il menu utente e gli script delle
macro.

*~/.cache/bluerig/temp*
: Un file temporaneo dell'editor.

Per cambiare la directory radice dei file dell'utente si può usare la
variabile d'ambiente
**BLUERIG_PROFILE_ROOT**.
Il valore di BLUERIG_PROFILE_ROOT deve essere un percorso assoluto. Se
BLUERIG_PROFILE_ROOT non è impostata o è vuota, si usa la variabile HOME. Se
HOME non è impostata o è vuota, le directory vengono prese dalla libreria
GLib.

# LICENZA <!-- help:skip -->

Questo programma è distribuito secondo i termini della GNU General Public
License pubblicata dalla Free Software Foundation. Vedere l'aiuto interno
per i dettagli sulla licenza e sull'assenza di garanzia.

# DISPONIBILITÀ <a id="availability"></a>

L'ultima versione di questo programma si trova all'indirizzo
<https://github.com/blue-panels/bluerig/releases> .

# VEDERE ANCHE <a id="see-also"></a>

cooledit(1), aspell(1), hunspell(1), ctags(1), etags(1), gpm(1),
terminfo(5), sh(1).

# AUTORI <a id="authors"></a>

Paul Sheer (psheer@obsidian.co.za) è l'autore originale di cooledit, che
bluerig continua. Gli autori e i collaboratori sono elencati nel file AUTHORS
della distribuzione dei sorgenti.

# BUG <a id="bugs"></a>

I bug vanno segnalati all'indirizzo
<https://github.com/blue-panels/bluerig/issues> .

Fornire una descrizione dettagliata del bug, la versione del programma in
uso
*(bluerig -V*
mostra questa informazione), e il sistema operativo su cui si usa il
programma. Se il programma va in crash, una traccia dello stack è molto
gradita.
