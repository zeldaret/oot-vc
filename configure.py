#!/usr/bin/env python3

###
# Generates build files for the project.
# This file also includes the project configuration,
# such as compiler flags and the object NotLinked status.
#
# Usage:
#   python3 configure.py
#   ninja
#
# Append --help to see available options.
###

import argparse
import sys
from pathlib import Path
from typing import Any, Dict, List

from tools.project import (
    Object,
    ProjectConfig,
    calculate_progress,
    generate_build,
    is_windows,
)

### Script's arguments

parser = argparse.ArgumentParser()
parser.add_argument(
    "mode",
    choices=["configure", "progress"],
    default="configure",
    help="script mode (default: configure)",
    nargs="?",
)
parser.add_argument(
    "--non-matching",
    action="store_true",
    help="create non-matching build for modding",
)
parser.add_argument(
    "--build-dir",
    metavar="DIR",
    type=Path,
    default=Path("build"),
    help="base build directory (default: build)",
)
parser.add_argument(
    "--binutils",
    metavar="BINARY",
    type=Path,
    help="path to binutils (optional)",
)
parser.add_argument(
    "--compilers",
    metavar="DIR",
    type=Path,
    help="path to compilers (optional)",
)
parser.add_argument(
    "--map",
    action="store_true",
    help="generate map file(s)",
    default=True,
)
parser.add_argument(
    "--no-asm",
    action="store_true",
    help="don't incorporate .s files from asm directory",
)
if not is_windows():
    parser.add_argument(
        "--wrapper",
        metavar="BINARY",
        type=Path,
        help="path to wibo or wine (optional)",
    )
parser.add_argument(
    "--dtk",
    metavar="BINARY | DIR",
    type=Path,
    help="path to decomp-toolkit binary or source (optional)",
)
parser.add_argument(
    "--sjiswrap",
    metavar="EXE",
    type=Path,
    help="path to sjiswrap.exe (optional)",
)
parser.add_argument(
    "--progress-version",
    metavar="VERSION",
    help="version to print progress for",
)

args = parser.parse_args()

### Project configuration

config = ProjectConfig()

# Only configure versions for which content1.app exists
ALL_VERSIONS = [
    "oot-j",
]
config.versions = [
    version
    for version in ALL_VERSIONS
    if (Path("orig") / version / "content1.app").exists()
]
if "oot-j" in config.versions:
    config.default_version = "oot-j"

config.warn_missing_config = True
config.warn_missing_source = False
config.progress_all = False

config.build_dir = args.build_dir
config.dtk_path = args.dtk
config.binutils_path = args.binutils
config.compilers_path = args.compilers
config.generate_map = args.map
config.sjiswrap_path = args.sjiswrap
config.non_matching = args.non_matching

if not is_windows():
    config.wrapper = args.wrapper

if args.no_asm:
    config.asm_dir = None

### Tool versions

config.binutils_tag = "2.42-1"
config.compilers_tag = "20231018"
config.dtk_tag = "v0.9.2"
config.sjiswrap_tag = "v1.1.1"
config.wibo_tag = "0.6.11"
config.linker_version = "GC/3.0a5"

### Flags

config.asflags = [
    "-mgekko",
    "-I include",
    "-I libc",
]

config.ldflags = [
    "-fp hardware",
    "-nodefaults",
    "-warn off",
]

cflags_base = [
    "-Cpp_exceptions off",
    "-proc gekko",
    "-fp hardware",
    "-fp_contract on",
    "-enum int",
    "-align powerpc",
    "-nosyspath",
    "-RTTI off",
    "-str reuse",
    "-inline auto",
    "-nodefaults",
    "-msgstyle gcc",
    "-sym on",
    "-i include",
    "-i libc",
]

if config.non_matching:
    cflags_base.append("-DNON_MATCHING")

### Helper functions

def EmulatorLib(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/3.0a5",
        "cflags": [*cflags_base, "-O4,p", "-enc SJIS"],
        "host": False,
        "objects": objects,
    }

def RevolutionLib(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/3.0a5",
        "cflags": [*cflags_base, "-O4,p", "-ipa file", "-enc SJIS"],
        "host": False,
        "objects": objects,
    }

