# Clocked Wavetable — disting NT

**Clocked Wavetable** is a native C++ modulation generator for the [Expert Sleepers disting NT](https://www.expert-sleepers.co.uk/distingNT.html). It plays and morphs through the NT's SD-card wavetable banks as a tempo-synchronized **LFO, one-shot envelope, or gated cycling envelope**. A graphical waveform display, phase warping, voltage controls, and optional phase-shifted outputs make it useful for both conventional modulation and more experimental CV patterns.

> Public release source: **v1.0.0**. Before publishing the ARM binary, compile and check this exact source on a disting NT; the newly renamed phase-output labels have not yet been hardware-verified here. Firmware **1.15.0 or later** is required by the API v13 page-refresh functionality. This project is community-created, not an official Expert Sleepers product.

## Features

- **Tempo sync:** External CV clock (4, 24, or 1 PPQN; one pulse per bar; or one pulse every N bars, N=2–128), incoming MIDI clock with Start/Stop/Continue transport, internal BPM, and CV tap tempo.
- **Musical rates:** divisions and multiplications from **÷64** to **×64**, with one cycle per quarter note at ×1 for PPQN clocks, or one cycle per bar at ×1 in bar-clock modes.
- **Three playback modes:** LFO, triggerable one-shot Envelope, and Gated cycle mode.
- **Wavetable morphing:** scans existing NT wavetable banks. Supports alternate banks selected on the Wavetable parameter. Includes Morph CV.
- **Wave shaping:** **Warp** shifts the temporal midpoint, **Squish** bends phase progression, with separate Phase, Amplitude and Offset controls. Warp CV supported.
- **CV ranges:** −5…+5 V, 0…5 V and 0…10 V.
- **Character:** Clean, 8/6/4/2-bit output quantization, or 32/16/8/4 positions per cycle for stepped modulation.
- **Optional phased outputs B/C/D:** independent enable/routing, each with its own relative phase offset (defaults 90°/180°/270°); disabled by default. First turn from Off automatically suggests the next unused physical output (typically Out 2/3/4), rather than Input 1. Manual bus selection is retained.
- **On-device waveform preview** and grouped, conditional parameter pages (Main, Shape, Clock, Inputs, Outputs).
- **Adaptive CV renderer**, designed to reduce CPU load compared with full-rate wavetable processing. Full rate remains selectable.

## Installation (no compilation needed)

1. Open the **Releases** section on this GitHub repository and download `clocked_wavetable.o` from the most recent release. Alternatively use the packaged `clocked_wavetable_plugin.zip` if provided.
2. Place `clocked_wavetable.o` on the disting NT MicroSD card in **`/programs/plug-ins/`**.
3. Safely eject the card from your computer and reboot the module.
4. Open **Misc → Plug-ins → View info** to confirm the plugin scanned successfully, then add **Clocked Wavetable** to a preset.

A built `.o` **must** be attached to a GitHub Release; GitHub's automatic source ZIP is not an installable plugin. You only need additional SD-card wavetable files if you want to use banks beyond the NT's existing library. **No third-party wavetable banks are bundled with this repository.**

## Quick start

- Set **Clock source** to `External`, then choose the actual rate of the pulses you send (default **4 PPQN**, one pulse per sixteenth note).
- Set **Shape → Rate** to `×1` for one LFO cycle per quarter note (PPQN) or per incoming bar (bar format).
- Set **Wavetable** to your preferred NT bank; use **Morph** to scan its shapes.
- Change **Warp** and **Squish**, then adjust **Voltage**, **Amplitude**, and **Offset** for your target CV destination.
- Route output A and optionally B/C/D (phase-shifted copies); Envelope mode also needs a trigger source, Gated mode a gate.
- For MIDI synchronization, select **Clock source = MIDI clock** and ensure your MIDI device sends realtime clock and Start/Stop/Continue as needed.

Bar-clock BPM display assumes 4/4 and is therefore an **estimate**; bar-relative synchronization itself does not require knowing the time signature.

## Building from source (Windows)

Requires [Arm GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) (`arm-none-eabi-g++`) and the official [disting NT API](https://github.com/expertsleepersltd/distingNT_API).

```powershell
git clone https://github.com/expertsleepersltd/distingNT_API.git
.\build.ps1
```

`build.ps1` checks unresolved symbol imports and generates `clocked_wavetable.o`. After confirming that the binary loads on your NT, attach it to a GitHub Release.

The repository intentionally **does not commit the SDK directory or a local `.o`**. Publish tested binaries as GitHub Release assets.

## Repository files

- `clocked_wavetable.cpp` — native disting NT plugin source
- `sin_lut.inc` — required DSP lookup-table data
- `build.ps1` — Windows build script
- `LICENSE` — MIT license
- `.gitignore` — ignores the downloaded SDK and generated binary

## License and acknowledgements

MIT License — see [LICENSE](LICENSE).

Uses the [Expert Sleepers disting NT API](https://github.com/expertsleepersltd/distingNT_API), which is a separate upstream dependency. Wavetable references to other devices are descriptive only; this project is not affiliated with Industrial Music Electronics or Expert Sleepers.
