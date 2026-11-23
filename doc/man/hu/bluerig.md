---
date: 2026. november
---

<!-- help:topics "Tartalomjegyzék" -->
# NÉV <!-- help:skip -->

bluerig - terminálos IDE.

# ALKALMAZÁSA <!-- help:skip -->

**bluerig**
[kapcsolók] [+sor] [fájl1[:sor]] [fájl2[:sor]] ...

# LEÍRÁS <a id="description"></a>

A
**bluerig**
terminálos IDE. A
**cooledit**
folytatása: ez Paul Sheer szerkesztője, amelynek terminálos változatát sok
éven át a GNU Midnight Commander belső szerkesztőjeként fejlesztették; a
bluerig önálló programként viszi tovább ezt a szerkesztőt. A
**brg**
ugyanennek a programnak a rövidebb neve.

Egyszerre több fájlt szerkeszt, mindegyiket a saját ablakában, és bináris
fájlokat is szerkeszt. Tud blokkot másolni, mozgatni, törölni, kivágni és
beilleszteni; billentyűnként visszavonni és újra végrehajtani; vannak
legördülő menüi; fájlt szúr be; vannak makrói; reguláris kifejezéssel keres és
cserél; van sorszűrője, összecsukása és könyvjelzői; Shift-nyilakkal jelöl ki
szöveget (ahol a terminál ezt ismeri); van oszlopblokkja; beszúró és felülíró
módja; automatikus behúzása; sortörése és bekezdésformázása; állítható a
tabulátor mérete; sok fájltípushoz van szintaxiskiemelése; van felhasználói
menüje, amely a szöveget shell-parancsokon vezeti át; vannak szerkesztő
bővítményei (címkék szerinti navigálás, helyesírás-ellenőrzés) és
Lua-szkriptjei.

A parancssorban megadott minden fájl a saját ablakában nyílik meg. Fájl nélkül
a szerkesztő üres ablakot nyit. Az
*editor_filesize_threshold*
értéknél (alapértelmezésben 64 MB) nagyobb fájl kérdés után nyílik meg, a
program nem utasítja el.

# KAPCSOLÓK <a id="options"></a>

*+sor*
: Ugrás a megadott számú sorra (a
*+*
jel és a szám közé ne tegyen szóközt). Több sorszám is megadható, de csak az
utolsó számít, és csak az első fájlra.

*fájl:sor[:]*
: A fájl megnyitása annál a sornál, ahogy a fordítóprogramok és a grep
megadják a helyet a fájlban. Ha létezik olyan fájl, amelynek a neve a
kettőspontot és a számot is tartalmazza, akkor az a fájl nyílik meg.

*-V, --version*
: A program verziójának kiírása.

*-f, --datadir*
: A bluerig adatfájljai beépített keresési útvonalának kiírása.

*-F, --datadir-info*
: Részletes adatok kiírása a bluerig által használt könyvtárakról: az
adatfájlokról, a felhasználói fájlokról és a bővítményekről.

*--configure-options*
: A program konfigurálásakor megadott kapcsolók kiírása.

*--no-lua*
: Indítás Lua futtatókörnyezet nélkül: egyetlen Lua-szkript sem töltődik be.

*-D, --debug=könyvtár*
: Az adott könyvtár projektjének megnyitása hibakeresésre: a projekt
legutóbbi fájlja, a projekt fája és jobbra a hibakereső panelje. A
fájlablakok a hibakereső billentyűit kapják: az F6 töréspontot tesz, az F5
lefordítja és elindítja a programot, első alkalommal elkészítve a
hibakeresési beállítását, az F7 és az F8 léptet, az F3 indexeli a projekt
szimbólumait; az F2, az F9 és az F10 marad a mentés, a menü és a kilépés, az
Alt-F5 pedig a panelre lép és vissza. A project, a build és a debugger
bővítmény kell hozzá, az F3-hoz a ctags is. A Drawing, a Format és a Plugins
menü nincs a menüsorban; a billentyűik maradnak, és a Beállítások menüben
megmarad a Manage plugins.

