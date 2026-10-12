# Long Range Telemetry Module (Phase-I)

[![SVNIT Surat](https://img.shields.io/badge/SVNIT-Surat%2C%20India-orange.svg)](https://www.svnit.ac.in/)
[![Platform](https://img.shields.io/badge/Platform-ATmega328P%20%7C%20AVR-blue.svg)](https://www.microchip.com/)
[![Carrier Frequency](https://img.shields.io/badge/Carrier%20Freq-433%20MHz-green.svg)](#rf-front-end--simulation)
[![Baseband](https://img.shields.io/badge/Baseband-100%20kHz%20Chip%20Rate%20(DSSS%2FCDMA)-purple.svg)](#firmware-architecture--timing)

> **B.Tech Phase-I Project Report & Codebase**  
> **Department of Electronics Engineering**, Sardar Vallabhbhai National Institute of Technology (SVNIT), Surat  
> **Academic Year:** 2025–26 | **Project Group:** G60  
> **Project Guide:** Dr. Suresh Dahiya (Assistant Professor, DoECE)  
> **Authors:** Moksha Parikh (`U23EC001`), Ridham Chaudhary (`U23EC030`), Omkar Sinh (`U23EC048`)

---

## 📌 Overview

This repository contains the embedded firmware, circuit schematics, simulation models, and validation benchmarks for the **Long Range Telemetry Module (Phase-I)** designed for high-altitude drone and UAV/CanSat payload communication.

Operating under the severe resource constraints of an 8-bit **ATmega328P** microcontroller (16 MHz clock, 2 KB SRAM), the system implements a **Direct Sequence Spread Spectrum (DSSS) / Code Division Multiple Access (CDMA)** baseband transmitter alongside a discrete 433 MHz RF front-end architecture.

### Key Capabilities
- **Dual-Channel CDMA Modulation:**
  - **In-Phase (I) Channel:** Carries GPS navigation data ($f_d = 100\text{ Hz}$) spread by a 1000-chip PRN sequence ($N = 1000$, theoretical processing gain $G_p = 30\text{ dB}$).
  - **Quadrature (Q) Channel:** Carries high-rate environmental telemetry ($f_d = 10\text{ kHz}$) spread by a 10-chip sequence ($N = 10$).
- **Cycle-Accurate 100 kHz Baseband Chip Rate:** $10.0\ \mu\text{s}$ per chip generated through Direct Port Manipulation (DPM), interrupt jitter elimination, and deterministic cycle calibration.
- **RF Front-End Design & Simulation:** Colpitts Local Oscillator (MPSH10 BJT) and dual-gate MOSFET (BF998) mixer stage simulated in NI Multisim and laid out on a custom 2-layer mixed-signal PCB in KiCad.

---

## 🏗️ System Architecture

```
                       +------------------------------+
                       |      2.5V - 3V Battery       |
                       +--------------+---------------+
                                      |
                            [DC-DC Boost Converter]
                                      |
                 +--------------------+--------------------+
                 | (3.3V / 5V Logic)                       | (Vcc RF)
                 v                                         v
+------------------------------------+    +----------------------------------+
|           Sensors & Inputs         |    |        Discrete RF Chain         |
| - u-blox NEO-6M GPS (UART @ 9600)  |    | - MPSH10 Colpitts LO (~358 MHz)  |
| - SHT-40 Humidity & Temp (I2C)     |    | - Dual BF998 MOSFET Mixers       |
| - PT1000-S Platinum RTD (I2C)      |    | - SAW Filter FL1 (433.92 MHz)    |
| - On-board NTC Thermistor (ADC)    |    | - RD01MUS2B RF Power Amplifier   |
+-----------------+------------------+    | - Pi-Matching Network & SMA Port |
                  |                        +-----------------+----------------+
                  v                                          ^
+------------------------------------+                       |
|       ATmega328P Controller        |                       |
| - LFSR PRN Generator (10-bit)      |                       |
| - 8x Bit-Packed Memory Storage     |                       |
| - Single-Cycle Port Writes (DPM)   |                       |
+--------+--------------------+------+                       |
         | (Dout-1: I-Chan)   | (Dout-2: Q-Chan)             |
         +--------------------+------------------------------+
```

---

## 📂 Repository File Structure

| Folder / File | Description |
| :--- | :--- |
| `GPS-Neo6M/` | Arduino firmware for NEO-6M GPS NMEA sentence extraction, checksum validation, and coordinate parsing using TinyGPS++. |
| `I_trail/` & `I_trial1/` | Baseband firmware implementation for the I-Channel (1000-chip LFSR generation, bit-packing, cycle-accurate delay loops). |
| `Q_trail/` | Baseband firmware implementation for the Q-Channel (10-chip array spreading code, corrected timing loops). |
| `task1/` & `task2/` | Firmware test suites, sensor serialization routines, and hardware debugging logs. |
| `BF998 MOSFET LIB/` | Spice simulation libraries and component footprints for the BF998 dual-gate N-channel MOSFET. |
| `Datasheet/` | Technical documentation and datasheets for active and passive components (MPSH10, BF998, RD01MUS2B, NEO-6M, SHT-40). |
| `CDMA transferring rate.pdf` | Documentation on CDMA transmission speed, spreading factors, and modulation characteristics. |
| `G48- Update-LO Simulation...` | Report and simulation data for the Local Oscillator circuit. |
| `Design1.ms14` | NI Multisim top-level schematic and RF modulator simulation circuit. |
| `L_oscillator.ms14` | NI Multisim Colpitts oscillator simulation model. |
| `d2.ms14` & `d2out.scp` | Multisim circuit simulation and oscilloscope graph captures. |
| `oscillator.cir` | SPICE netlist for the Colpitts Local Oscillator stage. |

---

## ⚡ Firmware & Timing Engineering

### 1. Memory Optimization (8× SRAM Reduction)
The ATmega328P has only 2 KB of SRAM. Allocating 1000 chips as individual `uint8_t` bytes would consume 1000 bytes (almost 50% of total SRAM):
- **Solution:** Bit-packing stores 8 chips per byte, reducing the 1000-chip array to just **125 bytes** (`uint8_t I_CDMA_PACKED[125]`).
- **LFSR PRN Generator:** Generated dynamically at startup via a 10-bit LFSR using primitive polynomial $x_n = x_{n-10} \oplus x_{n-7}$ with initial seed `0x02DF`, eliminating large hardcoded arrays in flash memory.

### 2. Elimination of Interrupt Jitter & Direct Port Manipulation
- Standard Arduino `digitalWrite()` requires ~50–60 clock cycles.
- **Direct Port Manipulation (DPM)** writes directly to `PORTB`, changing pin states in a single clock cycle ($62.5\text{ ns}$ at $16\text{ MHz}$):
  ```cpp
  PORTB |=  (1 << PORTB2); // Set Pin 10 HIGH (1 cycle)
  PORTB &= ~(1 << PORTB2); // Set Pin 10 LOW  (1 cycle)
  ```
- The Arduino core timer (`Timer0`) introduces up to $2\ \mu\text{s}$ interrupt jitter every $1.024\text{ ms}$. This is eliminated by calling `noInterrupts()` before entering the continuous transmission loop.

### 3. Cycle-Accurate Calibration Loop
To achieve a target chip rate of $f_c = 100\text{ kHz}$ ($T_c = 10.0\ \mu\text{s}$):
$$\text{Total Clock Budget} = 10.0\ \mu\text{s} \times 16\text{ MHz} = 160\text{ cycles}$$
Accounting for loop overhead, bit shifting, and DPM write (~3 cycles), the calibrated delay is:
```cpp
__builtin_avr_delay_cycles(157); // 157 cycles @ 16 MHz = 9.81 us
```

---

## 🔬 Hardware Verification & Measurement Results

Because random PRN sequences produce consecutive identical chips that cause auto-counter readouts to fluctuate on oscilloscopes (the *PRN frequency illusion*), a **DSO Calibration Mode** with an alternating `0xAA` (`10101010b`) test pattern was used for hardware verification:
$$f_{\text{DSO}} = \frac{f_c}{2} = 50.0\text{ kHz}$$

| Parameter | Theoretical / Target | Measured (Tektronix DSO) | Timing Deviation |
| :--- | :---: | :---: | :---: |
| **I-Channel Baseline (Alternating)** | $50.0\text{ kHz}$ ($20.0\ \mu\text{s}$ period) | $45.89\text{ kHz}$ ($22.20\ \mu\text{s}$) | $-8.22\%$ |
| **Q-Channel Baseline (Post-Correction)**| $50.0\text{ kHz}$ ($20.0\ \mu\text{s}$ period) | $45.03\text{ kHz}$ ($22.20\ \mu\text{s}$) | $-9.94\%$ |
| **Single Chip Duration** | $10.0\ \mu\text{s}$ | $\approx 11.10\ \mu\text{s}$ | Within $\pm 10\%$ SDR budget |
| **Logic Level (Rail-to-Rail)** | $5.0\text{ V}$ | $5.60\text{ V} - 7.60\text{ V}_{\text{pk-pk}}$* | Rail-to-rail confirmed |

*\*Note: High peak-to-peak reading on the DSO is attributed to probe measurement ringing on the nanosecond-fast DPM rise/fall edges.*

---

## 📡 RF Front-End & PCB Design

- **Colpitts Local Oscillator:** Built with an **MPSH10 NPN BJT** ($f_T \approx 650\text{ MHz}$).
  $$C_{eq} = \frac{C_1 C_2}{C_1 + C_2} = \frac{22 \times 22}{22 + 22} = 11\text{ pF}$$
  $$f_0 = \frac{1}{2\pi \sqrt{L_2 \cdot C_{eq}}} = \frac{1}{2\pi \sqrt{0.18\ \mu\text{H} \cdot 11\text{ pF}}} \approx 358\text{ MHz}$$
- **Multisim Simulation:** Sustained oscillation confirmed at $357\text{ MHz}$ with an output amplitude of $3.54\text{ V}_{\text{pk}}$ ($< 0.3\%$ discrepancy from analytical equations).
- **Custom PCB (KiCad):** 2-layer mixed-signal board featuring segregated functional partitions:
  - Top-Right: NEO-6M GPS module placed near the perimeter for antenna clearance.
  - Top-Center: ATmega328P TQFP-32 with SPI header and adjacent clock crystal.
  - Left: Power management (NCP1640T, HT3608) and Local Oscillator.
  - Bottom-Right: RF chain with dual BF998 mixers, SAW filter, RD01MUS2B PA, and SMA connector backed by a solid stitched ground plane.

---

## 👥 Authors & Academic Affiliation

* **Moksha Parikh** (`U23EC001`)  
* **Ridham Chaudhary** (`U23EC030`) — [GitHub Profile](https://github.com/chaudharyridham)  
* **Omkar Sinh** (`U23EC048`) — [GitHub Profile](https://github.com/Omkar1464)  

**Project Guide:**  
**Dr. Suresh Dahiya**, Assistant Professor, Department of Electronics and Communication Engineering  
**Sardar Vallabhbhai National Institute of Technology (SVNIT)**, Surat, Gujarat, India.
