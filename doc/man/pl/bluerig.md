---
date: listopad 2026
---

<!-- help:topics "Spis treści:" -->
# NAZWA <!-- help:skip -->

bluerig - terminalowe IDE.

# UŻYTKOWANIE <!-- help:skip -->

**bluerig**
[opcje] [+wiersz] [plik1[:wiersz]] [plik2[:wiersz]] ...

# OPIS <a id="description"></a>

**bluerig**
to terminalowe IDE. Jest kontynuacją
**cooledit**,
edytora Paula Sheera, którego terminalowa wersja przez wiele lat była rozwijana
jako wbudowany edytor GNU Midnight Commandera; bluerig rozwija ten edytor dalej
jako osobny program.
**brg**
to krótsza nazwa tego samego programu.

Edytuje kilka plików naraz, każdy we własnym oknie, także pliki binarne. Ma
kopiowanie, przenoszenie, usuwanie, wycinanie i wklejanie bloków; cofanie i
ponawianie klawisz po klawiszu; menu rozwijane; wstawianie plików; makra;
szukanie i zastępowanie wyrażeniami regularnymi; filtr wierszy; zwijanie;
zakładki; zaznaczanie tekstu strzałkami z Shiftem (jeśli terminal to
obsługuje); bloki kolumnowe; tryby wstawiania i zastępowania; automatyczne
wcięcia; zawijanie wierszy i formatowanie akapitów; ustawialną szerokość
tabulacji; podświetlanie składni dla wielu typów plików; menu użytkownika,
które przepuszcza tekst przez polecenia powłoki; wtyczki edytora (nawigacja
po tagach, sprawdzanie pisowni) i skrypty Lua.

Każdy plik podany w wierszu poleceń otwiera się we własnym oknie. Bez pliku
edytor otwiera puste okno. Plik większy niż
*editor_filesize_threshold*
(domyślnie 64 MB) otwiera się po pytaniu, a nie jest odrzucany.

# OPCJE <a id="options"></a>

*+wiersz*
: Przechodzi do wiersza o podanym numerze (bez spacji między znakiem
*+*
a liczbą). Można podać kilka numerów, ale użyty zostanie tylko ostatni i tylko
dla pierwszego pliku.

*plik:wiersz[:]*
: Otwiera plik w tym wierszu, tak jak miejsce w pliku podają kompilatory i
grep. Jeśli istnieje plik o całej tej nazwie, razem z dwukropkiem i numerem,
otwiera się ten plik.

*-V, --version*
: Pokazuje wersję programu.

*-f, --datadir*
: Pokazuje wkompilowaną ścieżkę wyszukiwania plików danych bluerig.

*-F, --datadir-info*
: Pokazuje rozszerzone informacje o katalogach, których używa bluerig: pliki
danych, pliki użytkownika i wtyczki.

*--configure-options*
: Pokazuje opcje, z którymi program został skonfigurowany.

*--no-lua*
: Uruchamia program bez środowiska Lua: żaden skrypt Lua nie jest wczytywany.

*-D, --debug=katalog*
: Otwiera projekt tego katalogu do debugowania: ostatni plik projektu, drzewo
projektu i panel debugera po prawej. Okna plików dostają klawisze debugera:
F6 stawia punkt przerwania, F5 buduje program i uruchamia go, za pierwszym
razem tworząc jego konfigurację debugowania, F7 i F8 wykonują krok, F3
indeksuje symbole projektu; F2, F9 i F10 pozostają Zapisz, menu i Zakończ, a
Alt-F5 przechodzi do panelu i z powrotem. Potrzebne są wtyczki projektu,
budowania i debugera, a dla F3 także ctags. Menu Drawing, Format i Plugins nie
ma na pasku menu; ich klawisze działają, a Opcje zachowują Manage plugins.

