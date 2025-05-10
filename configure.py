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
import glob
from pathlib import Path
from typing import Any, Dict, List

from tools.project import (
    Object,
    ProgressCategory,
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
    "--no-asm-processor",
    action="store_true",
    help="disable asm_processor for progress calculation",
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
    metavar="DIR",
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
    "--objdiff",
    metavar="BINARY | DIR",
    type=Path,
    help="path to objdiff-cli binary or source (optional)",
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


# Only configure versions for which an orig file exists
def version_exists(version: str) -> bool:
    return glob.glob(str(Path("orig") / version / "*")) != []


ALL_VERSIONS = [
    "oot-j",
    "oot-u",
    "oot-e",
]
config.versions = [
    version
    for version in ALL_VERSIONS
    if version_exists(version)
]

if not config.versions:
    sys.exit("Error: no orig files found for any version")

if "oot-j" in config.versions:
    config.default_version = "oot-j"
else:
    # Use the earliest version as default
    config.default_version = config.versions[0]

config.warn_missing_config = True
config.warn_missing_source = False
config.progress_all = True

config.build_dir = args.build_dir
config.dtk_path = args.dtk
config.objdiff_path = args.objdiff
config.binutils_path = args.binutils
config.compilers_path = args.compilers
config.generate_map = args.map
config.sjiswrap_path = args.sjiswrap
config.non_matching = args.non_matching
config.asm_processor = not args.no_asm_processor

if not is_windows():
    config.wrapper = args.wrapper

if args.no_asm:
    config.asm_dir = None

### Tool versions

config.binutils_tag = "2.42-1"
config.compilers_tag = "20240706"
config.dtk_tag = "v1.4.1"
config.objdiff_tag = "v2.7.1"
config.sjiswrap_tag = "v1.2.0"
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
    "-i src",
]

if config.non_matching:
    cflags_base.append("-DNON_MATCHING")

### Helper functions

def EmulatorLib(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_versions": {
            "oot-j": "GC/3.0a5",
            "oot-u": "GC/3.0a5",
            "oot-e": "GC/3.0a5",
        },
        "cflags": [*cflags_base, "-Cpp_exceptions off", "-O4,p", "-enc SJIS", "-ipa file"],
        "progress_category": "emulator",
        "objects": objects,
    }

def RevolutionLib(lib_name: str, objects: List[Object], cpp_exceptions: str = "off") -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/3.0a5",
        "cflags": [*cflags_base, f"-Cpp_exceptions {cpp_exceptions}", "-O4,p", "-ipa file", "-enc SJIS", "-fp_contract off"],
        "progress_category": "revolution",
        "objects": objects,
    }

def LibC(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/3.0a3",
        "cflags": [*cflags_base, "-Cpp_exceptions on", "-O4,p", "-rostr", "-use_lmw_stmw on", "-lang c"],
        "progress_category": "libc",
        "objects": objects,
    }

def RuntimeLib(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/3.0a3",
        "cflags": [*cflags_base, "-Cpp_exceptions off", "-O4,p", "-rostr", "-use_lmw_stmw on", "-enc SJIS"],
        "progress_category": "runtime",
        "objects": objects,
    }

def MetroTRKLib(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/2.7",
        "cflags": [*cflags_base, "-Cpp_exceptions off", "-O4,p", "-rostr", "-use_lmw_stmw on", "-lang c", "-inline on,deferred", "-func_align 4", "-sdata 0", "-sdata2 0"],
        "progress_category": "metrotrk",
        "objects": objects,
    }


### Link order

# Not matching for any version
NotLinked: List[str] = []

# Linked for all versions
Linked = config.versions

# Linked for specific versions
def LinkedFor(*versions):
    return versions

