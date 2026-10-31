# Pin Head

Synthi AKS-inspired pin-matrix firmware for the M-VAVE FM-1, built on [fm1-base](https://github.com/charlesvestal/fm1-base).

## Status
Complete application skeleton ready to build:
- Matrix UI: Core 10×10 (wrapping) and Full 16×16 (hard stops)
- Sticky A–P / 1–16 headers, pin cycling (none/white/red/green)
- DSP engine: 3 oscillators, noise, ring mod, SVF filter, trapezoid envelope, spring, lo-fi tape
- Matrix evaluation with white/red/green pin gains
- Play mode: keys trigger notes, K1/K2 = joystick X/Y (±1.5 V style)
- GLO mode: 4 pages (Lush, Ping-Pong, Tape, Joystick) with live knob control
- Factory presets: On the Run, Any Colour Lead, Irregular Clicks, Noise Sweep, Spring Drone, Metallic Perc, Empty
- Octave buttons, PRESET button cycles patches

## Build
From the fm1-base root (requires JieLi toolchain):
```
tools/build.py pinhead
tools/build.py pinhead --host
tools/build.py pinhead --web
```

Install the resulting `pinhead/build/pinhead.fwsc` via the web installer or `tools/fm1_install.py`.
