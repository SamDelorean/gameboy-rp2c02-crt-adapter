# Clock Generator V0.1 — Si5351A 10-MSOP

Status: **proposal 1 developed to schematic level; bench validation still required**.

This sheet is intentionally separate from the central Game Boy DMG / SGB → Pico 2 → RP2C02 interconnect. It terminates at the two clock handoff nets already defined by the main schematic:

- `PPU_CLK_IN` ≈ 21.477272727 MHz
- `GB_CLK_IN` ≈ 4.220355488 MHz

## 1. Device/package choice

Use **Si5351A-B-GT**, 10-pin MSOP, for the first clock-generator prototype.

Reasons:

- only two clock outputs are required; the part provides three;
- MSOP is much easier to hand solder than the 16/20-pin QFN alternatives;
- the 10-MSOP has only one core supply and one output-buffer supply;
- no external `OEB`, `A0`, VCXO or CLKIN circuitry is required;
- all configuration is performed over the existing Pico 2 I2C bus.

Relevant pinout:

| Pin | Name | V0.1 connection |
|---:|---|---|
| 1 | VDD | 3.3 V |
| 2 | XA | 27 MHz crystal |
| 3 | XB | 27 MHz crystal |
| 4 | SCL | Pico 2 `GPIO21`, pull-up to 3.3 V |
| 5 | SDA | Pico 2 `GPIO20`, pull-up to 3.3 V |
| 6 | CLK2 | unused, leave open |
| 7 | VDDO | 3.3 V |
| 8 | GND | GND |
| 9 | CLK1 | `GB_CLK_RAW` |
| 10 | CLK0 | `PPU_CLK_RAW` |

## 2. Reference crystal

V0.1 uses a **27.000000 MHz crystal with 8 pF load capacitance**.

A qualified example from Skyworks AN551 is:

- Kyocera `CX3225SB27000D0FLJZ1`, 27 MHz, 8 pF, ±10 ppm initial accuracy, ±15 ppm temperature stability.

Another qualified option is NDK `NX3225GA-27.000M-STD-CRG-2`, 27 MHz, 8 pF.

The Si5351 provides selectable internal crystal load capacitance. With an 8 pF crystal, set `XTAL_CL` to 8 pF and **do not populate external crystal load capacitors** in the baseline circuit.

Crystal layout rules from Skyworks:

- mount the crystal immediately beside XA/XB;
- keep XA/XB traces shorter than 9 mm;
- keep clock/digital traces away from the crystal;
- use clean ground beneath/around the crystal area;
- avoid routing fast digital signals directly below it on a two-layer PCB.

## 3. Power and decoupling

Tie both `VDD` and `VDDO` to the project's 3.3 V rail. This automatically satisfies the requirement that VDDO be powered before or at the same time as VDD.

Place one local decoupling capacitor at each supply pin:

```text
3V3 ---+---- U3 pin 1 VDD
       |
      C1 100 nF
       |
      GND

3V3 ---+---- U3 pin 7 VDDO
       |
      C2 100 nF
       |
      GND
```

Skyworks recommends 0.1 to 1.0 µF per power-supply pin and states that additional external supply filtering is normally unnecessary because of the device's internal filtering/LDO structure.

An optional shared 1 µF local bulk-capacitor footprint may be provided but is not required by the baseline datasheet recommendation.

## 4. I2C interface

Use the already reserved Pico 2 pins:

```text
Pico 2 GPIO20 / SDA ----+---- U3 pin 5 SDA
                        |
                       R1 4.7 kΩ
                        |
                       3V3

Pico 2 GPIO21 / SCL ----+---- U3 pin 4 SCL
                        |
                       R2 4.7 kΩ
                        |
                       3V3
```

The datasheet requires external pull-ups of at least 1 kΩ. 4.7 kΩ is the V0.1 practical value for the short local bus; final rise time should be checked on the assembled board.

The blank `Si5351A-B-GT` has no user boot-frequency plan, so the RP2350 firmware must configure it over I2C before the two target devices are expected to run normally.

## 5. Frequency plan

Use **PLLA only** for both project clocks. PLLB is not needed.

Choose:

```text
XTAL = 27.000000 MHz
PLLA feedback ratio = 350 / 11
VCOA = 859.090909090... MHz
```

This lies inside the documented 600–900 MHz VCO range.

### CLK0 — PPU master clock

Use an **integer** output Multisynth divider:

```text
MS0 = 40
R0  = 1

859.090909090... MHz / 40
= 21.477272727... MHz
```

This is deliberate. Skyworks recommends an integer output-divider ratio where jitter matters; the RP2C02 clock is the more jitter-sensitive branch because it ultimately determines NTSC raster/chroma timing.

### CLK1 — synchronized Game Boy clock

Maintain the project frequency ratio exactly:

```text
f_GB / f_PPU = 798 / 4061
```

With the same VCO:

```text
MS1 = 81220 / 399
    = 203 + 223/399
R1  = 1

859.090909090... MHz / (81220/399)
= 4.2203554879... MHz
```