config.libs = [
    EmulatorLib(
        "emulator",
        [
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "emulator/vc64_RVL.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "emulator/system.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "emulator/ai.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "emulator/vi.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "emulator/si.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "emulator/pi.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "emulator/mi.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "emulator/disk.c"),
            Object(NotLinked, "emulator/cpu.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "emulator/pif.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "emulator/ram.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "emulator/rom.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "emulator/rdb.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "emulator/pak.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "emulator/sram.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "emulator/flash.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "emulator/_frameGCNcc.c"),
            Object(NotLinked, "emulator/_buildtev.c"),
            Object(NotLinked, "emulator/frame.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "emulator/library.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "emulator/codeRVL.c"),
            Object(NotLinked, "emulator/helpRVL.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "emulator/soundRVL.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "emulator/video.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "emulator/storeRVL.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "emulator/controller.c", mw_version="GC/3.0a5.2"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "emulator/errordisplay.c"),
            Object(NotLinked, "emulator/banner.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "emulator/stringtable.c"),
            Object(NotLinked, "emulator/rsp.c"),
            Object(NotLinked, "emulator/rdp.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "emulator/xlCoreRVL.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "emulator/xlPostRVL.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "emulator/xlFileRVL.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "emulator/xlText.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "emulator/xlList.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "emulator/xlHeap.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "emulator/xlFile.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "emulator/xlObject.c"),
        ]
    ),
    RevolutionLib(
        "NdevExi2AD",
        [
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/NdevExi2AD/DebuggerDriver.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/NdevExi2AD/exi2.c"),
        ]
    ),
    RevolutionLib(
        "vcmv",
        [
            Object(NotLinked, "revolution/vcmv/code_80083070.cpp"),
        ],
        cpp_exceptions="on"
    ),
    RevolutionLib(
        "base",
        [
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/base/PPCArch.c"),
        ]
    ),
    RevolutionLib(
        "os",
        [
            Object(NotLinked, "revolution/os/OS.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/os/OSAlarm.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/os/OSAlloc.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/os/OSArena.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/os/OSAudioSystem.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/os/OSCache.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/os/OSContext.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/os/OSError.c"),
            Object(NotLinked, "revolution/os/OSExec.c", cflags=[*cflags_base, "-Cpp_exceptions off", "-O4,p", "-ipa off"]),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/os/OSFatal.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/os/OSFont.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/os/OSInterrupt.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/os/OSLink.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/os/OSMessage.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/os/OSMemory.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/os/OSMutex.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/os/OSReboot.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/os/OSReset.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/os/OSRtc.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/os/OSSync.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/os/OSThread.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/os/OSTime.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/os/OSUtf.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/os/OSIpc.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/os/OSStateTM.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/os/time.dolphin.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/os/OSPlayRecord.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/os/OSStateFlags.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/os/__start.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/os/__ppc_eabi_init.c"),
        ]
    ),
    RevolutionLib(
        "exi",
        [
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/exi/EXIBios.c", cflags=[*cflags_base, "-Cpp_exceptions off", "-O3,p", "-ipa file"]),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/exi/EXIUart.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/exi/EXICommon.c"),
        ]
    ),
    RevolutionLib(
        "si",
        [
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/si/SIBios.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/si/SISamplingRate.c"),
        ]
    ),
    RevolutionLib(
        "db",
        [
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/db/db.c"),
        ]
    ),
    RevolutionLib(
        "vi",
        [
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/vi/vi.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/vi/i2c.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/vi/vi3in1.c"),
        ]
    ),
    RevolutionLib(
        "mtx",
        [
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/mtx/mtx.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/mtx/mtxvec.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/mtx/mtx44.c"),
        ]
    ),
    RevolutionLib(
        "gx",
        [
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/gx/GXInit.c"),
            Object(NotLinked, "revolution/gx/GXFifo.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/gx/GXAttr.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/gx/GXMisc.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/gx/GXGeometry.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/gx/GXFrameBuf.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/gx/GXLight.c"),
            Object(NotLinked, "revolution/gx/GXTexture.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/gx/GXBump.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/gx/GXTev.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/gx/GXPixel.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/gx/GXTransform.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/gx/GXPerf.c"),
        ]
    ),
    RevolutionLib(
        "dvd",
        [
            Object(NotLinked, "revolution/dvd/dvdfs.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/dvd/dvd.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/dvd/dvdqueue.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/dvd/dvderror.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/dvd/dvdidutils.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/dvd/dvdFatal.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/dvd/dvd_broadway.c"),
        ]
    ),
    RevolutionLib(
        "demo",
        [
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/demo/DEMOInit.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/demo/DEMOPuts.c", cflags=[*cflags_base, "-Cpp_exceptions off", "-O4,p"]),
        ]
    ),
    RevolutionLib(
        "ai",
        [
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/ai/ai.c"),
        ]
    ),
    RevolutionLib(
        "ax",
        [
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/ax/AX.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/ax/AXAlloc.c"),
            Object(NotLinked, "revolution/ax/AXAux.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/ax/AXCL.c"),
            Object(NotLinked, "revolution/ax/AXOut.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/ax/AXSPB.c"),
            Object(NotLinked, "revolution/ax/AXVPB.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/ax/AXComp.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/ax/DSPCode.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/ax/AXProf.c"),
        ]
    ),
    RevolutionLib(
        "axfx",
        [
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/axfx/AXFXReverbHi.c"),
            Object(NotLinked, "revolution/axfx/AXFXReverbHiExp.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/axfx/AXFXHooks.c"),
        ]
    ),
    RevolutionLib(
        "mem",
        [
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/mem/mem_heapCommon.c"),
            Object(NotLinked, "revolution/mem/mem_expHeap.c"),
            Object(NotLinked, "revolution/mem/mem_frameHeap.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/mem/mem_allocator.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/mem/mem_list.c"),
        ]
    ),
    RevolutionLib(
        "code_800B17A8",
        [
            Object(NotLinked, "revolution/code_800B17A8.c")
        ]
    ),
    RevolutionLib(
        "dsp",
        [
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/dsp/dsp.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/dsp/dsp_debug.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/dsp/dsp_task.c"),
        ]
    ),
    RevolutionLib(
        "nand",
        [
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/nand/nand.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/nand/NANDOpenClose.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/nand/NANDCore.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/nand/NANDCheck.c"),
        ]
    ),
    RevolutionLib(
        "sc",
        [
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/sc/scsystem.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/sc/scapi.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/sc/scapi_prdinfo.c"),
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
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/ipc/ipcMain.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/ipc/ipcclt.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/ipc/memory.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/ipc/ipcProfile.c"),
        ]
    ),
    RevolutionLib(
        "fs",
        [
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/fs/fs.c"),
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
    RevolutionLib(
        "kpad",
        [
            Object(NotLinked, "revolution/kpad/KPAD.c"),
        ]
    ),
    RevolutionLib(
        "usb",
        [
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/usb/usb.c"),
        ]
    ),
    RevolutionLib(
        "wud",
        [
            Object(NotLinked, "revolution/wud/WUD.c"),
            Object(NotLinked, "revolution/wud/WUDHidHost.c"),
            Object(NotLinked, "revolution/wud/debug_msg.c"),
        ]
    ),
    RevolutionLib(
        "code_800D1134",
        [
            Object(NotLinked, "revolution/code_800D1134.c"),
        ]
    ),
    RevolutionLib(
        "bte",
        [
            Object(NotLinked, "revolution/bte/code_800D1614.c"),
        ]
    ),
    RevolutionLib(
        "cnt",
        [
            Object(NotLinked, "revolution/cnt/cnt.c"),
        ]
    ),
    RevolutionLib(
        "esp",
        [
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/esp/esp.c"),
        ]
    ),
    RevolutionLib(
        "rso",
        [
            Object(NotLinked, "revolution/rso/RSOLink.c"),
        ]
    ),
    RevolutionLib(
        "tpl",
        [
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "revolution/tpl/TPL.c"),
        ]
    ),
    RevolutionLib(
        "hbm",
        [
            Object(NotLinked, "revolution/hbm/code_80109CB8.cpp"),
            Object(NotLinked, "revolution/hbm/nw4hbm/ut/ut_ResFont.cpp"),
            Object(NotLinked, "revolution/hbm/code_80144E2C.cpp"),
        ]
    ),
    RuntimeLib(
        "runtime",
        [
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "runtime/__mem.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "runtime/__va_arg.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "runtime/global_destructor_chain.c"),
            Object(NotLinked, "runtime/New.cpp", extra_cflags=["-Cpp_exceptions on"]),
            Object(NotLinked, "runtime/NMWException.cpp", extra_cflags=["-Cpp_exceptions on"]),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "runtime/ptmf.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "runtime/runtime.c"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "runtime/__init_cpp_exceptions.cpp"),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "runtime/Gecko_setjmp.c"),
            Object(NotLinked, "runtime/Gecko_ExceptionPPC.cpp", extra_cflags=["-Cpp_exceptions on"]),
            Object(LinkedFor("oot-j", "oot-u", "oot-e"), "runtime/GCN_mem_alloc.c"),
        ]
    ),
    LibC(
        "libc",
        [
            Object(NotLinked, "libc/alloc.c"),
            Object(NotLinked, "libc/ansi_files.c"),
            Object(NotLinked, "libc/ansi_fp.c"),
            Object(NotLinked, "libc/arith.c"),
            Object(NotLinked, "libc/bsearch.c"),
            Object(NotLinked, "libc/buffer_io.c"),
            Object(NotLinked, "libc/direct_io.c"),
            Object(NotLinked, "libc/file_io.c"),
            Object(NotLinked, "libc/file_pos.c"),
            Object(NotLinked, "libc/mbstring.c"),
            Object(NotLinked, "libc/mem.c"),
            Object(NotLinked, "libc/mem_funcs.c"),
            Object(NotLinked, "libc/misc_io.c"),
            Object(NotLinked, "libc/printf.c"),
            Object(NotLinked, "libc/qsort.c"),
            Object(NotLinked, "libc/rand.c"),
            Object(NotLinked, "libc/scanf.c"),
            Object(NotLinked, "libc/signal.c"),
            Object(NotLinked, "libc/string.c"),
            Object(NotLinked, "libc/strtold.c"),
            Object(NotLinked, "libc/strtoul.c"),
            Object(NotLinked, "libc/time.c"),
            Object(NotLinked, "libc/wstring.c"),
            Object(NotLinked, "libc/wchar_io.c"),
            Object(NotLinked, "libc/sysenv.c"),
            Object(NotLinked, "libc/uart_console_io.c"),
            Object(NotLinked, "libc/abort_exit_ppc_eabi.c"),
            Object(NotLinked, "libc/extras.c"),
            Object(NotLinked, "libc/e_acos.c"),
            Object(NotLinked, "libc/e_asin.c"),
            Object(NotLinked, "libc/e_atan2.c"),
            Object(NotLinked, "libc/e_exp.c"),
            Object(NotLinked, "libc/e_fmod.c"),
            Object(NotLinked, "libc/e_log.c"),
            Object(NotLinked, "libc/e_log10.c"),
            Object(NotLinked, "libc/e_pow.c"),
            Object(NotLinked, "libc/e_rem_pio2.c"),
            Object(NotLinked, "libc/k_cos.c"),
            Object(NotLinked, "libc/k_rem_pio2.c"),
            Object(NotLinked, "libc/k_sin.c"),
            Object(NotLinked, "libc/k_tan.c"),
            Object(NotLinked, "libc/s_atan.c"),
            Object(NotLinked, "libc/s_ceil.c"),
            Object(NotLinked, "libc/s_copysign.c"),
            Object(NotLinked, "libc/s_cos.c"),
            Object(NotLinked, "libc/s_floor.c"),
            Object(NotLinked, "libc/s_frexp.c"),
            Object(NotLinked, "libc/s_ldexp.c"),
            Object(NotLinked, "libc/s_sin.c"),
            Object(NotLinked, "libc/s_tan.c"),
            Object(NotLinked, "libc/w_acos.c"),
            Object(NotLinked, "libc/w_asin.c"),
            Object(NotLinked, "libc/w_atan2.c"),
            Object(NotLinked, "libc/w_exp.c"),
            Object(NotLinked, "libc/w_fmod.c"),
            Object(NotLinked, "libc/w_log.c"),
            Object(NotLinked, "libc/w_log10f.c"),
            Object(NotLinked, "libc/w_pow.c"),
            Object(NotLinked, "libc/e_sqrt.c"),
            Object(NotLinked, "libc/math_ppc.c"),
            Object(NotLinked, "libc/w_sqrt.c"),
        ]
    ),
    MetroTRKLib(
        "metrotrk",
        [
            Object(Linked, "metrotrk/mainloop.c"),
            Object(Linked, "metrotrk/nubevent.c"),
            Object(Linked, "metrotrk/nubinit.c"),
            Object(Linked, "metrotrk/msg.c"),
            Object(Linked, "metrotrk/msgbuf.c"),
            Object(Linked, "metrotrk/serpoll.c", extra_cflags=["-sdata 8"]),
            Object(Linked, "metrotrk/usr_put.c"),
            Object(Linked, "metrotrk/dispatch.c"),
            Object(Linked, "metrotrk/msghndlr.c"),
            Object(Linked, "metrotrk/support.c"),
            Object(Linked, "metrotrk/mutex_TRK.c"),
            Object(Linked, "metrotrk/notify.c"),
            Object(Linked, "metrotrk/flush_cache.c"),
            Object(Linked, "metrotrk/mem_TRK.c"),
            Object(Linked, "metrotrk/string_TRK.c"),
            Object(Linked, "metrotrk/targimpl.c"),
            Object(Linked, "metrotrk/targsupp.c", extra_cflags=["-func_align 16"]),
            Object(Linked, "metrotrk/mpc_7xx_603e.c"),
            Object(Linked, "metrotrk/mslsupp.c"),
            Object(Linked, "metrotrk/dolphin_trk.c"),
            Object(Linked, "metrotrk/main_TRK.c"),
            Object(Linked, "metrotrk/dolphin_trk_glue.c"),
            Object(Linked, "metrotrk/targcont.c"),
            Object(Linked, "metrotrk/target_options.c"),
            Object(Linked, "metrotrk/cc_udp.c"),
            Object(Linked, "metrotrk/cc_gdev.c", extra_cflags=["-sdata 8"]),
            Object(Linked, "metrotrk/CircleBuffer.c"),
            Object(Linked, "metrotrk/MWCriticalSection_gc.c"),
        ]
    )
]

# Optional extra categories for progress tracking
# Adjust as desired for your project
config.progress_categories = [
    ProgressCategory("emulator", "Emulator"),
    ProgressCategory("revolution", "Revolution SDK"),
    ProgressCategory("libc", "Libc"),
    ProgressCategory("runtime", "Runtime"),
    ProgressCategory("metrotrk", "MetroTRK"),
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
