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

**bluerig** to terminalowe IDE, które wyrosło z mcedit (GNU Midnight Commander) i cooledit Paula Sheera. **brg** to krótsza nazwa tego samego programu.

Edytor pracuje z kilkoma plikami naraz, każdym we własnym oknie, także z plikami binarnymi. Zaznacza bloki kolumnowe, filtruje wiersze, zwija tekst, nagrywa makra, przepuszcza tekst przez polecenia powłoki z menu użytkownika i uruchamia skrypty Lua. Wtyczki dodają projekty, budowanie i debugowanie, pokrycie kodu, serwery językowe (LSP), git i nawigację po kodzie oraz sprawdzanie pisowni.

Obsługę języków można rozszerzać: podświetlanie składni, budowanie, debugowanie przez adaptery DAP i serwery językowe ustawia się dla każdego języka osobno. Domyślna konfiguracja ma podświetlanie składni dla ponad 100 typów plików, budowanie dla C (Meson, CMake i Make), Rusta i Go oraz debugowanie dla C, Rusta, Pythona, Basha, Go i JavaScriptu.

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

**Tryby programu**

*-Q, --query*
: Otwiera plik w trybie zapytań do MongoDB albo PostgreSQL. Ustaw
*MC_QUERY_KIND=mongo* albo *MC_QUERY_KIND=postgres*. Jeśli ta zmienna nie
jest ustawiona, *MC_MONGO_URI* wybiera MongoDB, a plik z rozszerzeniem
*.sql* wybiera PostgreSQL, gdy *MC_MONGO_URI* nie jest ustawiona.
Potrzebna jest wtyczka Query.

*-D katalog, --debug=katalog*
: Otwiera projekt w trybie debugowania. Pokazuje drzewo projektu i panel
debugera. Potrzebne są wtyczki Project, Build i Debugger.

*-G katalog, --git=katalog*
: Otwiera okno Git dla tego katalogu. Zamknięcie okna kończy bluerig.
Po niej może stać jedna z poniższych opcji. Bez *--git* opcja dotyczy
bieżącego katalogu.

**Opcje Git**

*--git-log*
: Otwiera dziennik commitów.

*--git-show=commit*
: Pokazuje pliki i zmiany tego commita.

*--git-commit*
: Otwiera edytor opisu, aby zatwierdzić zmiany z indeksu.

*--git-amend*
: Zmienia ostatni commit, dodając do niego zmiany z indeksu.
Otwiera edytor opisu commita.

*--git-reword=commit*
: Otwiera edytor opisu tego commita. Zapisanie opisu
przepisuje historię Git.

**Opcje ogólne**

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

*-h, -?, --help*
: Pokazuje opcje wiersza poleceń i ich działanie.

*--help-all*
: Pokazuje opcje wiersza poleceń ze wszystkich grup.

*--help-terminal, --help-color*
: Pokazuje opcje terminala albo opcje kolorów.

**Opcje terminala**

*-x, --xterm*
: Włącza tryb xterm. Używa się go na terminalach, które potrafią to co xterm:
dwa tryby ekranu i sekwencje myszy.

*-X, --no-x11*
: Nie korzysta z systemu X Window przy odczycie stanu klawiszy modyfikujących.

*-g, --oldmouse*
: Używa starego sposobu śledzenia myszy przez terminal.

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

**Opcje kolorów**

*-b, --nocolor*
: Tryb czarno-biały.

*-c, --color*
: Tryb kolorowy na terminalach, które nie zgłaszają obsługi kolorów.