Therefore the two programmed outputs retain the exact intended frame-rate relationship in the synthesis math. Any static crystal frequency error is common to both outputs rather than becoming relative drift between them.

### Useful raw register parameters

For later firmware/register-map work, using the AN619 equations:

```text
PLLA 350/11 = 31 + 9/11
P1 = 3560
P2 = 8
P3 = 11

MS0 = 40 integer
P1 = 4608
P2 = 0
P3 = 1

MS1 = 81220/399 = 203 + 223/399
P1 = 25543
P2 = 215
P3 = 399
```

These values are a calculation basis; the final firmware must still set all required source, power-down, drive-strength, clock-control and PLL-reset registers according to AN619.

Spread spectrum must remain disabled.

## 6. Raw outputs and optional conditioning

Baseline raw handoff:

```text
U3 pin 10 CLK0 ---- R3 0 Ω footprint ---- PPU_CLK_RAW
U3 pin  9 CLK1 ---- R4 0 Ω footprint ---- GB_CLK_RAW
```

The Si5351 output buffers are CMOS and VDDO is limited to 3.3 V in this implementation. Do **not** assume yet that direct 3.3 V clock drive is electrically optimal for every RP2C02/clone, DMG or SGB revision.

Provide 0-ohm series footprints close to U3. These serve as isolation/debug points and can later be changed to a measured damping value if ringing/EMI requires it.

### Optional 5 V buffer stage

If bench measurements show that either vintage clock input should be driven with a 5 V TTL-compatible waveform, use a small **74AHCT125-class** quad buffer powered from 5 V:

```text
PPU_CLK_RAW -> U4A (74AHCT125 @ 5 V) -> PPU_CLK_IN
GB_CLK_RAW  -> U4B (74AHCT125 @ 5 V) -> GB_CLK_IN
```

The unused gates can be disabled. Add one 100 nF local decoupling capacitor to U4.

This buffer is **not mandatory in V0.1** until the destination thresholds/waveforms are measured. The schematic should make it easy either to bypass or populate the conditioning stage.

## 7. Minimum component count

### Core generator, no 5 V buffer

| Ref | Part | Qty |
|---|---|---:|
| U3 | Si5351A-B-GT, 10-MSOP | 1 |
| Y1 | 27 MHz, 8 pF crystal | 1 |
| C1/C2 | 100 nF decoupling | 2 |
| R1/R2 | 4.7 kΩ I2C pull-ups | 2 |
| R3/R4 | 0 Ω series/debug footprints | 2 optional |

Mandatory external parts around U3, excluding the IC itself: **5** (one crystal, two capacitors, two resistors).

Including the recommended two 0-ohm output footprints: **7 external parts**.

### With optional 5 V conditioning

Add:

- U4: one 74AHCT125-class buffer;
- C3: one 100 nF decoupling capacitor;
- optional output damping resistors/footprints if measurements justify them.

The clock subsystem therefore remains a very small circuit even with level restoration.

## 8. Schematic-level V0.1

```text
                         3V3
                          |
             +------------+------------+
             |                         |
          C1 100n                  C2 100n
             |                         |
            GND                       GND
             |                         |
        +-----------------------------------+
        | U3 Si5351A-B-GT, 10-MSOP         |
        |                                   |
  27MHz | XA 2                         1 VDD|---3V3
  Y1 ---|                                   |
        | XB 3                        7 VDDO|---3V3
        |                                   |
 GPIO21-| SCL 4                      CLK0 10|---R3--- PPU_CLK_RAW
 GPIO20-| SDA 5                       CLK1 9|---R4--- GB_CLK_RAW
        |                              CLK2 6|---NC
        |                               GND 8|---GND
        +-----------------------------------+
             |   |
          R2 |   | R1
        4.7k |   | 4.7k
             |   |
            3V3 3V3

PPU_CLK_RAW ---- [optional 5 V buffer/bypass] ---- PPU_CLK_IN
GB_CLK_RAW  ---- [optional 5 V buffer/bypass] ---- GB_CLK_IN
```

## 9. Bench validation before freeze

Measure:

1. actual 27 MHz reference frequency;
2. PLL lock/startup after RP2350 configuration;
3. `PPU_CLK_RAW` frequency, duty cycle, jitter/edge shape and ringing;
4. `GB_CLK_RAW` frequency, duty cycle, jitter/edge shape and ringing;
5. exact ratio of the two outputs over long acquisition;
6. behavior with direct 3.3 V drive at the RP2C02/clone clock input;
7. behavior with direct 3.3 V drive at the selected DMG/SGB clock input;
8. if required, repeat with the 5 V AHCT buffer populated;
9. CRT color stability and PPU `/INT` frame rate;
10. Game Boy operation at the synchronized clock.

## 10. Primary references

- Skyworks, `Si5351A/B/C-B` datasheet, rev. 1.3.
- Skyworks AN551, `Crystal Selection Guide for Si5350/51 Devices`.
- Skyworks AN619, `Manually Generating an Si5351 Register Map for 10-MSOP and 20-QFN Devices`.