*-Q, --query*
: Wykonuje zapytania z pliku w bazie danych, MongoDB przez mongosh albo
PostgreSQL przez libpq; odpowiedź pojawia się w kartach Output, Table i
Historia na dole. *MC_QUERY_KIND* to *mongo* albo *postgres*; bez niej
*MC_MONGO_URI* oznacza MongoDB, a plik *.sql* PostgreSQL. Dla MongoDB
*MC_MONGO_URI* i *MC_MONGO_DB* mówią, z czym się połączyć, *MC_MONGOSH* podaje
program; F5 wykonuje zaznaczenie, w przeciwnym razie część między wierszami
*// ---* wokół kursora, a w przeciwnym razie cały plik. Dla PostgreSQL libpq
bierze połączenie z *PGHOST*, *PGDATABASE*, *PGSERVICE* i podobnych, hasło z
*~/.pgpass*; F5 wykonuje zaznaczenie, w przeciwnym razie cały plik.
Ctrl-F5 wykonuje cały plik. Bez podanego pliku plikiem jest
*~/.local/share/bluerig/queries/* z nazwą bazy danych i *.js*
albo *postgres/*, nazwa połączenia oraz nazwa bazy danych i
*.sql*. Okna, które projekt miał ostatnim razem, nie są
pokazywane, a wtyczki projektu i kodu (project, build, debugger, ctags,
etags, git, terminal, spell) nie są uruchamiane; skrypty działają, a także
serwer językowy zapytań, jeśli podaje go lsp.json.
Menu Drawing, Format i Plugins nie ma na pasku menu, tak jak przy -D.
Potrzebna jest wtyczka query.

*-G, --git=katalog*
: Otwiera okno Git wtyczki git na drzewie roboczym tego katalogu; zamknięcie
go kończy edytor. Okna, które projekt miał ostatnim razem, nie są pokazywane,
a wtyczki projektu (project, build, debugger, ctags, etags) nie są
uruchamiane. Do panelu git w menedżerze plików: z jedną z poniższych opcji
po niej albo sama, dla bieżącego katalogu:

*--git-log*
: Okno Git z dziennikiem commitów.

*--git-show=commit*
: Okno Git z tym commitem: jego plikami i ich różnicami.

*--git-commit*, *--git-amend*
: Okno Git z opisem nowego commita zmian z indeksu albo ostatniego commita
poprawionego o te zmiany; edytor kończy pracę, gdy opis zostanie zapisany i
commit wykonany albo gdy okno zostanie zamknięte.

*--git-reword=commit*
: Nowy opis tego commita, w ten sam sposób; pod opisem są pliki commita.

*-h, -?, --help*
: Pokazuje opcje i ich działanie.

*--help-terminal, --help-color*
: Pokazuje opcje terminala albo opcje kolorów opisane poniżej.

Opcje terminala:

*-x, --xterm*
: Wymusza tryb xterm. Używa się go na terminalach zgodnych z xterm (dwa tryby
ekranu i możliwość wysyłania sekwencji sterujących myszy).

*-X, --no-x11*
: Nie korzysta z systemu X Window przy odczycie stanu klawiszy modyfikujących.

*-g, --oldmouse*
: Używa starego sposobu śledzenia myszy przez terminal (highlight tracking).

*-d, --nomouse*
: Wyłącza obsługę myszy.

*-t, --termcap*
: Używa bazy termcap zamiast terminfo. Opcja istnieje tylko wtedy, gdy
bluerig zbudowano z biblioteką S-Lang.

*-s, --slow*
: Tryb wolnego terminala: ekran jest odświeżany rzadziej.

*-a, --stickchars*
: Rysuje ramki zwykłymi znakami zamiast znaków semigraficznych terminala.

*-K plik, --keymap=plik*
: Wczytuje przypisania klawiszy z tego pliku. Zobacz
[Zmiana przypisań klawiszy](#keys_redefine).

*--nokeymap*
: Nie wczytuje żadnego pliku klawiszy: używa przypisań wbudowanych w program.

Opcje kolorów:

*-b, --nocolor*
: Wymusza tryb czarno-biały.

*-c, --color*
: Wymusza tryb kolorowy na terminalach, które wydają się nie obsługiwać
kolorów.

*-S skórka, --skin=skórka*
: Używa skórki o tej nazwie. Zobacz
[Skórki](#skins).

# Przegląd <a id="overview"></a>

Na górze ekranu bluerig jest
[pasek menu](#menu-bar),
pod nim okna edytora, a na dole etykiety klawiszy funkcyjnych. Pasek menu
może być ukryty: przywołuje go F9 albo kliknięcie w górny wiersz ekranu.

Każde okno pokazuje jeden plik: jego nazwa, stan pliku i miejsce kursora są w
wierszu stanu u góry okna. Można mieć otwartych kilka okien naraz;
[menu Okno](#window-menu)
przełącza między nimi, a każde z nich można przesuwać i zmieniać jego rozmiar.

# Edytor bluerig <a id="internal-file-editor"></a>

<a id="keys"></a>
**Klawisze**

<!-- {{key:Action|F8}}: Action is the action name of misc/keymap.default.ini and src/keymap.c (Remove = f8); the help shows the keys bound to it now, the man page the default keys after the bar -->
To są klawisze domyślne. Każdy z nich można zmienić, zobacz
[Zmiana przypisań klawiszy](#keys_redefine).

<a id="moving-around"></a>
**Ruch kursora**

|  |  |
|--|--|
| {{key:Left|Left}} | Znak w lewo |
| {{key:Right|Right}} | Znak w prawo |
| {{key:Up|Up}} | Wiersz w górę |
| {{key:Down|Down}} | Wiersz w dół |
| {{key:WordLeft|Ctrl-Left, Ctrl-z}} | Słowo w lewo |
| {{key:WordRight|Ctrl-Right, Ctrl-x}} | Słowo w prawo |
| {{key:ScrollUp|Ctrl-Up}} | Przewija ekran w górę |
| {{key:ScrollDown|Ctrl-Down}} | Przewija ekran w dół |
| {{key:PageUp|PgUp}} | Strona w górę |
| {{key:PageDown|PgDn}} | Strona w dół |
| {{key:Home|Home}} | Początek wiersza |
| {{key:End|End}} | Koniec wiersza |
| {{key:Top|Ctrl-Home, Alt-\<}} | Początek pliku |
| {{key:Bottom|Ctrl-End, Alt-\>}} | Koniec pliku |
| {{key:TopOnScreen|Ctrl-PgUp}} | Góra ekranu |
| {{key:BottomOnScreen|Ctrl-PgDn}} | Dół ekranu |

**Usuwanie**

|  |  |
|--|--|
| {{key:Delete|Delete, Ctrl-d}} | Usuwa znak |
| {{key:BackSpace|Backspace, Ctrl-h}} | Usuwa znak po lewej |
| {{key:DeleteLine|Ctrl-y}} | Usuwa wiersz |
| {{key:DeleteToEnd|Ctrl-k}} | Usuwa do końca wiersza |
| {{key:DeleteToWordBegin|Alt-Backspace}} | Usuwa do początku słowa |
| {{key:DeleteToWordEnd|Alt-d}} | Usuwa do końca słowa |

<a id="blocks"></a>
**Bloki**

|  |  |
|--|--|
| Shift+klawisze kursora | Zaznacza blok |
| Alt+klawisze kursora | Zaznacza blok kolumnowy |
| {{key:Mark|F3}} | Zaczyna albo kończy zaznaczanie bloku |
| {{key:MarkColumn|F13}} | Zaczyna blok kolumnowy |
| {{key:MarkAll|Ctrl-a}} | Zaznacza cały tekst |
| {{key:Copy|F5}} | Kopiuje blok do kursora |
| {{key:Move|F6}} | Przenosi blok do kursora |
| {{key:Remove|F8}} | Usuwa blok |
| {{key:Store|Ctrl-Ins}} | Kopiuje blok do pliku schowka |
| {{key:Cut|Shift-Del}} | Przenosi blok do pliku schowka |
| {{key:Paste|Shift-Ins}} | Wkleja blok z pliku schowka |
| {{key:BlockSave|Ctrl-f}} | Zapisuje blok do pliku |

<a id="search-keys"></a>
**Szukanie i zastępowanie**

|  |  |
|--|--|
| {{key:Search|F7}} | [Szukanie](#search-and-replace) |
| {{key:SearchContinue|F17}} | Szuka ponownie |
| {{key:Replace|F4}} | Zastępuje |
| {{key:ReplaceContinue|F14}} | Zastępuje ponownie |
| {{key:FilterToggle|Alt-s}} | Zdejmuje filtr wierszy albo stosuje ostatnie szukanie jako filtr |
| {{key:FilterWord|Alt-S}} | Filtruje według zaznaczonego tekstu albo słowa pod kursorem |

**Skoki i zakładki**

|  |  |
|--|--|
| {{key:Goto|Alt-l}} | Przechodzi do wiersza o podanym numerze |
| {{key:MatchBracket|Alt-b}} | Do pasującego nawiasu |
| {{key:Bookmark|Alt-k}} | Stawia albo zdejmuje zakładkę |
| {{key:BookmarkNext|Alt-j}}, {{key:BookmarkPrev|Alt-i}} | Do następnej albo poprzedniej zakładki |
| {{key:BookmarkFlush|Alt-o}} | Usuwa wszystkie zakładki |
| {{key:Find|Alt-Enter}} | Szuka definicji nazwy pod kursorem |
| {{key:FilePrev|Alt-Minus}}, {{key:FileNext|Alt-Plus}} | Wstecz i naprzód po miejscach, do których prowadziło szukanie definicji |

<a id="files-and-windows"></a>
**Pliki**

|  |  |
|--|--|
| {{key:Save|F2}} | Zapisuje plik |
| {{key:SaveAs|F12, Ctrl-F2}} | [Zapisuje plik pod inną nazwą](#save-file-as) |
| {{key:EditNew|Ctrl-n}} | Nowe puste okno |
| {{key:InsertFile|F15}} | Wstawia plik w miejscu kursora |
| {{key:History|Alt-E}} | Historia edytowanych plików |
| {{key:SelectCodepage|Alt-e}} | [Kodowanie tekstu](#codepages-translation) |
| {{key:Quit|F10, Esc}} | Zamyka okno; zamknięcie ostatniego kończy edytor |

<a id="editing"></a>
<a id="macros-and-plugins"></a>
**Inne polecenia**

|  |  |
|--|--|
| {{key:InsertOverwrite|Insert}} | Tryb wstawiania albo zastępowania |
| {{key:Undo|Ctrl-u}} | Cofa |
| {{key:Redo|Alt-r}} | Ponawia |
| {{key:UndoHistory|Ctrl-t, Alt-U}} | Historia cofania |
| {{key:Complete|Alt-Tab}} | [Uzupełnia słowo](#completion) słowami otwartych plików |
| {{key:FoldToggle|Alt-F}} | Zwija albo rozwija blok między pasującymi nawiasami |
| {{key:Sort|Alt-t}} | Sortuje zaznaczone wiersze |
| {{key:ExternalCommand|Alt-u}} | Wstawia wynik polecenia powłoki |
| {{key:ParagraphFormat|Alt-p}} | Formatuje akapit |
| {{key:InsertLiteral|Ctrl-q}} | Wstawia dosłownie następny klawisz albo kod U+XXXX |
| Ctrl-p | Sprawdza pisownię słowa pod kursorem |
| {{key:MacroStartStopRecord|Ctrl-r}} | Zaczyna albo kończy nagrywanie [makra](#macros) |
| {{key:MacroExplorer|Alt-M}} | [Macro Explorer](#macro-explorer) |
| {{key:ShowNumbers|Alt-n}} | Pokazuje albo ukrywa numery wierszy |
| {{key:ShowTabTws|Alt-\_}} | Pokazuje albo ukrywa tabulacje i spacje na końcu wierszy |
| {{key:SyntaxOnOff|Ctrl-s}} | Włącza albo wyłącza podświetlanie składni |
| {{key:Help|F1}} | Pomoc |
| {{key:Menu|F9}} | Pasek menu |
| {{key:UserMenu|F11}} | [Menu użytkownika](#edit-menu-file) |
| {{key:Shell|Ctrl-o}} | Pokazuje albo ukrywa terminal |
| Ctrl-Alt-p | Pokazuje albo ukrywa Preview |
| {{key:Refresh|Ctrl-l}} | Odświeża ekran |

|  |  |
|--|--|
| Zobacz także: | [Menu](#menu-bar) |
|  | [Szukanie, zastępowanie i filtr wierszy](#search-and-replace) |
|  | [Zwijanie](#folding) |
|  | [Makra](#macros) |
|  | [Menu użytkownika](#edit-menu-file) |
|  | [Wtyczki](#plugins) |
|  | [Podświetlanie składni](#syntax-highlighting) |
|  | [Opcje edytora](#editor-options) |
|  | [Ustawienia w pliku ini](#internal-file-editor-options) |

## Zmiana przypisań klawiszy <a id="keys_redefine"></a>

|  |  |
|--|--|
| Opcje \> [Key bindings](#key-bindings) | Przypisuje działaniu własne klawisze; zapisuje je w **~/.config/bluerig/keymap.ini** |
| Opcje \> [Określ klawisze](#learn-keys) | Przypisuje klawiszom ich sekwencje sterujące |
| Opcje \> [Key sniffer](#key-sniffer) | Pokazuje, co terminal wysyła dla klawisza i do jakiego działania klawisz jest przypisany |

Przy starcie bluerig bierze wbudowaną mapę klawiszy i nakłada na nią
**{{pkgdatadir}}/keymap.ini**,
a potem
**{{sysconfdir}}/bluerig/keymap.ini**.
Działanie wymienione w pliku dostaje klawisze z pliku, a jego wbudowane
klawisze przestają działać.

Pakiet instaluje mapy klawiszy
**keymap.default.ini**,
**keymap.emacs.ini**
i
**keymap.vim.ini**
w
**{{sysconfdir}}/bluerig**
i robi z
**keymap.ini**
dowiązanie do
**keymap.default.ini**.

Opcja
**--nokeymap**
wyłącza wczytywanie plików i działa tylko wbudowana mapa klawiszy.

Plikiem klawiszy użytkownika jest pierwszy znaleziony z:

```
1) command line option -K <keymap>, --keymap=<keymap>
2) environment variable BLUERIG_KEYMAP
3) the keymap setting in the [bluerig] section of the ini file
4) file ~/.config/bluerig/keymap.ini
```

Pierwsze trzy przyjmują nazwę albo ścieżkę bezwzględną. Nazwa, która nie
kończy się na
**.keymap**,
dostaje to rozszerzenie, a plik jest szukany w tych katalogach; liczy się
pierwsze trafienie:

```
1) ~/.config/bluerig/
2) {{pkgdatadir}}/
```

Opcja
**-K**,
zmienna
**BLUERIG_KEYMAP**
i ustawienie
**keymap**
dodają rozszerzenie
**.keymap**
do nazwy, która go nie ma, więc map klawiszy pakietu, kończących się na
**.ini**,
nie da się wybrać w ten sposób. Aby użyć jednej z nich, zrób z
**~/.config/bluerig/keymap.ini**
dowiązanie do niej: ten plik jest wczytywany, gdy żadne z trzech nie jest
podane.

```
ln -s {{sysconfdir}}/bluerig/keymap.vim.ini ~/.config/bluerig/keymap.ini
```

Sekcje pliku klawiszy:

|  |  |
|--|--|
| [editor] | Edytor |
| [editor:xmap] | Klawisze po prefiksie, działanie ExtendedKeyMap |
| [dialog] | Okna dialogowe |
| [menu] | Menu |
| [input] | Wiersze wprowadzania |
| [listbox] | Listy |
| [radio] | Przyciski opcji |
| [help] | Okno pomocy |
| [mcterm] | Terminal |

Wiersz sekcji podaje działanie i jego klawisze, rozdzielone średnikami:

```
[editor]
Save = f2
SaveAs = f12; ctrl-f2
```

## Klawisze wierszy wprowadzania <a id="input-line-keys"></a>

Wiersze wprowadzania okien dialogowych przyjmują te klawisze:

|  |  |
|--|--|
| Ctrl-a, Home | Początek wiersza |
| Ctrl-e, End | Koniec wiersza |
| Ctrl-b, Left | Znak w lewo |
| Ctrl-f, Right | Znak w prawo |
| Alt-f, Ctrl-Right | Słowo naprzód |
| Alt-b, Ctrl-Left | Słowo wstecz |
| Ctrl-h, Backspace | Usuwa znak przed kursorem |
| Ctrl-d, Delete | Usuwa znak pod kursorem |
| Ctrl-@ | Stawia znacznik |
| Ctrl-w | Wycina do bufora tekst od znacznika do kursora |
| Alt-w | Kopiuje do bufora tekst od znacznika do kursora |
| Ctrl-y | Wkleja bufor |
| Ctrl-k | Usuwa do końca wiersza |
| Ctrl-Insert | Kopiuje zaznaczenie, a bez niego cały wiersz, do pliku schowka |
| Shift-Delete | Wycina zaznaczenie do pliku schowka |
| Shift-Insert | Wkleja plik schowka jako jeden wiersz: końce wierszy stają się spacjami |
| Alt-p, Alt-n | Poprzedni albo następny wpis historii wiersza |
| Alt-h | [Historia](#history-query) wiersza jako lista |
| Alt-Backspace | Usuwa słowo przed kursorem |
| Alt-Tab | [Uzupełnia](#completion) nazwę pliku, zmienną albo nazwę użytkownika |

## Ogólne klawisze ruchu <a id="general-movement-keys"></a>

Okno pomocy i listy korzystają z tego samego kodu ruchu, więc przyjmują te
same klawisze:

|  |  |
|--|--|
| Up, Ctrl-p | Wiersz wstecz |
| Down, Ctrl-n | Wiersz naprzód |
| Page Up, Alt-v | Strona w górę |
| Page Down, Ctrl-v | Strona w dół |
| Home, A1 | Na początek |
| End, C1 | Na koniec |

Okno pomocy przyjmuje też te klawisze:

|  |  |
|--|--|
| b, Backspace | Strona w górę |
| Space, f | Strona w dół |
| u, d | Pół strony w górę albo w dół |
| g, G | Na początek albo na koniec |

# Obsługa myszy <a id="mouse-support"></a>

bluerig obsługuje mysz. Działa ona w terminalu
**xterm(1)**
(także przez połączenie telnet lub ssh z xterma) i na konsoli Linuksa z
uruchomionym serwerem myszy
**gpm**.

Kliknięcie stawia kursor w miejscu kliknięcia, przeciągnięcie zaznacza tekst,
a kliknięcie w menu, przycisk albo etykietę klawisza funkcyjnego wybiera go.
Kółko przewija tekst.

Domyślne tempo powtarzania przycisków myszy to 400 milisekund. Zmienia je
ustawienie
*mouse_repeat_rate*
pliku ini, a czas, w którym dwa kliknięcia są podwójnym kliknięciem,
ustawienie
*double_click_speed*.

Gdy bluerig działa z obsługą myszy, zwykłe zachowanie myszy terminala
(wycinanie i wklejanie tekstu) daje przytrzymanie klawisza Shift.

# Pasek menu <a id="menu-bar"></a>

Pasek menu pojawia się po naciśnięciu F9 albo kliknięciu myszą w górny wiersz
ekranu. Jego menu to Plik, Edycja, Wyszukaj, Polecenie, Navigate (tylko gdy
wtyczka albo skrypt coś do niego dodaje), Okno, Plugins i Opcje. Skrypt Lua
może dodać pozycje do każdego z nich albo własne menu.

## Menu Plik <a id="file-menu"></a>

**Otwórz plik...**
: Otwiera plik w nowym oknie.

**Nowy**
: Otwiera puste okno.

**Zamknij**
: Zamyka okno.

**Historia...**
: Lista wcześniej edytowanych plików, z której można otworzyć jeden ponownie.

**Zapisz, Zapisz jako...**
: Zapisuje plik pod jego nazwą albo pod inną. Zobacz
[Zapisz jako](#save-file-as).

**Wstaw plik..., Skopiuj do pliku...**
: Wstawia plik w miejscu kursora albo zapisuje zaznaczony blok do pliku.

**Menu użytkownika...**
: [Menu użytkownika](#edit-menu-file).

**O programie...**
: Wersja programu.

**Zakończ**
: Wychodzi z edytora, pytając o niezapisane pliki.

## Menu Edycja <a id="edit-menu"></a>

Cofnij, Ponów i Undo History; przełączanie między trybem wstawiania i
zastępowania; polecenia, które zaznaczają blok, blok kolumnowy albo cały plik,
i zdejmują zaznaczenie; kopiowanie, przenoszenie i usuwanie bloku; kopiowanie
i wycinanie do pliku schowka i wklejanie z niego; oraz przejście na początek i
na koniec pliku.

## Menu Wyszukaj <a id="search-menu"></a>

Wyszukaj, Wyszukaj ponownie i Zastąp; filtr wierszy (Toggle line filter,
Filter by word; zobacz
[Szukanie, zastępowanie i filtr wierszy](#search-and-replace));
oraz zakładki: stawianie i zdejmowanie zakładki w bieżącym wierszu, przejście
do następnej albo poprzedniej i usunięcie wszystkich.

## Menu Polecenie <a id="command-menu"></a>

**Przejdź do wiersza...**
: Przechodzi do wiersza o podanym numerze.

**Przełącz stan wiersza**
: Pokazuje albo ukrywa kolumnę numerów wierszy.

**Przejdź do pasującego nawiasu**
: Przechodzi do nawiasu pasującego do tego pod kursorem.

**Przełącz wyróżnianie składni, Przełącz prawy margines, Toggle control characters**
: Włączają i wyłączają te funkcje.

**Kodowanie...**
: Strona kodowa, w której zapisano tekst. Zobacz
[Wybór strony kodowej](#codepages-translation).

**Fold / Unfold block, Unfold all**
: Zobacz [Zwijanie](#folding).

**Odśwież ekran**
: Odświeża ekran.

**Rozpocznij/zatrzymaj nagrywanie makra, Macro explorer..., Nagraj/powtórz działania**
: Zobacz [Makra](#macros).

## Menu Navigate <a id="navigate-menu"></a>

Polecenia wtyczek: przejście do deklaracji, szukanie odwołań, symbole pliku i
inne. Zobacz
[Wtyczki](#plugins).

## Menu Okno <a id="window-menu"></a>

**Przenieś, Zmień rozmiar**
: Przesuwa okno albo zmienia jego rozmiar strzałkami; Enter kończy.

**Przełącz pełny ekran**
: Przełącza okno między pełnym ekranem a jego własnym rozmiarem.

**Toggle sticky windows**
: Gdy okna są sklejone, okna, których ramki przylegają do siebie, zmieniają
rozmiar razem: róg okna przesuwa krawędź, na której leży, razem ze wszystkimi
oknami wzdłuż niej, po obu stronach, i nie powstaje między nimi szczelina.
Tam, gdzie spotykają się cztery okna, róg przesuwa obie krawędzie. Bok, który
na początku zmiany rozmiaru leży na krawędzi ekranu, zostaje tam, okno
zachowuje swój najmniejszy rozmiar, a krawędź zatrzymuje się na oknie, które
stoi jej na drodze, i to okno zmienia wtedy rozmiar razem z innymi. Esc
przywraca wszystkie okna. Okno przeciągane za tytuł przesuwa się samo, bez
innych. Ustawienie jest przechowywane jako
**editor_sticky_windows**
w pliku ini.

**Następne, Poprzednie**
: Przechodzi do następnego albo poprzedniego okna.

**Lista...**
: Lista otwartych okien, z której można przejść do jednego z nich. Zobacz
[Otwarcie plików](#open-files).

Pod nimi są okna wtyczek: drzewo projektu, wynik budowania, panel i konsola
debugera, terminal, terminal programu uruchomionego przez debuger, Preview.
Pozycja otwiera swoje okno, wysuwa je na wierzch, gdy jest za innym, i
zamyka je, gdy już jest na wierzchu; otwarte okno jest oznaczone *\**. Wyłączona
wtyczka nie ma pozycji.

Okna wtyczek stoją w dokach. Drzewo, panel debugera i Preview trafiają do
kolumny po prawej, jedno nad drugim, na całą wysokość ekranu. Konsola, wynik
budowania i terminal trafiają do rzędu na dole, pod oknami plików, i dzielą
go jako karty: widać jedną, a jej tytuł wymienia nazwy wszystkich. Kliknięcie w
nazwę albo
**Ctrl-Alt-PgDn** i **Ctrl-Alt-PgUp** (*WindowTabNext* i *WindowTabPrev*
w mapie klawiszy) pokazuje inną. Okno pliku na pełnym ekranie wypełnia resztę
ekranu, a gdy doki są puste, znowu cały ekran. Zmiana rozmiaru zadokowanego
okna zmienia rozmiar doku; przesunięcie okna wyjmuje je z doku.

**Layout...**
: Układy okien: które okna wtyczek są otwarte, w którym doku i jak duże są
doki. *Edit* ma same pliki, *Project* drzewo projektu, *Debug* drzewo i panel
debugera. **Use** ustawia okna w ten sposób; **Save as...** zapisuje bieżące
ułożenie pod nazwą, także pod nazwą jednego z tych układów; **Delete** usuwa
zapisany układ i wraca wtedy wbudowany układ o tej nazwie. Przy **Debug
layout while debugging** start debugera ustawia okna tak, jak ma je *Debug*,
a jego zatrzymanie przywraca poprzednie, przy czym karty na dole zostają,
aby można je było przeczytać.

Okna projektu są zachowywane, gdy edytor kończy pracę, i wracają, gdy startuje
z plikiem projektu.
*bluerig --debug* startuje z układem *Debug*. Układy są przechowywane w
*layouts.ini* w katalogu ustawień.

## Menu Plugins <a id="plugins-menu"></a>

Wtyczki edytora, które mają własną pozycję, na przykład sprawdzanie pisowni;
ich okna są w [menu Okno](#window-menu). Wtyczki wyłączonej w
[Manage plugins](#manage-plugins)
nie ma w menu.

## Menu Opcje <a id="options-menu"></a>

**Ogólne...**
: [Opcje edytora](#editor-options).

**Tryb zapisu...**
: Jak zapisywany jest plik. Zobacz [Modyfikuj tryb zapisu](#edit-save-mode).

**Wyróżnianie elementów składni...**
: Wybiera ręcznie reguły składni dla pliku albo zostawia je nazwie i
pierwszemu wierszowi pliku.

**Wygląd...**
: Skórka i cienie. Zobacz [Wygląd](#appearance).

**Określ klawisze...**
: Zobacz [Określ klawisze](#learn-keys).

**Key bindings...**
: Zobacz [Key bindings](#key-bindings).

**Key sniffer...**
: Zobacz [Key sniffer](#key-sniffer).

**Manage plugins...**
: Zobacz [Manage plugins](#manage-plugins).

**Plugin info...**
: Zobacz [Plugin info](#plugin-info).

**Lua scripts...**
: Zobacz [Lua scripts](#lua-scripts). Pozycja jest tylko tam, gdzie bluerig
zbudowano z Lua.

**Plik składni**
: Otwiera w edytorze plik składni użytkownika,
*~/.local/share/bluerig/syntax/Syntax*.
Gdy nie istnieje, jest tworzony z krótkim komentarzem o tym, co zawiera, a nie
jako kopia pliku systemowego, który nadal jest czytany po nim. Zobacz
[Podświetlanie składni](#syntax-highlighting).

**Menu file**
: Otwiera w edytorze plik [menu użytkownika](#edit-menu-file).

**Zapisz ustawienia**
: Zapisuje ustawienia do
*~/.config/bluerig/ini*.
Przy włączonym ustawieniu
*auto_save_setup*
są zapisywane przy każdym zakończeniu pracy edytora.

### Opcje edytora <a id="editor-options"></a>

Pozycja
**Ogólne...**
menu Opcje otwiera okno opcji edytora. Każda opcja jest zapisywana do pliku
ini, zobacz
[Ustawienia w pliku ini](#internal-file-editor-options).

**Tryb zawijania**

|  |  |
|--|--|
| Brak | Wiersze nie są zawijane |
| Dynamiczne akapity | Formatuje akapit pod kursorem w trakcie pisania |
| Maszyna do pisania | Łamie wiersz na długości zawijania w trakcie pisania |

**Tabulacja**

|  |  |
|--|--|
| Połówki tabulacji | Między lewym marginesem a tekstem tabulacja przesuwa o połowę swojej szerokości, a w pliku są spacje i tabulacje |
| Backspace przez tabulacje | Backspace usuwa całe wcięcie do lewego marginesu, gdy między kursorem a marginesem nie ma tekstu |
| Wypełnianie tabulacji spacjami | Wstawia spacje do następnej pozycji tabulacji zamiast znaku tabulacji |
| Rozmiar tabulacji | Szerokość znaku tabulacji; domyślnie 8 |

**Keyboard**

|  |  |
|--|--|
| Esc timeout, ms | Ile milisekund Esc czeka na resztę klawisza, zanim zostanie uznany za Esc; domyślnie 1000. W pliku ini to *old_esc_mode_timeout* w mikrosekundach, a zmienna **KEYBOARD_KEY_TIMEOUT_US** przy starcie ma przed nim pierwszeństwo |

**Inne opcje**

|  |  |
|--|--|
| Automatyczne wcięcia | Enter zaczyna wiersz z wcięciem wiersza powyżej |
| Potwierdzenie przed zapisaniem | Pyta przed zapisaniem pliku |
| Zapisanie pozycji pliku | Otwiera plik tam, gdzie go zostawiono |
| Widoczne spacje kończące | Oznacza spacje na końcu wierszy |
| Widoczne tabulacje | Oznacza znaki tabulacji |
| Show control characters | Pokazuje znaki sterujące jako ^M zamiast je ukrywać |
| Wyróżnianie elementów składni | Koloruje tekst regułami składni jego typu |
| Kursor po wstawionym bloku | Zostawia kursor na końcu wstawionego bloku, a nie na jego początku |
| Trwałe zaznaczenie | Zachowuje zaznaczenie, gdy kursor się przesuwa |
| Kursor poza końcem wiersza | Pozwala kursorowi wyjść za ostatni znak wiersza |
| Grupowe cofnięcie | Cofa naraz serię zmian jednego rodzaju |
| Punkt łamania wiersza | Kolumna, w której tryby zawijania łamią wiersz; domyślnie 72 |

### Modyfikuj tryb zapisu <a id="edit-save-mode"></a>

Jak zapisywany jest plik; te same trzy tryby przyjmuje ustawienie
*editor_option_save_mode*:

**Szybki zapis**
: Od razu nadpisuje plik. Jest szybki, ale awaria w połowie zostawia plik
zapisany do połowy.

**Bezpieczny zapis**
: Najpierw zapisuje plik tymczasowy, a gdy jest kompletny, zastępuje nim
oryginał, więc awaria nie narusza oryginału.

**Wykonaj kopie zapasowe za pomocą tego rozszerzenia**
: Bezpieczny zapis, a oryginał zostaje pod swoją nazwą z dodanym
rozszerzeniem z wiersza wprowadzania, domyślnie "~". Kolejny zapis zastępuje
też kopię zapasową.

**Sprawdź nowe wiersze POSIX**
: Przed zapisem pyta o brakujący znak nowego wiersza na końcu pliku.

### Wygląd <a id="appearance"></a>

Skórka programu i to, czy okna dialogowe i menu
rozwijane mają cień.

**Skórka**
: Przycisk otwiera listę zainstalowanych skórek i skórek użytkownika; wybrana
jest stosowana od razu.

**Cienie**
: Rysuje cień pod każdym oknem dialogowym i menu rozwijanym. Bez kolorów nie
ma cieni.

Wybór jest zapisywany do ustawień
*skin*
i
*shadows*
pliku ini. O samych plikach skórek mówi rozdział
[Skórki](#skins).

### Określ klawisze <a id="learn-keys"></a>

To okno uczy bluerig sekwencji sterujących, które terminal wysyła dla
klawiszy funkcyjnych, strzałek i klawiszy nawigacji. Jego tytuł podaje nazwę
terminala, ze zmiennej
**TERM**.

Wybierz kombinację modyfikatorów (Ctrl, Alt, Shift) polami wyboru, a potem
naciśnij przycisk klawisza. Naciśnij klawisz na klawiaturze i poczekaj, aż
zniknie komunikat przechwytywania. Nauczona sekwencja pojawia się obok
przycisku. Sekwencja, którą ma już inny klawisz, jest zgłaszana jako konflikt.

**Del**
: Czyści nauczony klawisz, na którym stoi kursor.

**Zapisz**
: Zapisuje nauczone klawisze do
*~/.config/bluerig/term/\<TERM>*.

**Edit term file**
: Otwiera ten plik w edytorze.

### Key bindings <a id="key-bindings"></a>

Działania programu z klawiszami, na które odpowiadają, w tabeli. Przyciski u
góry wybierają grupę, Editor albo Widgets, a potem jej część: Main i Ctrl-X
dla edytora; Dialog, Menu, Input, Listbox, Radio i Help dla widżetów. Back
wraca do grup. Działania oznaczone \* mają klawisze inne niż domyślne.

**Enter**
: Zastępuje klawisz działania: naciśnij klawisz do przypisania.

**F5**
: Dodaje działaniu kolejny klawisz.

**F8, Del**
: Usuwa klawisz.

**Zapisz**
: Zapisuje zmiany do
*~/.config/bluerig/keymap.ini*.

**Edit keymap file**
: Otwiera
*keymap.ini*
w edytorze.

**Edit term file**
: Otwiera sekwencje klawiszy terminala, plik, który zapisuje
[Określ klawisze](#learn-keys).

Klawisz zajęty przez inne działanie jest zgłaszany z pytaniem, czy go stamtąd
zabrać.

### Key sniffer <a id="key-sniffer"></a>

Naciśnij
**Capture key**,
a potem dowolny klawisz. Okno pokazuje, co przyszło:

```
Shortcut   the symbolic name, e.g. Ctrl-F5
Action     the action of the key in the editor keymap
Raw        the escape sequence and its bytes in hex
Keycode    the internal numeric code
```

Tu należy zajrzeć, gdy klawisz nic nie robi albo robi nie to, co trzeba:
klawisza, który przychodzi jako sekwencja nierozpoznawana przez program,
trzeba nauczyć w
[Określ klawisze](#learn-keys),
a taki, który przychodzi, ale nie ma działania, trzeba przypisać w
[Key bindings](#key-bindings).

### Manage plugins <a id="manage-plugins"></a>

Wtyczki wczytane przez program, w tabeli: rodzaj, nazwa i to, co wtyczka
mówi o sobie. Są tu wtyczki edytora, a tam, gdzie bluerig zbudowano z Lua,
także silnik Lua jako wtyczka rodzaju
*runtime*.
Wtyczkę wyłącza się i włącza polem wyboru w jej wierszu, a wyłączona nie jest
też wczytywana następnym razem; wybór jest przechowywany w
*~/.config/bluerig/plugins.ini*.
Wyłączona wtyczka zostaje na liście, aby można ją było znowu włączyć.

**Enter, F4**
: Otwiera ustawienia wtyczki, na której stoi kursor. Wtyczka, która ich nie
ma, informuje o tym. W wierszu silnika Lua otwiera
[Lua scripts](#lua-scripts).

**IDE: project, build, debugger**
: Włącza razem trzy wtyczki, które robią z edytora IDE, albo wyłącza je
razem, gdy wszystkie są włączone: projekt i jego pliki, budowanie i debuger.
Każda z nich działa też bez pozostałych.

### Plugin info <a id="plugin-info"></a>

Wtyczki wczytane przez edytor: nazwa, czy jest włączona, co daje i co robi.
To lista do oglądania; wtyczkę wyłącza się i otwiera jej ustawienia w oknie
[Manage plugins](#manage-plugins).

### Lua scripts <a id="lua-scripts"></a>

Znalezione skrypty Lua, systemowe i użytkownika, w tabeli: czy jest włączony,
gdzie leży skrypt (globalny albo użytkownika), jego identyfikator, jego
przestrzeń robocza, co daje i co robi. Skrypt wyłącza się i włącza polem
wyboru w jego wierszu; zmiana działa od następnego startu bluerig.

**Settings**
: Uruchamia skrypt z ustawieniami pakietu, jeśli pakiet go ma.

O samych skryptach mówi [Skrypty Lua](#lua-scripting).

# Szukanie, zastępowanie i filtr wierszy <a id="search-and-replace"></a>

{{key:Search|F7}} otwiera okno szukania.

|  |  |
|--|--|
| Zwykły | Szuka napisu dosłownie |
| Wyrażenie regularne | Szuka według wyrażenia regularnego |
| Szesnastkowo | Szuka bajtów zapisanych jako liczby szesnastkowe |
| Wyszuk. wieloznaczników | Szuka według wzorca powłoki z \* i ? |
| Rozróżnianie wielkości | Odróżnia małe litery od wielkich |
| Wstecz | Szuka od kursora do początku pliku |
| W wybranych | Szuka tylko w zaznaczonym bloku |
| Całe słowa | Dopasowuje tylko całe słowo |
| Wszystkie zestawy znaków | Szuka w każdym kodowaniu, w jakim plik może być zapisany |
| Znajdź wszystkie | Stawia zakładkę w każdym wierszu z trafieniem |
| Filter | Ukrywa wiersze bez trafienia |

{{key:Replace|F4}} otwiera okno zastępowania: te same pola i wiersz
"Zastępujący napis". W zastępowaniu wyrażeniem regularnym \\1 do
\\9 wstawiają dopasowane grupy.

|  |  |
|--|--|
| Zastąp | Zastępuje to trafienie i szuka następnego |
| Wszystkie | Zastępuje każde trafienie bez pytania |
| Pomiń | Zostawia to trafienie i szuka następnego |

**Filter**

Ukrywa wiersze bez trafienia; numery wierszy się nie zmieniają, a kolumna
stanu wiersza oznacza każdy ukryty fragment. Wiersze są ukrywane raz, w
chwili naciśnięcia przycisku: wiersz zmieniony albo wpisany później zostaje
pokazany, nawet jeśli nie pasuje.

|  |  |
|--|--|
| {{key:FilterToggle|Alt-s}} | Zdejmuje filtr albo stosuje ostatnie szukanie jako filtr; gdy nic nie szukano, otwiera okno szukania |
| {{key:FilterWord|Alt-S}} | Filtruje według zaznaczonego tekstu albo słowa pod kursorem |
| Polecenie \> Unfold all | Zdejmuje filtr i rozwija wszystkie zwinięcia |

# Zwijanie <a id="folding"></a>

Zwinięcie chowa ciąg wierszy za jego pierwszym wierszem, który zostaje na
ekranie ze znakiem w kolumnie stanu wiersza.
**Alt-F**
(Fold / Unfold block w menu Polecenie) zwija zaznaczone wiersze, a gdy nic
nie jest zaznaczone, wiersze od nawiasu w bieżącym wierszu do pasującego do
niego nawiasu. Na pierwszym wierszu zwinięcia rozwija je.
**Unfold all**
rozwija wszystkie zwinięcia pliku i zdejmuje filtr wierszy.

# Makra <a id="macros"></a>

{{key:MacroStartStopRecord|Ctrl-r}}
zaczyna nagrywanie makra: klawisze naciśnięte po nim są nagrywane. Ponowne
naciśnięcie
**Ctrl-r**
kończy nagrywanie i czeka na klawisz, do którego makro zostanie przypisane;
ten klawisz potem je uruchamia.

Makra są przechowywane w sekcji
**[editor]**
pliku
**~/.local/share/bluerig/macros**,
a makro usuwa się, usuwając tam jego wiersz, albo w oknie
[Macro Explorer](#macro-explorer).

**Nagraj/powtórz działania**
w menu Polecenie nagrywa następne działania, dopóki nie zostanie wybrane
ponownie, a potem pyta, ile razy je powtórzyć.

Do klawisza można też przypisać skrypt powłoki, wierszem pliku
**macros**
takim jak ten:

```
[editor]
Ctrl-W=ExecuteScript:25;
```

Oznacza to, że Ctrl-W uruchamia działanie
*ExecuteScript(25)*,
które uruchamia skrypt powłoki
**~/.local/share/bluerig/macros.d/macro.25.sh**.
Skrypty są przechowywane w katalogu
**~/.local/share/bluerig/macros.d/**
i muszą nazywać się
**macro.XXXX.sh**,
gdzie
**XXXX**
to liczba od 0 do 9999. Przykłady są instalowane w
**{{pkgdatadir}}/examples/macros.d/**.
Skrypt ma format pozycji
[menu użytkownika](#edit-menu-file)
i przyjmuje te same podstawienia. Jego pierwszym wierszem może być

*#silent*
: Skrypt jest uruchamiany bez pytania o parametry.

Oto przykładowy skrypt:

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

Nagrane makra, z klawiszem, na który każde odpowiada, i tym, co robi.
Przyciski to

**Run**
: Odtwarza makro, na którym stoi kursor.

**Delete**
: Usuwa je po pytaniu.

**Modyfikacja pliku**
: Otwiera plik, w którym są zapisane makra, opisany w rozdziale
[Makra](#macros).

# Menu użytkownika <a id="edit-menu-file"></a>

{{key:UserMenu|F11}}
otwiera menu użytkownika: każda jego pozycja to skrypt powłoki uruchamiany na
tekście edytora. Menu jest czytane z pierwszego znalezionego pliku z:

```
1) ./.bluerig.menu in the current directory
2) ~/.config/bluerig/menu
3) ~/bluerig.menu
4) {{sysconfdir}}/bluerig/bluerig.menu
5) {{pkgdatadir}}/bluerig.menu
```

Plik bieżącego katalogu jest czytany tylko wtedy, gdy należy do użytkownika
albo do roota i nikt inny nie ma prawa go zapisywać: jego pozycje uruchamiają
polecenia. Pozycja
**Menu file**
menu Opcje otwiera plik menu w edytorze.

Wiersz, który zaczyna się czymkolwiek poza spacją albo tabulacją, jest
pozycją menu, a jego pierwszy znak to klawisz skrótu pozycji. Wiersze, które
zaczynają się spacją albo tabulacją, to polecenia pozycji. Wiersz, który
zaczyna się od '#', jest komentarzem.

Gdy pozycja zostanie wybrana, jej polecenia są zapisywane do pliku
tymczasowego, podstawienia w nich są zastępowane, a powłoka wykonuje ten plik:

|  |  |
|--|--|
| %f, %p, %s | Nazwa bieżącego pliku |
| %n | Nazwa bieżącego pliku bez rozszerzenia |
| %x | Rozszerzenie nazwy bieżącego pliku |
| %d | Bieżący katalog |
| %c | Pozycja kursora w pliku |
| %i | Wcięcie ze spacji tak szerokie jak kolumna kursora |
| %y | Typ składni bieżącego pliku |
| %b | Plik bloku: zaznaczony tekst jest zapisywany do **~/.cache/bluerig/block** przed uruchomieniem, a to, co polecenia tam zostawią, zastępuje blok |
| %m | Nazwa pliku menu |
| %% | Znak % |
| %{text} | Wiersz wprowadzania z zachętą "text"; wpisany tekst jest wstawiany, Esc albo F10 anulują pozycję |
| %view | Otwiera standardowe wyjście poleceń w nowym oknie edytora; słowa w nawiasach klamrowych po nim, jak w %view{ascii}, są pomijane |

W literach podstawień wielkość nie ma znaczenia. Liczba po % włącza
cytowanie (domyślnie) albo je wyłącza: %f i %1f cytują wstawioną wartość, %0f
nie.

Przykładowy plik menu:

```
s       Sort the block
	sort %b > %b.tmp && mv -f %b.tmp %b

w       Count the words of the file
	%view wc %f

m       Run make in the directory of the file
	make
```

**Pozycja domyślna**

Wiersz, który zaczyna się od '=' w pierwszej kolumnie, jest warunkiem pozycji
pod nim: gdy warunek jest prawdziwy, pozycja staje się domyślna.

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

Wzorzec to wzorzec powłoki albo wyrażenie regularne, zależnie od ustawienia
*shell_patterns*;
wiersz "shell_patterns=0" albo "shell_patterns=1" jako pierwszy wiersz pliku
menu ustawia je dla tego pliku.

Typ to jeden albo więcej znaków:

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

Na przykład 'rlf' to zwykły plik, dowiązanie albo FIFO.

Warunek, który zaczyna się od '=?' zamiast '=', pokazuje ślad debugowania
za każdym razem, gdy jest obliczany.

Warunki są obliczane od lewej do prawej; '&' nie wiąże silniej niż '|':

```
	= f *.c | f *.h & y C
```

jest obliczane jako

```
	( (f *.c) | (f *.h) ) & (y C)
```

**Warunki dodania**

Warunek, który zaczyna się od '+' (albo '+?'), wstawia pozycję do menu tylko
wtedy, gdy jest prawdziwy. Warunek z '+=' albo '=+' ('+=?' i '=+?' ze śladem)
działa jednocześnie jako warunek dodania i warunek domyślny. Dwa wiersze
warunków przed pozycją, jeden z '+' i jeden z '=', ustawiają oba warunki osobno.

# Wtyczki <a id="plugins"></a>

{{plugins}}
Wtyczkę włącza się i wyłącza w menu Opcje, pozycja
[Manage plugins](#manage-plugins).

# Okna dialogowe <a id="dialogs"></a>

## Zapisz jako <a id="save-file-as"></a>

Nazwa, pod którą zapisać plik, i końce wierszy, z którymi go zapisać: takie,
jakie ma plik, Unix (LF), Windows i DOS (CR LF) albo Macintosh (CR). Wybór
obowiązuje dla tego zapisu; plik zachowuje to, co dostał.

## Otwarcie plików <a id="open-files"></a>

Pliki otwarte w edytorze, po jednym w wierszu. Enter przechodzi do pliku, na
którym stoi kursor, Esc pozostaje przy bieżącym pliku. Ta sama lista to
pozycja
**Lista...**
menu Okno.

## Wybór ekranu <a id="screen-selector"></a>

Poza edytorem skrypt Lua może otwierać własne ekrany, widoki pełnoekranowe,
które zostają otwarte, gdy wracasz do edytora. Między ekranami przełącza się
na trzy sposoby:

**Alt-}**
: przełącza na następny ekran;

**Alt-{**
: przełącza na poprzedni ekran;

**Alt-\`**
: otwiera okno z listą otwartych ekranów, aby wybrać jeden.

## Uzupełnianie <a id="completion"></a>

{{key:Complete|Alt-Tab}}
w edytorze uzupełnia słowo przed kursorem słowami otwartych plików i pokazuje
listę, gdy pasuje kilka słów. Ustawienie
*editor_wordcompletion_collect_all_files*
pliku ini, domyślnie włączone, bierze słowa ze wszystkich otwartych plików;
gdy jest wyłączone, tylko z bieżącego pliku.

**Alt-Tab**
w wierszu wprowadzania okna dialogowego uzupełnia tekst:

|  |  |
|--|--|
| $name | Zmienną środowiska |
| ~name | Nazwę użytkownika |
| Inaczej | Nazwę pliku |

Gdy pasuje kilka, program wydaje dźwięk. Ustawienie
*show_all_if_ambiguous*
pliku ini, domyślnie wyłączone, pokazuje ich listę obok kursora:
**Enter**
wybiera jedną, wpisywane litery zawężają listę, Esc, F10, Left i Right ją
chowają.

## Wybór strony kodowej <a id="codepages-translation"></a>

Lista stron kodowych znanych programowi, z
**{{pkgdatadir}}/charsets**.
Wybór jednej mówi edytorowi, w jakiej stronie kodowej zapisano tekst pliku, a
tekst jest przekodowywany z niej na stronę kodową terminala do wyświetlenia;
**\<No translation>**
zostawia go jako bajty. Listę otwiera
**Alt-e**
i pozycja Kodowanie menu Polecenie.

## Historia <a id="history-query"></a>

Lista tego, co wcześniej wpisano w wiersz wprowadzania, od najnowszych,
którą
**Alt-h**
otwiera dla wiersza, w którym stoi kursor. Enter wstawia do wiersza wpis, na
którym stoi kursor, Esc zostawia wiersz taki, jaki był, a
**F8, Del**
usuwa z historii wpis, na którym stoi kursor. Historie są przechowywane w
*~/.local/share/bluerig/history*;
*num_history_items_recorded*
określa, ile wpisów zachowuje każda z nich.

# Podświetlanie składni <a id="syntax-highlighting"></a>

{{key:SyntaxOnOff|Ctrl-s}}
włącza i wyłącza podświetlanie składni. Reguły podświetlania są w plikach
składni: słowa kluczowe i konteksty, takie jak komentarze i napisy, dostają
tam swoje kolory.

|  |  |
|--|--|
| [Pliki składni](#syntax-files) | Gdzie są reguły i w jakiej kolejności są czytane |
| [Sekcja file](#syntax-file) | Które pliki podświetla sekcja reguł |
| [Konteksty](#syntax-contexts) | Obszary tekstu z własnymi kolorami |
| [Osadzanie i warstwy](#syntax-embed) | Reguły innej składni wewnątrz kontekstu i nad tekstem |
| [Słowa kluczowe](#syntax-keywords) | Słowa i wzorce kolorowane wewnątrz kontekstu |
| [Kolory i atrybuty](#syntax-colors) | Kolory, tokeny zakresów skórki, atrybuty |
| [Inne wiersze](#syntax-other) | Komentarze, caseinsensitive, line-local |

## Pliki składni <a id="syntax-files"></a>

|  |  |
|--|--|
| **~/.local/share/bluerig/syntax/Syntax** | Reguły użytkownika: to, co użytkownik dodał albo chce mieć inaczej |
| **{{pkgdatadir}}/syntax/Syntax** | Zainstalowane reguły, dla typów plików, których plik użytkownika nie wymienia |

Wpis użytkownika zastępuje zainstalowany dla plików, do których
pasuje. Pliki wymienione w wierszach
**include**
są szukane najpierw w
*~/.local/share/bluerig/syntax/*,
a potem wśród zainstalowanych. Plik użytkownika jest czytany ponownie za
każdym razem, gdy w edytorze otwiera się nowy plik.

Pełna kopia zainstalowanego pliku, jaką robiły starsze wersje, przesłania
wszystkie typy plików zainstalowane później i ich podświetlanie przestaje
działać: zostaw w kopii tylko zmienione wpisy.

## Sekcja file <a id="syntax-file"></a>

Plik składni składa się z sekcji. Każda zaczyna się wierszem
**file**
i zwykle jest umieszczona we własnym pliku z wierszem
**include**:

```
file <file name> <description> [<first line>]
```

*file name*
i
*first line*
to wyrażenia regularne: sekcja dotyczy pliku, gdy pasuje jego nazwa albo
pierwszy wiersz jego tekstu.
*description*
to nazwa składni, z \s zamiast spacji: używają jej wiersze
**embed**
i
**overlay**.
Sekcja kończy się tam, gdzie zaczyna się następna, i dzieli się na konteksty
ze słowami kluczowymi.

Prosta sekcja dla programu w C:

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

## Konteksty <a id="syntax-contexts"></a>

Kontekst to obszar tekstu z własnym kolorem i własnymi słowami kluczowymi,
taki jak komentarz C między /\* a \*/. Zaczyna się wierszem:

```
context [exclusive] [oneline] [whole|wholeright|wholeleft] [linestart] <delim>
        [linestart] <delim> [<foreground>] [<background>] [<attributes>]
```

Pierwszym kontekstem sekcji jest
**context default**
[*foreground*] [*background*] [*attributes*];
bez niego sekcja daje błąd.

|  |  |
|--|--|
| linestart | Ogranicznik stoi na początku wiersza |
| whole | Ogranicznik jest całym słowem |
| wholeleft | Ogranicznik jest na granicy słowa z lewej |
| wholeright | Ogranicznik jest na granicy słowa z prawej |
| exclusive | Koloruje tekst między ogranicznikami, a nie ograniczniki |
| oneline | Po exclusive: kończy kontekst na końcu wiersza, gdy nie ma w nim prawego ogranicznika |

Wiersz
**wholechars**
[**left**|**right**] *characters*
w dowolnym miejscu pliku ustala znaki całego słowa; left i right ustalają obie
strony osobno. Wiersz
**spellcheck**
po
**context**
przekazuje słowa kontekstu do sprawdzania pisowni.

## Osadzanie i warstwy <a id="syntax-embed"></a>

Wiersz

**embed**
[**soft**]
*type*

po
**context**
przekazuje tekst między ogranicznikami regułom innej składni;
*type*
to opis z jej wiersza file, z \s zamiast spacji. Typ jest szukany w pliku
Syntax, z którego czytane są reguły, a potem w tym zainstalowanym z bluerig.
Typ nieznaleziony w żadnym zostawia kontekst w jego własnych kolorach.
Ograniczniki zachowują kolory kontekstu. Prawy ogranicznik kończy osadzony
tekst, gdziekolwiek stoi, także wewnątrz napisu albo komentarza osadzonych
reguł. Osadzona składnia może osadzić inną, na kilka poziomów w głąb. Na
przykład HTML przekazuje skrypty regułom JavaScriptu:

```
  context <SCRIPT*> </SCRIPT> brightcyan
      embed JavaScript\sProgram
```

Z
**soft**
prawy ogranicznik kończy osadzony tekst tylko poza napisami, komentarzami i
słowami kluczowymi osadzonych reguł, więc w {{ "}}" }} szablonu Jinja2
pierwsze }} zostaje częścią napisu, a w ${x/\\}/y} skryptu powłoki pierwsze }
zostaje częścią sekwencji ucieczki.

Wiersz

**overlay**
*type*

wśród reguł robi z całej składni warstwę nad
*type*:
jej konteksty kolorują to, co zawierają, gdziekolwiek stoją, a resztę tekstu
czytają reguły
*type*,
które widzą zawartość kontekstów jako puste miejsca. W ten sposób tagi języka
szablonów są kolorowane jednakowo w tekście, wewnątrz tagu HTML, w napisie
atrybutu i w skrypcie, a HTML koloruje resztę. Kontekst domyślny warstwy
niczego nie koloruje: słowo kluczowe w nim tylko nie pozwala zacząć się
innemu kontekstowi. Typ nieznaleziony w żadnym pliku Syntax zostawia warstwę nad
zwykłym tekstem. Na przykład Smarty nad HTML:

```
  overlay HTML\sFile
  context default
      keyword {\s
  context {\* \*} brown
  context { } brightcyan
      embed soft Smarty\sTag
```

