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
SM64 = [
    "sm64-j",
    "sm64-u",
    "sm64-e",
]

OOT = [
    "oot-j",
    "oot-u",
    "oot-e",
]

ALL_VERSIONS = [
    *OOT,
    *SM64,
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
    "-i libcpp",
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

def RevolutionHBMLib(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/3.0a5",
        "cflags": [*cflags_base, "-Cpp_exceptions off", "-O4,p", "-ipa file", "-enc SJIS", "-fp_contract off", "-lang c++", "-DHBM_ASSERT", "-sym off"],
        "progress_category": "hbm",
        "objects": objects,
    }

def LibC(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/3.0a5",
        "cflags": [*cflags_base, "-Cpp_exceptions on", "-O4,p", "-ipa file", "-rostr", "-use_lmw_stmw on", "-lang c", "-fp_contract off"],
        "progress_category": "libc",
        "objects": objects,
    }

def MathLibC(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/3.0a5",
        "cflags": [*cflags_base, "-Cpp_exceptions off", "-O4,p", "-rostr", "-use_lmw_stmw on", "-lang c", "-fp_contract off"],
        "progress_category": "libc",
        "objects": objects,
    }

def RuntimeLib(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/3.0a5",
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
            Object(LinkedFor(*OOT), "emulator/vc64_RVL.c"),
            Object(LinkedFor(*OOT), "emulator/system.c"),
            Object(LinkedFor(*OOT), "emulator/ai.c"),
            Object(LinkedFor(*OOT), "emulator/vi.c"),
            Object(LinkedFor(*OOT), "emulator/si.c"),
            Object(LinkedFor(*OOT), "emulator/pi.c"),
            Object(LinkedFor(*OOT), "emulator/mi.c"),
            Object(LinkedFor(*OOT), "emulator/disk.c"),
            Object(NotLinked, "emulator/cpu.c"),
            Object(LinkedFor(*OOT), "emulator/pif.c"),
            Object(LinkedFor(*OOT), "emulator/ram.c"),
            Object(LinkedFor(*OOT), "emulator/rom.c"),
            Object(LinkedFor(*OOT), "emulator/rdb.c"),
            Object(LinkedFor(*OOT), "emulator/pak.c"),
            Object(LinkedFor(*OOT), "emulator/sram.c"),
            Object(LinkedFor(*OOT), "emulator/flash.c"),
            Object(LinkedFor(*OOT), "emulator/_frameGCNcc.c"),
            Object(NotLinked, "emulator/_buildtev.c"),
            Object(NotLinked, "emulator/frame.c"),
            Object(LinkedFor(*OOT), "emulator/library.c"),
            Object(LinkedFor(*OOT), "emulator/codeRVL.c"),
            Object(LinkedFor(*OOT), "emulator/helpRVL.c"),
            Object(LinkedFor(*OOT), "emulator/soundRVL.c"),
            Object(LinkedFor(*OOT), "emulator/video.c"),
            Object(LinkedFor(*OOT), "emulator/storeRVL.c"),
            Object(LinkedFor(*OOT), "emulator/controller.c", mw_version="GC/3.0a5.2"),
            Object(LinkedFor(*OOT), "emulator/errordisplay.c"),
            Object(LinkedFor("oot-j", "oot-u"), "emulator/banner.c"),
            Object(LinkedFor(*OOT), "emulator/stringtable.c"),
            Object(NotLinked, "emulator/rsp.c"),
            Object(NotLinked, "emulator/rdp.c"),
            Object(LinkedFor("sm64-u", *OOT), "emulator/xlCoreRVL.c"),
            Object(LinkedFor(*OOT), "emulator/xlPostRVL.c"),
            Object(LinkedFor(*OOT), "emulator/xlFileRVL.c"),
            Object(LinkedFor(*OOT), "emulator/xlText.c"),
            Object(LinkedFor(*OOT), "emulator/xlList.c"),
            Object(LinkedFor(*OOT), "emulator/xlHeap.c"),
            Object(LinkedFor(*OOT), "emulator/xlFile.c"),
            Object(LinkedFor(*OOT), "emulator/xlObject.c"),
        ]
    ),
    RevolutionLib(
        "NdevExi2AD",
        [
            Object(LinkedFor(*SM64, *OOT), "revolution/NdevExi2AD/DebuggerDriver.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/NdevExi2AD/exi2.c"),
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
            Object(LinkedFor(*SM64, *OOT), "revolution/base/PPCArch.c"),
        ]
    ),
    RevolutionLib(
        "os",
        [
            Object(LinkedFor(*SM64, *OOT), "revolution/os/OS.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/os/OSAlarm.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/os/OSAlloc.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/os/OSArena.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/os/OSAudioSystem.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/os/OSCache.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/os/OSContext.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/os/OSError.c"),
            Object(NotLinked, "revolution/os/OSExec.c", cflags=[*cflags_base, "-Cpp_exceptions off", "-O4,p", "-ipa off"]),
            Object(LinkedFor(*SM64, *OOT), "revolution/os/OSFatal.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/os/OSFont.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/os/OSInterrupt.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/os/OSLink.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/os/OSMessage.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/os/OSMemory.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/os/OSMutex.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/os/OSReboot.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/os/OSReset.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/os/OSRtc.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/os/OSSync.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/os/OSThread.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/os/OSTime.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/os/OSUtf.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/os/OSIpc.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/os/OSStateTM.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/os/time.dolphin.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/os/OSPlayRecord.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/os/OSStateFlags.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/os/__start.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/os/__ppc_eabi_init.c"),
        ]
    ),
    RevolutionLib(
        "exi",
        [
            Object(LinkedFor(*SM64, *OOT), "revolution/exi/EXIBios.c", cflags=[*cflags_base, "-Cpp_exceptions off", "-O3,p", "-ipa file"]),
            Object(LinkedFor(*SM64, *OOT), "revolution/exi/EXIUart.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/exi/EXICommon.c"),
        ]
    ),
    RevolutionLib(
        "si",
        [
            Object(LinkedFor(*SM64, *OOT), "revolution/si/SIBios.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/si/SISamplingRate.c"),
        ]
    ),
    RevolutionLib(
        "db",
        [
            Object(LinkedFor(*SM64, *OOT), "revolution/db/db.c"),
        ]
    ),
    RevolutionLib(
        "vi",
        [
            Object(LinkedFor(*SM64, *OOT), "revolution/vi/vi.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/vi/i2c.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/vi/vi3in1.c"),
        ]
    ),
    RevolutionLib(
        "mtx",
        [
            Object(LinkedFor(*SM64, *OOT), "revolution/mtx/mtx.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/mtx/mtxvec.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/mtx/mtx44.c"),
        ]
    ),
    RevolutionLib(
        "gx",
        [
            Object(LinkedFor(*SM64, *OOT), "revolution/gx/GXInit.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/gx/GXFifo.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/gx/GXAttr.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/gx/GXMisc.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/gx/GXGeometry.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/gx/GXFrameBuf.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/gx/GXLight.c"),
            Object(NotLinked, "revolution/gx/GXTexture.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/gx/GXBump.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/gx/GXTev.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/gx/GXPixel.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/gx/GXTransform.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/gx/GXPerf.c"),
        ]
    ),
    RevolutionLib(
        "dvd",
        [
            Object(LinkedFor(*SM64, *OOT), "revolution/dvd/dvdfs.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/dvd/dvd.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/dvd/dvdqueue.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/dvd/dvderror.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/dvd/dvdidutils.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/dvd/dvdFatal.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/dvd/dvd_broadway.c"),
        ]
    ),
    RevolutionLib(
        "demo",
        [
            Object(LinkedFor(*SM64, *OOT), "revolution/demo/DEMOInit.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/demo/DEMOPuts.c", cflags=[*cflags_base, "-Cpp_exceptions off", "-O4,p"]),
        ]
    ),
    RevolutionLib(
        "ai",
        [
            Object(LinkedFor(*SM64, *OOT), "revolution/ai/ai.c"),
        ]
    ),
    RevolutionLib(
        "ax",
        [
            Object(LinkedFor(*SM64, *OOT), "revolution/ax/AX.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/ax/AXAlloc.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/ax/AXAux.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/ax/AXCL.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/ax/AXOut.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/ax/AXSPB.c"),
            Object(NotLinked, "revolution/ax/AXVPB.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/ax/AXComp.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/ax/DSPCode.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/ax/AXProf.c"),
        ]
    ),
    RevolutionLib(
        "axfx",
        [
            Object(LinkedFor(*SM64, *OOT), "revolution/axfx/AXFXReverbHi.c"),
            Object(NotLinked, "revolution/axfx/AXFXReverbHiExp.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/axfx/AXFXHooks.c"),
        ]
    ),
    RevolutionLib(
        "mem",
        [
            Object(LinkedFor(*SM64, *OOT), "revolution/mem/mem_heapCommon.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/mem/mem_expHeap.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/mem/mem_frameHeap.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/mem/mem_unitHeap.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/mem/mem_allocator.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/mem/mem_list.c"),
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
            Object(LinkedFor(*SM64, *OOT), "revolution/dsp/dsp.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/dsp/dsp_debug.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/dsp/dsp_task.c"),
        ]
    ),
    RevolutionLib(
        "nand",
        [
            Object(LinkedFor(*SM64, *OOT), "revolution/nand/nand.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/nand/NANDOpenClose.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/nand/NANDCore.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/nand/NANDCheck.c"),
        ]
    ),
    RevolutionLib(
        "sc",
        [
            Object(LinkedFor(*SM64, *OOT), "revolution/sc/scsystem.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/sc/scapi.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/sc/scapi_prdinfo.c"),
        ]
    ),
    RevolutionLib(
        "wenc",
        [
            Object(LinkedFor(*SM64, *OOT), "revolution/wenc/wenc.c"),
        ]
    ),
    RevolutionLib(
        "arc",
        [
            Object(LinkedFor(*SM64, *OOT), "revolution/arc/arc.c"),
        ]
    ),
    RevolutionLib(
        "ipc",
        [
            Object(LinkedFor(*SM64, *OOT), "revolution/ipc/ipcMain.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/ipc/ipcclt.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/ipc/memory.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/ipc/ipcProfile.c"),
        ]
    ),
    RevolutionLib(
        "fs",
        [
            Object(LinkedFor(*SM64, *OOT), "revolution/fs/fs.c"),
        ]
    ),
    RevolutionLib(
        "pad",
        [
            Object(LinkedFor(*SM64, *OOT), "revolution/pad/Padclamp.c"),
            Object(LinkedFor(*SM64, *OOT), "revolution/pad/Pad.c"),
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
            Object(LinkedFor(*SM64, *OOT), "revolution/usb/usb.c"),
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
            Object(LinkedFor(*SM64, *OOT), "revolution/esp/esp.c"),
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
            Object(LinkedFor(*SM64, *OOT), "revolution/tpl/TPL.c"),
        ]
    ),
    RevolutionHBMLib(
        "hbm",
        [
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/homebutton/HBMBase.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/homebutton/HBMAnmController.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/homebutton/HBMFrameController.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/homebutton/HBMGUIManager.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/homebutton/HBMController.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/homebutton/HBMRemoteSpk.cpp"),
        ],
    ),
    RevolutionHBMLib(
        "nw4hbm/db",
        [
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/db/db_assert.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/db/db_console.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/db/db_directPrint.cpp"),
            Object(NotLinked, "revolution/hbm/nw4hbm/db/db_mapFile.cpp"),
        ],
    ),
    RevolutionHBMLib(
        "nw4hbm/lyt",
        [
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/lyt/lyt_animation.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/lyt/lyt_arcResourceAccessor.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/lyt/lyt_bounding.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/lyt/lyt_common.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/lyt/lyt_drawInfo.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/lyt/lyt_group.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/lyt/lyt_layout.cpp"),
            Object(NotLinked, "revolution/hbm/nw4hbm/lyt/lyt_material.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/lyt/lyt_pane.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/lyt/lyt_picture.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/lyt/lyt_resourceAccessor.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/lyt/lyt_textBox.cpp", mw_version="GC/3.0a5.2"),
            Object(NotLinked, "revolution/hbm/nw4hbm/lyt/lyt_window.cpp", mw_version="GC/3.0a5.2"),
        ],
    ),
    RevolutionHBMLib(
        "nw4hbm/math",
        [
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/math/math_triangular.cpp"),
        ],
    ),
    RevolutionHBMLib(
        "nw4hbm/snd",
        [
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_AxManager.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_AxVoice.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_Bank.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_BankFile.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_BasicSound.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_Channel.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_DisposeCallbackManager.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_DvdSoundArchive.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_EnvGenerator.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_ExternalSoundPlayer.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_FrameHeap.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_InstancePool.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_Lfo.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_MemorySoundArchive.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_MidiSeqPlayer.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_MidiSeqTrack.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_MmlParser.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_MmlSeqTrack.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_MmlSeqTrackAllocator.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_NandSoundArchive.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_RemoteSpeaker.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_RemoteSpeakerManager.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_SeqFile.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_SeqPlayer.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_SeqSound.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_SeqSoundHandle.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_SeqTrack.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_SoundArchive.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_SoundArchiveFile.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_SoundArchiveLoader.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_SoundArchivePlayer.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_SoundHandle.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_SoundHeap.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_SoundPlayer.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_SoundStartable.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_SoundSystem.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_SoundThread.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_StrmChannel.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_StrmFile.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_StrmPlayer.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_StrmSound.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_StrmSoundHandle.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_TaskManager.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_TaskThread.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_Util.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_WaveFile.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_WavePlayer.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_WaveSound.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_WaveSoundHandle.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_WsdFile.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_WsdPlayer.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/snd/snd_WsdTrack.cpp"),
        ],
    ),
    RevolutionHBMLib(
        "nw4hbm/ut",
        [
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/ut/ut_binaryFileFormat.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/ut/ut_CharStrmReader.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/ut/ut_CharWriter.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/ut/ut_DvdFileStream.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/ut/ut_FileStream.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/ut/ut_Font.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/ut/ut_IOStream.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/ut/ut_LinkList.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/ut/ut_list.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/ut/ut_NandFileStream.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/ut/ut_ResFont.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/ut/ut_ResFontBase.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/ut/ut_TagProcessorBase.cpp"),
            Object(LinkedFor(*SM64, *OOT), "revolution/hbm/nw4hbm/ut/ut_TextWriterBase.cpp"),
        ]
    ),
    RuntimeLib(
        "runtime",
        [
            Object(LinkedFor(*SM64, *OOT), "runtime/__mem.c"),
            Object(LinkedFor(*SM64, *OOT), "runtime/__va_arg.c"),
            Object(LinkedFor(*SM64, *OOT), "runtime/global_destructor_chain.c"),
            Object(LinkedFor(*SM64, *OOT), "runtime/New.cpp", extra_cflags=["-lang c++", "-Cpp_exceptions on", "-RTTI on"]),
            Object(LinkedFor(*SM64, *OOT), "runtime/NMWException.cpp",  extra_cflags=["-lang c++", "-Cpp_exceptions on", "-RTTI on"]),
            Object(LinkedFor(*SM64, *OOT), "runtime/ptmf.c"),
            Object(LinkedFor(*SM64, *OOT), "runtime/runtime.c"),
            Object(LinkedFor(*SM64, *SM64, *OOT), "runtime/__init_cpp_exceptions.cpp"),
            Object(LinkedFor(*SM64, *OOT), "runtime/Gecko_setjmp.c"),
            Object(NotLinked, "runtime/Gecko_ExceptionPPC.cpp",  extra_cflags=["-lang c++", "-Cpp_exceptions on", "-RTTI on"]),
            Object(LinkedFor(*SM64, *OOT), "runtime/GCN_mem_alloc.c"),
        ]
    ),
    LibC(
        "libc",
        [
            Object(LinkedFor(*SM64, *OOT), "libc/alloc.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/ansi_files.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/ansi_fp.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/arith.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/bsearch.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/buffer_io.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/direct_io.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/ctype.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/errno.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/file_io.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/file_pos.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/float.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/locale.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/mbstring.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/mem.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/mem_funcs.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/math_api.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/misc_io.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/printf.c", mw_version="GC/3.0a5.2"),
            Object(LinkedFor(*SM64, *OOT), "libc/qsort.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/rand.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/scanf.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/signal.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/string.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/strtold.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/strtoul.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/time.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/wstring.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/wchar_io.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/wctype.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/sysenv.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/uart_console_io.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/abort_exit_ppc_eabi.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/extras.c"),
        ]
    ),
    MathLibC(
        "libm",
        [
            Object(LinkedFor(*SM64, *OOT), "libc/math/e_acos.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/math/e_asin.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/math/e_atan2.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/math/e_exp.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/math/e_fmod.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/math/e_log.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/math/e_log10.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/math/e_pow.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/math/e_rem_pio2.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/math/k_cos.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/math/k_rem_pio2.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/math/k_sin.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/math/k_tan.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/math/s_atan.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/math/s_ceil.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/math/s_copysign.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/math/s_cos.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/math/s_floor.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/math/s_frexp.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/math/s_ldexp.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/math/s_sin.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/math/s_tan.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/math/w_acos.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/math/w_asin.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/math/w_atan2.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/math/w_exp.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/math/w_fmod.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/math/w_log.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/math/w_log10f.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/math/w_pow.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/math/e_sqrt.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/math/math_ppc.c"),
            Object(LinkedFor(*SM64, *OOT), "libc/math/w_sqrt.c"),
        ]
    ),
    MetroTRKLib(
        "metrotrk",
        [
            Object(LinkedFor(*SM64, *OOT), "metrotrk/mainloop.c"),
            Object(LinkedFor(*SM64, *OOT), "metrotrk/nubevent.c"),
            Object(LinkedFor(*SM64, *OOT), "metrotrk/nubinit.c"),
            Object(LinkedFor(*SM64, *OOT), "metrotrk/msg.c"),
            Object(LinkedFor(*SM64, *OOT), "metrotrk/msgbuf.c"),
            Object(LinkedFor(*SM64, *OOT), "metrotrk/serpoll.c", extra_cflags=["-sdata 8"]),
            Object(LinkedFor(*SM64, *OOT), "metrotrk/usr_put.c"),
            Object(LinkedFor(*SM64, *OOT), "metrotrk/dispatch.c"),
            Object(LinkedFor(*SM64, *OOT), "metrotrk/msghndlr.c"),
            Object(LinkedFor(*SM64, *OOT), "metrotrk/support.c"),
            Object(LinkedFor(*SM64, *OOT), "metrotrk/mutex_TRK.c"),
            Object(LinkedFor(*SM64, *OOT), "metrotrk/notify.c"),
            Object(LinkedFor(*SM64, *OOT), "metrotrk/flush_cache.c"),
            Object(LinkedFor(*SM64, *OOT), "metrotrk/mem_TRK.c"),
            Object(LinkedFor(*SM64, *OOT), "metrotrk/string_TRK.c"),
            Object(LinkedFor(*SM64, *OOT), "metrotrk/__exception.s"),
            Object(LinkedFor(*SM64, *OOT), "metrotrk/targimpl.c"),
            Object(LinkedFor(*SM64, *OOT), "metrotrk/targsupp.c", extra_cflags=["-func_align 16"]),
            Object(LinkedFor(*SM64, *OOT), "metrotrk/mpc_7xx_603e.c"),
            Object(LinkedFor(*SM64, *OOT), "metrotrk/mslsupp.c"),
            Object(LinkedFor(*SM64, *OOT), "metrotrk/dolphin_trk.c"),
            Object(LinkedFor(*SM64, *OOT), "metrotrk/main_TRK.c"),
            Object(LinkedFor(*SM64, *OOT), "metrotrk/dolphin_trk_glue.c"),
            Object(LinkedFor(*SM64, *OOT), "metrotrk/targcont.c"),
            Object(LinkedFor(*SM64, *OOT), "metrotrk/target_options.c"),
            Object(LinkedFor(*SM64, *OOT), "metrotrk/cc_udp.c"),
            Object(LinkedFor(*SM64, *OOT), "metrotrk/cc_gdev.c", extra_cflags=["-sdata 8"]),
            Object(LinkedFor(*SM64, *OOT), "metrotrk/CircleBuffer.c"),
            Object(LinkedFor(*SM64, *OOT), "metrotrk/MWCriticalSection_gc.c"),
        ]
    )
]

# Optional extra categories for progress tracking
# Adjust as desired for your project
config.progress_categories = [
    ProgressCategory("emulator", "Emulator"),
    ProgressCategory("revolution", "Revolution SDK"),
    ProgressCategory("hbm", "Home Button Menu"),
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
