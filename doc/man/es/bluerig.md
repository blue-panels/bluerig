---
date: noviembre de 2026
---

<!-- help:topics "Índice de Contenidos:" -->
# NOMBRE <!-- help:skip -->

bluerig - un IDE para el terminal.

# SINOPSIS <!-- help:skip -->

**bluerig**
[opciones] [+línea] [arch1[:línea]] [arch2[:línea]] ...

# DESCRIPCIÓN <a id="description"></a>

**bluerig**
es un IDE para el terminal. Continúa
**cooledit**,
el editor de Paul Sheer, cuya versión para terminal se desarrolló durante
muchos años como el editor interno de GNU Midnight Commander; bluerig lo
continúa como programa independiente.
**brg**
es un nombre más corto del mismo programa.

Edita varios archivos a la vez, cada uno en su propia ventana, y puede editar
archivos binarios. Tiene copia, movimiento, borrado, corte y pegado de
bloques; deshacer y rehacer tecla a tecla; menús desplegables; inserción de
archivos; macros; búsqueda y reemplazo con expresiones regulares; un filtro de
líneas; plegado; marcas; selección de texto con Shift y las flechas (si el
terminal lo permite); bloques de columnas; modos de inserción y
sobrescritura; sangría automática; ajuste de líneas y formato de párrafos; un
tamaño de tabulación ajustable; resaltado de sintaxis para muchos tipos de
archivo; un menú de usuario que pasa el texto por órdenes del shell;
complementos del editor (navegación por etiquetas, revisión ortográfica) y
guiones Lua.

Cada archivo indicado en la línea de órdenes se abre en su propia ventana.
Sin archivos, el editor abre una ventana vacía. Un archivo mayor que
*editor_filesize_threshold*
(64 MB por omisión) se abre tras una pregunta, no se rechaza.

# OPCIONES <a id="options"></a>

*+línea*
: Ir a la línea con ese número (sin espacio entre el signo
*+*
y el número). Se admiten varios números, pero solo se usa el último, y solo
para el primer archivo.

*arch:línea[:]*
: Abrir el archivo en esa línea, en la forma en que los compiladores y grep
indican un lugar de un archivo. Si existe un archivo con el nombre completo,
dos puntos y número incluidos, se abre ese archivo.

*-V, --version*
: Mostrar la versión del programa.

*-f, --datadir*
: Mostrar la ruta de búsqueda compilada de los archivos de datos de bluerig.

*-F, --datadir-info*
: Mostrar información ampliada sobre los directorios que usa bluerig: los
archivos de datos, los archivos del usuario y los complementos.

*--configure-options*
: Mostrar las opciones con las que se configuró el programa.

*--no-lua*
: Arrancar sin el entorno Lua: no se carga ningún guion Lua.

*-D, --debug=directorio*
: Abrir el proyecto de ese directorio para depurarlo: el último archivo del
proyecto, el árbol del proyecto y el panel del depurador a la derecha. Las
ventanas de archivos toman las teclas del depurador: F6 pone un punto de
parada, F5 compila el programa y lo ejecuta, y crea su configuración de
depuración la primera vez, F7 y F8 avanzan paso a paso, F3 indexa los
símbolos del proyecto; F2, F9 y F10 siguen siendo Guardar, el menú y Salir, y
Alt-F5 va al panel y vuelve. Necesita los complementos project, build y
debugger, y ctags para F3. Los menús Drawing, Format y Plugins no están en la
barra de menús; sus teclas siguen, y Opciones conserva Manage plugins.

