# IPC - [I]nter[P]rocess [C]omunication

El t'ermino IPC se refiere a los mecanismos de comunicaci'on entre distintos procesos que conviven dentro de un mismo sistema. Este tema est'a t'ecnicamente explicado en el material [^IPC_DOC_EN] [^IPC_DOC_ES] (cap'itulo 6), por lo tanto lo que busco es presentarlo de manera did'actica.

[^IPC_DOC_EN]:
[The Linux Programmer's Guide](https://tldp.org/LDP/lpg-0.4.pdf#chapter.6%21)
[^IPC_DOC_ES]:
[Guia del Programador en Linux](https://www.academia.edu/24622349/Gu%C3%ADa_Linux_de_Programaci%C3%B3n_GULP)

En un sistema (computador) de la vida real todo se desarrolla a trav'es de procesos. Un '_proceso_' es una tarea b'asica desarrollada por el sistema operativo (SO). Cada vez que abrimos el procesador de texto, una ventana del navegador web, etc, el SO '_genera_' o engendra (`spawn`) un proceso. Por lo tanto, los procesos son las entidades que hacen vida dentro del sistema. 

Pero estos procesos necesitan comunicarse entre si, de otro modo no podr'ia llevarse a cabo ninguna actividad realmente '_importante_' en la vida real. Por ejemplo, en una aplicaci'on que resguarde informaci'on en una Base de Datos (como cualquier sistema de backend t'ipico de una p'agina web), tenemos como m'inimo un primer proceso interactuando con el usuario, y recibiendo las peticiones de 'este (por ejemplo escrito en PHP), pero que delega el manejo de la informaci'on en otra aplicaci'on especializada como MySQL. 

Los procesos se pueden comunicar entre si mediante (a) _Sockets_, y (b) mecanismos IPC de System V. Los sockets no ser'an objeto de estudio en este material, aunque s'i los IPC. Estos 'ultimos se dividen en:

- Colas de mensajes (**m**e**s**sage-**q**ueues, o `msq`)
- Sem'aforos (**sem**aphores, o `sem`)
- Segmentos de memoria compartida (**sh**ared-**m**emory, o `shm`)

Ahora bien, debe existir una identificaci'on 'unica para cada canal de comunicaci'on abierto entre dos procesos. Es decir, si el proceso A abre una cola para enviar mensajes al proceso B, debe existir una manera en que el receptor B pueda reconocer los mensajes que provienen de su emisor A, y no otros. O incluso, en caso de que existan varias colas abiertas entre dos procesos, debe haber una manera de identificar cada objeto IPC de manera 'unica. 

Para este respecto, existe lo que se conoce como **clave IPC**, generada con la funci'on `ftok` de la que hablaremos en la secci'on siguiente. En el libro de [^IPC_DOC_EN] se dice que la clave es como el an'alogo al n'umero telef'onico que discamos cuando intentamos hacer una llamada con otra persona. 

## Claves de IPC

La clave es el identificador del mecanismo IPC, e idealmente deber'ia haber una clave 'unica por cada IPC creado. De otro modo, la informaci'on transmitida por un objecto IPC, se confundir'ia con la informaci'on proveniente de otro.

Para generar una clave, se utiliza la funci'on `ftok()`, cuyo prototipo es:

```
#include <sys/types.h>
#include <sys/ipc.h>

key_t ftok(const char *pathname, int proj_id);
```

Aqu'i queremos aprovechar para comentar las formas en que, al menos a mi gusto, se puede buscar documentaci'on de alguna funci'on o procedimiento en Linux.

- El _manual_ de Linux, o manpage: teclear `man ftok` en consola. Esto dar'a la vesi'on Linux.
- El sitio de ayuda de **Open Group**: [ftok](https://pubs.opengroup.org/onlinepubs/007904975/functions/ftok.html), el cual normalmente describe la versi'on POSIX, conforme a IEEE. Por lo tanto, se puede considerar una versi'on bastante estandarizada de la funci'on buscada.
- El sitio de **Linux Die**: [ftok](https://linux.die.net/man/3/ftok), que normalmente describir'a la versi'on POSIX igualmente.
- Opcionalmente, el sitio de ayuda de **IBM**: [ftok](https://www.ibm.com/docs/en/zos/2.3.0?topic=functions-ftok-generate-interprocess-communication-ipc-key) el cual he descubierto contiene excelente documentaci'on y ejemplos. Aunque pueden incluir la definici'on de ciertas macros, como en este caso `_XOPEN_SOURCE_EXTENDED 1`, no necesariamente las mismas incluidas en las versiones POSIX m'as actualizadas.

En lo personal, cada vez que se vaya a investigar un tema Linux, recomiendo buscar de ser posible fuentes _oficiales_, de buena reputaci'on, comenzando por supuesto por las p'aginas del manual (_manpages_), y luego sitios reconocidos como p'aginas de empresa (Microsoft, IBM), o foros como StackOverflow, GeekforKeeks, etc.

Es bueno buscar en fuentes _diversas_, variadas, comparar la informaci'on obtenida entre ellas, y probar en nuestra propia versi'on de sistema operativo los ejemplos suministrados, hacer cambios en los ejemplos y ver los resultados, crear nuestros propios ejemplo, etc.; esta es la mejor manera de aprender.

Volviendo al caso de la funci'on `ftok`

```
#include <sys/types.h>
#include <sys/ipc.h>

key_t ftok(const char *pathname, int proj_id);
```

nos dice en primer lugar que debemos incluir los ficheros de cabecera `<sys/types.h`, y `<sys/ipc.h>` antes de invocar una llamada a `ftok` (es decir, incluir esta funci'on en nuestro c'odigo). Estas cabeceras contendr'an definiciones que `ftok` utiliza, adem'as del prototipo de la misma `ftok`, y de no incluir dichas cabeceras, el compilador posiblemente de un error.

La idea de `ftok` es generar una clave 'unica por directorio de trabajo e identificador de proyecto. El argumento `path` deber'ia ser la ruta del directorio donde se encuentre nuestro programa que utiliza IPC's, el cual como dice el _manpage_ debe referirse a una ruta existente y accesible:

> _The  `ftok()`  function uses the identity of the file named by the given pathname (which must refer to an existing, accessible file) ..._

El argumento `proj_id` o "identificador de proyecto" es un codigo num'erico cuyos 8 bits menos significativos deben ser distintos de cero, e identifica una aplicaci'on dentro de un directorio de trabajo. Es decir, mediante la combinaci'on

```
pathname + proj_id
```

podemos tener varios sistemas IPC, desde un mismo directorio de trabajo, cada uno de las cuales dispondr'a de una cola de mensajes, un sem'aforo y un segmento de memoria compartida. Si se desean varios IPC para una misma aplicaci'on, se deben combinar distintos `proj_id` con el mismo `pathname`.

_NOTA_: En versiones antiguas, el `proj_id` era definido de tipo `char`, y por razones de compatibilidad en la actualidad se declara `int` pero solamente son tomados los 8 bits menos significativos.

El `pathname` puede ser un nombre simb'olico como `'.'`, que la librer'ia resolver'a a la ruta del directorio desde donde se haya lanzado la aplicaci'on. Incluso, puede ser un enlace simb'olico, que igualmente ser'a resuelto a la ruta definitiva.

La _colisi'on_ ocurre cuando dos claves que deber'ian ser diferentes resultan ser iguales. Por ejemplo, el _manpage_ especifica que dos `pathname` diferentes pudieran producir la misma clave, ya que se utiliza el n'umero de `inodo` en lugar del nombre. Esto podr'ia pasar por ejemplo con `"/dev/hda1"`, y `"/dev/sda1"`. O, si se usan enlaces simb'olicos que apuntan al mismo directorio.

Para resolver la colisi'on, una aplicaci'on bien dise~nada podr'ia por ejemplo almacenar las claves previamente generadas, comparar con ellas, e intentar generar otra clave. 

En caso de 'exito, la funci'on `ftok()` devuelve la clave generada de tipo `key_t` (un valor num'erico). En caso de error, devuelve -1 y el el c'odigo de error es almacenado en la variable global `errno` (por lo que puede usarse `perror()` para imprimir un mensaje de error). 

La p'agina de [OpenGroup](https://pubs.opengroup.org/onlinepubs/007904975/functions/ftok.html) establece que debe devolver obligatoriamente los siguientes errores, en las siguientes situaciones:

[EACCES]
: Search permission is denied for a component of the path prefix.
: [**Traducción**] _Alg'un componente de la ruta especificada en el argumento no puede resolverse por falta de los permisos necesarios (como `rx`, por ejemplo)_.

[ELOOP]
: A loop exists in symbolic links encountered during resolution of the path argument. 
: [**Traducción**] _La resoluci'on de la ruta contiene un enlace simb'olico que conduce a un ciclo infinito (un enlace que apunta a si mismo, o a un directorio que contiene al mismo enlace)_.

[ENAMETOOLONG]
: The length of the path argument exceeds `{PATH_MAX}` or a pathname component is longer than `{NAME_MAX}`. 
: [**Traducción**] _la ruta, o un componente de la misma, es demasiado largo(a)_.

[ENOENT]
: A component of path does not name an existing file or path is an empty string. 
: [**Traducción**] _la ruta es vac'ia, o un componente de la misma no existe_.

[ENOTDIR]
: A component of the path prefix is not a directory. 
: [**Traducción**] _un prefijo de la ruta no es un directorio_.

Adicionalmente, la funci'on `ftok()` _puede_ fallar en las siguiente situaciones:

[ELOOP]
: More than `{SYMLOOP_MAX}` symbolic links were encountered during resolution of the path argument. 
: [**Traducción**] _demasiados enlaces simb'olicos en la resoluci'on de la ruta._

[ENAMETOOLONG]
: Pathname resolution of a symbolic link produced an intermediate result whose length exceeds `{PATH_MAX}`. 
: [**Traducción**] _la ruta es un enlace simb'olico cuya resoluci'on contiene un componente que es demasiado largo_.

Una buena pr'actica de programaci'on deber'ia contemplar estas situaciones y manejar los errores adecuadamente, por ejemplo mediante `switch/case`, o `if/else`.

### Un ejemplo sencillo.

Como primer ejemplo, vamos a generar una clave IPC por medio de varios m'etodos:

1. poniendo `path` como un directorio existente 
2. poniendo `path` como un directorio que no exista
3. poniendo `path` como la concatenaci'on de un directorio existente, y un archivo que no existe dentro de 'el.
4. poniendo `path` como la concatenaci'on de un directorio existente, y un archivo existente dentro de 'el.
5. poniendo `path` como una ruta v'alida, pero sin los necesarios permisos de lectura
6. poniendo `path` como el directorio actual, por medio de tres maneras diferentes:
    - escribiendo `"."`
    - usando la funci'on GNU `get_current_dir_name`
    - usando la funcion POSIX `getcwd`

Los tres m'etodos especificados en (5), deber'ian producir la misma clave, pues se trata del mismo directorio.

Para este ejercicio, se pide que se cree previamente un directorio, por ejemplo `/home/yoel/test/ipc`, que llamaremos `real_dir`, y se ponga dentro de 'el alg'un fichero, por ejemplo `1.txt`. La contatenaci'on de `<real_dir>/` y este nombre de fichero (e.g. `/home/yoel/test/ipc/1.txt`) se llamar'a `real_file`.

- En el caso (1), el programa intentar'a la ruta del directorio mencionado como path.
- En el caso (2), intentar'a `/home/nonexist`  (que probablemente no exista!)
- En el caso (3), intentar'a `<real_dir>`, pero con un fichero no existente. Por ejemplo `/home/yoel/test/ipc/nofile.txt`. Esto deber'ia fallar.
- En el caso (4), intentar'a `<real_file>`, lo cual deber'ia generar la clave correctamente.
- En el caso (5), intentar'a `<real_file>`, pero antes quitar'a los permisos de lectura y ejecuci'n al directorio padre, lo cual impedir'a resolver correctamente la ruta del archivo, y producir'a un error.
- En el caso (6), deber'ia generarse la misma clave en los tres sub-casos.

El programa se puede conseguir, dentro de la ruta de este repositorio, en `/ipc/examples/ftok/src/ex-01.c`. Puede compilarlo desde la carpeta `/ipc/examples/ftok/src` con:

```bash
gcc -W -std=c99 -o ex-01 src/ex-01.c && ./ex-01
```

La salida es:

```
* Case 1: existing directory: '/home/yoel/test/ipc' ...
The key is: 1131 (0x173ad1b0)

* Case 2: non-existing directory: '/home/nonexist' ...
ftok: No such file or directory

* Case 3: existing directory / non-existing file: '/home/yoel/test/ipc/nofile.txt' ...
ftok: No such file or directory

* Case 4: existing directory / existing file: '/home/yoel/test/ipc/1.txt' ...
The key is: 4577 (0x173ad1b0)

* Case 5: existing directory / existing file, but with no permissions: '/home/yoel/test/ipc/1.txt' ...
ftok: Permission denied

* Case 6a: current directory, using "." ...
The key is: 594 (0x173ad1b0)

* Case 6b: current directory, from `get_current_dir_name`: '/home/yoel/projects/LINUX_learning/ipc/examples/ftok' ...
The key is: 594 (0x173ad1b0)

* Case 6c: current directory, from `getcwd`: '/home/yoel/projects/LINUX_learning/ipc/examples/ftok' ...
The key is: 594 (0x173ad1b0)
```