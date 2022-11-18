## Dependencias

-  g++
-  ffmpeg
-  ncurses
-  SDL
-  sdl2_image
-  curl-devel

## Variables de entorno

-  AUDIOTRONIC_DRIVER: Ej: `export AUDIOTRONIC_DRIVER="alsa"`
-  AUDIOTRONIC_INPUT: Ej: `export AUDIOTRONIC_INPUT="front:CARD=USB,DEV=0"`
-  AUDIOTRONIC_SERVER: Ej: `export AUDIOTRONIC_SERVER="http://192.168.4.1"`

## Comandos

-  Limpiar: `make clean`
-  Compilar: `make all`
-  Ejecutar: `make run`
-  Compilar + ejecutar: `make`
-  Debug (gdb): `make debug`

## Pendiente

-  Filtro de banda (BPF)
-  Lectura de pipe fmpeg no bloqueante y acumulativa
-  Visualizador de frecuencias consola
-  Filtro por frecuencia: (frecuencia, q) => BPF
-  Modificación en vivo de filtro de frecuencias
-  Visualizador de frecuencias SDL
-  Escuchar audio original
-  Escuchar audio filtrado
-  Mostrar audio seleccionado
-  Web server de parametrización
