# Clocked Wavetable

![Clocked Wavetable artwork](wtlogo.png)

**Clocked Wavetable** is a free, open-source wavetable LFO and envelope generator for the [Expert Sleepers disting NT](https://www.expert-sleepers.co.uk/distingNT.html). It turns the NT's wavetable library into a clock-synchronized modulation source with continuously morphable shapes, creative timing controls, and up to four phase-related CV outputs.

Inspired by the clockable wavetable LFO on the disting mk4 and the modulation possibilities of the Kermit Mk3.

## Features

- **Three playback modes:** Continuous **LFO**, triggered one-shot **Envelope**, and **Gated** looping modulation.
- **Flexible synchronization:** External CV clock, MIDI clock with Start/Stop/Continue transport, internal BPM, and tempo tapped through a CV input.
- **Musical timing:** Divisions and multiplications from **÷64 to ×64**. External clock formats include **4 PPQN** (default), 24 PPQN, 1 PPQN, one pulse per bar, and one pulse every 2–128 bars.
- **Wavetable morphing:** Continuously blend neighboring waveforms or switch discretely between them, with dedicated Morph CV control.
- **Shape controls:** **Warp**, **Squish**, Phase, Amplitude, and DC Offset, plus a dedicated Warp CV input.
- **Character modes:** 8-, 6-, 4-, and 2-bit voltage quantization, or 32-, 16-, 8-, and 4-position stepping within each waveform cycle.
- **Up to four outputs:** One main output and three optional phase-shifted outputs, each with its own output routing and phase offset.
- **CV ranges:** −5 to +5 V, 0–5 V, or 0–10 V.
- **Waveform display:** Live shape preview and CPU-conscious **Adaptive** rendering.

## Installation

1. Open [Releases](../../releases) and download **`clocked_wavetable.o`** from the latest release.
2. Copy it to `/programs/plug-ins/` on the disting NT MicroSD card.
3. Safely eject the card, restart the NT, and add **Clocked Wavetable** from the algorithm list.

**Requires disting NT firmware 1.15.0 or later.**

Clocked Wavetable works with the NT's existing wavetable library. The optional **3moji Modulation Wavetables Vol. 1** collection provides additional shapes; it is not required to use the plugin.

## Quick start

1. On **Main**, choose **LFO**, **Envelope**, or **Gated**, set your output voltage range, and leave Morph Mode on **Auto**.
2. On **Clock**, select **External**, **MIDI clock**, **Internal**, or **Tap CV**. When using External, select the clock format that matches your source. The default **4 PPQN** corresponds to one clock pulse per sixteenth note.
3. On **Shape**, select a **Wavetable**, choose a musical **Rate**, and explore Morph, Warp, Squish, Phase, Amplitude, Offset, and Character.
4. On **Inputs**, assign optional Morph CV and Warp CV sources. Use Trigger/gate for Envelope and Gated playback.
5. On **Outputs**, route Output A and, if desired, enable the optional **Phased outputs B, C, and D**.

### Parameter pages

| Page | Controls |
| --- | --- |
| **Main** | Mode, Voltage, Morph Mode, CV quality, Playhead |
| **Shape** | Wavetable, Rate, Morph, Warp, Squish, Phase, Amplitude, Offset, Character |
| **Clock** | Clock source, relevant timing settings, Sync status, On clock stop (when applicable) |
| **Inputs** | Clock in, Trigger/gate, Reset in, Morph CV, Warp CV (as applicable) |
| **Outputs** | Output A, optional Phased outputs B–D and their relative phase offsets, Output mode |

The parameter pages display the controls relevant to the current mode and routing.

## Clock synchronization

**Clock Format** tells the plugin how to interpret the incoming CV clock. **Rate** determines the modulation cycle length relative to that clock.

| External clock format | Input interpretation | Rate ×1 |
| --- | --- | --- |
| **4 PPQN** (default) | One pulse per sixteenth note | One cycle per quarter note |
| **24 PPQN** | 24 pulses per quarter note | One cycle per quarter note |
| **1 PPQN** | One pulse per quarter note | One cycle per quarter note |
| **1 Bar** | One pulse per bar | One cycle per bar |
| **Every N Bars** | One pulse every 2–128 bars | One cycle per bar |

**Sync status** reports the detected tempo. With bar-based clocks, the displayed BPM is estimated assuming 4/4; synchronization itself follows the incoming bar interval and does not require a time-signature setting.

**MIDI transport:** Start resets the waveform phase, Stop freezes it, and Continue resumes it. Your MIDI source must send transport messages as well as MIDI clock ticks.

**Tap CV** measures quarter-note intervals from pulses on the assigned Clock input; it does not use a front-panel tap button.

## Wavetable morphing

Morph moves through the individual stored waveforms in the selected bank.

| Morph Mode | Behavior |
| --- | --- |
| **Auto** (default) | Smooth for normal banks; Stepped when the filename contains a standalone `-step` tag. |
| **Smooth** | Blends continuously between adjacent stored waveforms. |
| **Stepped** | Selects complete stored waveforms without blending between them. |

In Auto, the Main page can show **Auto (Smooth)** or **Auto (Stepped)**. Morph and Morph CV remain active in both behaviors.

### The `-step` filename convention

A bank can request discrete waveform selection by including the standalone `-step` tag in its filename:

```text
3moji-Tidal_Forms-256.wav          → Auto (Smooth)
3moji-Euclidean_Gates-step-256.wav → Auto (Stepped)
Arps-step-512.wav                  → Auto (Stepped)
Patterns-step-2048.wav             → Auto (Stepped)
```

The final numeric suffix (e.g. `-256`, `-512`, `-2048`) identifies the number of samples in **each waveform** to the NT. The `-step` tag is a **Clocked Wavetable convention** and applies to the entire bank. You can always override it with Smooth or Stepped in the menu.

Stepped **Morph Mode** changes how the plugin selects *between waveforms*. The stepped **Character** settings change the number of positions sampled *within one waveform cycle*.

## Optional wavetable collection

**3moji Modulation Wavetables Vol. 1** is available as a separate ZIP under [Releases](../../releases). It contains **15 original banks, 64 waveforms per bank (960 total)**, designed for modulation rather than conventional audio-rate oscillator playback.

| Bank | Character | Auto Morph |
| --- | --- | --- |
| **Angular Circuits** | Jagged, angular and irregular contours | Smooth |
| **Envelope Sculptures** | Plucks, swells, holds, and evolving envelope shapes | Smooth |
| **Envelope Sculptures II** | Dramatic attacks, staircases, bouncing pulses, and asymmetric tails | Smooth |
| **Euclidean Gates** | 16-step distributed gate rhythms and rotations | Stepped |
| **Folded Signals** | Folded, complex and multi-peaked waveforms | Smooth |
| **Gentle Arpeggios** | Quantizer-oriented melodic patterns spanning approximately one octave | Stepped |
| **Human Gestures** | Knob flicks, overshoots, exponential turns, hand-pumped wubs, and backspins | Smooth |
| **Ratchet Bursts** | Front-loaded bursts, accelerating/decelerating ratchets, and rising/falling intensity | Stepped |
| **Tidal Forms** | Smooth, organic, harmonically evolving curves | Smooth |
| **Two-Octave Arpeggios** | Wide arpeggios, octave echoes, pendulums and melodic motifs | Stepped |
| **Wub Exponential** | Repeating exponential-decay plucks | Smooth |
| **Wub Ramp Down** | Repeating falling sawtooth sweeps | Smooth |
| **Wub Ramp Up** | Repeating rising sawtooth sweeps | Smooth |
| **Wub Sine** | Smooth rounded pulses | Smooth |
| **Wub Square** | Sharp high/low pulses | Smooth |

The five **Wub** banks progress through **1, 2, 3, 4, 6, and 8 cycles per waveform**, fading to silence between each subdivision and at the beginning and end of the bank's Morph range. Their Smooth Morph behavior produces continuous fade-ins and fade-outs.

Each WAV is mono, 16-bit, and contains 64 waveforms of 256 samples. Filenames begin with `3moji-` and end in `-256.wav`; banks intended for discrete selection include `-step`.

To install, download `3moji_Modulation_Wavetables_Vol_1.zip`, extract the **15 WAV files** into `/wavetables/` on the NT MicroSD card, and restart the module.

### Using the arpeggio banks

The arpeggio WAVs have intentionally low amplitude for pitch-CV use. For their intended range, start with:

- **Voltage:** −5 to +5 V
- **Amplitude:** 100%
- **Offset:** 0 V
- **Morph Mode:** Auto (Stepped)
- **Character:** Clean
- **Warp:** 50%
- **Squish:** 0%

**Gentle Arpeggios** spans approximately **0–1 V** (one octave at 1 V/oct); **Two-Octave Arpeggios** spans approximately **0–2 V** (two octaves). Feed the output into a quantizer or compatible pitch input. These banks generate pitch CV, not note gates; use a separate gate or trigger source for articulated notes.

**Do not normalize the arpeggio WAVs** in an audio editor if you want to preserve their intended voltage ranges.

## Building from source

On Windows, install the [Arm GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) and make sure `arm-none-eabi-g++` is on your PATH. From the repository directory:

```powershell
git clone https://github.com/expertsleepersltd/distingNT_API.git
.\build.ps1
```

The build produces `clocked_wavetable.o`. The Expert Sleepers API is an external dependency and is not bundled in this repository.

## License

Clocked Wavetable is released under the [MIT License](LICENSE). This is an independent community plugin, not an official Expert Sleepers product.