*-Q, --query*
: Ejecutar las consultas del archivo en una base de datos, MongoDB con
mongosh o PostgreSQL con libpq; la respuesta llega en las pestañas Output,
Table e Historia de abajo. *MC_QUERY_KIND* es *mongo* o *postgres*; sin ella,
*MC_MONGO_URI* indica MongoDB y un archivo *.sql* PostgreSQL. Para MongoDB
*MC_MONGO_URI* y *MC_MONGO_DB* dicen a qué conectarse, *MC_MONGOSH* el
programa; F5 ejecuta la selección, si no, la parte entre las líneas *// ---*
que rodean el cursor, si no, el archivo entero. Para PostgreSQL libpq toma la
conexión de *PGHOST*, *PGDATABASE*, *PGSERVICE* y similares, la contraseña de
*~/.pgpass*; F5 ejecuta la selección, si no, el archivo entero. Ctrl-F5
ejecuta el archivo entero. Sin un archivo indicado, el archivo es
*~/.local/share/bluerig/queries/*, con el nombre de la base de datos y *.js*,
o *postgres/*, el nombre de la conexión, y el de la base de datos y *.sql*.
Las ventanas que tenía el proyecto la última vez no se muestran, y los
complementos de proyecto y de código (project, build, debugger, ctags, etags,
git, terminal, spell) no se inician; los guiones siguen, y el servidor de
lenguaje de las consultas cuando lsp.json tiene uno. Los menús Drawing,
Format y Plugins no están en la barra de menús, como con -D. Necesita el
complemento query.

*-G, --git=directorio*
: Abrir la ventana Git del complemento git sobre el árbol de trabajo de ese
directorio; al cerrarla termina el editor. Las ventanas que tenía el proyecto
la última vez no se muestran, y los complementos de proyecto (project, build,
debugger, ctags, etags) no se inician. Para un panel de git en un gestor de
archivos, con una de estas detrás, o sola para el directorio actual:

*--git-log*
: La ventana Git sobre el registro.

*--git-show=commit*
: La ventana Git sobre ese commit: sus archivos y sus diferencias.

*--git-commit*, *--git-amend*
: La ventana Git con el mensaje de un commit de lo preparado, o del último
commit corregido con ello; el editor termina cuando el mensaje se guarda y se
hace el commit, o cuando se cierra su ventana.

*--git-reword=commit*
: Otro mensaje para ese commit, de la misma forma, con los archivos del
commit debajo.

*-h, -?, --help*
: Mostrar las opciones y lo que hacen.

*--help-terminal, --help-color*
: Mostrar las opciones del terminal o las opciones de color de abajo.

Las opciones del terminal:

*-x, --xterm*
: Forzar el modo xterm. Se usa en terminales compatibles con xterm (dos modos
de pantalla, y capaces de enviar secuencias de escape del ratón).

*-X, --no-x11*
: No usar el sistema X Window para conocer el estado de las teclas
modificadoras.

*-g, --oldmouse*
: Usar el antiguo seguimiento del ratón con resaltado del terminal.

*-d, --nomouse*
: Desactivar el ratón.

*-t, --termcap*
: Usar la base de datos termcap en lugar de terminfo. La opción solo existe
cuando bluerig se compiló con la biblioteca S-Lang.

*-s, --slow*
: Trabajar en un terminal lento: la pantalla se dibuja con menos
actualizaciones.

*-a, --stickchars*
: Dibujar los marcos con caracteres simples en lugar de los caracteres de
líneas del terminal.

*-K arch, --keymap=arch*
: Cargar las asociaciones de teclas de ese archivo. Véase
[Redefinir las teclas](#keys_redefine).

*--nokeymap*
: No cargar ningún archivo de teclas: usar las asociaciones internas del
programa.

Las opciones de color:

*-b, --nocolor*
: Forzar la pantalla en blanco y negro.

*-c, --color*
: Forzar el modo de color en terminales que no parecen admitir colores.

*-S skin, --skin=skin*
: Usar la skin con ese nombre. Véase
[Skins](#skins).

# Vista general <a id="overview"></a>

La pantalla de bluerig tiene la
[barra de menús](#menu-bar)
arriba, las ventanas del editor debajo y las etiquetas de las teclas de
función abajo. La barra de menús puede estar oculta: F9 o un clic en la línea
superior de la pantalla la muestra.

Cada ventana muestra un archivo: su nombre, el estado del archivo y la
posición del cursor están en la línea de estado de su parte superior. Puede
haber varias ventanas abiertas a la vez; el
[menú Ventana](#window-menu)
pasa de una a otra, y cada una se puede mover y cambiar de tamaño.

# El editor bluerig <a id="internal-file-editor"></a>

<a id="keys"></a>
**Teclas**

<!-- {{key:Action|F8}}: Action is the action name of misc/keymap.default.ini and src/keymap.c (Remove = f8); the help shows the keys bound to it now, the man page the default keys after the bar -->
Estas son las teclas por omisión. Cualquiera de ellas se puede cambiar, véase
[Redefinir las teclas](#keys_redefine).

<a id="moving-around"></a>
**Movimiento del cursor**

|  |  |
|--|--|
| {{key:Left|Left}} | Un carácter a la izquierda |
| {{key:Right|Right}} | Un carácter a la derecha |
| {{key:Up|Up}} | Una línea arriba |
| {{key:Down|Down}} | Una línea abajo |
| {{key:WordLeft|Ctrl-Left, Ctrl-z}} | Una palabra a la izquierda |
| {{key:WordRight|Ctrl-Right, Ctrl-x}} | Una palabra a la derecha |
| {{key:ScrollUp|Ctrl-Up}} | Desplazar la pantalla hacia arriba |
| {{key:ScrollDown|Ctrl-Down}} | Desplazar la pantalla hacia abajo |
| {{key:PageUp|PgUp}} | Una página arriba |
| {{key:PageDown|PgDn}} | Una página abajo |
| {{key:Home|Home}} | Principio de la línea |
| {{key:End|End}} | Fin de la línea |
| {{key:Top|Ctrl-Home, Alt-\<}} | Principio del archivo |
| {{key:Bottom|Ctrl-End, Alt-\>}} | Fin del archivo |
| {{key:TopOnScreen|Ctrl-PgUp}} | Parte superior de la pantalla |
| {{key:BottomOnScreen|Ctrl-PgDn}} | Parte inferior de la pantalla |

**Borrado**

|  |  |
|--|--|
| {{key:Delete|Delete, Ctrl-d}} | Borrar el carácter |
| {{key:BackSpace|Backspace, Ctrl-h}} | Borrar el carácter de la izquierda |
| {{key:DeleteLine|Ctrl-y}} | Borrar la línea |
| {{key:DeleteToEnd|Ctrl-k}} | Borrar hasta el fin de la línea |
| {{key:DeleteToWordBegin|Alt-Backspace}} | Borrar hasta el principio de la palabra |
| {{key:DeleteToWordEnd|Alt-d}} | Borrar hasta el fin de la palabra |

<a id="blocks"></a>
**Bloques**

|  |  |
|--|--|
| Shift+teclas del cursor | Marcar un bloque |
| Alt+teclas del cursor | Marcar un bloque de columnas |
| {{key:Mark|F3}} | Empezar o terminar de marcar un bloque |
| {{key:MarkColumn|F13}} | Empezar un bloque de columnas |
| {{key:MarkAll|Ctrl-a}} | Marcar todo el texto |
| {{key:Copy|F5}} | Copiar el bloque al cursor |
| {{key:Move|F6}} | Mover el bloque al cursor |
| {{key:Remove|F8}} | Borrar el bloque |
| {{key:Store|Ctrl-Ins}} | Copiar el bloque al archivo del portapapeles |
| {{key:Cut|Shift-Del}} | Mover el bloque al archivo del portapapeles |
| {{key:Paste|Shift-Ins}} | Pegar el bloque del archivo del portapapeles |
| {{key:BlockSave|Ctrl-f}} | Escribir el bloque en un archivo |

<a id="search-keys"></a>
**Búsqueda y reemplazo**

|  |  |
|--|--|
| {{key:Search|F7}} | [Buscar](#search-and-replace) |
| {{key:SearchContinue|F17}} | Buscar de nuevo |
| {{key:Replace|F4}} | Reemplazar |
| {{key:ReplaceContinue|F14}} | Reemplazar de nuevo |
| {{key:FilterToggle|Alt-s}} | Quitar el filtro de líneas, o aplicar la última búsqueda como filtro |
| {{key:FilterWord|Alt-S}} | Filtrar por el texto marcado o la palabra bajo el cursor |

**Saltos y marcas**

|  |  |
|--|--|
| {{key:Goto|Alt-l}} | Ir a una línea por su número |
| {{key:MatchBracket|Alt-b}} | Al paréntesis correspondiente |
| {{key:Bookmark|Alt-k}} | Poner o quitar una marca |
| {{key:BookmarkNext|Alt-j}}, {{key:BookmarkPrev|Alt-i}} | A la marca siguiente o a la anterior |
| {{key:BookmarkFlush|Alt-o}} | Quitar todas las marcas |
| {{key:Find|Alt-Enter}} | Buscar la definición del nombre bajo el cursor |
| {{key:FilePrev|Alt-Minus}}, {{key:FileNext|Alt-Plus}} | Atrás y adelante por los lugares visitados al buscar definiciones |

<a id="files-and-windows"></a>
**Archivos**

|  |  |
|--|--|
| {{key:Save|F2}} | Guardar el archivo |
| {{key:SaveAs|F12, Ctrl-F2}} | [Guardar el archivo con otro nombre](#save-file-as) |
| {{key:EditNew|Ctrl-n}} | Nueva ventana vacía |
| {{key:InsertFile|F15}} | Insertar un archivo en el cursor |
| {{key:History|Alt-E}} | Historial de los archivos editados |
| {{key:SelectCodepage|Alt-e}} | [Codificación del texto](#codepages-translation) |
| {{key:Quit|F10, Esc}} | Cerrar la ventana; al cerrar la última se sale del editor |

<a id="editing"></a>
<a id="macros-and-plugins"></a>
**Otras órdenes**

|  |  |
|--|--|
| {{key:InsertOverwrite|Insert}} | Modo de inserción o de sobrescritura |
| {{key:Undo|Ctrl-u}} | Deshacer |
| {{key:Redo|Alt-r}} | Rehacer |
| {{key:UndoHistory|Ctrl-t, Alt-U}} | Historial de deshacer |
| {{key:Complete|Alt-Tab}} | [Completar la palabra](#completion) con las palabras de los archivos abiertos |
| {{key:FoldToggle|Alt-F}} | Plegar o desplegar el bloque entre dos paréntesis correspondientes |
| {{key:Sort|Alt-t}} | Ordenar las líneas marcadas |
| {{key:ExternalCommand|Alt-u}} | Insertar la salida de una orden del shell |
| {{key:ParagraphFormat|Alt-p}} | Formatear el párrafo |
| {{key:InsertLiteral|Ctrl-q}} | Insertar la tecla siguiente, o un código U+XXXX, tal cual |
| Ctrl-p | Revisar la ortografía de la palabra bajo el cursor |
| {{key:MacroStartStopRecord|Ctrl-r}} | Empezar o terminar de grabar una [macro](#macros) |
| {{key:MacroExplorer|Alt-M}} | [Macro Explorer](#macro-explorer) |
| {{key:ShowNumbers|Alt-n}} | Mostrar u ocultar los números de línea |
| {{key:ShowTabTws|Alt-\_}} | Mostrar u ocultar las tabulaciones y los espacios finales |
| {{key:SyntaxOnOff|Ctrl-s}} | Activar o desactivar el resaltado de sintaxis |
| {{key:Help|F1}} | Ayuda |
| {{key:Menu|F9}} | Barra de menús |
| {{key:UserMenu|F11}} | [Menú de usuario](#edit-menu-file) |
| {{key:Shell|Ctrl-o}} | Mostrar u ocultar el terminal |
| Ctrl-Alt-p | Mostrar u ocultar Preview |
| {{key:Refresh|Ctrl-l}} | Redibujar la pantalla |

|  |  |
|--|--|
| Véase también: | [Menús](#menu-bar) |
|  | [Búsqueda, reemplazo y filtro de líneas](#search-and-replace) |
|  | [Plegado](#folding) |
|  | [Macros](#macros) |
|  | [Menú de usuario](#edit-menu-file) |
|  | [Complementos](#plugins) |
|  | [Resaltado de sintaxis](#syntax-highlighting) |
|  | [Opciones del editor](#editor-options) |
|  | [Ajustes del archivo ini](#internal-file-editor-options) |

## Redefinir las teclas <a id="keys_redefine"></a>

|  |  |
|--|--|
| Opciones \> [Key bindings](#key-bindings) | Dar a una acción teclas propias; se guardan en **~/.config/bluerig/keymap.ini** |
| Opciones \> [Redefinir teclas](#learn-keys) | Dar a las teclas sus secuencias de escape |
| Opciones \> [Key sniffer](#key-sniffer) | Mostrar qué envía el terminal para una tecla y a qué acción está asociada |

Al arrancar, bluerig toma el mapa de teclas interno y carga sobre él
**{{pkgdatadir}}/keymap.ini**,
y después
**{{sysconfdir}}/bluerig/keymap.ini**.
Una acción nombrada en un archivo recibe las teclas del archivo, y sus teclas
internas dejan de funcionar.

El paquete instala los mapas de teclas
**keymap.default.ini**,
**keymap.emacs.ini**
y
**keymap.vim.ini**
en
**{{sysconfdir}}/bluerig**
y hace de
**keymap.ini**
un enlace a
**keymap.default.ini**.

La opción
**--nokeymap**
desactiva la carga de los archivos, y solo funciona el mapa interno.

El archivo de teclas del usuario es el primero que se encuentre de esta lista:

```
1) command line option -K <keymap>, --keymap=<keymap>
2) environment variable BLUERIG_KEYMAP
3) the keymap setting in the [bluerig] section of the ini file
4) file ~/.config/bluerig/keymap.ini
```

Los tres primeros admiten un nombre o una ruta absoluta. Un nombre que no
termina en
**.keymap**
recibe esa extensión, y el archivo se busca en estos directorios; gana el
primero que coincide:

```
1) ~/.config/bluerig/
2) {{pkgdatadir}}/
```

La opción
**-K**,
la variable
**BLUERIG_KEYMAP**
y el ajuste
**keymap**
añaden la extensión
**.keymap**
a un nombre que no la tiene, así que los mapas del paquete, que terminan en
**.ini**,
no se pueden elegir de esta forma. Para usar uno de ellos, haga de
**~/.config/bluerig/keymap.ini**
un enlace a él: este archivo se carga cuando no se da ninguno de los tres.

```
ln -s {{sysconfdir}}/bluerig/keymap.vim.ini ~/.config/bluerig/keymap.ini
```

Las secciones de un archivo de teclas:

|  |  |
|--|--|
| [editor] | Editor |
| [editor:xmap] | Teclas tras el prefijo, acción ExtendedKeyMap |
| [dialog] | Diálogos |
| [menu] | Menús |
| [input] | Líneas de entrada |
| [listbox] | Listas |
| [radio] | Botones de opción |
| [help] | Ventana de ayuda |
| [mcterm] | Terminal |

Una línea de una sección nombra una acción y sus teclas, separadas por punto y
coma:

```
[editor]
Save = f2
SaveAs = f12; ctrl-f2
```

## Teclas de las líneas de entrada <a id="input-line-keys"></a>

Las líneas de entrada de los diálogos aceptan estas teclas:

|  |  |
|--|--|
| Ctrl-a, Home | Principio de la línea |
| Ctrl-e, End | Fin de la línea |
| Ctrl-b, Left | Un carácter a la izquierda |
| Ctrl-f, Right | Un carácter a la derecha |
| Alt-f, Ctrl-Right | Una palabra adelante |
| Alt-b, Ctrl-Left | Una palabra atrás |
| Ctrl-h, Backspace | Borrar el carácter antes del cursor |
| Ctrl-d, Delete | Borrar el carácter bajo el cursor |
| Ctrl-@ | Poner la marca |
| Ctrl-w | Cortar al búfer el texto de la marca al cursor |
| Alt-w | Copiar al búfer el texto de la marca al cursor |
| Ctrl-y | Pegar el búfer |
| Ctrl-k | Borrar hasta el fin de la línea |
| Ctrl-Insert | Copiar la selección, o sin ella la línea entera, al archivo del portapapeles |
| Shift-Delete | Cortar la selección al archivo del portapapeles |
| Shift-Insert | Pegar el archivo del portapapeles como una línea: los saltos de línea pasan a espacios |
| Alt-p, Alt-n | Entrada anterior o siguiente del historial de la línea |
| Alt-h | [Historial](#history-query) de la línea como lista |
| Alt-Backspace | Borrar la palabra antes del cursor |
| Alt-Tab | [Completar](#completion) un nombre de archivo, una variable o un nombre de usuario |

## Teclas generales de movimiento <a id="general-movement-keys"></a>

La ventana de ayuda y las listas comparten el mismo código de movimiento, así
que aceptan las mismas teclas:

|  |  |
|--|--|
| Up, Ctrl-p | Una línea atrás |
| Down, Ctrl-n | Una línea adelante |
| Page Up, Alt-v | Una página arriba |
| Page Down, Ctrl-v | Una página abajo |
| Home, A1 | Al principio |
| End, C1 | Al final |

La ventana de ayuda acepta además estas teclas:

|  |  |
|--|--|
| b, Backspace | Una página arriba |
| Space, f | Una página abajo |
| u, d | Media página arriba o abajo |
| g, G | Al principio o al final |

# Ratón <a id="mouse-support"></a>

bluerig admite el ratón. Funciona en un terminal
**xterm(1)**
(incluso por una conexión telnet o ssh desde el xterm) y en la consola de
Linux con el servidor de ratón
**gpm**
en marcha.

Un clic pone el cursor donde se pulsa, arrastrar resalta texto, y un clic en
un menú, un botón o una etiqueta de tecla de función lo elige. La rueda
desplaza el texto.

La repetición automática por omisión de los botones del ratón es de 400
milisegundos. Se puede cambiar con el ajuste
*mouse_repeat_rate*
del archivo ini, y el tiempo en el que dos clics son un doble clic con
*double_click_speed*.

Si bluerig funciona con el ratón, se obtiene el comportamiento normal del
ratón en el terminal (cortar y pegar texto) manteniendo pulsada la tecla
Shift.

# Barra de menús <a id="menu-bar"></a>

La barra de menús aparece al pulsar F9 o al hacer clic en la fila superior
de la pantalla. Sus menús son Archivo, Edición, Buscar, Utilidades, Navigate
(solo cuando un complemento o un guion le añade algo), Ventana, Plugins y
Opciones. Un guion Lua puede añadir entradas a cualquiera de ellos, o menús
propios.

## Menú Archivo <a id="file-menu"></a>

**Abrir archivo...**
: Abrir un archivo en una ventana nueva.

**Nuevo**
: Abrir una ventana vacía.

**Cerrar**
: Cerrar la ventana.

**Historia...**
: Los archivos editados antes, para abrir de nuevo uno de ellos.

**Guardar, Guardar como...**
: Escribir el archivo, con su nombre o con otro. Véase
[Guardar como](#save-file-as).

**Insertar archivo..., Copiar a un archivo...**
: Insertar un archivo en el cursor, o escribir el bloque resaltado en un
archivo.

**Menú de usuario**
: El [menú de usuario](#edit-menu-file).

**Acerca de...**
: La versión del programa.

**Salir**
: Salir del editor, con una pregunta por los archivos sin guardar.

## Menú Edición <a id="edit-menu"></a>

Deshacer, Rehacer y Undo History...; el cambio entre los modos de inserción
y sobrescritura; las órdenes que resaltan un bloque, un bloque de columnas o
el archivo entero, y quitan el resaltado; copiar, mover y borrar el bloque;
copiar y cortar al archivo del portapapeles y pegar desde él; e ir al
principio y al fin del archivo.

## Menú Buscar <a id="search-menu"></a>

Buscar..., Buscar siguiente y Reemplazar...; el filtro de líneas (Toggle line
filter, Filter by word; véase
[Búsqueda, reemplazo y filtro de líneas](#search-and-replace)); y las marcas:
poner o quitar una marca en la línea actual, ir a la siguiente o a la
anterior, y quitarlas todas.

## Menú Utilidades <a id="command-menu"></a>

**Ir a la línea...**
: Ir a una línea, dada por su número.

**Numeración de líneas**
: Mostrar u ocultar la columna de números de línea.

**Buscar pareja del operador**
: Ir al paréntesis correspondiente al que está bajo el cursor.

**Poner/quitar sintaxis, Margen derecho, Toggle control characters**
: Activar y desactivar estas opciones.

**Código carácter...**
: La página de códigos en la que está escrito el texto. Véase
[Codificación de caracteres](#codepages-translation).

**Fold / Unfold block, Unfold all**
: Véase [Plegado](#folding).

**Refrescar la pantalla**
: Redibujar la pantalla.

**Iniciar/parar grabación macro, Macro explorer..., Grabar/repetir acciones**
: Véase [Macros](#macros).

## Menú Navigate <a id="navigate-menu"></a>

Las órdenes de los complementos: ir a la declaración, buscar las
referencias, los símbolos del archivo y otras. Véase
[Complementos](#plugins).

## Menú Ventana <a id="window-menu"></a>

**Mover, Cambiar tamaño**
: Mover la ventana o cambiar su tamaño con las flechas; Enter termina.

**Pantalla completa**
: Alternar la ventana entre pantalla completa y su propio tamaño.

**Toggle sticky windows**
: Con las ventanas pegadas, las ventanas cuyos marcos están uno junto a otro
cambian de tamaño juntas: la esquina de una ventana mueve el borde en el que
está con todas las ventanas a lo largo de él, a ambos lados, y no se abre
hueco entre ellas. Donde se juntan cuatro ventanas, la esquina mueve los dos
bordes. Un lado que está en el borde de la pantalla cuando empieza el cambio
de tamaño se queda allí, una ventana conserva su tamaño mínimo, y un borde se
para en una ventana que encuentra en su camino, y esa ventana cambia entonces
de tamaño con las demás. Esc devuelve cada ventana a su sitio. Una ventana
arrastrada por su título se mueve sola. El ajuste se guarda como
**editor_sticky_windows**
en el archivo ini.

**Siguiente, Anterior**
: Ir a la ventana siguiente o a la anterior.

**Listado...**
: Las ventanas abiertas, para ir a una de ellas. Véase
[Archivos abiertos](#open-files).

Debajo vienen las ventanas de los complementos: el árbol del proyecto, la
salida de la compilación, el panel y la consola del depurador, el terminal,
el terminal del programa que ejecuta el depurador, Preview. Una entrada abre
su ventana, la trae al frente cuando está detrás de otra, y la cierra cuando
ya está al frente; las abiertas llevan la marca *\**. Un complemento
desactivado no tiene entrada.

Las ventanas de los complementos están en zonas fijas. El árbol, el panel del
depurador y Preview van en la columna de la derecha, uno sobre otro, a toda
la altura de la pantalla. La consola, la salida de la compilación y el
terminal van en la fila de abajo, bajo las ventanas de los archivos: comparten
la fila como pestañas: se muestra una, y su título lista los nombres de
todas. Un clic en un nombre, o
**Ctrl-Alt-PgDn** y **Ctrl-Alt-PgUp** (*WindowTabNext* y *WindowTabPrev*
en el mapa de teclas), muestra otra. Una ventana de archivo a pantalla
completa llena el resto de la pantalla, y de nuevo la pantalla entera cuando
las zonas están vacías. Cambiar el tamaño de una ventana en una zona cambia
el tamaño de la zona; moverla la saca de la zona.

**Layout...**
: Las disposiciones de las ventanas: qué ventanas de los complementos están
abiertas, en qué zona, y qué tamaño tienen las zonas. *Edit* tiene solo los
archivos, *Project* el árbol del proyecto, *Debug* el árbol y el panel del
depurador. **Use** coloca las ventanas así; **Save as...** las guarda tal
como están con un nombre, que puede ser el de una de estas; **Delete** olvida
una disposición guardada, y vuelve la del editor de ese nombre. Con **Debug
layout while debugging**, al iniciar el depurador las ventanas se colocan como
en *Debug*, y al pararlo vuelven a su sitio, pero las pestañas de abajo siguen
abiertas para leerlas.

Las ventanas de un proyecto se guardan cuando termina el editor, y vuelven
cuando arranca con un archivo del proyecto.
*bluerig --debug* arranca con la disposición *Debug*. Las disposiciones se
guardan en *layouts.ini* en el directorio de configuración.

## Menú Plugins <a id="plugins-menu"></a>

Los complementos del editor que tienen una entrada propia, por ejemplo el
corrector ortográfico; sus ventanas están en el
[menú Ventana](#window-menu). Un complemento desactivado en
[Manage plugins](#manage-plugins)
no está en el menú.

## Menú Opciones <a id="options-menu"></a>

**General...**
: Las [opciones del editor](#editor-options).

**Modo de guardar...**
: Cómo se escribe un archivo. Véase [Modo de guardar](#edit-save-mode).

**Sintaxis coloreada...**
: Elegir a mano las reglas de sintaxis del archivo, o dejar que las decidan
el nombre y la primera línea del archivo.

**Aspecto...**
: La skin y las sombras. Véase [Aspecto](#appearance).

**Redefinir teclas...**
: Véase [Redefinir teclas](#learn-keys).

**Key bindings...**
: Véase [Key bindings](#key-bindings).

**Key sniffer...**
: Véase [Key sniffer](#key-sniffer).

**Manage plugins...**
: Véase [Manage plugins](#manage-plugins).

**Plugin info...**
: Véase [Plugin info](#plugin-info).

**Lua scripts...**
: Véase [Lua scripts](#lua-scripts). La entrada solo existe si bluerig se
compiló con Lua.

**Editar archivo de sintaxis**
: Abrir en el editor el archivo de sintaxis del usuario,
*~/.local/share/bluerig/syntax/Syntax*.
Si no existe, se crea con un breve comentario sobre lo que contiene, no como
copia del archivo del sistema, que se sigue leyendo después de él. Véase
[Resaltado de sintaxis](#syntax-highlighting).

**Menu file**
: Abrir en el editor el archivo del [menú de usuario](#edit-menu-file).

**Guardar configuración**
: Escribir los ajustes en
*~/.config/bluerig/ini*.
Con el ajuste
*auto_save_setup*
activado, se escriben siempre que termina el editor.

### Opciones del editor <a id="editor-options"></a>

La entrada
**General...**
del menú Opciones abre el diálogo de las opciones del editor. Cada opción se
escribe en el archivo ini, véase
[Ajustes del archivo ini](#internal-file-editor-options).

**Autoajuste de párrafo**

|  |  |
|--|--|
| Ninguno | Las líneas no se ajustan |
| Párrafos dinámicos | Formatea el párrafo bajo el cursor mientras se escribe |
| Máquina de escribir | Corta la línea en la longitud de ajuste mientras se escribe |

**Tabulación**

|  |  |
|--|--|
| Fingir medias tabulaciones | Entre el margen izquierdo y el texto una tabulación avanza la mitad de su ancho, y el archivo recibe espacios y tabulaciones |
| Borrar en tabulaciones | Backspace borra toda la sangría hasta el margen izquierdo cuando no hay texto entre el cursor y el margen |
| Tabulaciones x espacios | Inserta espacios hasta la siguiente posición de tabulación en lugar de un carácter de tabulación |
| Tamaño de tabulaciones | El ancho de un carácter de tabulación; 8 por omisión |

**Keyboard**

|  |  |
|--|--|
| Esc timeout, ms | Cuántos milisegundos se espera, tras Esc, al resto de una tecla antes de tomarlo como Esc; 1000 por omisión. En el archivo ini es *old_esc_mode_timeout* en microsegundos, y la variable **KEYBOARD_KEY_TIMEOUT_US** lo sustituye al arrancar |

**Otras opciones**

|  |  |
|--|--|
| Autoindentar | Enter empieza la línea con la sangría de la línea de arriba |
| Confirmar al guardar | Pregunta antes de escribir el archivo |
| Recordar posiciones | Abre el archivo donde se dejó |
| Espacios finales visibles | Marca los espacios al final de las líneas |
| Tabulaciones visibles | Marca los caracteres de tabulación |
| Show control characters | Muestra los caracteres de control como ^M en lugar de ocultarlos |
| Sintaxis coloreada | Colorea el texto según las reglas de sintaxis de su tipo |
| Cursor tras inserción | Deja el cursor al final de un bloque insertado, no al principio |
| Selección persistente | Conserva la selección cuando se mueve el cursor |
| Cursor pasa fin de línea | Deja que el cursor pase del último carácter de la línea |
| Deshacer en grupo | Deshace de una vez una serie de cambios del mismo tipo |
| Ancho del párrafo auto | La columna en la que los modos de ajuste cortan la línea; 72 por omisión |

### Modo de guardar <a id="edit-save-mode"></a>

Cómo se escribe un archivo, los mismos tres modos que admite el ajuste
*editor_option_save_mode*:

**Guardar rápido**
: Escribir sobre el archivo en el acto. Es rápido, pero un fallo a mitad deja el
archivo escrito a medias.

**Guardar seguro**
: Escribir primero un archivo temporal y renombrarlo sobre el original cuando
está entero, de modo que un fallo deja intacto el original.

**Conservar copias con extensión**
: Guardado seguro, y el original se conserva con su nombre más la extensión
de la línea de entrada, "~" por omisión. Guardar dos veces también sustituye
la copia.

**Verificar salto de línea POSIX**
: Preguntar por el salto de línea que falta al final del archivo antes de
escribirlo.

### Aspecto <a id="appearance"></a>

La skin con la que se dibuja el programa, y si los diálogos y los menús
desplegables tienen sombra.

**Skin**
: El botón abre la lista de las skins instaladas y las del usuario; la
elegida se aplica en el acto.

**Sombras**
: Dibujar una sombra bajo cada diálogo y menú desplegable. Sin colores no hay
sombras.

La elección se escribe en los ajustes
*skin*
y
*shadows*
del archivo ini. Véase la sección
[Skins](#skins)
para los archivos de skin.

### Redefinir teclas <a id="learn-keys"></a>

Este diálogo enseña a bluerig las secuencias de escape que envía su terminal
para las teclas de función, las flechas y las teclas de navegación. Su título
nombra el terminal, tomado de la variable
**TERM**.

Elija una combinación de modificadores (Ctrl, Alt, Shift) con las casillas, y
pulse después el botón de una tecla. Pulse la tecla en el teclado y espere a
que desaparezca el mensaje de captura. La secuencia aprendida aparece junto
al botón. Si otra tecla ya tiene esa secuencia, se avisa del conflicto.

**Del**
: Borrar la tecla aprendida en la que está el cursor.

**Guardar**
: Escribir las teclas aprendidas en
*~/.config/bluerig/term/\<TERM>*.

**Edit term file**
: Abrir ese archivo en el editor.

### Key bindings <a id="key-bindings"></a>

Las acciones del programa con las teclas a las que responden, en una tabla.
Los botones de arriba eligen el grupo, Editor o Widgets, y después la parte
del grupo: Main y Ctrl-X para el editor; Dialog, Menu, Input, Listbox, Radio y
Help para los widgets. Back vuelve a los grupos. Las acciones marcadas
con \* tienen teclas distintas de las teclas por omisión.

**Enter**
: Sustituir la tecla de la acción: pulse la tecla que quiere asignar.

**F5**
: Añadir otra tecla a la acción.

**F8, Del**
: Quitar la tecla.

**Guardar**
: Escribir los cambios en
*~/.config/bluerig/keymap.ini*.

**Edit keymap file**
: Abrir
*keymap.ini*
en el editor.

**Edit term file**
: Abrir las secuencias de teclas del terminal, el archivo que escribe
[Redefinir teclas](#learn-keys).

Si la tecla ya es de otra acción, se avisa y se pregunta si quitársela a esa
acción.

### Key sniffer <a id="key-sniffer"></a>

Pulse
**Capture key**,
y después cualquier tecla. El diálogo muestra lo que ha llegado:

```
Shortcut   the symbolic name, e.g. Ctrl-F5
Action     the action of the key in the editor keymap
Raw        the escape sequence and its bytes in hex
Keycode    the internal numeric code
```

Es el sitio donde mirar cuando una tecla no hace nada o hace otra cosa: una
tecla que llega como una secuencia que el programa no reconoce hay que
enseñarla en
[Redefinir teclas](#learn-keys),
y una que llega pero no tiene acción hay que asociarla en
[Key bindings](#key-bindings).

### Manage plugins <a id="manage-plugins"></a>

Los complementos que ha cargado el programa, en una tabla: el tipo, el nombre
y lo que el complemento dice de sí mismo. Aquí se listan los complementos del
editor y, si bluerig se compiló con Lua, el motor Lua como complemento del
tipo
*runtime*.
Un complemento se desactiva y se activa con la casilla de su fila, y lo
desactivado tampoco se carga la próxima vez; la elección se guarda en
*~/.config/bluerig/plugins.ini*.
Un complemento desactivado se queda en la lista, para poder activarlo de
nuevo.

**Enter, F4**
: Abrir la configuración del complemento en el que está el cursor. Si el
complemento no tiene configuración, lo indica. En la fila del motor Lua abre
[Lua scripts](#lua-scripts).

**IDE: project, build, debugger**
: Activar juntos los tres complementos que hacen del editor un IDE, o
desactivarlos juntos cuando están todos activos: el proyecto y sus archivos,
la compilación y el depurador. Cada uno funciona también sin los otros.

### Plugin info <a id="plugin-info"></a>

Los complementos que ha cargado el editor: el nombre, si está activo, qué
ofrece y qué hace. Es una lista solo de consulta: los complementos se
desactivan y se configuran en el diálogo
[Manage plugins](#manage-plugins).

### Lua scripts <a id="lua-scripts"></a>

Los guiones Lua encontrados, los del sistema y los del usuario, en una tabla:
si está activo, dónde está el guion (global o de usuario), su identificador,
su espacio de trabajo, qué ofrece y qué hace. Un guion se desactiva y se
activa con la casilla de su fila; el cambio tiene efecto la próxima vez que
arranca bluerig.

**Settings**
: Ejecutar el guion que lleva la configuración del paquete, si tiene uno.

Véase [Guiones Lua](#lua-scripting) para los guiones en sí.

# Búsqueda, reemplazo y filtro de líneas <a id="search-and-replace"></a>

{{key:Search|F7}} abre el diálogo de búsqueda.

|  |  |
|--|--|
| Normal | Busca la cadena tal cual |
| Expresión regular | Busca por una expresión regular |
| Hexadecimal | Busca bytes escritos como números hexadecimales |
| Caracteres comodín | Busca por un patrón del shell con \* y ? |
| Distinguir may/min | Distingue minúsculas de mayúsculas |
| Hacia atrás | Busca desde el cursor hacia el principio del archivo |
| Solo en selección | Busca solo en el bloque marcado |
| Palabras completas | Solo coincide con palabras completas |
| Cualquier codificación | Busca en todas las codificaciones en las que puede estar escrito el archivo |
| Buscar todos | Pone una marca en cada línea con una coincidencia |
| Filter | Oculta las líneas sin coincidencia |

{{key:Replace|F4}} abre el diálogo de reemplazo: los mismos campos y la línea
"Teclee el cambio a realizar". En un reemplazo con expresión regular, de \\1
a \\9 insertan los grupos capturados.

|  |  |
|--|--|
| Reemplazar | Reemplaza esta coincidencia y busca la siguiente |
| Todos | Reemplaza todas las coincidencias sin preguntar |
| Saltar | Deja esta coincidencia y busca la siguiente |

**Filter**

Oculta las líneas sin coincidencia; los números de línea no cambian, y la
columna de estado de las líneas marca cada tramo oculto. Las líneas se ocultan
una vez, al pulsar el botón: una línea cambiada o escrita después sigue
visible aunque no coincida.

|  |  |
|--|--|
| {{key:FilterToggle|Alt-s}} | Quita el filtro, o aplica la última búsqueda como filtro; sin búsqueda, abre el diálogo de búsqueda |
| {{key:FilterWord|Alt-S}} | Filtra por el texto marcado o por la palabra bajo el cursor |
| Utilidades \> Unfold all | Quita el filtro y todos los pliegues |

# Plegado <a id="folding"></a>

Un pliegue oculta un tramo de líneas tras su primera línea, que se queda en
la pantalla con una marca en la columna de estado de las líneas.
**Alt-F**
(Fold / Unfold block en el menú Utilidades) pliega las líneas resaltadas o,
sin nada resaltado, las líneas desde un paréntesis de la línea actual hasta el
paréntesis que hace pareja con él. En la primera línea de un pliegue lo
despliega.
**Unfold all**
abre todos los pliegues del archivo, y quita el filtro de líneas.

# Macros <a id="macros"></a>

{{key:MacroStartStopRecord|Ctrl-r}}
empieza a grabar una macro: se graban las teclas pulsadas después. Pulsar
**Ctrl-r**
de nuevo termina la grabación y espera la tecla a la que se asocia la macro;
después esa tecla la ejecuta.

Las macros se guardan en la sección
**[editor]**
del archivo
**~/.local/share/bluerig/macros**,
y una macro se borra borrando allí su línea, o en
[Macro Explorer](#macro-explorer).

**Grabar/repetir acciones**
en el menú Utilidades graba las acciones que siguen hasta que se elige de
nuevo, y después pregunta cuántas veces repetirlas.

También se puede asignar un guion del shell a una tecla, con una línea del
archivo
**macros**
como esta:

```
[editor]
Ctrl-W=ExecuteScript:25;
```

Esto significa que Ctrl-W ejecuta la acción
*ExecuteScript(25)*,
que ejecuta el guion del shell
**~/.local/share/bluerig/macros.d/macro.25.sh**.
Los guiones se guardan en el directorio
**~/.local/share/bluerig/macros.d/**
y deben llamarse
**macro.XXXX.sh**,
donde
**XXXX**
es un número de 0 a 9999. Se instalan ejemplos en
**{{pkgdatadir}}/examples/macros.d/**.
Un guion tiene el formato de las entradas del
[menú de usuario](#edit-menu-file)
y admite las mismas sustituciones. Su primera línea puede ser

*#silent*
: El guion se ejecuta sin pedir sus parámetros.

Este es un guion de ejemplo:

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

Las macros grabadas, con la tecla a la que responde cada una y lo que hace.
Los botones son

**Run**
: Reproducir la macro en la que está el cursor.

**Suprimir**
: Quitarla, tras una pregunta.

**Editar archivo**
: Abrir el archivo de las macros, descrito en la sección
[Macros](#macros).

# Menú de usuario <a id="edit-menu-file"></a>

{{key:UserMenu|F11}}
abre el menú de usuario: cada una de sus entradas es un guion del shell que
se ejecuta sobre el texto del editor. El menú se lee del primer archivo que
se encuentre de esta lista:

```
1) ./.bluerig.menu in the current directory
2) ~/.config/bluerig/menu
3) ~/bluerig.menu
4) {{sysconfdir}}/bluerig/bluerig.menu
5) {{pkgdatadir}}/bluerig.menu
```

El archivo del directorio actual solo se lee si pertenece al usuario o a root
y nadie más puede escribir en él: sus entradas ejecutan órdenes. La entrada
**Menu file**
del menú Opciones abre el archivo del menú en el editor.

Una línea que empieza con algo que no sea un espacio o una tabulación es una
entrada del menú, y su primer carácter es la tecla rápida de la entrada. Las
líneas que empiezan con un espacio o una tabulación son las órdenes de la
entrada. Una línea que empieza con '#' es un comentario.

Al elegir una entrada, sus órdenes se escriben en un archivo temporal, se
reemplazan las sustituciones que contienen, y el shell ejecuta ese archivo:

|  |  |
|--|--|
| %f, %p, %s | El nombre del archivo actual |
| %n | El nombre del archivo actual sin su extensión |
| %x | La extensión del nombre del archivo actual |
| %d | El directorio actual |
| %c | La posición del cursor en el archivo |
| %i | Una sangría de espacios tan ancha como la columna del cursor |
| %y | El tipo de sintaxis del archivo actual |
| %b | El archivo de bloque: el texto marcado se escribe en **~/.cache/bluerig/block** antes de ejecutar, y lo que las órdenes dejan allí sustituye al bloque |
| %m | El nombre del archivo del menú |
| %% | El carácter % |
| %{text} | Una línea de entrada con la indicación "text"; lo que se escribe se inserta, Esc o F10 cancelan la entrada |
| %view | Abre la salida estándar de las órdenes en una ventana nueva del editor; las palabras entre llaves tras ella, como en %view{ascii}, se saltan |

Las letras de sustitución no distinguen mayúsculas de minúsculas. Un número
tras % activa el entrecomillado (por omisión) o lo desactiva: %f y %1f
entrecomillan el valor insertado, %0f no.

Un archivo de menú de ejemplo:

```
s       Sort the block
	sort %b > %b.tmp && mv -f %b.tmp %b

w       Count the words of the file
	%view wc %f

m       Run make in the directory of the file
	make
```

**Entrada por omisión**

Una línea que empieza con '=' en la primera columna es una condición de la
entrada que tiene debajo: cuando la condición es cierta, la entrada pasa a
ser la entrada por omisión.

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

El patrón es un patrón del shell o una expresión regular, según el ajuste
*shell_patterns*;
una línea "shell_patterns=0" o "shell_patterns=1" como primera línea del
archivo de menú lo fija para ese archivo.

El tipo es uno o más caracteres:

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

Por ejemplo, 'rlf' es un archivo regular, un enlace o una FIFO.

Una condición que empieza con '=?' en lugar de '=' muestra una traza de
depuración cada vez que se calcula.

Las condiciones se calculan de izquierda a derecha; '&' no tiene más
prioridad que '|':

```
	= f *.c | f *.h & y C
```

se calcula como

```
	( (f *.c) | (f *.h) ) & (y C)
```

**Condiciones de adición**

Una condición que empieza con '+' (o '+?') pone la entrada en el menú solo
cuando es cierta. Una condición con '+=' o '=+' ('+=?' y '=+?' con traza)
funciona a la vez como condición de adición y como condición por omisión.
Para separarlas, se ponen antes de la entrada dos líneas de condición, una con
'+' y otra con '='.

# Complementos <a id="plugins"></a>

{{plugins}}
Un complemento se activa y se desactiva en el menú Opciones, entrada
[Manage plugins](#manage-plugins).

# Diálogos <a id="dialogs"></a>

## Guardar como <a id="save-file-as"></a>

El nombre con el que escribir el archivo, y los saltos de línea con los que
escribirlo: como los tiene el archivo, Unix (LF), Windows y DOS (CR LF), o
Macintosh (CR). La elección vale para ese guardado; el archivo se queda con
los saltos de línea que se le dan.

## Archivos abiertos <a id="open-files"></a>

Los archivos que tiene abiertos el editor, uno por línea. Enter va al archivo
en el que está el cursor, Esc se queda en el archivo que se muestra. La
misma lista es la entrada
**Listado...**
del menú Ventana.

## Selector de pantallas <a id="screen-selector"></a>

Además del editor, un guion Lua puede abrir pantallas propias, vistas a
pantalla completa que siguen abiertas mientras se vuelve al editor. Hay tres
formas de pasar de una pantalla a otra:

**Alt-}**
: pasar a la pantalla siguiente;

**Alt-{**
: pasar a la pantalla anterior;

**Alt-\`**
: abrir una ventana de diálogo con la lista de las pantallas abiertas, para
elegir una.

## Completado <a id="completion"></a>

{{key:Complete|Alt-Tab}}
en el editor completa la palabra antes del cursor con las palabras de los
archivos abiertos, y muestra una lista cuando encajan varias palabras. El
ajuste
*editor_wordcompletion_collect_all_files*
del archivo ini, activado por omisión, toma palabras de todos los archivos
abiertos; desactivado, solo del archivo actual.

**Alt-Tab**
en una línea de entrada de un diálogo completa el texto:

|  |  |
|--|--|
| $name | Una variable de entorno |
| ~name | Un nombre de usuario |
| Lo demás | Un nombre de archivo |

Si encajan varios, el programa pita. El ajuste
*show_all_if_ambiguous*
del archivo ini, desactivado por omisión, muestra su lista junto al cursor:
**Enter**
elige uno, las letras tecleadas reducen la lista, y Esc, F10, Left y Right
la ocultan.

## Codificación de caracteres <a id="codepages-translation"></a>

La lista de las páginas de códigos que conoce el programa, de
**{{pkgdatadir}}/charsets**.
Elegir una le dice al editor en qué página de códigos está escrito el texto
del archivo, y el texto se recodifica de ella a la página de códigos del
terminal para la pantalla;
**\<Sin traducción>**
lo deja como bytes. La lista se abre con
**Alt-e**
y con la entrada Código carácter... del menú Utilidades.

## Historia <a id="history-query"></a>

La lista de lo que se escribió antes en una línea de entrada, lo más reciente
primero. La abre
**Alt-h**
para la línea en la que está el cursor. Enter lleva a la línea la entrada en
la que está el cursor, Esc deja la línea como estaba, y
**F8, Del**
quita del historial la entrada en la que está el cursor. Los historiales se
guardan en
*~/.local/share/bluerig/history*;
*num_history_items_recorded*
dice cuántas entradas guarda cada uno.

# Resaltado de sintaxis <a id="syntax-highlighting"></a>

{{key:SyntaxOnOff|Ctrl-s}}
activa y desactiva el resaltado de sintaxis. Las reglas del resaltado están
en los archivos de sintaxis: allí reciben sus colores las palabras clave y los
contextos, como los comentarios y las cadenas.

|  |  |
|--|--|
| [Archivos de sintaxis](#syntax-files) | Dónde están las reglas y en qué orden se leen |
| [La sección file](#syntax-file) | Qué archivos resalta una sección de reglas |
| [Contextos](#syntax-contexts) | Zonas del texto con colores propios |
| [Incrustación y capas](#syntax-embed) | Reglas de otra sintaxis dentro de un contexto y sobre el texto |
| [Palabras clave](#syntax-keywords) | Palabras y patrones coloreados dentro de un contexto |
| [Colores y atributos](#syntax-colors) | Colores, tokens de ámbito de la skin, atributos |
| [Otras líneas](#syntax-other) | Comentarios, caseinsensitive, line-local |

## Archivos de sintaxis <a id="syntax-files"></a>

|  |  |
|--|--|
| **~/.local/share/bluerig/syntax/Syntax** | Las reglas del usuario: lo que el usuario añadió o quiere de otra forma |
| **{{pkgdatadir}}/syntax/Syntax** | Las reglas instaladas, para los tipos de archivo que no nombra el archivo del usuario |

Una entrada del usuario sustituye a una instalada para los archivos con los
que coincide. Los archivos nombrados por las líneas
**include**
se buscan primero en
*~/.local/share/bluerig/syntax/*,
y después entre los instalados. El archivo del usuario se lee de nuevo cada
vez que se abre un archivo nuevo en el editor.

Una copia completa del archivo instalado, como hacían versiones anteriores,
oculta todos los tipos de archivo instalados después de ella, y su resaltado
deja de funcionar: deje en la copia solo las entradas cambiadas.

## La sección file <a id="syntax-file"></a>

Un archivo de sintaxis se compone de secciones. Cada una empieza con una línea
**file**
y suele estar en un archivo propio con una línea
**include**:

```
file <file name> <description> [<first line>]
```

El
*nombre de archivo*
y la
*primera línea*
son expresiones regulares: la sección se aplica a un archivo cuando su nombre
o la primera línea de su texto coinciden. La
*descripción*
es el nombre de la sintaxis, con \s para un espacio: la usan las líneas
**embed**
y
**overlay**.
Una sección termina donde empieza la siguiente y se divide en contextos con
palabras clave.

Una sección trivial para un programa en C:

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

## Contextos <a id="syntax-contexts"></a>

Un contexto es una zona del texto con su propio color y sus propias palabras
clave, como un comentario de C entre /\* y \*/. Empieza con la línea:

```
context [exclusive] [oneline] [whole|wholeright|wholeleft] [linestart] <delim>
        [linestart] <delim> [<foreground>] [<background>] [<attributes>]
```

El primer contexto de una sección es
**context default**
[*foreground*] [*background*] [*attributes*];
sin él la sección da un error.

|  |  |
|--|--|
| linestart | El delimitador está al principio de una línea |
| whole | El delimitador es una palabra completa |
| wholeleft | El delimitador está en un límite de palabra a la izquierda |
| wholeright | El delimitador está en un límite de palabra a la derecha |
| exclusive | Colorea el texto entre los delimitadores, no los delimitadores |
| oneline | Tras exclusive: termina el contexto al final de la línea cuando el delimitador derecho no está en ella |

Una línea
**wholechars**
[**left**|**right**] *caracteres*
en cualquier lugar del archivo fija los caracteres de una palabra completa;
left y right fijan los dos lados por separado. Una línea
**spellcheck**
tras
**context**
pasa las palabras del contexto a la revisión ortográfica.

## Incrustación y capas <a id="syntax-embed"></a>

Una línea

**embed**
[**soft**]
*tipo*

tras
**context**
pasa el texto entre los delimitadores a las reglas de otra sintaxis;
*tipo*
es la descripción de su línea file, con \s para un espacio. El tipo se busca
en el archivo Syntax del que se leen las reglas, y después en el instalado con
bluerig. Un tipo que no está en ninguno deja el contexto con sus propios
colores. Los delimitadores conservan los colores del contexto. El delimitador
derecho termina el texto incrustado esté donde esté, también dentro de una
cadena o de un comentario de las reglas incrustadas. Una sintaxis incrustada
puede incrustar otra, hasta unos pocos niveles. Por ejemplo, HTML pasa los
scripts a las reglas de JavaScript:

```
  context <SCRIPT*> </SCRIPT> brightcyan
      embed JavaScript\sProgram
```

Con
**soft**
el delimitador derecho termina el texto incrustado solo fuera de las cadenas,
los comentarios y las palabras clave de las reglas incrustadas, de modo que en
{{ "}}" }} de una plantilla Jinja2 el primer }} sigue siendo parte de la
cadena, y en ${x/\\}/y} de un guion del shell el primer } sigue siendo parte
del escape.

Una línea

**overlay**
*tipo*

entre las reglas hace de toda la sintaxis una capa sobre
*tipo*:
sus contextos colorean lo que contienen estén donde estén, y el resto del
texto lo leen las reglas de
*tipo*,
que ven el contenido de los contextos como espacios en blanco. Así las
etiquetas de un lenguaje de plantillas se colorean igual en el texto, dentro
de una etiqueta HTML, en una cadena de un atributo y en un script, y HTML
colorea el resto. El contexto por omisión de una capa no colorea nada: una
palabra clave allí solo impide que empiece un contexto. Un tipo que no está en
ningún archivo Syntax deja la capa sobre texto simple. Por ejemplo, Smarty
sobre HTML:

```
  overlay HTML\sFile
  context default
      keyword {\s
  context {\* \*} brown
  context { } brightcyan
      embed soft Smarty\sTag
```

