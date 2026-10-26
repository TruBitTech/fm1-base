# Pin Head

Synthi AKS-inspired pin-matrix firmware for the M-VAVE FM-1, built on [fm1-base](https://github.com/charlesvestal/fm1-base).

## Status
- Matrix UI: Core 10×10 (wrapping cursor) and Full 16×16 (hard stops)
- Sticky row/column headers (A–P / 1–16)
- Pin cycling: none → white → red → green
- Modes: Matrix, Play (joystick knobs), GLO (4 pages)
- DSP headers and matrix/joystick modules present; full engine integration next

## Build
From the fm1-base root:
```
tools/build.py pinhead
tools/build.py pinhead --host
tools/build.py pinhead --web
```
