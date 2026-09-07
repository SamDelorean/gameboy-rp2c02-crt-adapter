# Adaptador CRT Game Boy RP2C02

Proyecto abierto de hardware y firmware para **conectar un Nintendo Game Boy a una televisión CRT** utilizando como etapa final de video la **PPU del Nintendo Entertainment System / NES** (**RP2C02**).

Dicho de forma sencilla: el proyecto toma la información de imagen que normalmente va a la pantalla LCD del Game Boy, la adapta y escala digitalmente, y la entrega al chip de video del Nintendo para generar una señal compuesta NTSC que pueda mostrarse directamente en una televisión.

Además, el diseño busca aprovechar dos ventajas importantes:

- disponer de **paletas de color modificables**, tanto mediante presets elegidos por el usuario como, de forma opcional, mediante información de paleta recuperada de señales compatibles con Super Game Boy;
- mostrar la imagen original de 160x144 píxeles del Game Boy **a pantalla completa en 256x240**, utilizando un algoritmo fijo de repetición muy simple y determinista, sin necesidad de un escalador de video general ni de hardware o software de gran complejidad.

De esta manera, la PPU del NES no sólo genera la señal de televisión, sino que también sirve como etapa final de color para los cuatro tonos originales del Game Boy.

> **Estado:** fase de arquitectura y selección de componentes. Todavía no existe un esquemático de producción ni firmware validado como versión final.

## Arquitectura resumida

```text
Game Boy DMG / SGB-CPU
        |
        | LD0, LD1, CP, CPL, ST, S
        v
controlador digital económico
        |
        | captura + doble framebuffer
        | escalado simple a pantalla completa
        | 160x144 -> 256x240
        | paletas + SGB-lite opcional
        v
EXT0..EXT3
        |
PPU de video del Nintendo / NES
          (RP2C02)
        |
video compuesto NTSC
        v
  televisión / CRT
```

## Decisiones consolidadas

- PPU NTSC RP2C02 o clon discreto funcionalmente compatible.
- Captura directa de `LD0`, `LD1`, `CP`, `CPL`, `ST` y `S`.
- `P14/P15` opcionales para escucha pasiva de comandos de paleta Super Game Boy.
- Dos framebuffers completos de 160x144x2 bits: 11,520 bytes en total.
- Escalado a pantalla completa mediante nearest-neighbor fijo de 160x144 a 256x240.
- Relación horizontal `8/5` y vertical `5/3`.
- El escalado debe ser simple y determinista; no se requiere un escalador de video de propósito general.
- Render normal de tiles/sprites de NES deshabilitado en la primera versión.
- Índices externos entregados por `EXT0..EXT3`.
- Un botón para recorrer paletas globales curadas.
- Las paletas provenientes de SGB son opcionales y siempre pueden ser sustituidas manualmente mediante el mismo botón.
- Overscan inicial negro fijo.
- Sin identificación de juego ni colorización regional en la versión 1.

## Relojes de trabajo

- PPU RP2C02: aproximadamente **21.4772727 MHz**.
- Game Boy modificado: aproximadamente **4.2203555 MHz**.

La intención es derivar ambos relojes de una referencia común.

## Controlador

La selección todavía está abierta. El candidato principal actual es **RP2040 / Raspberry Pi Pico** por RAM, PIO, DMA, costo y facilidad de montaje, pero la decisión se cerrará con pruebas de temporización e I/O.

## Sistema de paletas

Los cuatro tonos del Game Boy se mapean globalmente a cuatro colores de la PPU. La paleta activa no es fija: se busca que pueda cambiarse de forma sencilla durante el uso.

La primera versión contempla:

- un conjunto pequeño de paletas manuales curadas;
- selección mediante un único botón;
- recepción opcional de paletas SGB si existe tráfico compatible en `P14/P15`;
- prioridad permanente del usuario: cualquier paleta SGB puede ser reemplazada con una pulsación del botón.

## SGB-lite opcional

Si se conectan `P14/P15`, el firmware podrá intentar decodificar inicialmente sólo:

- `PAL01`
- `PAL23`
- `PAL03`
- `PAL12`

La función es opcional y no garantiza que todos los juegos SGB transmitan comandos en cualquier configuración de Game Boy.

## Clones de PPU

El proyecto no exige una RP2C02 Ricoh original. Se podrán usar clones discretos NTSC reciclados o de stock antiguo si pasan una prueba específica de:

- `EXT0..EXT3`,
- RAM de paleta,
- reset/registros,
- `/INT` / VBlank,
- temporización NTSC,
- salida compuesta.

El hecho de que un chip funcione en un Famiclone no demuestra por sí mismo compatibilidad con este uso particular.

## Licencias

Se está preparando una estructura mixta:

- hardware: propuesta **CERN-OHL-W-2.0**,
- firmware: propuesta **MIT**,
- documentación: propuesta **CC BY-SA 4.0**.

La decisión se formalizará antes de la primera liberación de hardware reproducible.