## Palabras clave <a id="syntax-keywords"></a>

Una palabra clave de un contexto se da con la línea:

```
keyword [whole|wholeright|wholeleft] [linestart] <string> [<foreground>] [<background>] [<attributes>]
```

Las cadenas de las palabras clave y de los delimitadores de contexto se leen
así:

|  |  |
|--|--|
| \\t, \\s | Tabulación y espacio: los blancos separan las palabras de la línea y no se pueden usar tal cual |
| \\n, \\r | Salto de línea y retorno de carro |
| `\\` | Barra invertida |
| \\\* | Asterisco |
| \* | Cualquier carácter, en cualquier número |
| + | Cualquier número de caracteres de palabra, también ninguno |
| \\[abc\\] | Cualquier número de caracteres del conjunto |
| \\{abc\\} | Un carácter del conjunto |

Un patrón no puede ser ni el primer ni el último carácter de un delimitador
de contexto. Por ejemplo,

```
  keyword         '*'      green
```

colorea de verde las constantes de un solo carácter de C, y

```
  keyword         "*"      green
```

colorea las constantes de cadena, siempre que no abarquen un salto de línea.

La línea

```
  keyword  \\\n  brightgreen
```

define una palabra clave formada por una barra invertida y un salto de línea.
Las palabras clave se comprueban antes que los delimitadores de contexto, así
que esta impide que el contexto termine al final de una línea con barra
invertida, y una directiva del preprocesador de C sigue en varias líneas.