*-Q, --query*
: A fájl lekérdezéseinek futtatása adatbázisban: MongoDB-ben a mongosh,
PostgreSQL-ben a libpq révén; a válasz alul az Output, a Table és a History
fülön jelenik meg. A *MC_QUERY_KIND* értéke *mongo* vagy *postgres*; nélküle a
*MC_MONGO_URI* MongoDB-t jelent, a *.sql* fájl PostgreSQL-t. MongoDB-nél a
*MC_MONGO_URI* és a *MC_MONGO_DB* mondja meg, mihez kapcsolódjon, a
*MC_MONGOSH* a programot; az F5 a kijelölést futtatja, ha nincs, akkor a
kurzor körüli *// ---* sorok közötti részt, ha az sincs, az egész fájlt.
PostgreSQL-nél a libpq a kapcsolatot a *PGHOST*, *PGDATABASE*, *PGSERVICE* és
hasonló változókból veszi, a jelszót a *~/.pgpass* fájlból; az F5 a
kijelölést futtatja, ha nincs, az egész fájlt. A Ctrl-F5 az egész fájlt
futtatja. Ha nincs megadva fájl, a fájl a
*~/.local/share/bluerig/queries/* könyvtárban van, az adatbázis nevével és
*.js* kiterjesztéssel, illetve a *postgres/* könyvtárban, a kapcsolat
nevével, az adatbázis nevével és *.sql* kiterjesztéssel. A projekt legutóbbi
ablakai nem jelennek meg, és a projekt és a kód bővítményei (project, build,
debugger, ctags, etags, git, terminal, spell) nem indulnak el; a szkriptek
maradnak, és a lekérdezések nyelvi kiszolgálója is, ha a lsp.json megad
ilyet. A Drawing, a Format és a Plugins menü nincs a menüsorban, mint a -D
esetén. A query bővítmény kell hozzá.

*-G, --git=könyvtár*
: A git bővítmény Git ablakának megnyitása az adott könyvtár munkafáján; az
ablak bezárása kilép a szerkesztőből. A projekt legutóbbi ablakai nem
jelennek meg, és a projekt bővítményei (project, build, debugger, ctags,
etags) nem indulnak el. Fájlkezelőben git panelként használható: utána a
következők egyikével, vagy egymagában az aktuális könyvtárra:

*--git-log*
: A Git ablak a naplón.

*--git-show=commit*
: A Git ablak azon a commiton: a fájljai és azok eltérései.

*--git-commit*, *--git-amend*
: A Git ablak az előkészített (staged) változások commitjának üzenetével, vagy
az utolsó commit üzenetével, amelyet ezekkel módosít; a szerkesztő kilép,
amint az üzenetet menti és a commit elkészül, vagy ha bezárja az ablakot.

*--git-reword=commit*
: Ugyanígy új üzenet az adott commitnak, alatta a commit fájljaival.

*-h, -?, --help*
: A kapcsolók és jelentésük kiírása.

*--help-terminal, --help-color*
: Az alábbi terminál- vagy színkapcsolók kiírása.

A terminálkapcsolók:

*-x, --xterm*
: Az xterm mód kikényszerítése. Xterm-képes terminálokon használatos (két
képernyőmód, és egér-vezérlősorozatokat tud küldeni).

*-X, --no-x11*
: A módosítóbillentyűk állapotát ne az X Window rendszertől kérdezze.

*-g, --oldmouse*
: A terminál régi, kiemelő egérkövetésének használata.

*-d, --nomouse*
: Az egér kikapcsolása.

*-t, --termcap*
: A terminfo helyett a termcap adatbázis használata. A kapcsoló csak akkor
létezik, ha a bluerig az S-Lang könyvtárral készült.

*-s, --slow*
: Lassú terminál: a képernyő kevesebb frissítéssel rajzolódik.

*-a, --stickchars*
: A keretek rajzolása egyszerű karakterekkel a terminál vonalrajzoló
karakterei helyett.

*-K fájl, --keymap=fájl*
: A billentyűtársítások betöltése ebből a fájlból. Lásd
[A billentyűk átdefiniálása](#keys_redefine).

*--nokeymap*
: Semmilyen billentyűkiosztás-fájl ne töltődjön be: a programba épített
társítások érvényesek.

A színkapcsolók:

*-b, --nocolor*
: Fekete-fehér megjelenítés kikényszerítése.

*-c, --color*
: Színes mód kikényszerítése olyan terminálon, amely látszólag nem ismeri a
színeket.

*-S téma, --skin=téma*
: A megadott nevű téma (skin) használata. Lásd
[Témák](#skins).

# Áttekintés <a id="overview"></a>

A bluerig képernyőjén felül a
[menüsor](#menu-bar)
van, alatta a szerkesztőablakok, alul a funkcióbillentyűk feliratai. A
menüsor rejtve is lehet: az F9 vagy egy kattintás a képernyő felső sorára
előhozza.

Minden ablak egy fájlt mutat: a fájl neve, állapota és a kurzor helye az
ablak tetején lévő állapotsorban áll. Egyszerre több ablak is nyitva lehet; az
[Ablak menü](#window-menu)
lép közöttük, és mindegyik mozgatható és átméretezhető.

# A bluerig szerkesztő <a id="internal-file-editor"></a>

<a id="keys"></a>
**Billentyűk**

<!-- {{key:Action|F8}}: Action is the action name of misc/keymap.default.ini and src/keymap.c (Remove = f8); the help shows the keys bound to it now, the man page the default keys after the bar -->
Ezek az alapértelmezett billentyűk. Bármelyik megváltoztatható, lásd
[A billentyűk átdefiniálása](#keys_redefine).

<a id="moving-around"></a>
**Kurzormozgatás**

|  |  |
|--|--|
| {{key:Left|Left}} | Egy karakterrel balra |
| {{key:Right|Right}} | Egy karakterrel jobbra |
| {{key:Up|Up}} | Egy sorral feljebb |
| {{key:Down|Down}} | Egy sorral lejjebb |
| {{key:WordLeft|Ctrl-Left, Ctrl-z}} | Egy szóval balra |
| {{key:WordRight|Ctrl-Right, Ctrl-x}} | Egy szóval jobbra |
| {{key:ScrollUp|Ctrl-Up}} | A képernyő görgetése felfelé |
| {{key:ScrollDown|Ctrl-Down}} | A képernyő görgetése lefelé |
| {{key:PageUp|PgUp}} | Egy oldallal feljebb |
| {{key:PageDown|PgDn}} | Egy oldallal lejjebb |
| {{key:Home|Home}} | A sor eleje |
| {{key:End|End}} | A sor vége |
| {{key:Top|Ctrl-Home, Alt-\<}} | A fájl eleje |
| {{key:Bottom|Ctrl-End, Alt-\>}} | A fájl vége |
| {{key:TopOnScreen|Ctrl-PgUp}} | A képernyő teteje |
| {{key:BottomOnScreen|Ctrl-PgDn}} | A képernyő alja |

**Törlés**

|  |  |
|--|--|
| {{key:Delete|Delete, Ctrl-d}} | A karakter törlése |
| {{key:BackSpace|Backspace, Ctrl-h}} | A balra álló karakter törlése |
| {{key:DeleteLine|Ctrl-y}} | A sor törlése |
| {{key:DeleteToEnd|Ctrl-k}} | Törlés a sor végéig |
| {{key:DeleteToWordBegin|Alt-Backspace}} | Törlés a szó elejéig |
| {{key:DeleteToWordEnd|Alt-d}} | Törlés a szó végéig |

<a id="blocks"></a>
**Blokkok**

|  |  |
|--|--|
| Shift+kurzorbillentyűk | Blokk kijelölése |
| Alt+kurzorbillentyűk | Oszlopblokk kijelölése |
| {{key:Mark|F3}} | A blokk kijelölésének kezdete vagy vége |
| {{key:MarkColumn|F13}} | Oszlopblokk kezdete |
| {{key:MarkAll|Ctrl-a}} | Az egész szöveg kijelölése |
| {{key:Copy|F5}} | A blokk másolása a kurzorhoz |
| {{key:Move|F6}} | A blokk áthelyezése a kurzorhoz |
| {{key:Remove|F8}} | A blokk törlése |
| {{key:Store|Ctrl-Ins}} | A blokk másolása a vágólapfájlba |
| {{key:Cut|Shift-Del}} | A blokk áthelyezése a vágólapfájlba |
| {{key:Paste|Shift-Ins}} | A blokk beillesztése a vágólapfájlból |
| {{key:BlockSave|Ctrl-f}} | A blokk kiírása fájlba |

<a id="search-keys"></a>
**Keresés és csere**

|  |  |
|--|--|
| {{key:Search|F7}} | [Keresés](#search-and-replace) |
| {{key:SearchContinue|F17}} | Keresés tovább |
| {{key:Replace|F4}} | Csere |
| {{key:ReplaceContinue|F14}} | Csere tovább |
| {{key:FilterToggle|Alt-s}} | A sorszűrő levétele, vagy az utolsó keresés alkalmazása szűrőként |
| {{key:FilterWord|Alt-S}} | Szűrés a kijelölt szövegre vagy a kurzor alatti szóra |

**Ugrások és könyvjelzők**

|  |  |
|--|--|
| {{key:Goto|Alt-l}} | Ugrás sorra a száma alapján |
| {{key:MatchBracket|Alt-b}} | A zárójel párjára |
| {{key:Bookmark|Alt-k}} | Könyvjelző elhelyezése vagy törlése |
| {{key:BookmarkNext|Alt-j}}, {{key:BookmarkPrev|Alt-i}} | A következő vagy az előző könyvjelzőre |
| {{key:BookmarkFlush|Alt-o}} | Minden könyvjelző törlése |
| {{key:Find|Alt-Enter}} | A kurzor alatti név definíciójának megkeresése |
| {{key:FilePrev|Alt-Minus}}, {{key:FileNext|Alt-Plus}} | Vissza és előre azokon a helyeken, ahová a definíciókeresés ugrott |

<a id="files-and-windows"></a>
**Fájlok**

|  |  |
|--|--|
| {{key:Save|F2}} | A fájl mentése |
| {{key:SaveAs|F12, Ctrl-F2}} | [A fájl mentése más néven](#save-file-as) |
| {{key:EditNew|Ctrl-n}} | Új üres ablak |
| {{key:InsertFile|F15}} | Fájl beszúrása a kurzorhoz |
| {{key:History|Alt-E}} | A szerkesztett fájlok előzményei |
| {{key:SelectCodepage|Alt-e}} | [A szöveg kódolása](#codepages-translation) |
| {{key:Quit|F10, Esc}} | Az ablak bezárása; az utolsó bezárása kilép a szerkesztőből |

<a id="editing"></a>
<a id="macros-and-plugins"></a>
**Egyéb parancsok**

|  |  |
|--|--|
| {{key:InsertOverwrite|Insert}} | Beszúró vagy felülíró mód |
| {{key:Undo|Ctrl-u}} | Visszavonás |
| {{key:Redo|Alt-r}} | Újra |
| {{key:UndoHistory|Ctrl-t, Alt-U}} | A visszavonás előzményei |
| {{key:Complete|Alt-Tab}} | [A szó kiegészítése](#completion) a nyitott fájlok szavaiból |
| {{key:FoldToggle|Alt-F}} | Az egymáshoz tartozó zárójelek közötti blokk összecsukása vagy kinyitása |
| {{key:Sort|Alt-t}} | A kijelölt sorok rendezése |
| {{key:ExternalCommand|Alt-u}} | Egy shell-parancs kimenetének beszúrása |
| {{key:ParagraphFormat|Alt-p}} | A bekezdés formázása |
| {{key:InsertLiteral|Ctrl-q}} | A következő billentyű vagy egy U+XXXX kód beszúrása úgy, ahogy van |
| Ctrl-p | A kurzor alatti szó helyesírásának ellenőrzése |
| {{key:MacroStartStopRecord|Ctrl-r}} | [Makró](#macros) felvételének indítása vagy leállítása |
| {{key:MacroExplorer|Alt-M}} | [Macro Explorer](#macro-explorer) |
| {{key:ShowNumbers|Alt-n}} | A sorszámok megjelenítése vagy elrejtése |
| {{key:ShowTabTws|Alt-\_}} | A tabulátorok és a sorvégi szóközök megjelenítése vagy elrejtése |
| {{key:SyntaxOnOff|Ctrl-s}} | A szintaxiskiemelés be- vagy kikapcsolása |
| {{key:Help|F1}} | Súgó |
| {{key:Menu|F9}} | Menüsor |
| {{key:UserMenu|F11}} | [Felhasználói menü](#edit-menu-file) |
| {{key:Shell|Ctrl-o}} | A terminál megjelenítése vagy elrejtése |
| Ctrl-Alt-p | A Preview megjelenítése vagy elrejtése |
| {{key:Refresh|Ctrl-l}} | A képernyő újrarajzolása |

|  |  |
|--|--|
| Lásd még: | [Menük](#menu-bar) |
|  | [Keresés, csere és a sorszűrő](#search-and-replace) |
|  | [Összecsukás](#folding) |
|  | [Makrók](#macros) |
|  | [Felhasználói menü](#edit-menu-file) |
|  | [Bővítmények](#plugins) |
|  | [Szintaxiskiemelés](#syntax-highlighting) |
|  | [A szerkesztő beállításai](#editor-options) |
|  | [Beállítások az ini fájlban](#internal-file-editor-options) |

## A billentyűk átdefiniálása <a id="keys_redefine"></a>

|  |  |
|--|--|
| Beállítások \> [Key bindings](#key-bindings) | Saját billentyűk adása egy műveletnek; a **~/.config/bluerig/keymap.ini** fájlban maradnak meg |
| Beállítások \> [Billentyűk](#learn-keys) | A billentyűk vezérlősorozatainak megtanítása |
| Beállítások \> [Key sniffer](#key-sniffer) | Megmutatja, mit küld a terminál egy billentyűre, és melyik művelethez van társítva a billentyű |

Induláskor a bluerig a beépített kiosztást veszi, és fölé tölti a
**{{pkgdatadir}}/keymap.ini**,
majd a
**{{sysconfdir}}/bluerig/keymap.ini**
fájlt. Az a művelet, amelyet egy fájl megnevez, a fájlban megadott
billentyűket kapja, és a beépített billentyűi nem működnek tovább.

A csomag a
**keymap.default.ini**,
a
**keymap.emacs.ini**
és a
**keymap.vim.ini**
kiosztást telepíti a
**{{sysconfdir}}/bluerig**
könyvtárba, és a
**keymap.ini**
fájlt a
**keymap.default.ini**
fájlra mutató hivatkozássá teszi.

A
**--nokeymap**
kapcsoló kikapcsolja a fájlok betöltését, és csak a beépített kiosztás
működik.

A felhasználó billentyűkiosztás-fájlja az alábbiak közül az első, amelyet a
program megtalál:

```
1) command line option -K <keymap>, --keymap=<keymap>
2) environment variable BLUERIG_KEYMAP
3) the keymap setting in the [bluerig] section of the ini file
4) file ~/.config/bluerig/keymap.ini
```

Az első három nevet vagy abszolút útvonalat vár. Az a név, amely nem
**.keymap**
végű, megkapja ezt a kiterjesztést, és a fájlt ezekben a könyvtárakban
keresi, az első találat nyer:

```
1) ~/.config/bluerig/
2) {{pkgdatadir}}/
```

A
**-K**
kapcsoló, a
**BLUERIG_KEYMAP**
változó és a
**keymap**
beállítás a
**.keymap**
kiterjesztést adja a kiterjesztés nélküli névhez, ezért a csomag kiosztásai,
amelyek
**.ini**
végűek, így nem választhatók. Ha ezek egyikét szeretné, tegye a
**~/.config/bluerig/keymap.ini**
fájlt rá mutató hivatkozássá: ez a fájl töltődik be, ha a három közül egyik
sincs megadva.

```
ln -s {{sysconfdir}}/bluerig/keymap.vim.ini ~/.config/bluerig/keymap.ini
```

A billentyűkiosztás-fájl szakaszai:

|  |  |
|--|--|
| [editor] | Szerkesztő |
| [editor:xmap] | Az előtag utáni billentyűk, ExtendedKeyMap művelet |
| [dialog] | Párbeszédablakok |
| [menu] | Menük |
| [input] | Beviteli sorok |
| [listbox] | Listák |
| [radio] | Választógombok |
| [help] | Súgóablak |
| [mcterm] | Terminál |

Egy szakasz sora egy műveletet és annak billentyűit nevezi meg,
pontosvesszővel elválasztva:

```
[editor]
Save = f2
SaveAs = f12; ctrl-f2
```

## A beviteli sorok billentyűi <a id="input-line-keys"></a>

A párbeszédablakok beviteli sorai ezeket a billentyűket fogadják:

|  |  |
|--|--|
| Ctrl-a, Home | A sor eleje |
| Ctrl-e, End | A sor vége |
| Ctrl-b, Left | Egy karakterrel balra |
| Ctrl-f, Right | Egy karakterrel jobbra |
| Alt-f, Ctrl-Right | Egy szóval előre |
| Alt-b, Ctrl-Left | Egy szóval vissza |
| Ctrl-h, Backspace | A kurzor előtti karakter törlése |
| Ctrl-d, Delete | A kurzor alatti karakter törlése |
| Ctrl-@ | A jel elhelyezése |
| Ctrl-w | A jeltől a kurzorig tartó szöveg kivágása a pufferbe |
| Alt-w | A jeltől a kurzorig tartó szöveg másolása a pufferbe |
| Ctrl-y | A puffer beillesztése |
| Ctrl-k | Törlés a sor végéig |
| Ctrl-Insert | A kijelölés, vagy ha nincs, az egész sor másolása a vágólapfájlba |
| Shift-Delete | A kijelölés kivágása a vágólapfájlba |
| Shift-Insert | A vágólapfájl beillesztése egy sorként: a sortörésekből szóköz lesz |
| Alt-p, Alt-n | A sor előzményeinek előző vagy következő tétele |
| Alt-h | A sor [előzményei](#history-query) listaként |
| Alt-Backspace | A kurzor előtti szó törlése |
| Alt-Tab | Fájlnév, változó vagy felhasználónév [kiegészítése](#completion) |

## Általános mozgatóbillentyűk <a id="general-movement-keys"></a>

A súgóablak és a listák ugyanazt a mozgatókódot használják, így ugyanazokat a
billentyűket fogadják:

|  |  |
|--|--|
| Up, Ctrl-p | Egy sorral vissza |
| Down, Ctrl-n | Egy sorral előre |
| Page Up, Alt-v | Egy oldallal feljebb |
| Page Down, Ctrl-v | Egy oldallal lejjebb |
| Home, A1 | Az elejére |
| End, C1 | A végére |

A súgóablak ezeket a billentyűket is fogadja:

|  |  |
|--|--|
| b, Backspace | Egy oldallal feljebb |
| Space, f | Egy oldallal lejjebb |
| u, d | Fél oldallal feljebb vagy lejjebb |
| g, G | Az elejére vagy a végére |

# Egérkezelés <a id="mouse-support"></a>

A bluerig kezeli az egeret. Működik
**xterm(1)**
terminálban (akkor is, ha az xtermből telnet- vagy ssh-kapcsolaton át fut) és
a Linux konzolon, ha fut a
**gpm**
egérkiszolgáló.

A kattintás oda teszi a kurzort, ahová kattint, a húzás kijelöli a szöveget,
a kattintás egy menün, gombon vagy funkcióbillentyű-feliraton pedig kiválasztja
azt. A görgő görgeti a szöveget.

Az egérgombok alapértelmezett ismétlési ideje 400 ezredmásodperc. Ez az ini
fájl
*mouse_repeat_rate*
beállításával módosítható, és az az idő, amelyen belül két kattintás
dupla kattintásnak számít, a
*double_click_speed*
beállítással.

Ha a bluerig egérkezeléssel fut, a Shift billentyű nyomva tartásával a
terminál saját egérkezelése (szöveg kivágása és beillesztése) érhető el.

# Menüsor <a id="menu-bar"></a>

A menüsor az F9 megnyomására vagy a képernyő felső sorára kattintva jelenik
meg. A menüi: Fájl, Szerkeszt, Keresés, Parancs, Navigate (csak akkor, ha egy
bővítmény vagy szkript tesz bele valamit), Ablak, Plugins és Beállítások. Egy
Lua-szkript bármelyikbe tehet tételeket, vagy saját menüket.

## Fájl menü <a id="file-menu"></a>

**Megnyitás...**
: Fájl megnyitása új ablakban.

**Új**
: Üres ablak nyitása.

**Bezárás**
: Az ablak bezárása.

**History...**
: A korábban szerkesztett fájlok listája, hogy egyet újra megnyisson.

**Mentés, Mentés másként**
: A fájl kiírása a saját nevén vagy más néven. Lásd
[Mentés másként](#save-file-as).

**Fájl beszúrása..., Másolat fájlba...**
: Fájl beszúrása a kurzorhoz, vagy a kijelölt blokk kiírása fájlba.

**Felhasználói menü**
: A [felhasználói menü](#edit-menu-file).

**Névjegy...**
: A program verziója.

**Kilépés**
: Kilépés a szerkesztőből; a nem mentett fájlokról kérdez.

## Szerkeszt menü <a id="edit-menu"></a>

Vissza, Újra és az Undo History; váltás a beszúró és a felülíró mód között; a
parancsok, amelyek blokkot, oszlopblokkot vagy az egész fájlt jelölik ki, és
amelyek a kijelölést megszüntetik; a blokk másolása, áthelyezése és törlése;
másolás a vágólapfájlba, kivágás oda és beillesztés onnan; és ugrás a fájl
elejére és végére.

## Keresés menü <a id="search-menu"></a>

Keresés, Keresés tovább és Csere; a sorszűrő (Toggle line filter, Filter by
word; lásd [Keresés, csere és a sorszűrő](#search-and-replace)); és a
könyvjelzők: könyvjelző be- és kikapcsolása az aktuális soron, ugrás a
következőre vagy az előzőre, és mindegyik törlése.

## Parancs menü <a id="command-menu"></a>

**Ugrás sorra...**
: Ugrás a számával megadott sorra.

**Sorszámok**
: A sorszámok oszlopának megjelenítése vagy elrejtése.

**Ugrás zárójel párjára**
: Ugrás a kurzor alatti zárójel párjára.

**Szintaktikus szövegkiemelés ki/be, Toggle right margin, Toggle control characters**
: Ezek be- és kikapcsolása.

**Kódolás...**
: Az a kódlap, amelyben a szöveg íródott. Lásd
[Válasszon kódlapot](#codepages-translation).

**Fold / Unfold block, Unfold all**
: Lásd [Összecsukás](#folding).

**Képernyő frissítése**
: A képernyő újrarajzolása.

**Makró rögzítésének indítása/leállítása, Macro explorer..., Akciók felvétele/ismétlése**
: Lásd [Makrók](#macros).

## Navigate menü <a id="navigate-menu"></a>

A bővítmények parancsai: ugrás a deklarációra, a hivatkozások megkeresése, a
fájl szimbólumai és mások. Lásd
[Bővítmények](#plugins).

## Ablak menü <a id="window-menu"></a>

**Áthelyezés, Átméretezés**
: Az ablak áthelyezése vagy átméretezése a nyílbillentyűkkel; az Enter
befejezi.

**Teljes képernyő váltás**
: Az ablak váltása a teljes képernyő és a saját mérete között.

**Toggle sticky windows**
: Összetapadó ablakoknál az egymás mellett álló keretű ablakok együtt
méreteződnek: az ablak sarka elmozdítja azt az élt, amelyen áll, az él
mentén lévő összes ablakkal együtt, mindkét oldalon, és nem nyílik rés
közöttük. Ahol négy ablak találkozik, a sarok mindkét élt mozgatja. Az az
oldal, amely az átméretezés kezdetekor a képernyő szélén van, ott marad, az
ablak megtartja a legkisebb méretét, és az él megáll egy útjában álló
ablaknál, amely ezután a többivel együtt méreteződik. Az Esc minden ablakot
visszatesz. A címénél fogva húzott ablak egyedül mozog. A beállítás az ini
fájlban
**editor_sticky_windows**
néven marad meg.

**Következő, Előző**
: Ugrás a következő vagy az előző ablakra.

**Lista**
: A nyitott ablakok, hogy valamelyikre át lehessen lépni. Lásd
[Nyitott fájlok](#open-files).

Alattuk a bővítmények ablakai következnek: a projekt fája, a fordítás
kimenete, a hibakereső panelje és konzolja, a terminál, a hibakereső által
futtatott program terminálja, a Preview. Egy tétel megnyitja az ablakát,
előre hozza, ha egy másik mögött van, és bezárja, ha már elöl van; a nyitott
ablak *\** jelet kap. A kikapcsolt bővítménynek nincs tétele.

A bővítmények ablakai dokkokban állnak. A fa, a hibakereső panelje és a
Preview a jobb oldali oszlopba kerül, egymás alá, a képernyő teljes
magasságában. A konzol, a fordítás kimenete és a terminál az alsó sorba
kerül, a fájlok ablakai alá: a sort fülekként osztják meg: egy látszik, és a
címe felsorolja a neveiket. Egy névre kattintva, vagy a
**Ctrl-Alt-PgDn** és **Ctrl-Alt-PgUp** billentyűvel (*WindowTabNext* és
*WindowTabPrev* a kiosztásban) egy másik jelenik meg. A teljes képernyős
fájlablak kitölti a képernyő többi részét, és ismét az egész képernyőt, ha a
dokkok üresek. Egy dokkolt ablak átméretezése a dokkot méretezi át; az
áthelyezése kiveszi a dokkból.

**Layout...**
: Az ablakok elrendezései: a bővítmények mely ablakai vannak nyitva, melyik
dokkban, és mekkorák a dokkok. Az *Edit* csak a fájlokat tartalmazza, a
*Project* a projekt fáját, a *Debug* a fát és a hibakereső paneljét. A
**Use** így rendezi el az ablakokat; a **Save as...** a jelenlegi állapotukban
egy név alatt megőrzi őket, akár ezek egyikének nevén is; a **Töröl**
elfelejt egy megőrzött elrendezést, és visszajön a szerkesztő azonos nevű
elrendezése. A **Debug layout while debugging** bekapcsolásával a hibakereső
indítása a *Debug* szerint rendezi el az ablakokat, a leállítása pedig
visszaállítja őket, de az alsó fülek nyitva maradnak, hogy olvashatók legyenek.

A projekt ablakai a szerkesztő kilépésekor megmaradnak, és visszajönnek, ha a
szerkesztő a projekt egy fájljával indul.
A *bluerig --debug* a *Debug* elrendezéssel indul. Az elrendezések a
beállítások könyvtárában, a *layouts.ini* fájlban maradnak meg.

## Plugins menü <a id="plugins-menu"></a>

Azok a szerkesztő bővítmények, amelyeknek saját tételük van, például a
helyesírás-ellenőrző; az ablakaik az [Ablak menüben](#window-menu) vannak. A
[Manage plugins](#manage-plugins)
ablakban kikapcsolt bővítmény nincs a menüben.

## Beállítások menü <a id="options-menu"></a>

**Általános...**
: A [szerkesztő beállításai](#editor-options).

**Mentési mód...**
: Hogyan íródik ki a fájl. Lásd [Mentési mód](#edit-save-mode).

**Szintaktikus szövegkiemelés**
: A fájl szintaxisszabályainak kézi kiválasztása, vagy rábízása a fájl nevére
és első sorára.

**Megjelenítés...**
: A téma és az árnyékok. Lásd [Megjelenítés](#appearance).

**Billentyűk...**
: Lásd [Billentyűk](#learn-keys).

**Key bindings...**
: Lásd [Key bindings](#key-bindings).

**Key sniffer...**
: Lásd [Key sniffer](#key-sniffer).

**Manage plugins...**
: Lásd [Manage plugins](#manage-plugins).

**Plugin info...**
: Lásd [Plugin info](#plugin-info).

**Lua scripts...**
: Lásd [Lua scripts](#lua-scripts). A tétel csak akkor van meg, ha a bluerig
Luával készült.

**Szintaxisfájl szerkesztése**
: A felhasználó szintaxisfájljának,
*~/.local/share/bluerig/syntax/Syntax*,
megnyitása a szerkesztőben. Ha nem létezik, a program egy rövid, a
tartalmát leíró megjegyzéssel hozza létre, nem a rendszerfájl másolataként; a
rendszerfájl utána is beolvasódik. Lásd
[Szintaxiskiemelés](#syntax-highlighting).

**Menu file**
: A [felhasználói menü](#edit-menu-file) fájljának megnyitása a szerkesztőben.

**Beállítások mentése**
: A beállítások kiírása a
*~/.config/bluerig/ini*
fájlba. Ha az
*auto_save_setup*
beállítás be van kapcsolva, a szerkesztő minden kilépéskor kiírja őket.

### A szerkesztő beállításai <a id="editor-options"></a>

A Beállítások menü
**Általános...**
tétele nyitja meg a szerkesztő beállításainak párbeszédablakát. Minden
beállítás az ini fájlba íródik, lásd
[Beállítások az ini fájlban](#internal-file-editor-options).

**Tördelés**

|  |  |
|--|--|
| Nincs | A sorok nem tördelődnek |
| Dinamikus bekezdések | Gépelés közben formázza a kurzor alatti bekezdést |
| Sortörés | Gépelés közben a tördelési hossznál töri a sort |

**Tabulálás**

|  |  |
|--|--|
| Hamis fél-tabulátorok | A bal margó és a szöveg között a tabulátor a szélessége felével lép, és a fájlba szóközök és tabulátorok kerülnek |
| Backspace: tabokat egyszerre | A Backspace a bal margóig az egész behúzást törli, ha a kurzor és a margó között nincs szöveg |
| Tabulálás szóközökkel | Tabulátor karakter helyett szóközöket szúr be a következő tabulátorpozícióig |
| Tabulátor méret | A tabulátor karakter szélessége; alapértelmezésben 8 |

**Keyboard**

|  |  |
|--|--|
| Esc timeout, ms | Hány ezredmásodpercig várja az Esc a billentyű többi részét, mielőtt Escnek számít; alapértelmezésben 1000. Az ini fájlban ez az *old_esc_mode_timeout*, mikroszekundumban, és indításkor a **KEYBOARD_KEY_TIMEOUT_US** változó felülírja |

**További opciók**

|  |  |
|--|--|
| Új sor auto. behúzás | Az Enter a fölötte lévő sor behúzásával kezdi a sort |
| Mentés előtt megerősítés | A fájl kiírása előtt kérdez |
| Fájlbeli pozíció mentése | A fájl ott nyílik meg, ahol legutóbb elhagyták |
| Látható sorvégi szóközök | Megjelöli a sorok végén álló szóközöket |
| Látható tabulátorok | Megjelöli a tabulátor karaktereket |
| Show control characters | A vezérlőkaraktereket ^M alakban mutatja, nem rejti el |
| Szintaktikus szövegkiemelés | A szöveget a típusa szintaxisszabályai szerint színezi |
| Kurzor a beillesztett blokk után | A kurzort a beszúrt blokk végén hagyja, nem az elején |
| Tartós kijelölés | A kurzor mozgatásakor megtartja a kijelölést |
| Kurzor a sor vége mögött | A kurzor a sor utolsó karakterén túl is mehet |
| Csoportos visszavonás | Az egyforma változtatások sorozatát egyszerre vonja vissza |
| Sortörési pozíció | Az oszlop, ahol a tördelési módok törik a sort; alapértelmezésben 72 |

### Mentési mód <a id="edit-save-mode"></a>

Hogyan íródik ki a fájl, ugyanaz a három mód, amelyet az
*editor_option_save_mode*
beállítás felvehet:

**Gyors mentés**
: Közvetlenül felülírja a fájlt. Gyors, de egy közbeni hiba félig kiírt
fájlt hagy hátra.

**Biztonságos mentés**
: Előbb ideiglenes fájlba ír, és amikor az teljesen elkészült, átnevezi az
eredetire, így egy hiba érintetlenül hagyja az eredetit.

**Biztonsági másolatok az alábbi kiterjesztéssel**
: Biztonságos mentés, és az eredeti a saját nevén megmarad, a beviteli sor
kiterjesztésével kiegészítve, alapértelmezésben "~". A második mentés a
biztonsági másolatot is lecseréli.

**POSIX soremelés vizsgálata**
: Kiírás előtt rákérdez, ha a fájl végéről hiányzik az újsor karakter.

### Megjelenítés <a id="appearance"></a>

A téma, amellyel a program rajzolódik, és hogy a párbeszédablakoknak és a
legördülő menüknek van-e árnyéka.

**Skin**
: A gomb megnyitja a telepített és a felhasználói témák listáját; a
kiválasztott azonnal érvénybe lép.

**Shadows**
: Árnyék rajzolása minden párbeszédablak és legördülő menü alá. Színek nélkül
nincs árnyék.

A választás az ini fájl
*skin*
és
*shadows*
beállításába íródik. A témafájlokról magukról lásd a
[Témák](#skins)
szakaszt.

### Billentyűk <a id="learn-keys"></a>

Ez a párbeszédablak megtanítja a bluerignek azokat a vezérlősorozatokat,
amelyeket a terminál a funkcióbillentyűkre, a nyilakra és a
navigációs billentyűkre küld. A címe a terminált nevezi meg, a
**TERM**
változó alapján.

A jelölőnégyzetekkel válasszon módosító kombinációt (Ctrl, Alt, Shift), majd
nyomja meg egy billentyű gombját. Nyomja le a billentyűt a billentyűzeten, és
várja meg, amíg az elfogásról szóló üzenet eltűnik. A megtanult sorozat a gomb
mellett jelenik meg. Az olyan sorozatot, amely már egy másik billentyűé,
ütközésként jelzi.

**Del**
: Annak a megtanult billentyűnek a törlése, amelyen a kurzor áll.

**Mentés**
: A megtanult billentyűk kiírása a
*~/.config/bluerig/term/\<TERM>*
fájlba.

**Edit term file**
: Ennek a fájlnak a megnyitása a szerkesztőben.

### Key bindings <a id="key-bindings"></a>

A program műveletei azokkal a billentyűkkel, amelyekre válaszolnak,
táblázatban. A felül lévő gombok választják ki a csoportot, Editor vagy
Widgets, majd annak részét: Main és Ctrl-X a szerkesztőnél; Dialog, Menu,
Input, Listbox, Radio és Help a vezérlőelemeknél. A Back visszavisz a
csoportokhoz. A \* jellel jelölt műveletek billentyűi eltérnek az
alapértelmezettől.

**Enter**
: A művelet billentyűjének cseréje: nyomja le a hozzárendelendő billentyűt.

**F5**
: Újabb billentyű hozzáadása a művelethez.

**F8, Del**
: A billentyű eltávolítása.

**Mentés**
: A változtatások kiírása a
*~/.config/bluerig/keymap.ini*
fájlba.

**Edit keymap file**
: A
*keymap.ini*
megnyitása a szerkesztőben.

**Edit term file**
: A terminál billentyűsorozatainak megnyitása, abban a fájlban, amelyet a
[Billentyűk](#learn-keys)
ír.

A más művelethez tartozó billentyűt jelzi, és megkérdezi, elvegye-e onnan.

### Key sniffer <a id="key-sniffer"></a>

Nyomja meg a
**Capture key**
gombot, majd egy tetszőleges billentyűt. A párbeszédablak megmutatja, mi
érkezett:

```
Shortcut   the symbolic name, e.g. Ctrl-F5
Action     the action of the key in the editor keymap
Raw        the escape sequence and its bytes in hex
Keycode    the internal numeric code
```

Ide érdemes nézni, ha egy billentyű nem csinál semmit, vagy mást csinál: a
billentyűt, amely a program számára ismeretlen sorozatként érkezik, a
[Billentyűk](#learn-keys)
ablakban kell megtanítani, azt pedig, amely megérkezik, de nincs művelete, a
[Key bindings](#key-bindings)
ablakban kell társítani.

### Manage plugins <a id="manage-plugins"></a>

A program által betöltött bővítmények táblázatban: a fajta, a név és amit a
bővítmény magáról mond. Itt szerepelnek a szerkesztő bővítményei, és ahol a
bluerig Luával készült, a Lua-motor is, mint
*runtime*
fajtájú bővítmény. A bővítmény a sorában lévő jelölőnégyzettel kapcsolható ki
és be, és ami ki van kapcsolva, az a következő alkalommal sem töltődik be; a
választás a
*~/.config/bluerig/plugins.ini*
fájlban marad meg. A kikapcsolt bővítmény a listában marad, hogy újra be
lehessen kapcsolni.

**Enter, F4**
: Annak a bővítménynek a beállításait nyitja meg, amelyen a kurzor áll. Ha a
bővítménynek nincs beállítása, ezt jelzi. A Lua-motor sorában a
[Lua scripts](#lua-scripts)
ablakot nyitja meg.

**IDE: project, build, debugger**
: Az a három bővítmény, amely IDE-t csinál a szerkesztőből, együtt kapcsolható
be, vagy együtt ki, ha mind a három be van kapcsolva: a projekt és a
fájljai, a fordítás és a hibakereső. Mindegyik a többi nélkül is működik.

### Plugin info <a id="plugin-info"></a>

A szerkesztő által betöltött bővítmények: a név, be van-e kapcsolva, mit nyújt
és mit csinál. Ez megtekintésre szolgáló lista; a bővítményt a
[Manage plugins](#manage-plugins)
ablakban lehet kikapcsolni és a beállításait megnyitni.

### Lua scripts <a id="lua-scripts"></a>

A megtalált Lua-szkriptek, a rendszerszintűek és a felhasználóéi,
táblázatban: be van-e kapcsolva, hol van a szkript (global vagy user), az
azonosítója, a munkaterülete, mit nyújt és mit csinál. A szkript a sorában
lévő jelölőnégyzettel kapcsolható ki és be; a változás a bluerig következő
indításakor lép érvénybe.

**Settings**
: Annak a szkriptnek a futtatása, amely a csomag beállításait tartalmazza, ha
van ilyen.

A szkriptekről magukról lásd [Lua-szkriptek](#lua-scripting).

# Keresés, csere és a sorszűrő <a id="search-and-replace"></a>

A {{key:Search|F7}} megnyitja a keresés párbeszédablakát.

|  |  |
|--|--|
| Normál | A szöveget úgy keresi, ahogy van |
| Reguláris kifejezés | Reguláris kifejezés szerint keres |
| Hexadecimális | Hexadecimális számokkal megadott bájtokat keres |
| Wildcard keresés | \* és ? jeleket tartalmazó shell-minta szerint keres |
| Kis-/nagybetű számít | Megkülönbözteti a kis- és a nagybetűket |
| Visszafelé | A kurzortól a fájl elejéig keres |
| Kijelöltben | Csak a kijelölt blokkban keres |
| Csak teljes szót | Csak teljes szóra illeszkedik |
| Minden karakterkészlet | Minden kódolásban keres, amelyben a fájl íródhatott |
| Mindet | Minden találatot tartalmazó sorra könyvjelzőt tesz |
| Filter | Elrejti a találat nélküli sorokat |

A {{key:Replace|F4}} megnyitja a csere párbeszédablakát: ugyanazok a mezők,
és az "Új szöveg" sor. Reguláris kifejezéses cserénél a \\1 - \\9 a talált
csoportokat szúrja be.

|  |  |
|--|--|
| Csere | Lecseréli ezt a találatot, és keresi a következőt |
| Mindet | Kérdés nélkül lecserél minden találatot |
| Kihagyás | Meghagyja ezt a találatot, és keresi a következőt |

**Filter**

Elrejti a találat nélküli sorokat; a sorszámok nem változnak, és a
sorállapot-oszlop megjelöl minden elrejtett szakaszt. A sorok egyszer
rejtődnek el, a gomb megnyomásakor: az utána megváltoztatott vagy beírt sor
látható marad, akkor is, ha nem illeszkedik.

|  |  |
|--|--|
| {{key:FilterToggle|Alt-s}} | Leveszi a szűrőt, vagy az utolsó keresést alkalmazza szűrőként; ha nem volt keresés, megnyitja a keresés párbeszédablakát |
| {{key:FilterWord|Alt-S}} | A kijelölt szövegre vagy a kurzor alatti szóra szűr |
| Parancs \> Unfold all | Leveszi a szűrőt és minden összecsukást |

# Összecsukás <a id="folding"></a>

Az összecsukás sorok egy szakaszát rejti el az első sora mögé, amely a
képernyőn marad, egy jellel a sorállapot-sávban. Az
**Alt-F**
(a Parancs menü Fold / Unfold block tétele) a kijelölt sorokat csukja össze,
vagy ha nincs kijelölés, az aktuális soron lévő zárójeltől a párjáig tartó
sorokat. Az összecsukott rész első során kinyitja azt. Az
**Unfold all**
a fájl minden összecsukását kinyitja, és leveszi a sorszűrőt.

# Makrók <a id="macros"></a>

A {{key:MacroStartStopRecord|Ctrl-r}}
elindítja a makró felvételét: rögzíti az utána lenyomott billentyűket. A
**Ctrl-r**
újabb megnyomása befejezi a felvételt, és várja azt a billentyűt, amelyhez a
makró tartozik; ezután az a billentyű futtatja.

A makrók a
**~/.local/share/bluerig/macros**
fájl
**[editor]**
szakaszában maradnak meg, és egy makró a sorának ottani törlésével, vagy a
[Macro Explorer](#macro-explorer)
ablakban törölhető.

A Parancs menü
**Akciók felvétele/ismétlése**
tétele felveszi az utána következő műveleteket, amíg újra ki nem választják,
majd megkérdezi, hányszor ismételje meg őket.

Billentyűhöz shell-szkript is rendelhető, a
**macros**
fájl egy ilyen sorával:

```
[editor]
Ctrl-W=ExecuteScript:25;
```

Ez azt jelenti, hogy a Ctrl-W az
*ExecuteScript(25)*
műveletet futtatja, amely a
**~/.local/share/bluerig/macros.d/macro.25.sh**
shell-szkriptet futtatja. A szkriptek a
**~/.local/share/bluerig/macros.d/**
könyvtárban vannak, és a nevük
**macro.XXXX.sh**
kell legyen, ahol az
**XXXX**
egy 0 és 9999 közötti szám. Példák a
**{{pkgdatadir}}/examples/macros.d/**
könyvtárba települnek. A szkript formátuma a
[felhasználói menü](#edit-menu-file)
tételeiével azonos, és ugyanazokat a helyettesítéseket fogadja. Az első sora
lehet

*#silent*
: A szkript a paramétereinek bekérése nélkül fut.

Íme egy példaszkript:

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

A felvett makrók, azzal a billentyűvel, amelyre mindegyik válaszol, és azzal,
amit csinál. A gombok:

**Run**
: Lejátssza azt a makrót, amelyen a kurzor áll.

**Töröl**
: Kérdés után eltávolítja.

**Fájl szerkesztése**
: Megnyitja azt a fájlt, amelyben a makrók vannak, és amelyet a
[Makrók](#macros)
szakasz leír.

# Felhasználói menü <a id="edit-menu-file"></a>

A {{key:UserMenu|F11}}
megnyitja a felhasználói menüt: minden tétele egy shell-szkript, amely a
szerkesztő szövegén fut. A menü az alábbiak közül az első megtalált fájlból
olvasódik be:

```
1) ./.bluerig.menu in the current directory
2) ~/.config/bluerig/menu
3) ~/bluerig.menu
4) {{sysconfdir}}/bluerig/bluerig.menu
5) {{pkgdatadir}}/bluerig.menu
```

Az aktuális könyvtár fájlja csak akkor olvasódik be, ha a felhasználóé vagy a
rooté, és más nem írhatja: a tételei parancsokat futtatnak. A Beállítások
menü
**Menu file**
tétele megnyitja a menüfájlt a szerkesztőben.

Az a sor, amely nem szóközzel vagy tabulátorral kezdődik, menütétel, az első
karaktere a tétel gyorsbillentyűje. A szóközzel vagy tabulátorral kezdődő
sorok a tétel parancsai. A '#' jellel kezdődő sor megjegyzés.

Ha egy tételt kiválasztanak, a parancsai egy ideiglenes fájlba íródnak, a
bennük lévő helyettesítő jelek helyére az értékük kerül, és a shell ezt a
fájlt futtatja:

|  |  |
|--|--|
| %f, %p, %s | Az aktuális fájl neve |
| %n | Az aktuális fájl neve a kiterjesztése nélkül |
| %x | Az aktuális fájl nevének kiterjesztése |
| %d | Az aktuális könyvtár |
| %c | A kurzor helye a fájlban |
| %i | Szóközökből álló behúzás, olyan széles, mint a kurzor oszlopa |
| %y | Az aktuális fájl szintaxistípusa |
| %b | A blokkfájl: a kijelölt szöveg a futtatás előtt a **~/.cache/bluerig/block** fájlba íródik, és amit a parancsok ott hagynak, az lép a blokk helyére |
| %m | A menüfájl neve |
| %% | A % karakter |
| %{text} | Beviteli sor "text" kérdéssel; a beírt szöveg kerül a helyére, az Esc vagy az F10 megszakítja a tételt |
| %view | A parancsok szabványos kimenetét új szerkesztőablakban nyitja meg; az utána kapcsos zárójelben álló szavak, mint a %view{ascii}, kimaradnak |

A helyettesítő betűknél a kis- és nagybetű nem számít. A % utáni szám
bekapcsolja (ez az alapértelmezés) vagy kikapcsolja az idézést: a %f és a %1f
idézőjelbe teszi a beszúrt értéket, a %0f nem.

Egy példa menüfájl:

```
s       Sort the block
	sort %b > %b.tmp && mv -f %b.tmp %b

w       Count the words of the file
	%view wc %f

m       Run make in the directory of the file
	make
```

**Alapértelmezett tétel**

Az első oszlopban '=' jellel kezdődő sor az alatta álló tétel feltétele: ha a
feltétel igaz, a tétel lesz az alapértelmezett.

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

A minta shell-minta vagy reguláris kifejezés, a
*shell_patterns*
beállítás szerint; a menüfájl első soraként egy "shell_patterns=0" vagy
"shell_patterns=1" sor erre a fájlra állítja be.

A típus egy vagy több karakter:

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

Például az 'rlf' közönséges fájl, hivatkozás vagy FIFO.

Az a feltétel, amely '=' helyett '=?' jellel kezdődik, minden kiértékelésekor
hibakeresési nyomkövetést mutat.

A feltételek balról jobbra értékelődnek ki; a '&' nem köt erősebben, mint a
'|':

```
	= f *.c | f *.h & y C
```

kiértékelése:

```
	( (f *.c) | (f *.h) ) & (y C)
```

**Hozzáadási feltételek**

A '+' (vagy '+?') jellel kezdődő feltétel csak akkor teszi a tételt a menübe,
ha igaz. A '+=' vagy '=+' (nyomkövetéssel '+=?' és '=+?') jelű feltétel
hozzáadási és alapértelmezési feltételként is működik. Egy tétel előtti két
feltételsor, egy '+' és egy '=' jelű, külön adja meg a kettőt.

# Bővítmények <a id="plugins"></a>

{{plugins}}
A bővítmény a Beállítások menü
[Manage plugins](#manage-plugins)
tételében kapcsolható be és ki.

# Párbeszédablakok <a id="dialogs"></a>

## Mentés másként <a id="save-file-as"></a>

A név, amelyen a fájl kiíródik, és a sortörések, amelyekkel kiíródik: ahogy a
fájlban vannak, Unix (LF), Windows és DOS (CR LF), vagy Macintosh (CR). A
választás erre a mentésre érvényes; a fájl azt tartja meg, amit kap.

## Nyitott fájlok <a id="open-files"></a>

A szerkesztőben nyitott fájlok, soronként egy. Az Enter arra a fájlra lép,
amelyen a kurzor áll, az Esc a megjelenített fájlt hagyja a helyén. Ugyanez a
lista az Ablak menü
**Lista**
tétele.

## Képernyőválasztó <a id="screen-selector"></a>

A szerkesztőn kívül egy Lua-szkript saját képernyőket is nyithat: teljes
képernyős nézeteket, amelyek nyitva maradnak, miközben visszatér a
szerkesztőbe. A
képernyők között háromféleképpen lehet váltani:

**Alt-}**
: váltás a következő képernyőre;

**Alt-{**
: váltás az előző képernyőre;

**Alt-\`**
: párbeszédablak megnyitása a nyitott képernyők listájával, hogy egyet
kiválasszon.

## Kiegészítés <a id="completion"></a>

A {{key:Complete|Alt-Tab}}
a szerkesztőben a kurzor előtti szót egészíti ki a nyitott fájlok szavaiból,
és listát mutat, ha több szó is illik. Az ini fájl
*editor_wordcompletion_collect_all_files*
beállítása, amely alapértelmezésben be van kapcsolva, minden nyitott fájlból
vesz szavakat; kikapcsolva csak az aktuális fájlból.

Az
**Alt-Tab**
egy párbeszédablak beviteli sorában a szöveget egészíti ki:

|  |  |
|--|--|
| $név | Környezeti változó |
| ~név | Felhasználónév |
| Egyébként | Fájlnév |

Ahol több is illik, a program sípol. Az ini fájl
*show_all_if_ambiguous*
beállítása, amely alapértelmezésben ki van kapcsolva, a listájukat a kurzor
mellett mutatja: az
**Enter**
kiválaszt egyet, a beírt betűk szűkítik a listát, az Esc, az F10, a Left és a
Right elrejti.

## Válasszon kódlapot <a id="codepages-translation"></a>

A program által ismert kódlapok listája, a
**{{pkgdatadir}}/charsets**
fájlból. Egy kódlap kiválasztása megmondja a szerkesztőnek, milyen kódlapon
íródott a fájl szövege, és a szöveg a képernyőre ebből a terminál kódlapjára
kódolódik át; a
**\<No translation>**
bájtokként hagyja. A listát az
**Alt-e**
és a Parancs menü Kódolás tétele nyitja meg.

## Előzmények <a id="history-query"></a>

Annak a listája, amit korábban egy beviteli sorba beírtak, a legújabb elöl;
az
**Alt-h**
nyitja meg annak a sornak, amelyben a kurzor van. Az Enter a sorba veszi azt
a tételt, amelyen a kurzor áll, az Esc a sort úgy hagyja, ahogy volt, az
**F8, Del**
pedig eltávolítja az előzményekből azt a tételt, amelyen a kurzor áll. Az
előzmények a
*~/.local/share/bluerig/history*
fájlban maradnak meg; a
*num_history_items_recorded*
mondja meg, hány tételt őriz meg mindegyik.

# Szintaxiskiemelés <a id="syntax-highlighting"></a>

A {{key:SyntaxOnOff|Ctrl-s}}
be- és kikapcsolja a szintaxiskiemelést. A kiemelés szabályai a
szintaxisfájlokban vannak: ott kapják a színeiket a kulcsszavak és a
kontextusok, például a megjegyzések és a karakterláncok.

|  |  |
|--|--|
| [Szintaxisfájlok](#syntax-files) | Hol vannak a szabályok, és milyen sorrendben olvasódnak be |
| [A file szakasz](#syntax-file) | Mely fájlokat emel ki egy szabályszakasz |
| [Kontextusok](#syntax-contexts) | A szöveg saját színű tartományai |
| [Beágyazás és rétegek](#syntax-embed) | Másik szintaxis szabályai egy kontextuson belül és a szöveg fölött |
| [Kulcsszavak](#syntax-keywords) | Egy kontextuson belül színezett szavak és minták |
| [Színek és attribútumok](#syntax-colors) | Színek, a téma hatókör-tokenjei, attribútumok |
| [Egyéb sorok](#syntax-other) | Megjegyzések, caseinsensitive, line-local |

## Szintaxisfájlok <a id="syntax-files"></a>

|  |  |
|--|--|
| **~/.local/share/bluerig/syntax/Syntax** | A felhasználó szabályai: amit a felhasználó hozzáadott, vagy másképp szeretne |
| **{{pkgdatadir}}/syntax/Syntax** | A telepített szabályok, azokhoz a fájltípusokhoz, amelyeket a felhasználói fájl nem nevez meg |

A felhasználói bejegyzés felülírja a telepítettet azoknál a fájloknál,
amelyekre illeszkedik. Az
**include**
sorokban megnevezett fájlokat előbb a
*~/.local/share/bluerig/syntax/*
könyvtárban keresi, aztán a telepítettek között. A felhasználói fájl minden
alkalommal újra beolvasódik, amikor egy új fájl nyílik meg a szerkesztőben.

A telepített fájl teljes másolata, ahogy a régebbi változatok készítették,
elrejt minden utána telepített fájltípust, és azok kiemelése nem működik: a
másolatban csak a megváltoztatott bejegyzéseket tartsa meg.

## A file szakasz <a id="syntax-file"></a>

A szintaxisfájl szakaszokból áll. Mindegyik egy
**file**
sorral kezdődik, és többnyire saját fájlba kerül, egy
**include**
sorral:

```
file <file name> <description> [<first line>]
```

A
*file name*
és a
*first line*
reguláris kifejezés: a szakasz akkor vonatkozik egy fájlra, ha a neve vagy a
szövegének első sora illeszkedik. A
*description*
a szintaxis neve, a szóköz helyén \s jellel: az
**embed**
és az
**overlay**
sor ezt használja. A szakasz ott ér véget, ahol a következő kezdődik, és
kulcsszavakat tartalmazó kontextusokra oszlik.

Egy egyszerű szakasz C programhoz:

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

## Kontextusok <a id="syntax-contexts"></a>

A kontextus a szöveg egy tartománya saját színnel és saját kulcsszavakkal,
például egy C megjegyzés a /\* és a \*/ között. Ezzel a sorral kezdődik:

```
context [exclusive] [oneline] [whole|wholeright|wholeleft] [linestart] <delim>
        [linestart] <delim> [<foreground>] [<background>] [<attributes>]
```

A szakasz első kontextusa a
**context default**
[*foreground*] [*background*] [*attributes*];
nélküle a szakasz hibát ad.

|  |  |
|--|--|
| linestart | A határoló a sor elején áll |
| whole | A határoló teljes szó |
| wholeleft | A határoló bal oldalon szóhatáron van |
| wholeright | A határoló jobb oldalon szóhatáron van |
| exclusive | A határolók közötti szöveget színezi, a határolókat nem |
| oneline | Az exclusive után: a sor végén lezárja a kontextust, ha a jobb határoló nincs a sorban |

Egy
**wholechars**
[**left**|**right**] *characters*
sor a fájlban bárhol megadja a teljes szó karaktereit; a left és a right
külön adja meg a két oldalt. Egy
**spellcheck**
sor a
**context**
után a kontextus szavait a helyesírás-ellenőrzésnek adja át.

## Beágyazás és rétegek <a id="syntax-embed"></a>

Az

**embed**
[**soft**]
*type*

sor a
**context**
után a határolók közötti szöveget egy másik szintaxis szabályainak adja át; a
*type*
a file sorának leírása, a szóköz helyén \s jellel. A típust abban a Syntax
fájlban keresi, amelyből a szabályok olvasódnak, aztán a bluerig-gel
telepítettben. Ha egyikben sincs meg a típus, a kontextus a saját színeiben
marad. A határolók a kontextus színeit tartják meg. A jobb határoló bárhol
lezárja a beágyazott szöveget, a beágyazott szabályok karakterláncán vagy
megjegyzésén belül is. Egy beágyazott szintaxis beágyazhat egy másikat,
néhány szint mélységig. Például a HTML a szkripteket a JavaScript
szabályainak adja át:

```
  context <SCRIPT*> </SCRIPT> brightcyan
      embed JavaScript\sProgram
```

A
**soft**
megadásával a jobb határoló csak a beágyazott szabályok karakterláncain,
megjegyzésein és kulcsszavain kívül zárja le a beágyazott szöveget, így egy
Jinja2 sablon {{ "}}" }} részében az első }} a karakterlánc része marad, egy
shell-szkript ${x/\\}/y} részében pedig az első } az escape része marad.

Az

**overlay**
*type*

sor a szabályok között az egész szintaxist a
*type*
fölötti réteggé teszi: a kontextusai bárhol színezik, amit tartalmaznak, a
szöveg többi részét pedig a
*type*
szabályai olvassák, amelyek a kontextusok tartalmát üres helynek látják. Így
egy sablonnyelv címkéi ugyanúgy színeződnek a szövegben, egy HTML címkén
belül, egy attribútum karakterláncában és egy szkriptben, a többit pedig a
HTML színezi. A réteg alapértelmezett kontextusa semmit sem színez: egy
kulcsszó ott csak megakadályozza, hogy egy kontextus elkezdődjön. Ha egyik
Syntax fájlban sincs meg a típus, a réteg sima szöveg fölött marad. Például
Smarty a HTML fölött:

```
  overlay HTML\sFile
  context default
      keyword {\s
  context {\* \*} brown
  context { } brightcyan
      embed soft Smarty\sTag
```