*-S skórka, --skin=skórka*
: Używa skórki o tej nazwie. Zobacz
[Skórki](#skins).

# Przegląd <a id="overview"></a>

Na górze ekranu bluerig jest
[pasek menu](#menu-bar),
pod nim okna edytora, a na dole etykiety klawiszy funkcyjnych. Pasek menu
może być ukryty: przywołuje go F9 albo kliknięcie w górny wiersz ekranu.

Wiersz stanu u góry okna pokazuje nazwę pliku, stan pliku i miejsce kursora.
[Menu Okno](#window-menu) przełącza między otwartymi oknami. Okna można
przesuwać i zmieniać ich rozmiar.

# Edytor bluerig <a id="internal-file-editor"></a>

<a id="keys"></a>
**Klawisze**

<!-- {{key:Action|F8}}: Action is the action name of misc/keymap.default.ini and src/keymap.c (Remove = f8); the help shows the keys bound to it now, the man page the default keys after the bar -->
[Jak zmienić przypisania](#keys_redefine)

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

Kliknięcie w tekst przenosi kursor, a przeciągnięcie zaznacza tekst. Myszą
otwiera się menu, naciska przyciski i uruchamia polecenia z dolnego wiersza
ekranu. Kółko myszy przewija tekst.

Domyślne tempo powtarzania przycisków myszy to 400 milisekund. Zmienia je
ustawienie
*mouse_repeat_rate*
pliku ini, a czas, w którym dwa kliknięcia są podwójnym kliknięciem,
ustawienie
*double_click_speed*.

Gdy bluerig działa z obsługą myszy, zwykłe zachowanie myszy terminala
(wycinanie i wklejanie tekstu) daje przytrzymanie klawisza Shift.

# Pasek menu <a id="menu-bar"></a>

Pasek menu pojawia się po naciśnięciu {{key:Menu|F9}} albo kliknięciu myszą w
górny wiersz ekranu. Jego menu to Plik, Edycja, Wyszukaj, Polecenie, Nawigacja
(gdy wtyczka albo skrypt coś do niego dodaje), Okno, Wtyczki i Opcje. Skrypt
Lua może dodać pozycje do każdego z nich albo własne menu.

## Menu Plik <a id="file-menu"></a>

|  |  |
|--|--|
| Otwórz plik... | Otwiera plik w nowym oknie |
| Nowy | Otwiera puste okno |
| Zamknij | Zamyka bieżące okno |
| Historia... | Otwiera listę ostatnio edytowanych plików |
| Zapisz | Zapisuje bieżący plik |
| Zapisz jako... | [Zapisuje plik pod nową nazwą](#save-file-as) |
| Wstaw plik... | Wstawia zawartość pliku w miejscu kursora |
| Skopiuj do pliku... | Zapisuje zaznaczony blok do pliku |
| Menu użytkownika... | [Otwiera listę poleceń przetwarzających tekst](#edit-menu-file) |
| O programie... | Pokazuje wersję programu |
| Zakończ | Zamyka edytor. Proponuje zapisanie niezapisanych zmian |

## Menu Edycja <a id="edit-menu"></a>

Cofnij, Ponów i Undo history; przełączanie między trybem wstawiania i
zastępowania; polecenia, które zaznaczają blok, blok kolumnowy albo cały plik,
i zdejmują zaznaczenie; kopiowanie, przenoszenie i usuwanie bloku; kopiowanie
i wycinanie do pliku schowka i wklejanie z niego; przejście na początek i na
koniec pliku.

## Menu Wyszukaj <a id="search-menu"></a>

Polecenia szukania, zastępowania i filtra opisuje
[Szukanie, zastępowanie i filtr wierszy](#search-and-replace).
To samo menu stawia albo zdejmuje zakładkę w bieżącym wierszu, przechodzi do
następnej albo poprzedniej zakładki i usuwa wszystkie zakładki.

## Menu Polecenie <a id="command-menu"></a>

|  |  |
|--|--|
| Przejdź do wiersza... | Przenosi kursor do wiersza o podanym numerze |
| Przełącz stan wiersza | Włącza i wyłącza kolumnę stanu wiersza |
| Przejdź do pasującego nawiasu | Przenosi kursor do pasującego nawiasu |
| Przełącz wyróżnianie składni | Włącza i wyłącza podświetlanie w edytorze |
| Przełącz prawy margines | Zaznacza kolumnę, w której zawijają się wiersze |
| Przełącz znaki sterujące | Pokazuje znaki sterujące jako ^M, ^L i tak dalej |
| Kodowanie... | [Wybiera kodowanie pliku](#codepages-translation) |
| Zwiń / rozwiń blok | [Zwija albo rozwija blok](#folding) |
| Rozwiń wszystko | Rozwija wszystkie zwinięcia i usuwa filtr wierszy |
| Odśwież ekran | Odświeża ekran |
| Rozpocznij/zatrzymaj nagrywanie makra | [Nagrywa makro i przypisuje je do klawisza](#macros) |
| Przeglądarka makr... | [Otwiera listę nagranych makr](#macro-explorer) |
| Nagraj/powtórz działania | Nagrywa działania i powtarza je podaną liczbę razy |

## Menu Nawigacja <a id="navigate-menu"></a>

Polecenia wtyczek: przejście do deklaracji, szukanie odwołań, symbole pliku i
inne. Zobacz
[Wtyczki](#plugins).

## Menu Okno <a id="window-menu"></a>

**Przenieś, Zmień rozmiar**
: Przesuwa okno albo zmienia jego rozmiar strzałkami; Enter kończy.

**Przełącz pełny ekran**
: Przełącza okno między pełnym ekranem a jego własnym rozmiarem.

**Toggle sticky windows**
: Gdy przeciągasz róg okna, ramki sąsiednich okien przesuwają się razem z nim,
więc okna zachowują wspólną krawędź. Tam, gdzie w rogu spotykają się cztery
okna, można naraz przesunąć krawędź poziomą i pionową. Krawędź, która na
początku leżała na brzegu ekranu, zostaje tam. Gdy przesuwana krawędź dojdzie
do innego okna, to okno też zaczyna zmieniać rozmiar. Każde okno ma
najmniejszy rozmiar. Esc anuluje zmianę. Okno przeciągane za tytuł przesuwa
się samo.
Ustawienie jest przechowywane jako
**editor_sticky_windows**
w pliku ini.

**Następne, Poprzednie**
: Przechodzi do następnego albo poprzedniego okna.

**Lista...**
: Otwarte okna, aby przejść do jednego z nich. Zobacz
[Otwarcie plików](#open-files).

## Menu Wtyczki <a id="plugins-menu"></a>

Wtyczki edytora, które mają własną pozycję w menu, na przykład sprawdzanie
pisowni. Wtyczki wyłączonej w
[Manage plugins](#manage-plugins)
nie ma w menu.

## Menu Opcje <a id="options-menu"></a>

|  |  |
|--|--|
| Ogólne... | [Ustawia wcięcia, zawijanie wierszy i inne opcje edytora](#editor-options) |
| Tryb zapisu... | [Wybiera sposób zapisywania plików](#edit-save-mode) |
| Wyróżnianie elementów składni... | Wybiera reguły podświetlania ręcznie albo według nazwy pliku i pierwszego wiersza |
| Wygląd... | [Ustawia skórkę i cienie](#appearance) |
| Określ klawisze... | [Przypisuje klawisze do sekwencji terminala](#learn-keys) |
| Przypisania klawiszy... | [Przypisuje klawisze do działań](#key-bindings) |
| Podgląd klawisza... | [Pokazuje sekwencję terminala i przypisane działanie](#key-sniffer) |
| Zarządzaj wtyczkami... | [Włącza i wyłącza wtyczki i ustawia je](#manage-plugins) |
| Informacje o wtyczkach... | [Pokazuje stan i funkcje wtyczek](#plugin-info) |
| Skrypty Lua... | [Zarządza skryptami Lua](#lua-scripts). Jest, gdy bluerig zbudowano z Lua |
| Plik składni | Otwiera w edytorze [plik składni użytkownika](#syntax-highlighting) |
| Plik menu | Otwiera w edytorze [plik menu użytkownika](#edit-menu-file) |
| Zapisz ustawienia | Zapisuje ustawienia do *~/.config/bluerig/ini* |

"Plik składni" otwiera *~/.local/share/bluerig/syntax/Syntax*. Jeśli plik
jeszcze nie istnieje, edytor tworzy go z krótkim komentarzem. Plik systemowy
jest czytany po pliku użytkownika.

Przy włączonym *auto_save_setup* edytor zapisuje ustawienia przy wyjściu.

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
| Połówki tabulacji | We wcięciu kursor przesuwa się o pół tabulacji. W pliku są spacje i znaki tabulacji |
| Backspace przez tabulacje | Backspace usuwa całe wcięcie do lewego marginesu, gdy między kursorem a marginesem nie ma tekstu |
| Wypełnianie tabulacji spacjami | Wstawia spacje do następnej pozycji tabulacji zamiast znaku tabulacji |
| Rozmiar tabulacji | Szerokość znaku tabulacji; domyślnie 8 |

**Keyboard**

|  |  |
|--|--|
| Esc timeout, ms | Ile milisekund edytor czeka po Esc na resztę sekwencji sterującej. Domyślnie 1000. Ustawienie *old_esc_mode_timeout* jest w mikrosekundach. Przy starcie pierwszeństwo ma zmienna **KEYBOARD_KEY_TIMEOUT_US** |

**Inne opcje**

|  |  |
|--|--|
| Automatyczne wcięcia | Enter zaczyna wiersz z wcięciem wiersza powyżej |
| Potwierdzenie przed zapisaniem | Pyta przed zapisaniem pliku |
| Zapisanie pozycji pliku | Przy otwarciu pliku przywraca pozycję kursora |
| Widoczne spacje kończące | Oznacza spacje na końcu wierszy |
| Widoczne tabulacje | Oznacza znaki tabulacji |
| Show control characters | Pokazuje znaki sterujące jako ^M zamiast je ukrywać |
| Wyróżnianie elementów składni | Koloruje tekst regułami składni jego typu |
| Kursor po wstawionym bloku | Zostawia kursor na końcu wstawionego bloku, a nie na jego początku |
| Trwałe zaznaczenie | Zachowuje zaznaczenie, gdy kursor się przesuwa |
| Kursor poza końcem wiersza | Pozwala kursorowi wyjść za koniec wiersza |
| Grupowe cofnięcie | Cofa naraz serię zmian jednego rodzaju |
| Punkt łamania wiersza | Kolumna, w której tryby zawijania łamią wiersz; domyślnie 72 |

### Modyfikuj tryb zapisu <a id="edit-save-mode"></a>

Jak zapisywany jest plik; te same trzy tryby przyjmuje ustawienie
*editor_option_save_mode*:

**Szybki zapis**
: Zapisuje dane od razu do oryginalnego pliku. Jeśli zapis się nie uda, plik
może zostać niekompletny.

**Bezpieczny zapis**
: Zapisuje dane do pliku tymczasowego i zastępuje nim oryginał dopiero wtedy,
gdy zapis jest kompletny.

**Wykonaj kopie zapasowe za pomocą tego rozszerzenia**
: Bezpieczny zapis, a oryginał zostaje pod swoją nazwą z dodanym
rozszerzeniem z wiersza wprowadzania, domyślnie "~". Kolejny zapis zastępuje
też kopię zapasową.

**Sprawdź nowe wiersze POSIX**
: Przed zapisem pyta o brakujący znak nowego wiersza na końcu pliku.

### Wygląd <a id="appearance"></a>

Tu wybiera się skórkę i ustawia cienie okien dialogowych i menu rozwijanych.

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
pliku ini. Kolejność, w jakiej szukane są pliki skórek, opisuje
[Skórki](#skins).

### Określ klawisze <a id="learn-keys"></a>

To okno przypisuje sekwencje sterujące terminala klawiszom funkcyjnym,
strzałkom i klawiszom nawigacji. Tytuł pokazuje nazwę terminala ze zmiennej
**TERM**.

Wybierz kombinację modyfikatorów (Ctrl, Alt, Shift) polami wyboru, a potem
naciśnij przycisk klawisza. Naciśnij ten klawisz na klawiaturze i poczekaj, aż
zniknie komunikat przechwytywania. Nauczona sekwencja pojawia się obok
przycisku. Jeśli ma ją już inny klawisz, edytor zgłasza konflikt.

**Del**
: Usuwa zapisaną sekwencję wybranego klawisza.

**Zapisz**
: Zapisuje przypisania klawiszy do sekwencji do
*~/.config/bluerig/term/\<TERM>*.

**Edit term file**
: Otwiera ten plik w edytorze.

### Key bindings <a id="key-bindings"></a>

Tabela pokazuje działania i przypisane do nich klawisze. Przyciski u góry
wybierają grupę, Editor albo Widgets. Grupa Editor ma części Main i Ctrl-X,
grupa Widgets ma Dialog, Menu, Input, Listbox, Radio i Help. Back wraca do
listy grup. Działania ze zmienionymi przypisaniami są oznaczone \*.

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
: Otwiera plik sekwencji sterujących zapisanych w oknie
[Określ klawisze](#learn-keys).

Jeśli klawisz jest już przypisany do innego działania, edytor prosi o
potwierdzenie nowego przypisania.

### Key sniffer <a id="key-sniffer"></a>

Naciśnij
**Capture key**,
a potem klawisz, który chcesz sprawdzić. Okno pokazuje jego dane:

```
Shortcut   the symbolic name, e.g. Ctrl-F5
Action     the action of the key in the editor keymap
Raw        the escape sequence and its bytes
Keycode    the internal numeric code
```

Jeśli edytor nie rozpoznaje sekwencji, ustaw ją w oknie
[Określ klawisze](#learn-keys). Jeśli klawisz jest rozpoznany, sprawdź
przypisane do niego działanie w oknie [Key bindings](#key-bindings).

### Manage plugins <a id="manage-plugins"></a>

Tabela pokazuje rodzaj, nazwę i opis każdej wtyczki edytora. Jeśli bluerig
zbudowano z Lua, w tabeli jest też środowisko Lua rodzaju
*runtime*.
Pole wyboru w wierszu wtyczki decyduje, czy zostanie wczytana przy następnym
starcie. Wybór jest przechowywany w *~/.config/bluerig/plugins.ini*.
Wyłączone wtyczki zostają na liście.

**Enter, F4**
: Otwiera ustawienia wybranej wtyczki. Jeśli wtyczka nie ma ustawień, edytor
o tym mówi. Dla środowiska Lua otwiera okno
[Lua scripts](#lua-scripts).

### Plugin info <a id="plugin-info"></a>

Lista wczytanych wtyczek pokazuje ich nazwy, stan, funkcje i opis. Ustawienia
i przełączniki wtyczek są w oknie
[Manage plugins](#manage-plugins).

### Lua scripts <a id="lua-scripts"></a>

Tabela pokazuje stan, położenie (globalny albo użytkownika), identyfikator,
przestrzeń roboczą, funkcje i opis każdego znalezionego skryptu Lua.
Pole wyboru w wierszu skryptu decyduje, czy zostanie wczytany przy następnym
starcie bluerig.

**Settings**
: Uruchamia skrypt z ustawieniami pakietu, jeśli taki jest.

Gdzie leżą pakiety i jak są zbudowane, opisuje
[Skrypty Lua](#lua-scripting).

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

Filtr ukrywa wiersze bez trafienia i zachowuje numery wierszy. Kolumna stanu
wiersza oznacza ukryte fragmenty. Filtr jest stosowany w chwili włączenia.
Nowe i zmienione wiersze zostają widoczne, dopóki filtr nie zostanie
zastosowany ponownie.

|  |  |
|--|--|
| {{key:FilterToggle|Alt-s}} | Zdejmuje filtr albo stosuje ostatnie szukanie jako filtr. Jeśli jeszcze nic nie szukano, otwiera okno szukania |
| {{key:FilterWord|Alt-S}} | Filtruje według zaznaczonego tekstu albo słowa pod kursorem |
| Polecenie \> Rozwiń wszystko | Zdejmuje filtr i rozwija wszystkie zwinięcia |

# Zwijanie <a id="folding"></a>

Zwinięcie pokazuje tylko pierwszy wiersz bloku. Kolumna stanu wiersza
oznacza ten wiersz.
{{key:FoldToggle|Alt-F}}
(pozycja Zwiń / rozwiń blok menu Polecenie) zwija zaznaczone wiersze, a gdy
nic nie jest zaznaczone, wiersze od nawiasu w bieżącym wierszu do pasującego
do niego nawiasu. Na pierwszym wierszu zwinięcia ten klawisz je rozwija.
**Rozwiń wszystko**
rozwija wszystkie zwinięcia pliku i usuwa filtr wierszy.

# Makra <a id="macros"></a>

{{key:MacroStartStopRecord|Ctrl-r}}
zaczyna nagrywanie makra. Aby zakończyć nagrywanie, naciśnij
{{key:MacroStartStopRecord|Ctrl-r}}
ponownie, a potem klawisz, który będzie uruchamiał makro.

Makra są przechowywane w sekcji
**[editor]**
pliku
**~/.local/share/bluerig/macros**.
Aby usunąć makro, usuń jego wiersz z pliku albo użyj okna
[Macro Explorer](#macro-explorer).

Pozycja
**Nagraj/powtórz działania**
menu Polecenie nagrywa następne działania, dopóki nie zostanie wybrane
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

Lista nagranych makr pokazuje ich klawisze i działania.

**Run**
: Uruchamia wybrane makro.

**Delete**
: Usuwa wybrane makro po pytaniu.

**Modyfikacja pliku**
: Otwiera plik makr. Jego format opisuje [Makra](#macros).

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
**Plik menu**
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

Podaj nazwę pliku i format końców wierszy. Można zachować format pliku albo
wybrać Unix (LF), Windows i DOS (CR LF) albo Macintosh (CR).
Wybrany format obowiązuje dla tego zapisu.

## Otwarcie plików <a id="open-files"></a>

Lista ma jeden wiersz dla każdego otwartego pliku. Enter przełącza edytor na
wybrany plik, Esc zamyka listę. Pozycja
**Lista...**
menu Okno otwiera tę samą listę.

## Wybór ekranu <a id="screen-selector"></a>

Poza edytorem skrypt Lua może otwierać własne ekrany, okna pełnoekranowe,
które zostają otwarte, gdy wracasz do edytora. Między ekranami przełącza się
na trzy sposoby:

|  |  |
|--|--|
| Alt-} | Następny ekran |
| Alt-{ | Poprzedni ekran |
| Alt-\` | Lista otwartych ekranów |

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

Gdy pasuje kilka, edytor wydaje dźwięk. Ustawienie
*show_all_if_ambiguous*
pliku ini, domyślnie wyłączone, pokazuje ich listę obok kursora:
**Enter**
wybiera jedną, wpisywane litery zawężają listę, Esc, F10, Left i Right ją
chowają.

## Wybór strony kodowej <a id="codepages-translation"></a>

Edytor wczytuje listę kodowań z **{{pkgdatadir}}/charsets**.
Wybierz kodowanie pliku; edytor przekodowuje z niego tekst dla terminala.
Pozycja **\<No translation>** wyłącza przekodowanie.
Listę otwiera {{key:SelectCodepage|Alt-e}} albo pozycja Kodowanie...
menu Polecenie.

## Historia <a id="history-query"></a>

**Alt-h** otwiera historię bieżącego wiersza wprowadzania. Najnowsze wpisy są
pierwsze. Enter wstawia wybrany wpis do wiersza, Esc zamyka historię bez
zmiany wiersza, a **F8, Del** usuwają wybrany wpis. Historia jest
przechowywana w
*~/.local/share/bluerig/history*;
ustawienie
*num_history_items_recorded*
określa, ile wpisów zachowuje każda historia.

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

Gdy do pliku pasują i reguły użytkownika, i zainstalowane, obowiązują reguły
użytkownika. Pliki wymienione w wierszach
**include**
są szukane najpierw w
*~/.local/share/bluerig/syntax/*,
a potem wśród zainstalowanych. Plik użytkownika jest czytany ponownie za
każdym razem, gdy w edytorze otwiera się nowy plik.

Pełna kopia zainstalowanego pliku może przesłonić reguły typów plików dodanych
w nowej wersji. Zostaw w pliku użytkownika tylko zmienione wpisy.

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
włącza sprawdzanie pisowni wewnątrz kontekstu.

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
reguł. Składnie można zagnieżdżać na kilka poziomów w głąb. Na przykład
reguły HTML mogą użyć składni JavaScriptu dla zawartości znacznika SCRIPT:

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
| \\t, \\s | Tabulacja i spacja. Wpisane wprost rozdzielają słowa wiersza, więc wzorzec używa tych form |
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

Zamiast koloru tekstu można podać token zakresu: słowo pisane wielkimi
literami, które nazywa rodzaj tekstu. Jego kolor ustawia skórka:

```
keyword  whole  if   KEYWORD_CONTROL
context  " " STRING_QUOTED_DOUBLE
    keyword  \\"   CONSTANT_CHARACTER_ESCAPE
keyword  linestart TODO  KEYWORD_OTHER_TODO black
```

Skórka ustawia kolory interfejsu i tokenów podświetlania dla wszystkich
języków. Zobacz
[Skórki](#skins).
Nazwę zakresu pisze się małymi literami, z kropkami zamiast podkreśleń. Na
przykład KEYWORD_CONTROL_IMPORT to keyword.control.import, rodzaj
keyword.control, który jest rodzajem keyword. To nazwy zakresów Sublime Text
i TextMate. Tło i atrybuty ze skórki mają pierwszeństwo przed wartościami
podanymi po tokenie w regule. Jeśli skórka ich nie ustawia, używane są
wartości reguły, na przykład black powyżej. Pozostałe wartości pochodzą ze
standardowego schematu mcedit. Token działa też w define oraz w wierszach
number, string i symbols reguł line-local. Zainstalowane pliki składni
używają tokenów. Lista tokenów i ich kolorów mcedit jest w
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
**COLORTERM**:
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
skórki. Klucz ustawia kolor zakresu. Aby ustawić kolor zakresu wewnątrz
kontekstu, podaj najpierw zakres kontekstu, a potem zakres tekstu, ze spacją
między nimi. W każdym przypadku wybierana jest najdłuższa pasująca nazwa
zakresu. Para z kontekstem jest sprawdzana jako pierwsza.

```
[syntax]
    comment = gray
    keyword = yellow
    keyword.control = brightmagenta
    string = green
    string constant.character.escape = white;red
    invalid = white;red;bold
```

Zakresy bez koloru w skórce używają standardowych kolorów mcedit. Kolory
podane wprost w regułach podświetlania pozostają takie same przy każdej
skórce. Zobacz
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
ustawia sekwencje sterujące uzupełniania. Zwykle robi to Alt-Tab. Można
podać sekwencję innego klawisza.

# Skrypty Lua <a id="lua-scripting"></a>

Gdy bluerig zbudowano z Lua, skrypty mogą dodawać polecenia do menu,
przypisywać je do klawiszy, obsługiwać zdarzenia edytora i otwierać okna
dialogowe i okna pełnoekranowe. W Lua są napisane na przykład sortowanie wierszy, formatowanie akapitu, wstawianie wyniku polecenia,
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
pliku ini wyłącza Lua także przy kolejnych startach.

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
protokołem klawiatury kitty. bluerig prosi terminal o zdarzenia naciśnięcia i
zwolnienia klawiszy oraz o zdarzenia fokusu. Domyślnie wyłączone.

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
: Ścieżka dziennika debugowania sprawdzania pisowni. Jeśli zmienna jest
ustawiona, sprawdzanie pisowni zapisuje dziennik do tego pliku.

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
: Plik pomocy programu (tłumaczenia to bluerig.ru.md i tak dalej); obok
niego są pliki pomocy wtyczek edytora.

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
