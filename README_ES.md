# Adaptador Game Boy RP2C02 CRT

Proyecto de prueba de concepto para estudiar una ruta alternativa de video de Game Boy usando el modelo de color/raster de una **PPU RP2C02 de NES** como etapa final de paleta y visualización.

El hito público actual es el **banco virtual híbrido de software**. SameBoy ejecuta el Game Boy, el código del proyecto aplica el escalado fijo 160×144 → 234×240 y la salida se previsualiza mediante una implementación Ricoh 2C02 fijada de johnmph/NESEmu.

> **Alcance de la versión actual:** prueba de concepto de software. La validación física con RP2C02/CRT queda como trabajo futuro y no es necesaria para reproducir el resultado del PoC.

## Qué demuestra

```text
SameBoy
  |
  | imagen 160x144 / 4 shades lógicos
  v
puente del proyecto
  |
  | escalado fijo 160x144 -> 234x240
  | 11 negro + 234 imagen + 11 negro
  | paleta global de 4 colores
  v
backend RP2C02
  |
  | Ricoh2C02 fijado de johnmph/NESEmu
  | códigos nativos de color de 6 bits
  v
salida comparativa SDL2 / PPM
```

La ventana muestra a la izquierda la referencia normal de SameBoy y a la derecha la misma imagen después del puente y de la ruta RP2C02.

## Implementado

- SameBoy como fuente de CPU, memoria, cartucho y LCD del Game Boy.
- Escalado horizontal fijo `160 -> 234`.
- Escalado vertical exacto `144 -> 240` con repetición `2,1,2`.
- Geometría `11 + 234 + 11` dentro de los 256 píxeles visibles de la PPU.
- Bordes negros independientes de los cuatro shades.
- Modelo RP2C02 reducido para regresiones sin dependencias.
- Backend opcional con la PPU Ricoh2C02 de **johnmph/NESEmu**.
- Modos de reloj `STOCK` y sincronizado `SYNC`.
- 16 paletas globales candidatas.
- Editor interactivo de paleta en SDL2.
- Soporte SGB limitado a una paleta RGB555 global de cuatro colores ya interpretada por SameBoy.
- Boot stub y ROM de smoke test escritos para el proyecto.
- CI para builds básicos, SDL2, NESEmu y SameBoy+NESEmu.

## Alcance SGB

El banco virtual **no** implementa transporte P14/P15/JOYP.

SameBoy interpreta el comportamiento SGB. Nuestro código recibe únicamente una paleta RGB555 sencilla de cuatro colores y la traduce al espacio de color del RP2C02 para `AUTO/SGB`.

No forman parte del PoC:

- atributos regionales SGB
- bordes gráficos
- transferencias de tiles
- identificación del juego
- base de datos de colorización
- emulación SGB completa

## Compilación del PoC completo

En Linux se requieren Git, CMake, toolchains C/C++, Make, archivos de desarrollo SDL2 y Clang.

Desde la raíz del repositorio:

```sh
sh ./emulator/scripts/bootstrap_sameboy.sh
sh ./emulator/scripts/bootstrap_nesemu.sh

cmake -S emulator -B build/poc \
  -DCMAKE_BUILD_TYPE=Release \
  -DGBCRT_ENABLE_SDL2=ON \
  -DGBCRT_ENABLE_SAMEBOY=ON \
  -DSAMEBOY_ROOT="$PWD/emulator/third_party/SameBoy" \
  -DGBCRT_ENABLE_NESEMU_PPU=ON \
  -DNESEMU_ROOT="$PWD/emulator/third_party/NESEmu"

cmake --build build/poc --parallel
ctest --test-dir build/poc --output-on-failure
```

Para generar la ROM de prueba propia:

```sh
python3 emulator/tests/generate_smoke_rom.py build/poc-smoke
```

Para ejecutar el visor:

```sh
./build/poc/gbcrt_viewer \
  --source-model dmg \
  --rom build/poc-smoke/gbcrt_smoke.gb \
  --boot build/poc-smoke/gbcrt_boot_stub.bin \
  --clock sync
```

Se puede sustituir la ROM de prueba por una imagen `.gb` obtenida legalmente. El repositorio no distribuye ROMs comerciales.

Consulta [`emulator/README.md`](emulator/README.md) para los detalles completos.

## Controles

- Flechas: D-pad
- `Z`: A
- `X`: B
- `Backspace`: Select
- `Enter`: Start
- `P`: siguiente paleta
- `E`: editor de paleta
- `M`: menú de reloj
- `C`: alternar STOCK/SYNC
- `Space`: pausa
- `Q`: salir

## Estado del hardware físico

El repositorio conserva investigación para una futura implementación física Game Boy → RP2C02, pero **no forma parte del criterio de aceptación de esta PoC**.

Si se retoma esa implementación:

- la imagen se obtiene de las señales lógicas LCD del Game Boy;
- se mantiene el escalado fijo y la geometría ya demostrados;
- se usa una referencia común de reloj;
- el criterio vigente de buffering es un registro/buffer mínimo de dos líneas, no dos framebuffers completos;
- una RP2C02 real o compatible será la etapa final de video/color;
- EXT, reloj, niveles eléctricos y video compuesto deberán demostrarse en hardware real.

El trabajo anterior de RP2350/Arduino-Pico se conserva como prototipo histórico y referencia, no como requisito de la arquitectura actual.

## Dependencias

El proyecto fija pero no incorpora dentro del repositorio:

- **SameBoy**
- **johnmph/NESEmu**

Consulta [`emulator/THIRD_PARTY.md`](emulator/THIRD_PARTY.md).

## Licencias

- software: **MIT**
- hardware: **CERN-OHL-W-2.0**
- documentación: **CC BY-SA 4.0**

Consulta [`LICENSES.md`](LICENSES.md) para el alcance exacto.