def RuntimeLib(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/3.0a3",
        "cflags": [*cflags_base, "-O4,p", "-rostr", "-use_lmw_stmw on", "-lang c", "-enc SJIS"],
        "host": False,
        "objects": objects,
    }

def MetroTRKLib(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/2.7",
        "cflags": [*cflags_base, "-O4,p", "-rostr", "-use_lmw_stmw on", "-lang c"],
        "host": False,
        "objects": objects,
    }

### Link order

# Not matching for any version
NotLinked = {}

# Matching for all versions
Linked = config.versions

# Matching for specific versions
def LinkedFor(*versions):
    return versions

config.libs = [
    EmulatorLib(
        "emulator",
        [
            Object(NotLinked, "emulator/vc64_RVL.c"),
            Object(NotLinked, "emulator/system.c"),
            Object(NotLinked, "emulator/ai.c"),
            Object(NotLinked, "emulator/vi.c"),
            Object(NotLinked, "emulator/si.c"),
            Object(NotLinked, "emulator/pi.c"),
            Object(NotLinked, "emulator/mi.c"),
            Object(NotLinked, "emulator/disk.c"),
            Object(NotLinked, "emulator/cpu.c", asm_processor=True),
            Object(NotLinked, "emulator/pif.c"),
            Object(NotLinked, "emulator/ram.c"),
            Object(NotLinked, "emulator/rom.c"),
            Object(NotLinked, "emulator/rdb.c"),
            Object(NotLinked, "emulator/eeprom.c"),
            Object(NotLinked, "emulator/sram.c"),
            Object(NotLinked, "emulator/flash.c"),
            Object(NotLinked, "emulator/_frameGCNcc.c"),
            Object(NotLinked, "emulator/_buildtev.c"),
            Object(NotLinked, "emulator/frame.c"),
            Object(NotLinked, "emulator/library.c"),
            Object(LinkedFor("oot-j"), "emulator/codeRVL.c"),
            Object(NotLinked, "emulator/helpRVL.c"),
            Object(LinkedFor("oot-j"), "emulator/soundRVL.c"),
            Object(LinkedFor("oot-j"), "emulator/video.c"),
            Object(NotLinked, "emulator/store.c"),
            Object(NotLinked, "emulator/controller.c"),
            Object(NotLinked, "emulator/rsp.c"),
            Object(NotLinked, "emulator/rdp.c"),
            Object(NotLinked, "emulator/xlCoreRVL.c"),
            Object(NotLinked, "emulator/xlPostRVL.c"),
            Object(NotLinked, "emulator/xlFileRVL.c"),
            Object(NotLinked, "emulator/xlText.c"),
            Object(NotLinked, "emulator/xlList.c", extra_cflags=["-ipa file"]),
            Object(NotLinked, "emulator/xlHeap.c"),
            Object(NotLinked, "emulator/xlFile.c"),
            Object(NotLinked, "emulator/xlObject.c"),
        ]
    ),
    RevolutionLib(
        "base",
        [
            Object(NotLinked, "revolution/base/PPCArch.c"),
        ]
    ),
    RevolutionLib(
        "os",
        [
            Object(NotLinked, "revolution/os/OS.c"),
            Object(NotLinked, "revolution/os/OSAlarm.c"),
            Object(NotLinked, "revolution/os/OSAlloc.c"),
            Object(NotLinked, "revolution/os/OSArena.c"),
            Object(NotLinked, "revolution/os/OSAudioSystem.c"),
            Object(NotLinked, "revolution/os/OSCache.c"),
            Object(NotLinked, "revolution/os/OSContext.c"),
            Object(NotLinked, "revolution/os/OSError.c"),
            Object(NotLinked, "revolution/os/OSExec.c", cflags=[*cflags_base, "-O4,p", "-ipa off"]),
            Object(NotLinked, "revolution/os/OSFatal.c"),
            Object(NotLinked, "revolution/os/OSFont.c"),
            Object(NotLinked, "revolution/os/OSInterrupt.c"),
            Object(NotLinked, "revolution/os/OSLink.c"),
            Object(NotLinked, "revolution/os/OSMessage.c"),
            Object(NotLinked, "revolution/os/OSMemory.c"),
            Object(NotLinked, "revolution/os/OSMutex.c"),
            Object(NotLinked, "revolution/os/OSReboot.c"),
            Object(NotLinked, "revolution/os/OSReset.c"),
            Object(NotLinked, "revolution/os/OSRtc.c"),
            Object(NotLinked, "revolution/os/OSSync.c"),
            Object(NotLinked, "revolution/os/OSThread.c"),
            Object(NotLinked, "revolution/os/OSTime.c"),
            Object(NotLinked, "revolution/os/OSUtf.c"),
            Object(NotLinked, "revolution/os/OSIpc.c"),
            Object(NotLinked, "revolution/os/OSStateTM.c"),
            Object(NotLinked, "revolution/os/time.dolphin.c"),
            Object(NotLinked, "revolution/os/OSPlayRecord.c"),
            Object(NotLinked, "revolution/os/OSStateFlags.c"),
            Object(NotLinked, "revolution/os/__start.c"),
            Object(NotLinked, "revolution/os/__ppc_eabi_init.c"),
        ]
    ),
    RevolutionLib(
        "exi",
        [
            Object(NotLinked, "revolution/exi/EXIBios.c", cflags=[*cflags_base, "-O3,p", "-ipa file"]),
            Object(NotLinked, "revolution/exi/EXIUart.c"),
            Object(NotLinked, "revolution/exi/EXICommon.c"),
        ]
    ),
    RevolutionLib(
        "si",
        [
            Object(NotLinked, "revolution/si/SIBios.c"),
            Object(NotLinked, "revolution/si/SISamplingRate.c"),
        ]
    ),
    RevolutionLib(
        "db",
        [
            Object(NotLinked, "revolution/db/db.c"),
        ]
    ),
    RevolutionLib(
        "vi",
        [
            Object(NotLinked, "revolution/vi/vi.c"),
            Object(NotLinked, "revolution/vi/i2c.c"),
            Object(NotLinked, "revolution/vi/vi3in1.c"),
        ]
    ),
    RevolutionLib(
        "mtx",
        [
            Object(NotLinked, "revolution/mtx/mtx.c"),
            Object(NotLinked, "revolution/mtx/mtxvec.c"),
            Object(NotLinked, "revolution/mtx/mtx44.c"),
        ]
    ),
    RevolutionLib(
        "gx",
        [
            Object(NotLinked, "revolution/gx/GXInit.c"),
            Object(NotLinked, "revolution/gx/GXFifo.c"),
            Object(NotLinked, "revolution/gx/GXAttr.c"),
            Object(NotLinked, "revolution/gx/GXMisc.c"),
            Object(NotLinked, "revolution/gx/GXGeometry.c"),
            Object(NotLinked, "revolution/gx/GXFrameBuf.c"),
            Object(NotLinked, "revolution/gx/GXLight.c"),
            Object(NotLinked, "revolution/gx/GXTexture.c"),
            Object(NotLinked, "revolution/gx/GXBump.c"),
            Object(NotLinked, "revolution/gx/GXTev.c"),
            Object(NotLinked, "revolution/gx/GXPixel.c"),
            Object(NotLinked, "revolution/gx/GXTransform.c"),
            Object(NotLinked, "revolution/gx/GXPerf.c"),
        ]
    ),
    RevolutionLib(
        "dvd",
        [    
            Object(NotLinked, "revolution/dvd/dvdfs.c"),
            Object(NotLinked, "revolution/dvd/dvd.c"),
            Object(NotLinked, "revolution/dvd/dvdqueue.c"),
            Object(NotLinked, "revolution/dvd/dvderror.c"),
            Object(NotLinked, "revolution/dvd/dvdidutils.c"),
            Object(NotLinked, "revolution/dvd/dvdFatal.c"),
            Object(NotLinked, "revolution/dvd/dvd_broadway.c"),
        ]
    ),
    RevolutionLib(
        "demo",
        [
            Object(NotLinked, "revolution/demo/DEMOPuts.c"),
        ]
    ),
    RevolutionLib(
        "ai",
        [
            Object(NotLinked, "revolution/ai/ai.c"),
        ]
    ),
    RevolutionLib(
        "ax",
        [
            Object(NotLinked, "revolution/ax/AX.c"),
            Object(NotLinked, "revolution/ax/AXAlloc.c"),
            Object(NotLinked, "revolution/ax/AXAux.c"),
            Object(NotLinked, "revolution/ax/AXCL.c"),
            Object(NotLinked, "revolution/ax/AXOut.c"),
            Object(NotLinked, "revolution/ax/AXSPB.c"),
            Object(NotLinked, "revolution/ax/AXVPB.c"),
            Object(NotLinked, "revolution/ax/AXComp.c"),
            Object(NotLinked, "revolution/ax/DSPCode.c"),
            Object(NotLinked, "revolution/ax/AXProf.c"),
        ]
    ),
    RevolutionLib(
        "axfx",
        [
            Object(NotLinked, "revolution/axfx/code_800AF5D8.c"),
        ]
    ),
    RevolutionLib(
        "dsp",
        [
            Object(NotLinked, "revolution/dsp/dsp.c"),
            Object(NotLinked, "revolution/dsp/dsp_debug.c"),
            Object(NotLinked, "revolution/dsp/dsp_task.c"),
        ]
    ),
    RevolutionLib(
        "nand",
        [
            Object(NotLinked, "revolution/nand/nand.c"),
            Object(NotLinked, "revolution/nand/NANDOpenClose.c"),
            Object(NotLinked, "revolution/nand/NANDCore.c"),
            Object(NotLinked, "revolution/nand/NANDCheck.c"),
        ]
    ),
    RevolutionLib(
        "sc",
        [
            Object(NotLinked, "revolution/sc/scsystem.c"),
            Object(NotLinked, "revolution/sc/scapi.c"),
            Object(NotLinked, "revolution/sc/scapi_prdinfo.c"),
        ]
    ),
    RevolutionLib(
        "arc",
        [
            Object(NotLinked, "revolution/arc/arc.c"),
        ]
    ),
    RevolutionLib(
        "ipc",
        [
            Object(NotLinked, "revolution/ipc/ipcMain.c"),
            Object(NotLinked, "revolution/ipc/ipcclt.c"),
            Object(NotLinked, "revolution/ipc/memory.c"),
            Object(NotLinked, "revolution/ipc/ipcProfile.c"),
        ]
    ),
    RevolutionLib(
        "fs",
        [
            Object(NotLinked, "revolution/fs/fs.c"),
        ]
    ),
    RevolutionLib(
        "pad",
        [
            Object(NotLinked, "revolution/pad/Padclamp.c"),
            Object(NotLinked, "revolution/pad/Pad.c"),
        ]
    ),
    RevolutionLib(
        "wpad",
        [
            Object(NotLinked, "revolution/wpad/WPAD.c"),
            Object(NotLinked, "revolution/wpad/WPADHIDParser.c"),
            Object(NotLinked, "revolution/wpad/WPADEncrypt.c"),
            Object(NotLinked, "revolution/wpad/debug_msg.c"),
        ]
    ),
    RuntimeLib(
        "runtime",
        [
            Object(NotLinked, "runtime/__mem.c"),
            Object(NotLinked, "runtime/__va_arg.c"),
            Object(NotLinked, "runtime/global_destructor_chain.c"),
            Object(NotLinked, "runtime/code_8015263C.c"),
            Object(NotLinked, "runtime/ptmf.c"),
            Object(NotLinked, "runtime/runtime.c"),
            Object(NotLinked, "runtime/__init_cpp_exceptions.cpp"), # TODO: matched but does not build OK
            Object(NotLinked, "runtime/Gecko_setjmp.c"),
            Object(NotLinked, "runtime/Gecko_ExceptionPPC.c"),
        ]
    ),
    MetroTRKLib(
        "metrotrk",
        [
            Object(NotLinked, "metrotrk/mem_TRK.c"),
            Object(NotLinked, "metrotrk/dolphin_trk.c"),
        ]
    )
]

### Execute mode

if args.mode == "configure":
    # Write build.ninja and objdiff.json
    generate_build(config)
elif args.mode == "progress":
    # Print progress and write progress.json
    calculate_progress(config, args.progress_version)
else:
    sys.exit("Unknown mode: " + args.mode)
