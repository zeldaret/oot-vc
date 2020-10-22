
# ootvc

This repo contains a disassembly of the Japanese version of the Wii Virtual Console N64 Emulator intended for use with the Wii VC release of The Legend of Zelda Ocarina of Time.

It builds the following DOL:

00000001.app   : `sha1: 47546e48467ae14d712b8c207e911821586d1043`

## Building

### Required tools

* [devkitPro](https://devkitpro.org/wiki/Getting_Started)
* python3

### Instructions

1. Obtain the original 00000001.app executable packaged in the WAD and place it in the base working directory.
2. Obtain a copy of the MWCC for embedded PowerPC and place it in the `tools/mwcc_compiler/3.0/` folder.
(NOTE: This compiler's executables [mwcceppc.exe mwasmeppc.exe and mwldeppc.exe] can be installed with Codewarrior 3.0 for Gamecube, but no license or crack is provided with this project. Please obtain access to the compiler on your own.)
3. Run make setup and make

