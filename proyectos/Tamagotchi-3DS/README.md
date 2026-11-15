# Instalación por QR

![QR de instalación CIA](docs/instalar-qr.png)

[Descargar CIA](https://raw.githubusercontent.com/AlvaroACG/Portafolio/main/descargas/TamaConnectionV2.cia) · [Todos los archivos y paquete SD](https://github.com/AlvaroACG/Portafolio/tree/main/descargas)

En una consola con homebrew y FBI instalado: abre **FBI → Remote Install → Scan QR Code** y escanea el QR. El enlace apunta directamente al CIA de esta publicación. Requiere conexión a Internet. No instala homebrew, firmware ni modifica la consola por sí mismo.

**Paso adicional obligatorio:** descarga el paquete `TamaConnectionV2-SD.zip` de la carpeta descargas y copia `3ds/` a la raíz de la SD. El CIA no incluye `sprites.bin`; el QR solo instala la aplicación.

Instalador ya existente de la carpeta local; no se afirma una nueva prueba física de instalación en consola. [Referencia: instalación remota de FBI](https://github.com/Steveice10/FBI).

---

# Tama Connection V2 · edición a color · v0.1

Recreación personal para New Nintendo 2DS XL basada en el Tamagotchi Connection V2 internacional de 2005 que se vendió en Chile. Es distinta del Original Gen 2 y del Familitchi V5.

El usuario confirmó en la consola que abre, se ve a color, nace la mascota y la partida se conserva al cerrar y volver a abrir.

## Instalar o abrir

- Desde el menú principal: FBI → SD → cias → TamaConnectionV2.cia → Install CIA.
- Desde Homebrew Launcher: /3ds/TamaConnectionV2/TamaConnectionV2.3dsx.
- El archivo sprites.bin debe permanecer en esa misma carpeta de la SD.

Ya se copiaron y verificaron los tres archivos de homebrew y el instalador CIA por FTP. No se sustituyó la app de IA, sus claves, el arranque ni archivos de sistema.

## Controles

- Cruceta: seleccionar iconos o moverse por menús.
- A: aceptar. B: volver.
- X: consultar la hora. SELECT: ajustes.
- START: guardar y salir.
- Pantalla táctil: iconos y menús.
- Los tres botones virtuales A/B/C del huevo funcionan como en el juguete: avanzar, aceptar y volver.

En SELECT puedes pausar la crianza, activar/desactivar sonido, cambiar el color del huevo, los nombres, el reloj y el cumpleaños. El modo rápido 60x acelera solo mientras la app está abierta.

**La crianza normal continúa al cerrar la aplicación.** El tiempo transcurrido se procesa al abrirla nuevamente. Usa SELECT → Pausar crianza antes de ausentarte si no quieres que necesite cuidados. Dormir sigue un horario propio de cada etapa. No hay avisos mientras el software está cerrado.

## Funciones

- Hambre, felicidad, educación, edad, peso y género.
- Comida, dulces y alimentos comprados; los dulces en exceso pueden causar dolor de muelas.
- Baño, limpieza, enfermedad, medicina, sueño y luces.
- Llamadas de atención, rechazo de comida, consuelo y disciplina.
- Huevo, bebé, niño, adolescente, adulto, mayor y final de vida.
- Evoluciones relacionadas con cuidados, educación y generación par/impar.
- Jump: reaccionar con izquierda/arriba/derecha cuando se ilumina el círculo.
- Bump: detener la barra con A para empujar al rival; ocho rondas.
- Heading: izquierda/derecha para colocarse y A para cabecear; veinte pelotas.
- Slot: apuesta de 1 a 9 puntos virtuales; A detiene cada casilla.
- Puntos Gotchi, tienda, inventario, juguetes, disfraces y cinco códigos clásicos.
- Amigos virtuales locales, visitas, regalos, juego compartido y pareja.
- Casamentera a partir de los siete años, bebé y relevo generacional tras dos días.
- Familia, álbum de personajes y celebraciones de cumpleaños/Navidad.
- Guardado automático cada treinta segundos, al actuar, al suspender y al salir, con checksum y respaldo anterior.

Los juegos se desbloquean por etapa. La mascota necesita superar su peso mínimo para jugar; si no quiere jugar y está en su mínimo, puedes darle un dulce.

## Personajes y arte

Hay 52 entradas en el álbum: los bebés, cuatro niños, diez adolescentes, treinta adultos, mayores, personaje especial, dos disfraces y Nazotchi como referencia. Incluye Mametchi, Mimitchi, Kuchipatchi, Memetchi, Pochitchi y el resto del plantel normal del V2. Los personajes solo obtenibles por depuración del juguete no forman parte de la crianza normal.

Los sprites a color son una interpretación visual nueva generada para esta app, no gráficos oficiales a color. El Connection V2 original era monocromo. Se usan 64 celdas de 32×32 píxeles, con personajes y objetos, preparadas a partir de recursos/personajes-v2.png. Nombres y personajes pertenecen a sus respectivos titulares; esta app no es oficial de Bandai.

## Límites de fidelidad

Es una recreación de las funciones y la experiencia de crianza, no una emulación del circuito ni una copia de su firmware. Los tiempos de necesidades, probabilidades de evolución, selección y precios de objetos y algunos eventos están adaptados; no se afirma equivalencia exacta con todos los algoritmos ocultos del juguete. Hay 28 tipos de alimento/objeto en esta primera versión.

La conexión infrarroja queda expresamente para una siguiente versión, a petición del usuario. Los amigos locales son vecinos simulados y no representan una conexión real con otro dispositivo. No se afirma compatibilidad con un Tamagotchi físico. Familitchi V5 no es la referencia de este proyecto.

## Archivos propios y recuperación

La partida está en /3ds/TamaConnectionV2/mascota.dat. La copia anterior está en mascota.bak. La app comprueba formato, tamaño, checksum y límites antes de cargar; si la principal falla, intenta la copia. Si ninguna es válida, no sobrescribe automáticamente los archivos encontrados. No elimines esas copias al actualizar.

La CIA tiene Title ID 000400000FF1A700, separado del de Nintendo 3DS IA. Solo utiliza su carpeta de la SD y los servicios normales de gráficos, entrada y sonido. No usa internet, IA ni un servidor en el PC.

## Compilación y pruebas

Ejecuta COMPILAR.cmd. Reutiliza las herramientas devkitPro portables instaladas en la carpeta del proyecto Nintendo-3DS-IA. El código está en source/ y el motor de reglas se puede probar en un PC con un compilador C11 usando tests/test_pet.c.

Validación realizada:

- Compilación ARM completada con GCC 16.1.0, libctru y citro2d/citro3d.
- 20 escenarios de reglas y 50.000 acciones combinadas: alimentación, salud, disciplina, compras, juegos, evolución, generaciones, límites y recuperación de un guardado corrupto.
- Transferencia FTP comprobada mediante descarga y SHA-256.
- Confirmación del usuario en el hardware: apertura, nacimiento y persistencia de la partida.

La evolución a lo largo de muchos días, cada personaje y las cuatro partidas completas de minijuegos todavía no se han verificado individualmente en hardware real.

## Referencias de diseño

- https://tamagotchi.fandom.com/wiki/Tamagotchi_Connection_Version_2
- https://tamagotchi.fandom.com/wiki/Tamagotchi_Connection_Version_2/Character_list
- https://tamagotchi.fandom.com/wiki/Tamagotchi_Connection_Version_2/List_of_Shop_Items
- https://www.spriters-resource.com/lcd_handhelds/tamagotchiconnectionversion2/ (referencia del plantel; no se incorporaron sus imágenes)
- https://www.virtualpet.com/vp/farm/bandai/infratam/bpress/05jun16release.pdf (comunicado de Bandai de 2005)


## Actualizacion visual v0.2
La mascota vive en una habitacion a pantalla completa en la pantalla superior, con ventana, papel mural, muebles y alfombra. Se elimina el marco que imitaba el juguete fisico. La pantalla inferior contiene el panel de cuidados texturizado y los menus. SELECT permite cambiar la paleta. Compatible con la partida anterior; no cambia el formato de guardado.


## Publicación para portafolio

Autor del proyecto personal/académico: Álvaro Carvajal. Publicado el 15 de noviembre de 2026. Los recursos y componentes externos conservan sus autores y licencias. No se incluyen credenciales, bases de datos personales ni configuraciones de servicios privadas. Las dependencias se instalan por separado.

El código propio se presenta para evaluación de portafolio. No se concede una licencia general de reutilización del código propio salvo donde exista una licencia expresa. Las licencias anteriores y de terceros prevalecen en sus componentes.
