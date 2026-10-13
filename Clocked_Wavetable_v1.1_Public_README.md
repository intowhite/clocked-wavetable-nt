# Clocked Wavetable

**Clocked Wavetable** is a free, open-source wavetable LFO and envelope generator for the [Expert Sleepers disting NT](https://www.expert-sleepers.co.uk/distingNT.html). It turns the NT's wavetable library into a flexible modulation source, with clock synchronization, waveform morphing, phase shaping, and up to four related CV outputs.

Inspired by the clockable wavetable LFO on the disting mk4 and the modulation possibilities of the Kermit Mk3.

## Features

- **LFO, Envelope, and Gated modes** — continuously looping modulation, triggered one-shots, or looping waveforms while a gate is held.
- **Flexible synchronization** — external CV clocks, MIDI clock with Start/Stop/Continue, internal BPM, and tap pulses via CV.
- **Musical rate control** — divisions and multiplications from ÷64 to ×64. External clock formats include 4, 24, and 1 PPQN, one pulse per bar, or one pulse every 2–128 bars.
- **Wavetable morphing** — Smooth, Stepped, or automatic selection based on a bank's filename; Morph can be modulated by CV.
- **Waveform shaping** — Warp, Squish, Phase, Amplitude, and DC Offset, with a dedicated Warp CV input.
- **Character options** — 8-, 6-, 4-, and 2-bit voltage quantization, or rhythmic stepping at 32, 16, 8, or 4 positions per waveform cycle.
- **Up to four related outputs** — a main CV output and three optional, independently routed phase-shifted outputs (90°, 180°, and 270° by default).
- **Selectable voltage ranges** — −5 to +5 V, 0–5 V, or 0–10 V.
- **Live waveform display** and CPU-conscious **Adaptive** rendering.

## Installation

1. Open [Releases](../../releases) and download `clocked_wavetable.o` from the latest release.
2. Copy it to `/programs/plug-ins/` on the disting NT's MicroSD card.
3. Safely eject the card and restart the NT.
4. Add **Clocked Wavetable** from the algorithm selection list.

**Requirements:** disting NT firmware **1.15.0 or newer**.

The plugin works with the NT's existing wavetable library; no additional downloads are required. Compatible WAV banks can be added to `/wavetables/` on the MicroSD card.

> Download the `.o` release asset for installation. GitHub's automatic source-code archives are for developers.

## Quick start

1. On **Main**, select **LFO**, **Envelope**, or **Gated**, choose a voltage range, and leave **Morph mode** on **Auto** to follow each bank's preferred morph behavior.
2. On **Clock**, select **External**, **MIDI clock**, **Internal**, or **Tap CV**. For an external clock, match **Clock format** to the pulses coming from your sequencer or clock generator. The default is **4 PPQN** (one pulse per sixteenth note).
3. On **Shape**, choose a **Wavetable**, set a musical **Rate**, and explore **Morph**, **Warp**, and **Squish**.
4. On **Inputs**, assign optional Morph CV and Warp CV sources. Envelope and Gated modes also use the **Trigger/gate** input.
5. On **Outputs**, choose **Output A** and optionally enable **Phased output B**, **C**, and **D** for phase-shifted copies of the same waveform.

### Parameter pages

| Page | Controls |
| --- | --- |
| **Main** | Mode, Voltage, Morph mode, CV quality, Playhead |
| **Shape** | Wavetable, Rate, Morph, Warp, Squish, Phase, Amplitude, Offset, Character |
| **Clock** | Clock source, applicable clock settings, Sync status, On clock stop (where applicable) |
| **Inputs** | Clock in, Trigger/gate, Reset in, Morph CV, Warp CV (as applicable) |
| **Outputs** | Output A, optional Phased outputs B–D and their phase offsets, Output mode |

The NT shows relevant controls according to the selected playback mode, clock source, and enabled outputs.

## Clock synchronization

**Clock format** describes the pulses arriving at the external CV input. The **Rate** setting determines how many waveform cycles play over the resulting musical interval.

| CV clock format | Incoming clock | Rate ×1 |
| --- | --- | --- |
| **4 PPQN** (default) | One pulse per sixteenth note | One cycle per quarter note |
| **24 PPQN** | 24 pulses per quarter note | One cycle per quarter note |
| **1 PPQN** | One pulse per quarter note | One cycle per quarter note |
| **1 Bar** | One pulse per bar | One cycle per bar |
| **Every N Bars** | One pulse every 2–128 bars | One cycle per bar |

**Sync status** shows the detected tempo. For bar-based clock formats, the displayed BPM assumes 4/4 and is marked as an estimate; the waveform remains synchronized to incoming bar pulses regardless of time signature.

**MIDI transport:** Start resets the waveform phase, Stop freezes it, and Continue resumes it. Your MIDI source needs to send transport messages as well as MIDI clock ticks.

**Tap CV** measures quarter-note timing from incoming pulses on the assigned Clock input; it does not use a front-panel tap button.

## Morph modes and wavetable naming

Morph scans through the individual waveforms within the selected WAV bank.

| Morph mode | Behavior |
| --- | --- |
| **Auto** (default) | Uses Smooth for ordinary banks and Stepped for banks marked with the `-step` filename tag. Displays **Auto (Smooth)** or **Auto (Stepped)**. |
| **Smooth** | Blends continuously between adjacent waveforms. |
| **Stepped** | Selects complete stored waveforms without blending between them. Morph and Morph CV still work. |

To choose Stepped automatically for a particular bank, add `-step` as a standalone part of its filename, before the suffix that specifies samples per waveform:

```text
Tidal_Forms-256.wav           → Auto (Smooth)
Arpeggios-step-256.wav        → Auto (Stepped)
Sequences-step-512.wav        → Auto (Stepped)
Patterns-step-2048.wav        → Auto (Stepped)
```

The final numeric suffix (such as `-256`, `-512`, or `-2048`) tells the NT the number of samples in each waveform. The `-step` tag is specific to **Clocked Wavetable** and applies to the entire bank, not individual waves. Smooth and Stepped can always be selected manually, regardless of the filename.

**Note:** Stepped Morph controls *switching between stored waveforms*. The Character settings such as **Choppy 16** and **Chunky 8** instead control the number of time positions *within* each waveform cycle.

## Optional 3moji wavetable collection

The [Releases](../../releases) page also offers **`3moji_Clocked_Wavetable_Curated_10_Banks.zip`**: ten original modulation-oriented banks, with **64 waveforms each** (640 total). Banks carrying the `-step` tag use Stepped Morph under Auto.

| Bank | Modulation style |
| --- | --- |
| **Angular Circuits** | Jagged, irregular, and angular contours |
| **Envelope Sculptures** | Plucks, swells, holds, and evolving envelope shapes |
| **Folded Signals** | Multi-peak and wavefolded modulation |
| **Tidal Forms** | Smooth, evolving LFO contours |
| **Euclidean Gates** | Distributed gate rhythms on a 16-step grid (`-step`) |
| **Ratchet Bursts** | Stutters, bursts, and rhythmic fills (`-step`) |
| **Gentle Arpeggios** | Quantizer-friendly pitch patterns (`-step`) |
| **Wub 1248 Ramp Down** | One, two, four, and eight falling ramps (`-step`) |
| **Wub 1248 Sine** | One, two, four, and eight sine cycles (`-step`) |
| **Wub 1248 Square** | One, two, four, and eight square cycles (`-step`) |

Download the optional ZIP, extract it, and copy the WAV files to `/wavetables/` on the NT's MicroSD card. The collection is not required to run the plugin.

### Using Gentle Arpeggios with a quantizer

The **Gentle Arpeggios** bank is deliberately authored at low amplitude for pitch modulation. With **Voltage = −5 to +5 V**, **Amplitude = 100%**, and **Offset = 0 V**, it produces approximately **0–1 V** of CV, corresponding to one octave at 1 V/oct. Use Morph Mode **Auto (Stepped)** to select complete note patterns. Avoid normalizing the WAV unless you intend to change its voltage range.

## Building from source

On Windows, install the [Arm GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) and make sure `arm-none-eabi-g++` is on your PATH. From the repository folder:

```powershell
git clone https://github.com/expertsleepersltd/distingNT_API.git
.\build.ps1
```

The resulting `clocked_wavetable.o` can be installed on the NT. The Expert Sleepers API is an external dependency and is not included in this repository.

## License

Released under the [MIT License](LICENSE). Clocked Wavetable is a community-created plugin, not an official Expert Sleepers product.