Una línea
**define**
*nombre palabras...*
da un nombre que equivale a esas palabras en las reglas, como
**define comment brown**.

## Colores y atributos <a id="syntax-colors"></a>

Los colores: black, gray, red, brightred, green, brightgreen, brown, yellow,
blue, brightblue, magenta, brightmagenta, cyan, brightcyan, lightgray y
white. "default" es el color por omisión del terminal, "base" los colores
principales del editor; fija atributos sin cambiar el color de fondo. Con
256 colores existen además color16 a color255, rgb000 a rgb555 y gray0 a
gray23.

El color de primer plano puede ser un token de ámbito en lugar de un color: una
palabra en mayúsculas que dice qué es el texto y no qué aspecto tiene:

```
keyword  whole  if   KEYWORD_CONTROL
context  " " STRING_QUOTED_DOUBLE
    keyword  \\"   CONSTANT_CHARACTER_ESCAPE
keyword  linestart TODO  KEYWORD_OTHER_TODO black
```

El color de un token viene de la skin, de modo que una skin colorea el
programa y el texto, y todos los lenguajes de la misma forma; véase
[Skins](#skins).
Un token se lee como un ámbito: en minúsculas, con puntos en lugar de los
guiones bajos. KEYWORD_CONTROL_IMPORT es keyword.control.import, un tipo de
keyword.control, que es un tipo de keyword. Son los nombres de ámbito de
Sublime Text y TextMate. Lo que la skin no colorea conserva los colores que
mcedit daba a ese texto. Un fondo o unos atributos tras un token se imponen a
los colores de mcedit, como black arriba, pero no a un fondo o unos atributos
que la skin da al propio token. Un token funciona también en un define, y en
las líneas number, string y symbols de las reglas line-local. Los archivos de
sintaxis instalados solo usan tokens; su lista con los colores de mcedit está
en
**{{pkgdatadir}}/syntax/README.txt**.

Un archivo de sintaxis compartido con
**cooledit**
da los colores para ambos con una barra, y bluerig toma el color de delante:

```
keyword  #include  red/Orange
```

Los atributos son bold, italic, underline, reverse y blink, varios unidos con
un signo más.

## Otras líneas <a id="syntax-other"></a>

|  |  |
|--|--|
| # ... | Un comentario, en una línea propia |
| caseinsensitive | Al principio del archivo: no importan mayúsculas ni minúsculas |
| line-local | El modo para texto simple: se reinicia en cada salto de línea y en el borde de la pantalla y solo lee el texto mostrado |
| number *máx* *color* | En line-local: números decimales de hasta *máx* cifras |
| string *comilla* *color* | En line-local: cadenas entre comillas simples o dobles |
| symbols *caracteres* *color* | En line-local: signos de puntuación coloreados uno a uno |

# Colores <a id="colors"></a>

bluerig intenta averiguar si su terminal admite colores con la base de datos
de terminales y el nombre de su terminal. A veces se equivoca, así que se
puede forzar o desactivar el modo de color con las opciones -c y -b
respectivamente.

Si el programa se compiló con el gestor de pantalla S-Lang en lugar de
ncurses, también comprueba la variable
**COLORTERM,**
que, si está definida, tiene el mismo efecto que la opción -c.

Se pueden indicar terminales en los que siempre se fuerza el color añadiendo la
variable
*color_terminals*
a la sección Colors del archivo de inicialización. Así bluerig no intenta
averiguar si su terminal admite colores. Ejemplo:

```
[Colors]
color_terminals=linux,xterm
color_terminals=terminal-name1,terminal-name2...
```

El programa se puede compilar con ncurses y con S-Lang; ncurses no ofrece una
forma de forzar el modo de color: ncurses usa solo la información de la base
de datos de terminales.

# Skins <a id="skins"></a>

Los colores del programa y las líneas de sus marcos se toman de un archivo de
skin. El diálogo
[Aspecto](#appearance)
elige uno de una lista.

Si su skin contiene definiciones de color verdadero, debe definir la clave
'truecolors' con el valor TRUE en la sección [skin]. Si no se usa color
verdadero pero sí 256 colores, debe definir '256colors' en su lugar.

La skin es la primera que se nombre en esta lista:

```
1) command line option -S <skin>, --skin=<skin>
2) environment variable BLUERIG_SKIN
3) setting skin of the [bluerig] section of the ini file
4) the skin called default
```

Un nombre puede ser la ruta absoluta de un archivo de skin (con la
extensión .ini o sin ella). Si no, el archivo de skin se busca en estos
directorios; gana el primero que se encuentre:

```
1) ~/.local/share/bluerig/skins/
2) {{sysconfdir}}/bluerig/skins/
3) {{pkgdatadir}}/skins/
```

El formato de los archivos de skin se describe en
**{{pkgdatadir}}/skins/README.txt**.
Los colores del resaltado de sintaxis vienen de la sección
**[syntax]**
de la skin: allí un ámbito nombra los colores de un tipo de texto, y un
ámbito y el ámbito del contexto en el que está, los dos con un espacio entre
ellos, nombran los colores de ese tipo de texto en ese contexto. Gana el
ámbito más largo que nombra la skin:

```
[syntax]
    comment = gray
    keyword = yellow
    keyword.control = brightmagenta
    string = green
    string constant.character.escape = white;red
    invalid = white;red;bold
```

Un tipo de texto que la skin no nombra conserva los colores que le daba
mcedit, así que una skin puede nombrar unos pocos tipos y dejar el resto. Las
reglas que nombran colores en lugar de tokens de ámbito conservan sus colores
sea cual sea la skin; véase
[Resaltado de sintaxis](#syntax-highlighting).

# Bases de datos de terminales <a id="terminal-databases"></a>

bluerig ofrece una forma de corregir la base de datos de terminales del
sistema sin necesidad de privilegios de root. El diálogo
[Redefinir teclas](#learn-keys)
escribe lo que aprende en
**~/.config/bluerig/term/\<TERM>**,
una línea por cada tecla en su sección
**[keys]**.
bluerig busca también en el archivo del sistema
**defaults.ini**
(en
**{{sysconfdir}}/bluerig**
o
**{{pkgdatadir}}**)
y en el archivo
**~/.config/bluerig/ini**
la sección "terminal:nombre-de-su-terminal" y después la sección
"terminal:general". Cada línea de la sección contiene un símbolo de tecla que
quiere definir, seguido de un signo igual y la definición de la tecla. Puede
usar la forma especial \\e para representar el carácter de escape y ^x para
representar el carácter control-x. Una línea
*copy=other*
toma también todas las teclas de la sección "terminal:other".

Los símbolos de tecla posibles son:

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

Por ejemplo, para definir la tecla insert como Escape + [ + O + p, se pone
esto en el archivo ini:

```
insert=\e[Op
```

Un símbolo de tecla puede llevar modificadores:

```
    ctrl-alt-right=\e[[1;6C
    ctrl-alt-left=\e[[1;6D
```

Esto significa que ctrl+alt+left envía la secuencia de escape \\e[[1;6D y
por tanto bluerig interpreta "\\e[[1;6D" como Ctrl-Alt-Left.

El símbolo de tecla
*complete*
representa las secuencias de escape que invocan el completado; se invoca con
Alt-tab, pero se pueden definir otras teclas para el mismo trabajo (en esos
teclados con montones de teclas bonitas y sin usar por todas partes).

# Guiones Lua <a id="lua-scripting"></a>

Si bluerig se compiló con Lua, ejecuta guiones Lua que amplían el editor:
acciones asociadas a teclas y a menús, reacciones a los eventos del editor
(se abre un archivo, se guarda, se pulsa una tecla), diálogos y vistas a
pantalla completa. Varias órdenes del editor son guiones así: ordenar las
líneas, formatear el párrafo, insertar la salida de una orden, insertar un
carácter por su código, insertar la fecha, decodificar base64 y dibujar
tablas.

Los guiones del sistema están en
**{{pkgdatadir}}/lua/scripts/**,
y los guiones del usuario en
**~/.local/share/bluerig/lua/scripts/**,
un directorio por guion, con su
**lua.ini**
y su
**init.lua**.
El diálogo
[Lua scripts](#lua-scripts)
los activa y los desactiva.

La opción
**--no-lua**,
o la variable
**BLUERIG_NO_LUA**
con el valor 1, arranca el editor sin ningún guion Lua, y
*enabled=false*
en la sección
**[Lua]**
del archivo ini lo hace de forma permanente.

Cómo escribir un guion, y las funciones que puede llamar un guion, se
describe en
**LUA_PLUGINS.md**
y
**LUA_API_REFERENCE.md**,
que se instalan con la documentación de bluerig.

# Ajustes generales <a id="general-settings"></a>

Estos ajustes de la sección
**[bluerig]**
del archivo ini atañen al programa más que al editor. Salvo que se diga otra
cosa, 1 activa un ajuste y 0 lo desactiva.

*auto_save_setup*
: Escribir los ajustes en el archivo ini siempre que termina el editor.

*skin*
: El nombre de la skin. Véase [Skins](#skins).

*shadows*
: Dibujar una sombra bajo los diálogos y los menús desplegables.

*keymap*
: El archivo de teclas que se usa. Véase [Redefinir las teclas](#keys_redefine).

*drop_menus*
: Abrir el primer menú en el acto al pulsar F9, en lugar de solo la barra de
menús.

*mouse_close_dialog*
: Un clic fuera de un diálogo lo cierra.

*mouse_repeat_rate, double_click_speed*
: La repetición automática de los botones del ratón, y el tiempo en el que dos
clics forman un doble clic, en milisegundos.

*old_esc_mode, old_esc_mode_timeout*
: Con old_esc_mode activado, un solo Esc se toma como Esc tras
old_esc_mode_timeout microsegundos, en lugar de esperar a la tecla siguiente.

*show_all_if_ambiguous*
: Mostrar en el acto la lista de los completados posibles cuando el
completado es ambiguo. Véase [Completado](#completion).

*shell_patterns*
: Los patrones de las condiciones del menú de usuario son patrones del shell
(1) o expresiones regulares (0).

*num_history_items_recorded*
: Cuántas entradas guarda cada historial de una línea de entrada.

*confirm_history_cleanup*
: Preguntar antes de borrar un historial.

*clear_before_exec*
: Borrar la pantalla antes de ejecutar una orden del menú de usuario.

*pause_after_run*
: Esperar una tecla después de ejecutar una orden del menú de usuario: nunca
(0), solo en terminales tontos (1, por omisión) o siempre (2).

*alternate_plus_minus*
: Mantener las teclas más y menos del teclado numérico como más y menos.

*keybar_modifiers*
: Si es verdadero, la barra de teclas muestra las órdenes de Shift, Ctrl o Alt
con las teclas F mientras se mantiene pulsado ese modificador. Necesita un
terminal con el protocolo de teclado de kitty: bluerig le pide entonces cada
tecla con su liberación, y los eventos de foco. Los demás terminales no se
ven afectados. Falso por omisión.

*verbose*
: Mostrar el progreso de las operaciones largas.

*keybar_visible*
: En la sección
**[Layout]**:
mostrar las etiquetas de las teclas de función en la parte inferior de la
pantalla.

# Ajustes del archivo ini <a id="internal-file-editor-options"></a>

Los diálogos del menú
**Opciones**
fijan la mayoría de los ajustes, y
**Guardar configuración**
los escribe en la sección
**[bluerig]**
de
**~/.config/bluerig/ini**.
Estos son los ajustes del editor de ese archivo: 1 activa un ajuste, 0 lo
desactiva, y el valor por omisión va al final.

|  |  |
|--|--|
| editor_tab_spacing | El ancho de una tabulación; 8 por omisión. editor_fake_half_tabs da medias tabulaciones |
| editor_fill_tabs_with_spaces | Inserta espacios en lugar de un carácter de tabulación; 0 |
| editor_return_does_auto_indent | Enter sangra como la línea con texto más cercana por encima; 1 |
| editor_backspace_through_tabs | Backspace borra todo el espacio en blanco hasta el margen izquierdo cuando no hay texto entre el cursor y el margen; 0 |
| editor_fake_half_tabs | Una tabulación entre el margen izquierdo y el texto avanza 4 columnas, y el archivo recibe espacios y tabulaciones de 8 columnas; 1 |
| editor_option_save_mode | Cómo se escribe el archivo, véase [Modo de guardar](#edit-save-mode): 0 - directamente en el archivo, y un fallo al escribir lo deja cortado; 1 - a través de un archivo temporal; 2 - con una copia de seguridad; 0 |
| editor_word_wrap_line_length | La longitud de línea para el ajuste; 72 |
| editor_backup_extension | El sufijo del nombre de una copia de seguridad; ~ |
| editor_line_state | Muestra la columna de estado de las líneas: números, pliegues, tramos ocultos; 0. Alt-n la activa y desactiva |
| editor_visible_spaces | Muestra los espacios finales como puntos; 1 |
| editor_visible_tabs | Muestra las tabulaciones como \<----\>; 1 |
| editor_show_control_chars | Muestra los caracteres de control como ^M, ^L, si no, como una celda en blanco; 1. Una entrada del menú Utilidades lo activa y desactiva |
| editor_persistent_selections | Conserva la selección después de mover el cursor; 1 |
| editor_drop_selection_on_copy | Quita la selección tras copiar al archivo del portapapeles; 1 |
| editor_cursor_beyond_eol | Deja que el cursor pase del fin de la línea; 0 |
| editor_cursor_after_inserted_block | Pone el cursor tras un bloque insertado; 0 |
| editor_syntax_highlighting | Activa el resaltado de sintaxis; 1 |
| editor_edit_confirm_save | Pregunta antes de escribir; 1 |
| editor_option_typewriter_wrap | Corta la línea en editor_word_wrap_line_length mientras se escribe; 0 |
| editor_option_auto_para_formatting | Formatea el párrafo bajo el cursor mientras se escribe; 0 |
| editor_option_save_position | Guarda la posición en el archivo al salir; 1 |
| editor_group_undo | Deshace de una vez varias acciones seguidas del mismo tipo; 0 |
| editor_wordcompletion_collect_entire_file | Toma las palabras para el completado de todo el archivo, no solo hasta el cursor; 0 |
| editor_wordcompletion_collect_all_files | Toma las palabras para el completado de todos los archivos abiertos; 1 |
| editor_stop_format_chars | El formato de párrafos no toca una línea que empieza con uno de estos caracteres, ni las líneas que la siguen; -+\*\\,.;:&\> |
| editor_state_full_filename | Muestra la ruta completa en la línea de estado, no el nombre del archivo; 0 |
| editor_filesize_threshold | El tamaño a partir del cual un archivo se abre tras una pregunta, con un sufijo K o M; 64M |
| editor_show_right_margin | Marca la columna editor_word_wrap_line_length como margen derecho; 0. Una entrada del menú Utilidades lo activa y desactiva |
| editor_simple_statusbar | Muestra una línea de estado corta, sin el desplazamiento ni el carácter bajo el cursor; 0 |
| editor_check_new_line | Pregunta por un salto de línea que falta al final del archivo al guardar; 0. Una casilla del diálogo del modo de guardar lo activa y desactiva |
| editor_sticky_windows | Cambia el tamaño de las ventanas cuyos marcos se tocan como si fueran una, véase [menú Ventana](#window-menu); 0 |

# ENTORNO <a id="environment"></a>

Las variables de abajo son las que lee bluerig. Variables como **TERM**,
**SHELL**, **HOME** o **PATH** no se listan aquí: el programa las lee para
saber dónde se ejecuta, no para configurarse con ellas.

**BLUERIG_DATADIR**
: El directorio del que se toman los archivos de datos, en lugar de
{{pkgdatadir}}. Véase [ARCHIVOS](#files).

**BLUERIG_PROFILE_ROOT**
: La raíz de los archivos del usuario, como ruta absoluta, en lugar del
directorio personal. Véase [ARCHIVOS](#files).

**BLUERIG_SKIN**
: La skin que se usa, por nombre o por ruta. Véase [Skins](#skins).

**BLUERIG_KEYMAP**
: El archivo de teclas que se usa. Véase [Redefinir las teclas](#keys_redefine).

**BLUERIG_TMPDIR**, **TMPDIR**
: El directorio de los archivos temporales del programa, la primera de las
dos que esté definida; /tmp si no lo está ninguna.

**BLUERIG_NO_LUA**
: Con el valor 1, arrancar sin el entorno Lua. No se carga ningún guion Lua.

**KEYBOARD_KEY_TIMEOUT_US**
: Cuánto esperar al resto de una secuencia de escape, en microsegundos.

**COLORTERM**
: Se lee al elegir los colores. Véase [Colores](#colors).

**BLUERIG_LOG_ENABLE**, **BLUERIG_LOG_FILE**
: Se escribe un registro de depuración cuando BLUERIG_LOG_ENABLE es 1. Sin
**BLUERIG_LOG_FILE** se usa el archivo que nombra *logfile* en la sección
*[Logging]* del archivo *ini*, y sin esa entrada
*~/.cache/bluerig/bluerig.log*.

**BLUERIG_SPELL_LOG**
: El archivo en el que el corrector ortográfico escribe su registro de
depuración. No tiene un interruptor propio: el registro se escribe cuando la
variable nombra un archivo.

# ARCHIVOS <a id="files"></a>

Las rutas completas de abajo pueden variar entre instalaciones. Los archivos
de datos dependen también de la variable de entorno
**BLUERIG_DATADIR**:
si está definida, su valor se usa en lugar de {{pkgdatadir}} en las rutas de
abajo. Los archivos del usuario siguen los directorios base de XDG:
*~/.config*,
*~/.local/share*
y
*~/.cache*
representan
**XDG_CONFIG_HOME**,
**XDG_DATA_HOME**
y
**XDG_CACHE_HOME**
si están definidas.

*{{pkgdatadir}}/help/bluerig.md*
: El archivo de ayuda del programa, y los archivos de ayuda de los
complementos del editor junto a él.

*{{sysconfdir}}/bluerig/bluerig.ini*, *{{pkgdatadir}}/bluerig.ini*
: Archivos de configuración generales, que solo se usan si el usuario no tiene
su propio archivo
**~/.config/bluerig/ini**.
Si existe {{sysconfdir}}/bluerig/bluerig.ini, no se usa
{{pkgdatadir}}/bluerig.ini.

*{{sysconfdir}}/bluerig/defaults.ini*, *{{pkgdatadir}}/defaults.ini*
: Ajustes globales, que afectan a todos los usuarios, tengan o no
~/.config/bluerig/ini. Por ahora solo se cargan de él los
[ajustes del terminal](#terminal-databases).

*{{sysconfdir}}/bluerig/keymap.ini*, *{{pkgdatadir}}/keymap.ini*
: Las asociaciones de teclas generales. Véase
[Redefinir las teclas](#keys_redefine).

*{{sysconfdir}}/bluerig/bluerig.menu*, *{{pkgdatadir}}/bluerig.menu*
: El [menú de usuario](#edit-menu-file) general.

*{{sysconfdir}}/bluerig/spell.keymap, ctags.keymap*
: Las teclas de los complementos spell y ctags; un archivo con el mismo nombre
en ~/.config/bluerig las sustituye.

*{{pkgdatadir}}/syntax/\**
: Los archivos de sintaxis generales, que solo se usan si falta el archivo
correspondiente del usuario en
**~/.local/share/bluerig/syntax/**.

*{{pkgdatadir}}/skins/\**
: Las skins que vienen con el programa.

*{{pkgdatadir}}/charsets*
: La lista de las páginas de códigos.

*{{pkgdatadir}}/lua/scripts/\**
: Los guiones Lua que vienen con el programa.

*{{pkgdatadir}}/examples/macros.d/\**
: Guiones de ejemplo para las macros.

*{{plugins_dir}}/*
: Los complementos del editor que se cargan en tiempo de ejecución.

*~/.config/bluerig/ini*
: La configuración propia del usuario. Si existe este archivo, la
configuración se carga de aquí en lugar del archivo de configuración general.

*~/.config/bluerig/keymap.ini*
: Las asociaciones de teclas propias del usuario.

*~/.config/bluerig/menu*
: El [menú de usuario](#edit-menu-file) propio del usuario.

*~/.config/bluerig/plugins.ini*
: Los complementos y los guiones Lua desactivados.

*~/.config/bluerig/term/\<TERM>*
: Las teclas enseñadas en [Redefinir teclas](#learn-keys).

*./.bluerig.menu*
: El menú de usuario del directorio actual. Si este archivo está en el
directorio actual, se usa en lugar del menú personal o del general.

*~/.local/share/bluerig/history*
: Los historiales de las líneas de entrada.

*~/.local/share/bluerig/filepos*
: Los lugares donde se dejaron los archivos, para
*editor_option_save_position*.

*~/.local/share/bluerig/clipboard*
: El archivo del portapapeles del editor.

*~/.local/share/bluerig/macros*
: Las [macros](#macros) grabadas.

*~/.local/share/bluerig/macros.d/macro.N.sh*
: Los guiones del shell de las macros.

*~/.local/share/bluerig/syntax/Syntax*
: El archivo de sintaxis propio del usuario.

*~/.local/share/bluerig/skins/*
: Las skins propias del usuario.

*~/.local/share/bluerig/lua/scripts/*
: Los guiones Lua propios del usuario.

*~/.cache/bluerig/block*
: El archivo de bloque con el que trabajan el menú de usuario y los guiones de
las macros.

*~/.cache/bluerig/temp*
: Un archivo temporal del editor.

Para cambiar el directorio raíz de los archivos del usuario, se puede usar la
variable de entorno
**BLUERIG_PROFILE_ROOT**.
El valor de BLUERIG_PROFILE_ROOT debe ser una ruta absoluta. Si
BLUERIG_PROFILE_ROOT no está definida o está vacía, se usa la variable HOME.
Si HOME no está definida o está vacía, los directorios se toman de la
biblioteca GLib.

# LICENCIA <!-- help:skip -->

Este programa se distribuye bajo los términos de la Licencia Pública General
de GNU tal como la publica la Free Software Foundation. Véase la ayuda
interna para los detalles de la Licencia y la falta de garantía.

# DISPONIBILIDAD <a id="availability"></a>

La última versión de este programa se encuentra en
<https://github.com/blue-panels/bluerig/releases> .

# VÉASE TAMBIÉN <a id="see-also"></a>

cooledit(1), aspell(1), hunspell(1), ctags(1), etags(1), gpm(1),
terminfo(5), sh(1).

# AUTORES <a id="authors"></a>

Paul Sheer (psheer@obsidian.co.za) es el autor original de cooledit, que
bluerig continúa. Los autores y colaboradores se listan en el archivo AUTHORS
de la distribución de las fuentes.

# ERRORES <a id="bugs"></a>

Los errores deben comunicarse en
<https://github.com/blue-panels/bluerig/issues> .

Dé una descripción detallada del error, la versión del programa que usa
*(bluerig -V*
muestra esta información), y el sistema operativo en el que ejecuta el
programa. Si el programa falla, agradeceremos una traza de la pila.