## Kulcsszavak <a id="syntax-keywords"></a>

A kontextus egy kulcsszavát ez a sor adja meg:

```
keyword [whole|wholeright|wholeleft] [linestart] <string> [<foreground>] [<background>] [<attributes>]
```

A kulcsszavak és a kontextushatárolók karakterláncai így olvasódnak:

|  |  |
|--|--|
| \\t, \\s | Tabulátor és szóköz: az üres helyek a sor szavait választják el, és nem használhatók úgy, ahogy vannak |
| \\n, \\r | Újsor és kocsivissza |
| `\\` | Fordított perjel |
| \\\* | Csillag |
| \* | Bármilyen karakterek, bármennyi |
| + | Bármennyi szókarakter, akár egy sem |
| \\[abc\\] | Bármennyi karakter a halmazból |
| \\{abc\\} | Egy karakter a halmazból |

Minta nem lehet a kontextushatároló első vagy utolsó karaktere. Például a

```
  keyword         '*'      green
```

zöldre színezi a C egykarakteres konstansait, a

```
  keyword         "*"      green
```

pedig a karakterlánc-konstansokat, amíg nem nyúlnak át újsoron.

A

```
  keyword  \\\n  brightgreen
```

sor egy fordított perjelből és egy újsorból álló kulcsszót ad. A kulcsszavak a
kontextushatárolók előtt illeszkednek, így ez megakadályozza, hogy a kontextus
egy fordított perjellel végződő sor végén véget érjen, így a C
előfeldolgozó-direktíva több soron át folytatódhat.

