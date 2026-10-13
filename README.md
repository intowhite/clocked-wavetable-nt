# Clocked Wavetable

![Clocked Wavetable artwork](wtlogo.png)

**Clocked Wavetable** is a free, open-source wavetable LFO and envelope generator for the [Expert Sleepers disting NT](https://www.expert-sleepers.co.uk/distingNT.html). Create clock-synchronized modulation from the NT’s wavetable library, morph between shapes, reshape their timing, and generate up to four phase-shifted CV outputs.

Designed for hands-on modulation: precise clocking when you need it, strange evolving waveforms when you don't.

## Features

- **Three playback modes:** continuously cycling LFO, triggered one-shot Envelope, and Gated cycling Envelope.
- **Flexible clock sync:** external CV, MIDI clock with transport controls, internal BPM, and CV tap tempo.
- **Divisions and multiplications:** ÷64 to ×64, with support for 4 PPQN, 24 PPQN, 1 PPQN, one pulse per bar, and one pulse every 2–128 bars.
- **Wavetable morphing:** scan and interpolate between waves in the NT’s existing or user-supplied wavetable banks; modulate Morph with CV.
- **Wave shaping:** Warp, Squish, Phase, Amplitude, and DC Offset, with a separate CV input for Warp.
- **Character modes:** 8-, 6-, 4-, and 2-bit quantization; plus stepped waveforms with 32, 16, 8, or 4 positions per cycle.
- **Four outputs:** a primary output plus three optional, independently routed phase-shifted copies (90°, 180°, and 270° by default).
- **Output ranges:** −5 to +5 V, 0–5 V, or 0–10 V.
- **Live waveform preview** and an **Adaptive** rendering mode for efficient CV generation.

## Installation

1. Open [Releases](../../releases) and download **`clocked_wavetable.o`** from the latest release.
2. Copy the file to **`/programs/plug-ins/`** on your disting NT's MicroSD card.
3. Safely eject the card and reboot the NT.
4. Add **Clocked Wavetable** from the NT’s algorithm list.

**Requirements:** disting NT with firmware **1.15.0 or newer**. No extra wavetable files are required—the plugin uses the NT's existing wavetable library. Compatible additional banks can be placed in the NT's `wavetables` directory.

> Download the `.o` asset under **Releases**. GitHub's automatically generated “Source code” ZIP is for developers and cannot be installed directly on the NT.

## Quick start

1. On **Main**, choose **LFO**, your output voltage range, and a wavetable bank. **Adaptive** CV quality is selected by default.
2. On **Clock**, select **External**, **MIDI clock**, **Internal**, or **Tap CV**. For external CV clock, choose the incoming clock format. The default is **4 PPQN** (one pulse per sixteenth note).
3. On **Shape**, set **Rate** and sweep **Morph**. Use **Warp** to shift the waveform's timing and **Squish** to reshape its phase progression.
4. On **Outputs**, route **Output A**. Enable **Phased output B**, **C**, or **D** to add phase-shifted copies.
5. For **Envelope** and **Gated** modes, assign a trigger or gate source on **Inputs**.

### Clock rates

The clock format tells the plugin how to interpret **incoming CV pulses**; it doesn't change the meaning of MIDI clock, which uses its standard 24 pulses per quarter note.

| CV clock format | Incoming pulses | Rate ×1 |
| --- | --- | --- |
| **4 PPQN** (default) | One pulse per 16th note | One cycle per quarter note |
| 24 PPQN | 24 pulses per quarter note | One cycle per quarter note |
| 1 PPQN | One pulse per quarter note | One cycle per quarter note |
| 1 Bar | One pulse per bar | One cycle per bar |
| Every N Bars | One pulse every 2–128 bars | One cycle per bar |

For bar-based clocks, the **Sync status** BPM readout assumes 4/4 and is shown as an estimate. Synchronization to the incoming bar pulses itself does not depend on time signature.

**MIDI transport:** Start resets the waveform phase, Stop freezes it, and Continue resumes. Your MIDI source must send transport messages as well as MIDI clock ticks.

## Parameters

| Page | Controls |
| --- | --- |
| **Main** | Mode, Voltage, Wavetable, CV quality, Playhead |
| **Shape** | Rate, Morph, Warp, Squish, Phase, Amplitude, Offset, Character |
| **Clock** | Clock source, applicable clock settings, Sync status, On clock stop (when applicable) |
| **Inputs** | Clock, trigger/gate, reset, Morph CV, Warp CV (shown where applicable) |
| **Outputs** | Output A, optional Phased outputs B–D, their phase offsets, Output mode |

Options that aren't relevant to the current mode or clock source are hidden automatically.

## Building from source

On Windows, install the [Arm GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) and make sure `arm-none-eabi-g++` is on your PATH. Then, from the repository folder:

```powershell
git clone https://github.com/expertsleepersltd/distingNT_API.git
.\build.ps1
```

The build produces `clocked_wavetable.o`. The SDK is an external dependency and is not included in this repository.

## License

Released under the [MIT License](LICENSE).

This is a community-created plugin for the Expert Sleepers disting NT, not an official Expert Sleepers product. Third-party wavetable banks are not included.