## Słowa kluczowe <a id="syntax-keywords"></a>

Słowo kluczowe kontekstu podaje wiersz:

```
keyword [whole|wholeright|wholeleft] [linestart] <string> [<foreground>] [<background>] [<attributes>]
```

Napisy słów kluczowych i ograniczników kontekstów są czytane tak:

|  |  |
|--|--|
| \\t, \\s | Tabulacja i spacja: odstępy rozdzielają słowa wiersza i nie mogą być użyte wprost |
| \\n, \\r | Nowy wiersz i powrót karetki |
| `\\` | Ukośnik wsteczny |
| \\\* | Gwiazdka |
| \* | Dowolne znaki, w dowolnej liczbie |
| + | Dowolna liczba znaków słowa, także zero |
| \\[abc\\] | Dowolna liczba znaków ze zbioru |
| \\{abc\\} | Jeden znak ze zbioru |

Wzorzec nie może być pierwszym ani ostatnim znakiem ogranicznika kontekstu.
Na przykład

```
  keyword         '*'      green
```

koloruje na zielono stałe znakowe C, a

```
  keyword         "*"      green
```

koloruje stałe napisowe, o ile nie przechodzą przez koniec wiersza.

Wiersz

```
  keyword  \\\n  brightgreen
```

definiuje słowo kluczowe z ukośnika wstecznego i końca wiersza. Słowa
kluczowe są dopasowywane przed ogranicznikami kontekstu, więc nie pozwala
ono kontekstowi skończyć się na końcu wiersza z ukośnikiem wstecznym, a
dyrektywa preprocesora C ciągnie się przez kilka wierszy.