Egy
**define**
*name words...*
sor olyan nevet ad, amely a szabályokban ezeket a szavakat jelenti, például
**define comment brown**.

## Színek és attribútumok <a id="syntax-colors"></a>

A színek: black, gray, red, brightred, green, brightgreen, brown, yellow,
blue, brightblue, magenta, brightmagenta, cyan, brightcyan, lightgray és
white. A "default" a terminál alapértelmezett színe, a "base" a szerkesztő fő
színei; ezzel attribútumok állíthatók be a háttérszín megváltoztatása
nélkül. 256 színnél van még color16 - color255, rgb000 - rgb555 és
gray0 - gray23 is.

Az előtérszín helyett hatókör-token (scope token) is állhat: nagybetűs szó,
amely azt mondja meg, mi a szöveg, nem azt, hogyan néz ki:

```
keyword  whole  if   KEYWORD_CONTROL
context  " " STRING_QUOTED_DOUBLE
    keyword  \\"   CONSTANT_CHARACTER_ESCAPE
keyword  linestart TODO  KEYWORD_OTHER_TODO black
```

A token színe a témából jön, így egy téma színezi a programot és a szöveget,
és minden nyelvet ugyanúgy; lásd
[Témák](#skins).
A token hatókörként olvasódik: kisbetűvel, az aláhúzások helyén pontokkal. A
KEYWORD_CONTROL_IMPORT a keyword.control.import, a keyword.control egy
fajtája, amely a keyword egy fajtája. Ezek a Sublime Text és a TextMate
hatókörnevei. Amit a téma nem színez, az megtartja azokat a színeket, amelyeket
az mcedit adott annak a szövegnek. A token utáni háttér vagy attribútumok az
mcedit színei fölött állnak, ahogy fent a black, de nem a téma által magának
a tokennek adott háttér vagy attribútumok fölött. A token a
define sorban is működik, és a line-local szabályok number, string és
symbols sorában is. A telepített szintaxisfájlok csak tokeneket használnak; a
listájuk az mcedit színeivel a
**{{pkgdatadir}}/syntax/README.txt**
fájlban van.

A
**cooledit**
programmal közös szintaxisfájl perjellel adja meg mindkettő színét, és a
bluerig az előtte állót veszi:

```
keyword  #include  red/Orange
```

Az attribútumok: bold, italic, underline, reverse és blink, több is,
pluszjellel összekapcsolva.

## Egyéb sorok <a id="syntax-other"></a>

|  |  |
|--|--|
| # ... | Megjegyzés, külön sorban |
| caseinsensitive | A fájl elején: a betűk kis- vagy nagybetűs volta nem számít |
| line-local | Sima szöveg módja: minden újsornál és a képernyő szélén alaphelyzetbe áll, és csak a megjelenített szöveget olvassa |
| number *max* *color* | A line-local módban: legfeljebb *max* jegyű decimális számok |
| string *quote* *color* | A line-local módban: egyszeres vagy kettős idézőjelek közötti karakterláncok |
| symbols *characters* *color* | A line-local módban: egyenként színezett írásjelek |

# Színek <a id="colors"></a>

A bluerig a terminál-adatbázis és a terminál neve alapján próbálja
megállapítani, hogy a terminál ismeri-e a színeket. Néha téved, ezért a színes
mód a -c kapcsolóval kikényszeríthető, a -b kapcsolóval pedig kikapcsolható.

Ha a program az ncurses helyett az S-Lang képernyőkezelővel készült, a
**COLORTERM,**
változót is megnézi; ha be van állítva, ugyanaz a hatása, mint a -c
kapcsolónak.

Megadhatók olyan terminálok, amelyeken mindig színes mód van: ehhez az
inicializáló fájl Colors szakaszába a
*color_terminals*
változót kell felvenni. Ez megakadályozza, hogy a bluerig megpróbálja
megállapítani, ismeri-e a terminál a színeket. Példa:

```
[Colors]
color_terminals=linux,xterm
color_terminals=terminal-name1,terminal-name2...
```

A program készülhet ncurses és S-Lang könyvtárral is; az ncurses nem ad módot
a színes mód kikényszerítésére: az ncurses csak a terminál-adatbázis adatait
használja.

# Témák <a id="skins"></a>

A program színei és kereteinek vonalai egy témafájlból jönnek. A
[Megjelenítés](#appearance)
párbeszédablak egy listából választ egyet.

Ha a téma true-color definíciókat tartalmaz, a [skin] szakaszban meg kell adni
a 'truecolors' kulcsot TRUE értékkel. Ha true-color nincs, de 256 szín van,
akkor helyette a '256colors' kulcsot kell megadni.

A téma az első, amelyet ezek megneveznek:

```
1) command line option -S <skin>, --skin=<skin>
2) environment variable BLUERIG_SKIN
3) setting skin of the [bluerig] section of the ini file
4) the skin called default
```

A név lehet egy témafájl abszolút útvonala (.ini kiterjesztéssel vagy
anélkül). Egyébként a témafájlt itt keresi (az első találatig):

```
1) ~/.local/share/bluerig/skins/
2) {{sysconfdir}}/bluerig/skins/
3) {{pkgdatadir}}/skins/
```

A témafájlok formátumát a
**{{pkgdatadir}}/skins/README.txt**
írja le. A szintaxiskiemelés színei a téma
**[syntax]**
szakaszából jönnek: ott egy hatókör egy szövegfajta színeit adja meg; két
hatókör üres hellyel elválasztva, a szövegé és annak a kontextusé, amelyben
áll, az adott szövegfajta színeit abban a kontextusban. A téma által
megnevezett leghosszabb hatókör nyer:

```
[syntax]
    comment = gray
    keyword = yellow
    keyword.control = brightmagenta
    string = green
    string constant.character.escape = white;red
    invalid = white;red;bold
```

Az a szövegfajta, amelyet a téma nem nevez meg, megtartja az mcedit által
adott színeit, így a téma megnevezhet néhány fajtát, és a többit meghagyhatja.
Azok a szabályok, amelyek hatókör-tokenek helyett színeket neveznek meg, a
témától függetlenül megtartják a színeiket; lásd
[Szintaxiskiemelés](#syntax-highlighting).

# Terminál-adatbázisok <a id="terminal-databases"></a>

A bluerig módot ad arra, hogy a rendszer terminál-adatbázisát root jogok
nélkül javítsa. A
[Billentyűk](#learn-keys)
párbeszédablak a megtanultakat a
**~/.config/bluerig/term/\<TERM>**
fájlba írja, minden billentyűt külön sorban, a
**[keys]**
szakaszban. A bluerig a
**defaults.ini**
rendszerfájlban (a
**{{sysconfdir}}/bluerig**
vagy a
**{{pkgdatadir}}**
könyvtárban) és a
**~/.config/bluerig/ini**
fájlban is keresi a "terminal:a-terminál-neve" szakaszt, majd a
"terminal:general" szakaszt. A szakasz minden sora egy megadandó
billentyűszimbólumot tartalmaz, utána egyenlőségjelet és a billentyű
definícióját. Az escape karaktert a különleges \\e alak, a control-x
karaktert a ^x jelöli. Egy
*copy=other*
sor a "terminal:other" szakasz összes billentyűjét is átveszi.

A lehetséges billentyűszimbólumok:

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

Például ha az insert billentyűt Escape + [ + O + p sorozatként akarja
megadni, ezt kell beállítani az ini fájlban:

```
insert=\e[Op
```

A billentyűszimbólum módosítókat is kaphat:

```
    ctrl-alt-right=\e[[1;6C
    ctrl-alt-left=\e[[1;6D
```

Ez azt jelenti, hogy a ctrl+alt+left \\e[[1;6D vezérlősorozatot küld, ezért a
bluerig a "\\e[[1;6D" sorozatot Ctrl-Alt-Left billentyűként értelmezi.

A
*complete*
billentyűszimbólum azokat a vezérlősorozatokat jelöli, amelyek a kiegészítést
indítják; ezt az Alt-tab indítja, de más billentyűk is megadhatók ugyanerre
(azokon a billentyűzeteken, amelyeken rengeteg szép és használatlan billentyű
van mindenfelé).

# Lua-szkriptek <a id="lua-scripting"></a>

Ahol a bluerig Luával készült, Lua-szkripteket futtat, amelyek a
szerkesztőt bővítik: billentyűkhöz és menükhöz kötött műveletek, válaszok a
szerkesztő eseményeire (fájl nyílik meg, mentődik, billentyűt nyomnak le),
párbeszédablakok és teljes képernyős nézetek. A szerkesztő több parancsa is
ilyen szkript: a sorok rendezése, a bekezdés formázása, egy parancs
kimenetének beszúrása, karakter beszúrása a kódpontja alapján, a dátum
beszúrása, base64 dekódolása és táblázatok rajzolása.

A rendszerszkriptek a
**{{pkgdatadir}}/lua/scripts/**,
a felhasználó szkriptjei a
**~/.local/share/bluerig/lua/scripts/**
könyvtárban vannak, minden szkriptnek saját könyvtára van a saját
**lua.ini**
és
**init.lua**
fájljával. A
[Lua scripts](#lua-scripts)
párbeszédablak kapcsolja be és ki őket.

A
**--no-lua**
kapcsoló, vagy az 1 értékre állított
**BLUERIG_NO_LUA**
változó, Lua-szkript nélkül indítja a szerkesztőt, az ini fájl
**[Lua]**
szakaszában az
*enabled=false*
pedig véglegesen.

Hogy hogyan kell szkriptet írni, és milyen függvényeket hívhat egy szkript,
azt a
**LUA_PLUGINS.md**
és a
**LUA_API_REFERENCE.md**
írja le, amelyek a bluerig dokumentációjával együtt települnek.

# Általános beállítások <a id="general-settings"></a>

Az ini fájl
**[bluerig]**
szakaszának ezek a beállításai a programra vonatkoznak, nem a szerkesztőre.
Ha nincs másképp jelezve, az 1 bekapcsolja, a 0 kikapcsolja a beállítást.

*auto_save_setup*
: A beállítások kiírása az ini fájlba a szerkesztő minden kilépésekor.

*skin*
: A téma neve. Lásd [Témák](#skins).

*shadows*
: Árnyék rajzolása a párbeszédablakok és a legördülő menük alá.

*keymap*
: A használandó billentyűkiosztás-fájl. Lásd
[A billentyűk átdefiniálása](#keys_redefine).

*drop_menus*
: Az F9 megnyomásakor azonnal az első menü nyílik meg, nem csak a menüsor.

*mouse_close_dialog*
: A párbeszédablakon kívüli kattintás bezárja azt.

*mouse_repeat_rate, double_click_speed*
: Az egérgombok ismétlési ideje, és az az idő, amelyen belül két kattintás
dupla kattintásnak számít, ezredmásodpercben.

*old_esc_mode, old_esc_mode_timeout*
: Ha az old_esc_mode be van kapcsolva, az egyedüli Esc
old_esc_mode_timeout mikroszekundum után Escnek számít, ahelyett, hogy a
következő billentyűre várna.

*show_all_if_ambiguous*
: A lehetséges kiegészítések listájának azonnali megjelenítése, ha a
kiegészítés nem egyértelmű. Lásd [Kiegészítés](#completion).

*shell_patterns*
: A felhasználói menü feltételeinek mintái shell-minták (1) vagy reguláris
kifejezések (0).

*num_history_items_recorded*
: Hány tételt őriz meg egy beviteli sor minden előzménye.

*confirm_history_cleanup*
: Kérdés az előzmények törlése előtt.

*clear_before_exec*
: A képernyő törlése a felhasználói menü egy parancsának futtatása előtt.

*pause_after_run*
: Billentyűre vár a felhasználói menü egy parancsának lefutása után: soha
(0), csak buta terminálokon (1, az alapértelmezés) vagy mindig (2).

*alternate_plus_minus*
: A numerikus billentyűzet plusz és mínusz billentyűje pluszként és mínuszként
működjön.

*keybar_modifiers*
: Ha igaz, a Shift, a Ctrl vagy az Alt nyomva tartása alatt a billentyűsor
azokat a parancsokat mutatja, amelyeket az F billentyűk ezzel a módosítóval
adnak. Ehhez a kitty
billentyűzet-protokollt ismerő terminál kell: a bluerig ekkor minden
billentyűt a felengedésével együtt kér tőle, és a fókuszeseményeket is. Más
terminálokat nem érint. Alapértelmezésben hamis.

*verbose*
: A hosszú műveletek előrehaladásának megjelenítése.

*keybar_visible*
: A
**[Layout]**
szakaszban: a funkcióbillentyűk feliratainak megjelenítése a képernyő alján.

# Beállítások az ini fájlban <a id="internal-file-editor-options"></a>

A legtöbb beállítást a
**Beállítások**
menü párbeszédablakai végzik, és a
**Beállítások mentése**
a
**~/.config/bluerig/ini**
fájl
**[bluerig]**
szakaszába írja őket. Ezek a fájl szerkesztőbeállításai: az 1 bekapcsolja, a
0 kikapcsolja a beállítást, és az alapértelmezés áll a végén.

|  |  |
|--|--|
| editor_tab_spacing | A tabulátor szélessége; alapértelmezésben 8. Az editor_fake_half_tabs fél tabulátorokat ad |
| editor_fill_tabs_with_spaces | Tabulátor karakter helyett szóközöket szúr be; 0 |
| editor_return_does_auto_indent | Az Enter úgy húz be, mint a fölötte lévő legközelebbi szöveges sor; 1 |
| editor_backspace_through_tabs | A Backspace a bal margóig minden üres helyet töröl, ha a kurzor és a margó között nincs szöveg; 0 |
| editor_fake_half_tabs | A bal margó és a szöveg közötti tabulátor 4 oszlopot lép, és a fájlba szóközök és 8 oszlopos tabulátorok kerülnek; 1 |
| editor_option_save_mode | Hogyan íródik ki a fájl, lásd [Mentési mód](#edit-save-mode): 0 - egyenesen a fájlba, és az írás közbeni hiba csonkán hagyja; 1 - ideiglenes fájlon át; 2 - biztonsági másolattal; 0 |
| editor_word_wrap_line_length | A tördelés sorhossza; 72 |
| editor_backup_extension | A biztonsági másolat nevének utótagja; ~ |
| editor_line_state | A sorállapot-oszlop megjelenítése: számok, összecsukások, elrejtett szakaszok; 0. Az Alt-n váltja |
| editor_visible_spaces | A sorvégi szóközöket pontként mutatja; 1 |
| editor_visible_tabs | A tabulátorokat \<----\> alakban mutatja; 1 |
| editor_show_control_chars | A vezérlőkaraktereket ^M, ^L alakban mutatja, egyébként üres cellaként; 1. A Parancs menü egy tétele váltja |
| editor_persistent_selections | A kurzor mozgatása után megtartja a kijelölést; 1 |
| editor_drop_selection_on_copy | A vágólapfájlba másolás után megszünteti a kijelölést; 1 |
| editor_cursor_beyond_eol | A kurzor a sor végén túl is mehet; 0 |
| editor_cursor_after_inserted_block | A kurzort a beszúrt blokk után teszi; 0 |
| editor_syntax_highlighting | Bekapcsolja a szintaxiskiemelést; 1 |
| editor_edit_confirm_save | Kiírás előtt kérdez; 1 |
| editor_option_typewriter_wrap | Gépelés közben az editor_word_wrap_line_length értéknél töri a sort; 0 |
| editor_option_auto_para_formatting | Gépelés közben formázza a kurzor alatti bekezdést; 0 |
| editor_option_save_position | Kilépéskor megjegyzi a helyet a fájlban; 1 |
| editor_group_undo | Az egymás utáni egyforma műveleteket egyszerre vonja vissza; 0 |
| editor_wordcompletion_collect_entire_file | A kiegészítés szavait az egész fájlból veszi, nem csak a kurzorig; 0 |
| editor_wordcompletion_collect_all_files | A kiegészítés szavait minden nyitott fájlból veszi; 1 |
| editor_stop_format_chars | A bekezdésformázás érintetlenül hagyja az ezen karakterek egyikével kezdődő sort és az utána következő sorokat; -+\*\\,.;:&\> |
| editor_state_full_filename | Az állapotsorban a teljes útvonalat mutatja, nem a fájlnevet; 0 |
| editor_filesize_threshold | Az a méret, amely fölött a fájl kérdés után nyílik meg, K vagy M utótaggal; 64M |
| editor_show_right_margin | Az editor_word_wrap_line_length oszlopot jobb margóként jelöli; 0. A Parancs menü egy tétele váltja |
| editor_simple_statusbar | Rövid állapotsort mutat, az eltolás és a kurzor alatti karakter nélkül; 0 |
| editor_check_new_line | Mentéskor kérdez a fájl végéről hiányzó sorvégről; 0. A mentési mód párbeszédablakának egy jelölőnégyzete váltja |
| editor_sticky_windows | Az egymáshoz érő keretű ablakokat egyként méretezi, lásd [Ablak menü](#window-menu); 0 |

# KÖRNYEZET <a id="environment"></a>

Az alábbi változókat olvassa a bluerig. Az olyan változók, mint a **TERM**,
a **SHELL**, a **HOME** vagy a **PATH**, itt nem szerepelnek: a program azért
olvassa őket, hogy megtudja, hol fut, nem azért, hogy azokkal állítsák be.

**BLUERIG_DATADIR**
: Az a könyvtár, amelyből az adatfájlok jönnek, a {{pkgdatadir}} helyett.
Lásd [FÁJLOK](#files).

**BLUERIG_PROFILE_ROOT**
: A felhasználói fájlok gyökere, abszolút útvonalként, a saját könyvtár
helyett. Lásd [FÁJLOK](#files).

**BLUERIG_SKIN**
: A használandó téma, név vagy útvonal szerint. Lásd [Témák](#skins).

**BLUERIG_KEYMAP**
: A használandó billentyűkiosztás-fájl. Lásd
[A billentyűk átdefiniálása](#keys_redefine).

**BLUERIG_TMPDIR**, **TMPDIR**
: A program ideiglenes fájljainak könyvtára, a kettő közül az első, amely be
van állítva; ha egyik sincs, a /tmp.

**BLUERIG_NO_LUA**
: 1 értékkel a program Lua futtatókörnyezet nélkül indul. Egyetlen
Lua-szkript sem töltődik be.

**KEYBOARD_KEY_TIMEOUT_US**
: Meddig várjon egy vezérlősorozat többi részére, mikroszekundumban.

**COLORTERM**
: A színek kiválasztásakor olvassa. Lásd [Színek](#colors).

**BLUERIG_LOG_ENABLE**, **BLUERIG_LOG_FILE**
: Hibakeresési napló íródik, ha a BLUERIG_LOG_ENABLE értéke 1. A
**BLUERIG_LOG_FILE** nélkül az *ini* fájl *[Logging]* szakaszának *logfile*
bejegyzése által megnevezett fájl használatos, és e bejegyzés nélkül a
*~/.cache/bluerig/bluerig.log*.

**BLUERIG_SPELL_LOG**
: Az a fájl, amelybe a helyesírás-ellenőrző a hibakeresési naplóját írja.
Nincs saját kapcsolója: a napló akkor íródik, ha a változó egy fájlt nevez
meg.

# FÁJLOK <a id="files"></a>

Az alábbi teljes útvonalak telepítésenként eltérhetnek. Az adatfájlokra a
**BLUERIG_DATADIR**
környezeti változó is hatással van: ha be van állítva, az értéke használatos
az alábbi útvonalakban a {{pkgdatadir}} helyett. A felhasználói fájlok az
XDG alapkönyvtárakat követik: a
*~/.config*,
a
*~/.local/share*
és a
*~/.cache*
a
**XDG_CONFIG_HOME**,
a
**XDG_DATA_HOME**
és a
**XDG_CACHE_HOME**
értékét jelenti, ahol ezek be vannak állítva.

*{{pkgdatadir}}/help/bluerig.md*
: A program súgófájlja, és mellette a szerkesztő bővítményeinek súgófájljai.

*{{sysconfdir}}/bluerig/bluerig.ini*, *{{pkgdatadir}}/bluerig.ini*
: A rendszerszintű beállítófájlok; csak akkor használatosak, ha a
felhasználónak nincs saját
**~/.config/bluerig/ini**
fájlja. Ha a {{sysconfdir}}/bluerig/bluerig.ini létezik, a
{{pkgdatadir}}/bluerig.ini nem használatos.

*{{sysconfdir}}/bluerig/defaults.ini*, *{{pkgdatadir}}/defaults.ini*
: Globális beállítások, amelyek minden felhasználót érintenek, akár van
~/.config/bluerig/ini fájljuk, akár nincs. Jelenleg csak a
[terminálbeállítások](#terminal-databases)
töltődnek be belőle.

*{{sysconfdir}}/bluerig/keymap.ini*, *{{pkgdatadir}}/keymap.ini*
: A rendszerszintű billentyűtársítások. Lásd
[A billentyűk átdefiniálása](#keys_redefine).

*{{sysconfdir}}/bluerig/bluerig.menu*, *{{pkgdatadir}}/bluerig.menu*
: A rendszerszintű [felhasználói menü](#edit-menu-file).

*{{sysconfdir}}/bluerig/spell.keymap, ctags.keymap*
: A spell és a ctags bővítmény billentyűi; a ~/.config/bluerig könyvtárban
lévő azonos nevű fájl felülírja őket.

*{{pkgdatadir}}/syntax/\**
: A rendszerszintű szintaxisfájlok; csak akkor használatosak, ha a
felhasználó megfelelő saját fájlja hiányzik a
**~/.local/share/bluerig/syntax/**
könyvtárból.

*{{pkgdatadir}}/skins/\**
: A programmal érkező témák.

*{{pkgdatadir}}/charsets*
: A kódlapok listája.

*{{pkgdatadir}}/lua/scripts/\**
: A programmal érkező Lua-szkriptek.

*{{pkgdatadir}}/examples/macros.d/\**
: Példaszkriptek a makrókhoz.

*{{plugins_dir}}/*
: A szerkesztő futás közben betöltött bővítményei.

*~/.config/bluerig/ini*
: A felhasználó saját beállításai. Ha ez a fájl létezik, a beállítások innen
töltődnek be a rendszerszintű beállítófájl helyett.

*~/.config/bluerig/keymap.ini*
: A felhasználó saját billentyűtársításai.

*~/.config/bluerig/menu*
: A felhasználó saját [felhasználói menüje](#edit-menu-file).

*~/.config/bluerig/plugins.ini*
: A kikapcsolt bővítmények és Lua-szkriptek.

*~/.config/bluerig/term/\<TERM>*
: A [Billentyűk](#learn-keys) ablakban megtanított billentyűk.

*./.bluerig.menu*
: Az aktuális könyvtár felhasználói menüje. Ha ez a fájl létezik az aktuális
könyvtárban, a saját könyvtárbeli vagy a rendszerszintű menü helyett ez
használatos.

*~/.local/share/bluerig/history*
: A beviteli sorok előzményei.

*~/.local/share/bluerig/filepos*
: A helyek, ahol a fájlokat elhagyták, az
*editor_option_save_position*
számára.

*~/.local/share/bluerig/clipboard*
: A szerkesztő vágólapfájlja.

*~/.local/share/bluerig/macros*
: A felvett [makrók](#macros).

*~/.local/share/bluerig/macros.d/macro.N.sh*
: A makrók shell-szkriptjei.

*~/.local/share/bluerig/syntax/Syntax*
: A felhasználó saját szintaxisfájlja.

*~/.local/share/bluerig/skins/*
: A felhasználó saját témái.

*~/.local/share/bluerig/lua/scripts/*
: A felhasználó saját Lua-szkriptjei.

*~/.cache/bluerig/block*
: A blokkfájl, amelyen a felhasználói menü és a makrószkriptek dolgoznak.

*~/.cache/bluerig/temp*
: A szerkesztő egy ideiglenes fájlja.

A felhasználói fájlok gyökérkönyvtárának megváltoztatásához a
**BLUERIG_PROFILE_ROOT**
környezeti változó használható. A BLUERIG_PROFILE_ROOT értéke abszolút
útvonal kell legyen. Ha a BLUERIG_PROFILE_ROOT nincs beállítva vagy üres, a
HOME változó használatos. Ha a HOME nincs beállítva vagy üres, a
könyvtárakat a GLib könyvtár adja.

# LICENC <!-- help:skip -->

Ez a program a Free Software Foundation által kiadott GNU General Public
License feltételei szerint terjeszthető. A licencről és a garancia hiányáról
a beépített súgó ad részleteket.

# ELÉRHETŐSÉG <a id="availability"></a>

A program legújabb változata itt található:
<https://github.com/blue-panels/bluerig/releases> .

# LÁSD MÉG <a id="see-also"></a>

cooledit(1), aspell(1), hunspell(1), ctags(1), etags(1), gpm(1),
terminfo(5), sh(1).

# SZERZŐK <a id="authors"></a>

Paul Sheer (psheer@obsidian.co.za) a cooledit eredeti szerzője; a bluerig a
cooledit folytatása. A szerzők és a közreműködők a forráscsomag AUTHORS
fájljában vannak felsorolva.

# HIBÁK <a id="bugs"></a>

A hibákat itt lehet bejelenteni:
<https://github.com/blue-panels/bluerig/issues> .

Adja meg a hiba részletes leírását, a futtatott program verzióját (a
*bluerig -V*
kiírja), és az operációs rendszert, amelyen a programot futtatja. Ha a
program összeomlik, örülnénk egy veremkivonatnak (stack trace).
