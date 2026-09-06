# Adaptador CRT Game Boy RP2C02

Proyecto abierto de hardware y firmware para tomar directamente la señal LCD de un **Game Boy DMG**, reconstruirla digitalmente y mostrarla en un CRT NTSC utilizando una **PPU compatible con RP2C02** como generador de raster, etapa de paleta/colorización y fuente de video compuesto.

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
        | escalado fijo 160x144 -> 256x240
        | paletas + SGB-lite opcional
        v
EXT0..EXT3
        |
PPU RP2C02-compatible NTSC
        |
video compuesto
        v
       CRT
```

## Decisiones consolidadas

- PPU NTSC RP2C02 o clon discreto funcionalmente compatible.
- Captura directa de `LD0`, `LD1`, `CP`, `CPL`, `ST` y `S`.
- `P14/P15` opcionales para escucha pasiva de comandos de paleta Super Game Boy.
- Dos framebuffers completos de 160x144x2 bits: 11,520 bytes en total.
- Escalado nearest-neighbor fijo a 256x240.
- Relación horizontal `8/5` y vertical `5/3`.
- Render normal de tiles/sprites de NES deshabilitado en la primera versión.
- Índices externos entregados por `EXT0..EXT3`.
- Un botón para recorrer paletas globales curadas.
- Overscan inicial negro fijo.
- Sin identificación de juego ni colorización regional en la versión 1.

## Relojes de trabajo

- PPU RP2C02: aproximadamente **21.4772727 MHz**.
- Game Boy modificado: aproximadamente **4.2203555 MHz**.

La intención es derivar ambos relojes de una referencia común.

## Controlador

La selección todavía está abierta. El candidato principal actual es **RP2040 / Raspberry Pi Pico** por RAM, PIO, DMA, costo y facilidad de montaje, pero la decisión se cerrará con pruebas de temporización e I/O.

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