Wiersz
**define**
*name words...*
definiuje nazwę, która oznacza te słowa w regułach, na przykład
**define comment brown**.

## Kolory i atrybuty <a id="syntax-colors"></a>

Kolory: black, gray, red, brightred, green, brightgreen, brown, yellow,
blue, brightblue, magenta, brightmagenta, cyan, brightcyan, lightgray i
white. "default" to domyślny kolor terminala, "base" główne kolory edytora;
ustawia atrybuty bez zmiany koloru tła. Przy 256 kolorach są też color16 do
color255, rgb000 do rgb555 i gray0 do gray23.

Kolor tekstu może być tokenem zakresu zamiast koloru: słowem pisanym
wielkimi literami, które mówi, czym jest tekst, a nie jak wygląda:

```
keyword  whole  if   KEYWORD_CONTROL
context  " " STRING_QUOTED_DOUBLE
    keyword  \\"   CONSTANT_CHARACTER_ESCAPE
keyword  linestart TODO  KEYWORD_OTHER_TODO black
```

Kolor tokenu pochodzi ze skórki, więc jedna skórka koloruje program i tekst,
a każdy język tak samo; zobacz
[Skórki](#skins).
Token jest czytany jako zakres: małymi literami, z kropkami zamiast
podkreśleń. KEYWORD_CONTROL_IMPORT to keyword.control.import, rodzaj
keyword.control, który jest rodzajem keyword. To nazwy zakresów Sublime Text
i TextMate. To, czego skórka nie koloruje, zachowuje kolory, które temu
tekstowi dawał mcedit. Tło albo atrybuty po tokenie mają pierwszeństwo przed
kolorami mcedit, jak black powyżej, ale nie przed tłem albo atrybutami,
które skórka daje samemu tokenowi. Token działa też w define oraz w
wierszach number, string i symbols reguł line-local. Zainstalowane pliki
składni używają tylko tokenów; ich lista z kolorami mcedit jest w
**{{pkgdatadir}}/syntax/README.txt**.

Plik składni wspólny z
**cooledit**
podaje kolory dla obu przez ukośnik, a bluerig bierze kolor przed nim:

```
keyword  #include  red/Orange
```

Atrybuty to bold, italic, underline, reverse i blink, kilka połączonych
znakiem plus.

## Inne wiersze <a id="syntax-other"></a>

|  |  |
|--|--|
| # ... | Komentarz, w osobnym wierszu |
| caseinsensitive | Na początku pliku: wielkość liter nie ma znaczenia |
| line-local | Tryb dla zwykłego tekstu: zeruje się przy każdym końcu wiersza i na krawędzi ekranu i czyta tylko pokazany tekst |
| number *max* *color* | W line-local: liczby dziesiętne o najwyżej *max* cyfrach |
| string *quote* *color* | W line-local: napisy w pojedynczych albo podwójnych cudzysłowach |
| symbols *characters* *color* | W line-local: znaki interpunkcyjne kolorowane pojedynczo |

# Kolory <a id="colors"></a>

bluerig próbuje wykryć, czy terminal obsługuje kolory, na podstawie bazy
terminali i nazwy terminala. Czasem się myli, więc tryb kolorowy można
wymusić albo wyłączyć odpowiednio flagami -c i -b.

Jeśli program skompilowano z menedżerem ekranu S-Lang zamiast ncurses,
sprawdza też zmienną
**COLORTERM,**
jeśli jest ustawiona, działa tak samo jak flaga -c.

Terminale, dla których tryb kolorowy jest zawsze wymuszany, podaje się zmienną
*color_terminals*
w sekcji Colors pliku inicjalizacyjnego. Wtedy bluerig nie próbuje wykryć,
czy terminal obsługuje kolory. Przykład:

```
[Colors]
color_terminals=linux,xterm
color_terminals=terminal-name1,terminal-name2...
```

Program można skompilować zarówno z ncurses, jak i z S-Lang; ncurses nie daje
sposobu na wymuszenie trybu kolorowego: ncurses używa tylko informacji z bazy
terminali.

# Skórki <a id="skins"></a>

Kolory programu i linie jego ramek są brane z pliku skórki. W oknie
[Wygląd](#appearance)
wybiera się ją z listy.

Jeśli skórka zawiera jakiekolwiek definicje true-color, w sekcji [skin]
należy ustawić klucz 'truecolors' na wartość TRUE. Jeśli true-color nie jest
używany, ale 256 kolorów tak, należy zamiast tego ustawić '256colors'.

Skórką jest pierwsza podana przez:

```
1) command line option -S <skin>, --skin=<skin>
2) environment variable BLUERIG_SKIN
3) setting skin of the [bluerig] section of the ini file
4) the skin called default
```

Nazwa może być ścieżką bezwzględną pliku skórki (z rozszerzeniem .ini albo
bez niego). W przeciwnym razie plik skórki jest szukany w (do pierwszego
znalezionego):

```
1) ~/.local/share/bluerig/skins/
2) {{sysconfdir}}/bluerig/skins/
3) {{pkgdatadir}}/skins/
```

Format plików skórek opisuje
**{{pkgdatadir}}/skins/README.txt**.
Kolory podświetlania składni pochodzą z sekcji
**[syntax]**
skórki: tam zakres podaje kolory rodzaju tekstu, a zakres i zakres
kontekstu, w którym stoi, oba rozdzielone odstępem, podają kolory tego
rodzaju tekstu w tym kontekście. Wygrywa najdłuższy zakres, który skórka
podaje:

```
[syntax]
    comment = gray
    keyword = yellow
    keyword.control = brightmagenta
    string = green
    string constant.character.escape = white;red
    invalid = white;red;bold
```

Rodzaj tekstu, którego skórka nie podaje, zachowuje kolory, które dawał mu
mcedit, więc skórka może podać kilka rodzajów i zostawić resztę. Reguły, które
podają kolory zamiast tokenów zakresów, zachowują swoje kolory niezależnie od
skórki; zobacz
[Podświetlanie składni](#syntax-highlighting).

# Bazy terminali <a id="terminal-databases"></a>

bluerig pozwala poprawić systemową bazę terminali bez uprawnień
roota. Okno
[Określ klawisze](#learn-keys)
zapisuje to, czego się nauczy, do
**~/.config/bluerig/term/\<TERM>**,
po jednym wierszu na każdy klawisz w sekcji
**[keys]**.
bluerig szuka też w pliku systemowym
**defaults.ini**
(w
**{{sysconfdir}}/bluerig**
albo
**{{pkgdatadir}}**)
i w pliku
**~/.config/bluerig/ini**
sekcji "terminal:nazwa-twojego-terminala", a potem sekcji
"terminal:general". Każdy wiersz sekcji zawiera symbol klawisza, który chcesz
zdefiniować, potem znak równości i definicję klawisza. Specjalna forma \\e
oznacza znak escape, a ^x znak control-x. Wiersz
*copy=other*
dołącza też wszystkie klawisze sekcji "terminal:other".

Możliwe symbole klawiszy:

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

Na przykład, aby klawisz insert był sekwencją Escape + [ + O + p, ustaw w
pliku ini:

```
insert=\e[Op
```

Symbol klawisza może mieć modyfikatory:

```
    ctrl-alt-right=\e[[1;6C
    ctrl-alt-left=\e[[1;6D
```

Oznacza to, że ctrl+alt+left wysyła sekwencję sterującą \\e[[1;6D, więc
bluerig odczytuje "\\e[[1;6D" jako Ctrl-Alt-Left.

Symbol klawisza
*complete*
oznacza sekwencje sterujące wywołujące uzupełnianie; wywołuje się je Alt-tab,
ale można zdefiniować inne klawisze do tego samego celu (na klawiaturach z
mnóstwem ładnych i nieużywanych klawiszy).

# Skrypty Lua <a id="lua-scripting"></a>

Tam, gdzie bluerig zbudowano z Lua, uruchamia on skrypty Lua, które
rozszerzają edytor: działania przypisane do klawiszy i menu, reakcje na
zdarzenia edytora (otwarcie pliku, jego zapis, naciśnięcie klawisza), okna
dialogowe i widoki pełnoekranowe. Kilka poleceń edytora to takie skrypty:
sortowanie wierszy, formatowanie akapitu, wstawianie wyniku polecenia,
wstawianie znaku według jego kodu, wstawianie daty, dekodowanie base64 i
rysowanie tabel.

Skrypty systemowe są w
**{{pkgdatadir}}/lua/scripts/**,
a skrypty użytkownika w
**~/.local/share/bluerig/lua/scripts/**,
po jednym katalogu na każdy skrypt, z jego
**lua.ini**
i jego
**init.lua**.
Okno
[Lua scripts](#lua-scripts)
włącza je i wyłącza.

Opcja
**--no-lua**
albo zmienna
**BLUERIG_NO_LUA**
ustawiona na 1 uruchamia edytor bez żadnego skryptu Lua, a
*enabled=false*
w sekcji
**[Lua]**
pliku ini robi to na stałe.

Jak napisać skrypt i jakie funkcje skrypt może wywołać, opisują
**LUA_PLUGINS.md**
i
**LUA_API_REFERENCE.md**,
instalowane razem z dokumentacją bluerig.

# Ustawienia ogólne <a id="general-settings"></a>

Te ustawienia sekcji
**[bluerig]**
pliku ini dotyczą programu, a nie edytora. O ile nie podano inaczej, 1
włącza ustawienie, a 0 je wyłącza.

*auto_save_setup*
: Zapisuje ustawienia do pliku ini przy każdym zakończeniu edytora.

*skin*
: Nazwa skórki. Zobacz [Skórki](#skins).

*shadows*
: Rysuje cień pod oknami dialogowymi i menu rozwijanymi.

*keymap*
: Plik klawiszy do użycia. Zobacz [Zmiana przypisań klawiszy](#keys_redefine).

*drop_menus*
: Po naciśnięciu F9 od razu otwiera pierwsze menu, a nie tylko pasek menu.

*mouse_close_dialog*
: Kliknięcie poza oknem dialogowym zamyka je.

*mouse_repeat_rate, double_click_speed*
: Tempo powtarzania przycisków myszy i czas, w którym dwa kliknięcia dają
podwójne kliknięcie, w milisekundach.

*old_esc_mode, old_esc_mode_timeout*
: Przy włączonym old_esc_mode pojedynczy Esc jest uznawany za Esc po
old_esc_mode_timeout mikrosekundach, zamiast czekać na następny klawisz.

*show_all_if_ambiguous*
: Pokazuje od razu listę możliwych uzupełnień, gdy uzupełnienie jest
niejednoznaczne. Zobacz [Uzupełnianie](#completion).

*shell_patterns*
: Wzorce warunków menu użytkownika są wzorcami powłoki (1) albo wyrażeniami
regularnymi (0).

*num_history_items_recorded*
: Ile wpisów zachowuje każda historia wiersza wprowadzania.

*confirm_history_cleanup*
: Pyta przed wyczyszczeniem historii.

*clear_before_exec*
: Czyści ekran przed uruchomieniem polecenia menu użytkownika.

*pause_after_run*
: Czeka na klawisz po wykonaniu polecenia menu użytkownika: nigdy (0), tylko na
prostych terminalach (1, domyślnie) albo zawsze (2).

*alternate_plus_minus*
: Zostawia klawisze plus i minus klawiatury numerycznej jako plus i minus.

*keybar_modifiers*
: Gdy włączone, pasek klawiszy pokazuje polecenia Shift, Ctrl albo Alt z
klawiszami F, gdy ten modyfikator jest wciśnięty. Wymaga to terminala z
protokołem klawiatury kitty: bluerig prosi go wtedy o każdy klawisz wraz z jego
zwolnieniem i o zdarzenia fokusu. Na inne terminale nie ma to wpływu.
Domyślnie wyłączone.

*verbose*
: Pokazuje postęp długich operacji.

*keybar_visible*
: W sekcji
**[Layout]**:
pokazuje etykiety klawiszy funkcyjnych na dole ekranu.

# Ustawienia w pliku ini <a id="internal-file-editor-options"></a>

Okna dialogowe menu
**Opcje**
zmieniają większość ustawień, a
**Zapisz ustawienia**
zapisuje je do sekcji
**[bluerig]**
pliku
**~/.config/bluerig/ini**.
To są ustawienia edytora w tym pliku: 1 włącza ustawienie, 0 wyłącza, a
wartość domyślna jest na końcu.

|  |  |
|--|--|
| editor_tab_spacing | Szerokość tabulacji; domyślnie 8. editor_fake_half_tabs daje połówki tabulacji |
| editor_fill_tabs_with_spaces | Wstawia spacje zamiast znaku tabulacji; 0 |
| editor_return_does_auto_indent | Enter daje wcięcie takie jak najbliższy wiersz z tekstem powyżej; 1 |
| editor_backspace_through_tabs | Backspace usuwa całe puste miejsce do lewego marginesu, gdy między kursorem a marginesem nie ma tekstu; 0 |
| editor_fake_half_tabs | Tabulacja między lewym marginesem a tekstem przesuwa o 4 kolumny, a w pliku są spacje i tabulacje 8-kolumnowe; 1 |
| editor_option_save_mode | Jak zapisywany jest plik, zobacz [Modyfikuj tryb zapisu](#edit-save-mode): 0 - prosto do pliku, a awaria podczas zapisu zostawia go obciętym; 1 - przez plik tymczasowy; 2 - z kopią zapasową; 0 |
| editor_word_wrap_line_length | Długość wiersza przy zawijaniu; 72 |
| editor_backup_extension | Przyrostek nazwy kopii zapasowej; ~ |
| editor_line_state | Pokazuje kolumnę stanu wiersza: numery, zwinięcia, ukryte fragmenty; 0. Przełącza ją Alt-n |
| editor_visible_spaces | Pokazuje spacje na końcu wiersza jako kropki; 1 |
| editor_visible_tabs | Pokazuje tabulacje jako \<----\>; 1 |
| editor_show_control_chars | Pokazuje znaki sterujące jako ^M, ^L, w przeciwnym razie jako pustą komórkę; 1. Przełącza je pozycja menu Polecenie |
| editor_persistent_selections | Zachowuje zaznaczenie po ruchu kursora; 1 |
| editor_drop_selection_on_copy | Zdejmuje zaznaczenie po skopiowaniu do pliku schowka; 1 |
| editor_cursor_beyond_eol | Pozwala kursorowi wyjść za koniec wiersza; 0 |
| editor_cursor_after_inserted_block | Stawia kursor za wstawionym blokiem; 0 |
| editor_syntax_highlighting | Włącza podświetlanie składni; 1 |
| editor_edit_confirm_save | Pyta przed zapisem; 1 |
| editor_option_typewriter_wrap | Łamie wiersz na editor_word_wrap_line_length w trakcie pisania; 0 |
| editor_option_auto_para_formatting | Formatuje akapit pod kursorem w trakcie pisania; 0 |
| editor_option_save_position | Zachowuje pozycję w pliku przy wyjściu; 1 |
| editor_group_undo | Cofa naraz kilka kolejnych działań jednego rodzaju; 0 |
| editor_wordcompletion_collect_entire_file | Bierze słowa do uzupełniania z całego pliku, a nie tylko do kursora; 0 |
| editor_wordcompletion_collect_all_files | Bierze słowa do uzupełniania ze wszystkich otwartych plików; 1 |
| editor_stop_format_chars | Formatowanie akapitu nie rusza wiersza, który zaczyna się jednym z tych znaków, ani wierszy po nim; -+\*\\,.;:&\> |
| editor_state_full_filename | Pokazuje w wierszu stanu pełną ścieżkę, a nie nazwę pliku; 0 |
| editor_filesize_threshold | Rozmiar, powyżej którego plik otwiera się po pytaniu, z przyrostkiem K albo M; 64M |
| editor_show_right_margin | Oznacza kolumnę editor_word_wrap_line_length jako prawy margines; 0. Przełącza je pozycja menu Polecenie |
| editor_simple_statusbar | Pokazuje krótki wiersz stanu, bez przesunięcia i znaku pod kursorem; 0 |
| editor_check_new_line | Przy zapisie pyta o brakujący koniec wiersza na końcu pliku; 0. Przełącza je pole wyboru okna trybu zapisu |
| editor_sticky_windows | Zmienia rozmiar okien o stykających się ramkach jak jednego, zobacz [menu Okno](#window-menu); 0 |

# ŚRODOWISKO <a id="environment"></a>

Poniżej są zmienne, które czyta bluerig. Zmiennych takich jak **TERM**,
**SHELL**, **HOME** czy **PATH** tu nie ma: program czyta je, aby ustalić,
gdzie działa, a nie jako ustawienia.

**BLUERIG_DATADIR**
: Katalog, z którego brane są pliki danych, zamiast
{{pkgdatadir}}. Zobacz [PLIKI](#files).

**BLUERIG_PROFILE_ROOT**
: Katalog główny plików użytkownika, jako ścieżka bezwzględna, zamiast katalogu
domowego. Zobacz [PLIKI](#files).

**BLUERIG_SKIN**
: Skórka do użycia, według nazwy albo ścieżki. Zobacz [Skórki](#skins).

**BLUERIG_KEYMAP**
: Plik klawiszy do użycia. Zobacz [Zmiana przypisań klawiszy](#keys_redefine).

**BLUERIG_TMPDIR**, **TMPDIR**
: Katalog plików tymczasowych programu: z pierwszej ustawionej z tych dwóch
zmiennych, a bez nich /tmp.

**BLUERIG_NO_LUA**
: Wartość 1 uruchamia program bez środowiska Lua. Żaden skrypt Lua nie jest
wczytywany.

**KEYBOARD_KEY_TIMEOUT_US**
: Jak długo czekać na resztę sekwencji sterującej, w mikrosekundach.

**COLORTERM**
: Czytana przy wyborze kolorów. Zobacz [Kolory](#colors).

**BLUERIG_LOG_ENABLE**, **BLUERIG_LOG_FILE**
: Dziennik debugowania jest zapisywany, gdy BLUERIG_LOG_ENABLE ma wartość 1.
Bez **BLUERIG_LOG_FILE** używany jest plik podany przez *logfile* w sekcji
*[Logging]* pliku *ini*, a bez tego wpisu *~/.cache/bluerig/bluerig.log*.

**BLUERIG_SPELL_LOG**
: Plik, do którego sprawdzanie pisowni zapisuje swój dziennik debugowania.
Nie ma własnego przełącznika: dziennik jest zapisywany, gdy zmienna podaje
plik.

# PLIKI <a id="files"></a>

Pełne ścieżki poniżej mogą się różnić między instalacjami. Na pliki danych
wpływa też zmienna środowiska
**BLUERIG_DATADIR**:
jeśli jest ustawiona, jej wartość jest używana zamiast
{{pkgdatadir}} w ścieżkach poniżej. Pliki użytkownika są zgodne z katalogami
bazowymi XDG:
*~/.config*,
*~/.local/share*
i
*~/.cache*
oznaczają
**XDG_CONFIG_HOME**,
**XDG_DATA_HOME**
i
**XDG_CACHE_HOME**,
gdy są ustawione.

*{{pkgdatadir}}/help/bluerig.md*
: Plik pomocy programu, a obok niego pliki pomocy wtyczek edytora.

*{{sysconfdir}}/bluerig/bluerig.ini*, *{{pkgdatadir}}/bluerig.ini*
: Systemowe pliki ustawień, używane tylko wtedy, gdy użytkownik nie ma
własnego pliku
**~/.config/bluerig/ini**.
Jeśli istnieje {{sysconfdir}}/bluerig/bluerig.ini, {{pkgdatadir}}/bluerig.ini
nie jest używany.

*{{sysconfdir}}/bluerig/defaults.ini*, *{{pkgdatadir}}/defaults.ini*
: Ustawienia globalne, które dotyczą wszystkich użytkowników, niezależnie od
tego, czy mają ~/.config/bluerig/ini. Obecnie wczytywane są z niego tylko
[ustawienia terminala](#terminal-databases).

*{{sysconfdir}}/bluerig/keymap.ini*, *{{pkgdatadir}}/keymap.ini*
: Systemowe przypisania klawiszy. Zobacz
[Zmiana przypisań klawiszy](#keys_redefine).

*{{sysconfdir}}/bluerig/bluerig.menu*, *{{pkgdatadir}}/bluerig.menu*
: Systemowe [menu użytkownika](#edit-menu-file).

*{{sysconfdir}}/bluerig/spell.keymap, ctags.keymap*
: Klawisze wtyczek spell i ctags; plik o tej samej nazwie w
~/.config/bluerig je zastępuje.

*{{pkgdatadir}}/syntax/\**
: Systemowe pliki składni, używane tylko wtedy, gdy brakuje odpowiadającego
im własnego pliku użytkownika w
**~/.local/share/bluerig/syntax/**.

*{{pkgdatadir}}/skins/\**
: Skórki dostarczane z programem.

*{{pkgdatadir}}/charsets*
: Lista stron kodowych.

*{{pkgdatadir}}/lua/scripts/\**
: Skrypty Lua dostarczane z programem.

*{{pkgdatadir}}/examples/macros.d/\**
: Przykładowe skrypty makr.

*{{plugins_dir}}/*
: Wtyczki edytora wczytywane w czasie działania.

*~/.config/bluerig/ini*
: Własne ustawienia użytkownika. Jeśli ten plik istnieje, ustawienia są
wczytywane z niego zamiast z systemowego pliku ustawień.

*~/.config/bluerig/keymap.ini*
: Własne przypisania klawiszy użytkownika.

*~/.config/bluerig/menu*
: Własne [menu użytkownika](#edit-menu-file).

*~/.config/bluerig/plugins.ini*
: Wyłączone wtyczki i skrypty Lua.

*~/.config/bluerig/term/\<TERM>*
: Klawisze nauczone w [Określ klawisze](#learn-keys).

*./.bluerig.menu*
: Menu użytkownika bieżącego katalogu. Jeśli ten plik jest w bieżącym
katalogu, jest używany zamiast menu z katalogu domowego albo systemowego.

*~/.local/share/bluerig/history*
: Historie wierszy wprowadzania.

*~/.local/share/bluerig/filepos*
: Miejsca, w których zostawiono pliki, dla
*editor_option_save_position*.

*~/.local/share/bluerig/clipboard*
: Plik schowka edytora.

*~/.local/share/bluerig/macros*
: Nagrane [makra](#macros).

*~/.local/share/bluerig/macros.d/macro.N.sh*
: Skrypty powłoki makr.

*~/.local/share/bluerig/syntax/Syntax*
: Własny plik składni użytkownika.

*~/.local/share/bluerig/skins/*
: Własne skórki użytkownika.

*~/.local/share/bluerig/lua/scripts/*
: Własne skrypty Lua użytkownika.

*~/.cache/bluerig/block*
: Plik bloku, na którym pracują menu użytkownika i skrypty makr.

*~/.cache/bluerig/temp*
: Plik tymczasowy edytora.

Katalog główny plików użytkownika można zmienić zmienną środowiska
**BLUERIG_PROFILE_ROOT**.
Wartość BLUERIG_PROFILE_ROOT musi być ścieżką bezwzględną.
Jeśli BLUERIG_PROFILE_ROOT nie jest ustawiona albo jest pusta, używana jest
zmienna HOME. Jeśli HOME nie jest ustawiona albo jest pusta, katalogi są
brane z biblioteki GLib.

# LICENCJA <!-- help:skip -->

Ten program jest rozpowszechniany na warunkach Powszechnej Licencji
Publicznej GNU opublikowanej przez Free Software Foundation. Szczegóły
licencji i braku gwarancji są we wbudowanej pomocy.

# DOSTĘPNOŚĆ <a id="availability"></a>

Najnowszą wersję tego programu można znaleźć pod adresem
<https://github.com/blue-panels/bluerig/releases> .

# ZOBACZ TAKŻE <a id="see-also"></a>

cooledit(1), aspell(1), hunspell(1), ctags(1), etags(1), gpm(1),
terminfo(5), sh(1).

# AUTORZY <a id="authors"></a>

Paul Sheer (psheer@obsidian.co.za) jest pierwotnym autorem cooledit, którego
kontynuacją jest bluerig. Autorzy i
współtwórcy są wymienieni w pliku AUTHORS w dystrybucji źródłowej.

# BŁĘDY <a id="bugs"></a>

Błędy należy zgłaszać pod adresem
<https://github.com/blue-panels/bluerig/issues> .

Podaj szczegółowy opis błędu, wersję używanego programu
*(bluerig -V*
pokazuje tę informację) i system operacyjny, na którym program działa. Jeśli
program ulega awarii, będziemy wdzięczni za ślad stosu.
