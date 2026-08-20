# Glide for the VR4440 Voodoo Rush

This fork adds working dual-TMU support to the Glide 2.x SST-96 driver for the
VR4440 Voodoo Rush card. It targets the Alliance AT3D/FBIjr design with two
normally strapped TREX chips and 4 MB of texture memory per TMU.

The driver:

- reports two TMUs and 4 MB per TMU by default;
- addresses each TREX independently through the SST-96 chip-select mechanism;
- routes texture uploads through the shared Rush texture aperture;
- configures downstream TMU0 to accept and combine the TMU1 texel stream;
- retains explicit one-TMU and reduced-memory compatibility modes; and
- builds a Win98-compatible `GLIDE2X.DLL` with the MinGW cross-toolchain.

Glide3x not currently supported (works with single TMU).

## Building

From WSL with `make`, `nasm`, and the `i686-w64-mingw32` toolchain installed:

```sh
make -C glide2x -f Makefile.mingw FX_GLIDE_HW=sst96 clean
make -C glide2x -f Makefile.mingw FX_GLIDE_HW=sst96 CROSS=i686-w64-mingw32-
```

The DLL is written to:

```text
glide2x/sst1/lib/sst96/glide2x.dll
```

Place `GLIDE2X.DLL` beside a Glide 2.x application or just copy it to the
regular Voodoo Rush installation package (overwrite the existing
`GLIDE2X.DLL`). No environment variables are required for the VR4440's normal
two-TMU, 4 MB-per-TMU configuration.

## Configuration

Set variables in the batch file that launches the application.

| Variable | Values | Default | Purpose |
| --- | --- | --- | --- |
| `SST96_NUM_TMUS` | `1`, `2` | `2` | Selects one- or two-TMU operation. |
| `SST96_TMUMEM_SIZE` | `1`, `2`, `4` | `4` | Texture memory in MB for each TMU. |

Example stock-compatible fallback:

```bat
@echo off
set SST96_NUM_TMUS=1
set SST96_TMUMEM_SIZE=2
game.exe
```

## Diagnostics And Tuning

These variables are intended for hardware development. Leave them unset for
normal operation. Integer values accept decimal or `0x` hexadecimal notation.

| Variable | Purpose |
| --- | --- |
| `SST96_TRACE_INIT=1` | Writes initialization tracing to `rush_init96.log`. |
| `SST96_TRACE_FILE=path` | Changes the initialization trace filename. |
| `SST96_TRACE_TMU_STATE=1` | Writes TMU state tracing to `rush_tmu_state.log`. |
| `SST96_GRXCLK=25..100` | Sets the Rush clock in integer MHz steps. |
| `SST96_CLOCK_TRACE=1` | Logs requested/actual clock, PLL values, readback, and restoration. |
| `SST96_CLOCK_TRACE_FILE=path` | Changes the clock trace filename from `rush_clock.log`. |
| `SST96_FT_CLK_DEL=n` | Overrides the 4-bit FJR-to-TMU clock delay; default 6. |
| `SST96_TF_CLK_DEL=n` | Overrides the 4-bit TF delay for both TMUs; default 6. |
| `SST96_TF0_CLK_DEL=n` | Overrides the TF delay for TMU0. |
| `SST96_TF1_CLK_DEL=n` | Overrides the TF delay for TMU1. |
| `SST96_TT_CLK_DEL=n` | Overrides both TMUs' 4-bit `TT_FIFO_SIL` field; default 8. |
| `SST96_TT0_CLK_DEL=n` | Overrides TMU0's `TT_FIFO_SIL` field. |
| `SST96_TT1_CLK_DEL=n` | Overrides TMU1's `TT_FIFO_SIL` field. |
| `SST_TREX0INIT0`, `SST_TREX1INIT0` | Override each TMU's `trexInit0`; normal value is `0x05441`. |
| `SST_TREX0INIT1` | Overrides TMU0 `trexInit1`; normal value is `0x0643c`. |
| `SST_TREX1INIT1` | Overrides TMU1 `trexInit1`; normal value is `0x3643c`. |
| `SST96_TMU1_CHIP_FIELD` | Overrides the TMU1 chip field with `0x4` or diagnostic `0x8`. |
| `SST96_PROBE_TMU_MEMORY=1` | Enables the old destructive memory detector. Do not use normally. |

Clock requests matching the original Alliance table retain its PLL values;
missing integer frequencies are synthesized. With `SST96_GRXCLK` unset, Glide
does not change the active clock. An overridden clock is read back after
programming and the original PLL values are restored when Glide closes.
`SST_GRXCLK` and `SST96_MEMCLOCK` are lower-priority clock aliases.

The equivalent Voodoo1-style `SST_FT_CLK_DEL`, `SST_TF_CLK_DEL`,
`SST_TF0_CLK_DEL`, `SST_TF1_CLK_DEL`, and `SST_TT*` names are accepted as
lower-priority timing aliases. `TT_CLK_DEL` is a convenient tuning name for the
TREX `TT_FIFO_SIL` field; its exact phase behavior is not documented.

The source also retains `SST96_RUNTIME_TMU1_CHIP_FIELD`,
`SST96_DIRECT_TMU1_PARAMS`, `SST96_DIRECT_TMU0_TEXTUREMODE`,
`SST96_TMU1_PACKET_TMU_ADDR`, and `SST96_TMU1_LEGACY_TEXDL` for regression
experiments. They are not supported runtime settings and should remain unset.

## License

The original Glide licenses and repository license remain applicable. See
`LICENSE` and `glide2x/glide_license.txt`.
