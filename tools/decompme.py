#!/usr/bin/env python3

# Script to upload a decomp.me. Usage example:
#
#   tools/decompme.py src/rom.c asm/non_matchings/rom/romGetPC.s
#
# Currently you must manually delete the function from the context and copy into
# the source if the function is already defined.

from __future__ import annotations

import argparse
import json
from pathlib import Path
import re
import urllib.parse
import urllib.request

INCLUDE_DIRS = [
    Path("."),
    Path("include"),
    Path("libc"),
    Path("build/oot-j/include"),
]
COMPILER_NAME = "mwcc_42_60422"
COMPILER_FLAGS = "-Cpp_exceptions off -proc gekko -fp hardware -fp_contract on -enum int -align powerpc -nosyspath -RTTI off -str reuse -inline auto -nodefaults -msgstyle gcc -sym on -O4,p -enc SJIS"

INCLUDE_PATTERN = re.compile(r'^#include\s*[<"](.+?)[>"]')
DEFINE_PATTERN = re.compile(r"^#define\s+(\w+)(?:\s+(.*))?")
IF_PATTERN = re.compile(r"^#if(n)?(?:def)?\s+(.*)")
ELSE_PATTERN = re.compile(r"^#else")
ENDIF_PATTERN = re.compile(r"^#endif")
INCBIN_PATTERN = re.compile(r"^#pragma INCBIN\(.*\)")

MACROS: list[tuple[re.Pattern, str]] = [
    (re.compile(r"\bAT_ADDRESS\b\(.*\)"), ""),
    (re.compile(r"\b_GX_TF_CTF\b"), "0x20"),
    (re.compile(r"\b_GX_TF_ZTF\b"), "0x10"),
    (re.compile(r"\bOS_THREAD_SPECIFIC_MAX\b"), "2"),
    (re.compile(r"\bCARD_FILENAME_MAX\b"), "32"),
    (re.compile(r"\bCARD_ICON_MAX\b"), "8"),
    (re.compile(r"\bN64_FRAME_WIDTH\b"), "320"),
    (re.compile(r"\bN64_FRAME_HEIGHT\b"), "240"),
    (re.compile(r"\bATTRIBUTE_ALIGN\b\(.*\)"), ""),
    (re.compile(r"\b_MATH_INLINE\b"), "static inline"),
    (re.compile(r"\bINIT\b"), ""),
    (re.compile(r"\bWEAK\b"), ""),
    (re.compile(r"\bOS_DEF_GLOBAL_VAR\b\(.*\);"), ""),
    (re.compile(r"\bOS_DEF_GLOBAL_ARR\b\(.*\);"), ""),
    (re.compile(r"\bOS_DEF_HW_REG\b\(.*\);"), ""),
    (re.compile(r"\bDECL_ROM_SECTION\b\(.*\);"), ""),
    (re.compile(r"\bDECL_BSS_SECTION\b\(.*\);"), ""),
    (re.compile(r"\bGX_DECL_PUBLIC_STRUCT\b\(GXFifoObj.*\);"), "typedef struct _GXFifoObj { u8 dummy[(128) - sizeof(GXFifoObjImpl) + sizeof(GXFifoObjImpl)]; } GXFifoObj;"),
    (re.compile(r"\bGX_DECL_PUBLIC_STRUCT\b\(GXLightObj.*\);"), "typedef struct _GXLightObj { u8 dummy[(64) - sizeof(GXLightObjImpl) + sizeof(GXLightObjImpl)]; } GXLightObj;"),
    (re.compile(r"typedef __builtin_va_list va_list;"), "typedef struct { char gpr; char fpr; char reserved[2]; char* input_arg_area; char* reg_save_area;} __va_list[1]; typedef __va_list va_list;"),
    (re.compile(r"\bDVD_ERROR_CMD_MAX\b"), "5"),
    (re.compile(r"\b__ungetc_buffer_size\b"), "2"),
    (re.compile(r"\bNO_INLINE\b"), ""),
    (re.compile(r"\bFS_MAX_PATH\b"), "64"),
    (re.compile(r"\bNAND_BANNER_TITLE_MAX\b"), "32"),
    (re.compile(r"\bNAND_BANNER_ICON_MAX_FRAME\b"), "8"),
    (re.compile(r"\bPAD_MAX_CONTROLLERS\b"), "4"),
    (re.compile(r"\bENCODING_NAME_LENGTH\b"), "18"),
    (re.compile(r"\bGX_DEFINE_GX_READ_COUNTER\b"), ""),
    # TODO: better fix for SystemRomType
    (re.compile(r"'NTEJ'"), "0x4E54454A"),
    (re.compile(r"'NTEA'"), "0x4E544541"),
    (re.compile(r"'NTEP'"), "0x4E544550"),
    (re.compile(r"'NBYJ'"), "0x4E42594A"),
    (re.compile(r"'NBYE'"), "0x4E425945"),
    (re.compile(r"'NBYP'"), "0x4E425950"),
    (re.compile(r"'NABJ'"), "0x4E41424A"),
    (re.compile(r"'NABE'"), "0x4E414245"),
    (re.compile(r"'NABP'"), "0x4E414250"),
    (re.compile(r"'NAFJ'"), "0x4E41464A"),
    (re.compile(r"'NAFE'"), "0x4E414645"),
    (re.compile(r"'NAFP'"), "0x4E414650"),
    (re.compile(r"'NBNJ'"), "0x4E424E4A"),
    (re.compile(r"'NBNE'"), "0x4E424E45"),
    (re.compile(r"'NBNP'"), "0x4E424E50"),
    (re.compile(r"'NBCJ'"), "0x4E42434A"),
    (re.compile(r"'NBCE'"), "0x4E424345"),
    (re.compile(r"'NBCP'"), "0x4E424350"),
    (re.compile(r"'NTSJ'"), "0x4E54534A"),
    (re.compile(r"'NFUJ'"), "0x4E46554A"),
    (re.compile(r"'NFUE'"), "0x4E465545"),
    (re.compile(r"'NFUP'"), "0x4E465550"),
    (re.compile(r"'NCUJ'"), "0x4E43554A"),
    (re.compile(r"'NCUE'"), "0x4E435545"),
    (re.compile(r"'NCUP'"), "0x4E435550"),
    (re.compile(r"'NDYJ'"), "0x4E44594A"),
    (re.compile(r"'NDYE'"), "0x4E445945"),
    (re.compile(r"'NDYP'"), "0x4E445950"),
    (re.compile(r"'NDOJ'"), "0x4E444F4A"),
    (re.compile(r"'NDOE'"), "0x4E444F45"),
    (re.compile(r"'NDOP'"), "0x4E444F50"),
    (re.compile(r"'NN6J'"), "0x4E4E364A"),
    (re.compile(r"'NN6E'"), "0x4E4E3645"),
    (re.compile(r"'NN6P'"), "0x4E4E3650"),
    (re.compile(r"'NFZJ'"), "0x4E465A4A"),
    (re.compile(r"'CFZE'"), "0x43465A45"),
    (re.compile(r"'NFZP'"), "0x4E465A50"),
    (re.compile(r"'NSIJ'"), "0x4E53494A"),
    (re.compile(r"'NK4J'"), "0x4E4B344A"),
    (re.compile(r"'NK4E'"), "0x4E4B3445"),
    (re.compile(r"'NK4P'"), "0x4E4B3450"),
    (re.compile(r"'NLRJ'"), "0x4E4C524A"),
    (re.compile(r"'NLRE'"), "0x4E4C5245"),
    (re.compile(r"'NLRP'"), "0x4E4C5250"),
    (re.compile(r"'NMFJ'"), "0x4E4D464A"),
    (re.compile(r"'NMFE'"), "0x4E4D4645"),
    (re.compile(r"'NMFP'"), "0x4E4D4650"),
    (re.compile(r"'NKTJ'"), "0x4E4B544A"),
    (re.compile(r"'NKTE'"), "0x4E4B5445"),
    (re.compile(r"'NKTP'"), "0x4E4B5450"),
    (re.compile(r"'CLBJ'"), "0x434C424A"),
    (re.compile(r"'CLBE'"), "0x434C4245"),
    (re.compile(r"'CLBP'"), "0x434C4250"),
    (re.compile(r"'NMWJ'"), "0x4E4D574A"),
    (re.compile(r"'NMWE'"), "0x4E4D5745"),
    (re.compile(r"'NMWP'"), "0x4E4D5750"),
    (re.compile(r"'NMVJ'"), "0x4E4D564A"),
    (re.compile(r"'NMVE'"), "0x4E4D5645"),
    (re.compile(r"'NMVP'"), "0x4E4D5650"),
    (re.compile(r"'NM8J'"), "0x4E4D384A"),
    (re.compile(r"'NM8E'"), "0x4E4D3845"),
    (re.compile(r"'NM8P'"), "0x4E4D3850"),
    (re.compile(r"'NRBJ'"), "0x4E52424A"),
    (re.compile(r"'NRBE'"), "0x4E524245"),
    (re.compile(r"'NRBP'"), "0x4E524250"),
    (re.compile(r"'NMQJ'"), "0x4E4D514A"),
    (re.compile(r"'NMQE'"), "0x4E4D5145"),
    (re.compile(r"'NMQP'"), "0x4E4D5150"),
    (re.compile(r"'NOBJ'"), "0x4E4F424A"),
    (re.compile(r"'NOBE'"), "0x4E4F4245"),
    (re.compile(r"'NOBP'"), "0x4E4F4250"),
    (re.compile(r"'NYLJ'"), "0x4E594C4A"),
    (re.compile(r"'NYLE'"), "0x4E594C45"),
    (re.compile(r"'NYLP'"), "0x4E594C50"),
    (re.compile(r"'NPOJ'"), "0x4E504F4A"),
    (re.compile(r"'NPOE'"), "0x4E504F45"),
    (re.compile(r"'NPOP'"), "0x4E504F50"),
    (re.compile(r"'NPWJ'"), "0x4E50574A"),
    (re.compile(r"'NPWE'"), "0x4E505745"),
    (re.compile(r"'NPWP'"), "0x4E505750"),
    (re.compile(r"'NQKJ'"), "0x4E514B4A"),
    (re.compile(r"'NQKE'"), "0x4E514B45"),
    (re.compile(r"'NQKP'"), "0x4E514B50"),
    (re.compile(r"'NRXJ'"), "0x4E52584A"),
    (re.compile(r"'NRXE'"), "0x4E525845"),
    (re.compile(r"'NRXP'"), "0x4E525850"),
    (re.compile(r"'NGUJ'"), "0x4E47554A"),
    (re.compile(r"'NGUE'"), "0x4E475545"),
    (re.compile(r"'NGUP'"), "0x4E475550"),
    (re.compile(r"'NFXJ'"), "0x4E46584A"),
    (re.compile(r"'NFXE'"), "0x4E465845"),
    (re.compile(r"'NFXP'"), "0x4E465850"),
    (re.compile(r"'NSQJ'"), "0x4E53514A"),
    (re.compile(r"'NSQE'"), "0x4E535145"),
    (re.compile(r"'NSQP'"), "0x4E535150"),
    (re.compile(r"'NSMJ'"), "0x4E534D4A"),
    (re.compile(r"'NSME'"), "0x4E534D45"),
    (re.compile(r"'NSMP'"), "0x4E534D50"),
    (re.compile(r"'NALJ'"), "0x4E414C4A"),
    (re.compile(r"'NALE'"), "0x4E414C45"),
    (re.compile(r"'NALP'"), "0x4E414C50"),
    (re.compile(r"'NZSJ'"), "0x4E5A534A"),
    (re.compile(r"'NZSE'"), "0x4E5A5345"),
    (re.compile(r"'NZSP'"), "0x4E5A5350"),
    (re.compile(r"'CZLJ'"), "0x435A4C4A"),
    (re.compile(r"'CZLE'"), "0x435A4C45"),
    (re.compile(r"'NZLP'"), "0x4E5A4C50"),
    (re.compile(r"'NRIJ'"), "0x4E52494A"),
    (re.compile(r"'NRIE'"), "0x4E524945"),
    (re.compile(r"'NRIP'"), "0x4E524950"),
    (re.compile(r"'NTUJ'"), "0x4E54554A"),
    (re.compile(r"'NTUE'"), "0x4E545545"),
    (re.compile(r"'NTUP'"), "0x4E545550"),
    (re.compile(r"'NWRJ'"), "0x4E57524A"),
    (re.compile(r"'NWRE'"), "0x4E575245"),
    (re.compile(r"'NWRP'"), "0x4E575250"),
    (re.compile(r"'NYSJ'"), "0x4E59534A"),
    (re.compile(r"'NYSE'"), "0x4E595345"),
    (re.compile(r"'NYSP'"), "0x4E595350"),
]

# Defined preprocessor macros (for conditions)
defines = set()
# Stack of preprocessor conditions
condition_stack = [True]


def find_include_file(filename: str) -> Path:
    for include_dir in INCLUDE_DIRS:
        path = include_dir / filename
        if path.exists():
            return path
    raise FileNotFoundError(f"Could not find include file {filename}")


def process_c_file(path: Path) -> str:
    lines = path.read_text().splitlines()
    out_text = ""
    for i, line in enumerate(lines):
        if match := IF_PATTERN.match(line.strip()):
            condition = match[2] in defines
            if match[1] == "n":
                condition = not condition
            condition_stack.append(condition)
        elif ELSE_PATTERN.match(line.strip()):
            condition_stack[-1] = not condition_stack[-1]
        elif ENDIF_PATTERN.match(line.strip()):
            condition_stack.pop()
        else:
            if not all(condition_stack):
                continue

            if match := DEFINE_PATTERN.match(line.strip()):
                defines.add(match[1])
                out_text += line
                out_text += "\n"
            elif match := INCLUDE_PATTERN.match(line.strip()):
                include_file = find_include_file(match[1])
                out_text += f'/* "{path}" line {i + 1} "{include_file}" */\n'
                out_text += process_c_file(include_file)
                out_text += f'/* end "{include_file}" */\n'
            elif match := INCBIN_PATTERN.match(line.strip()):
                out_text += "    0"
                out_text += "\n"
            else:
                for pattern, replacement in MACROS:
                    line = pattern.sub(replacement, line)
                out_text += line
                out_text += "\n"

    return out_text


def process_asm_file(path: Path) -> str:
    lines = path.read_text().splitlines()
    out_text = ""
    for line in lines:
        if line.startswith(".global "):
            continue
        out_text += line
        out_text += "\n"

    return out_text


def main():
    parser = argparse.ArgumentParser(description="Create a decomp.me scratch")
    parser.add_argument("c_file", metavar="C_FILE", type=Path, help="Input .c file")
    parser.add_argument("asm_file", metavar="ASM_FILE", type=Path, help="Input .s file")
    parser.add_argument(
        "--print-context", action="store_true", help="Print the context and exit"
    )

    args = parser.parse_args()

    func_name = args.asm_file.stem
    context = process_c_file(args.c_file)
    asm_cont = process_asm_file(args.asm_file)
    source_code = ""  # TODO: separate source code from context automatically

    if args.print_context:
        print(context)
        return

    print("Uploading...")
    try:
        post_data = urllib.parse.urlencode(
            {
                "name": func_name,
                "target_asm": asm_cont,
                "context": context,
                "source_code": source_code,
                "compiler": COMPILER_NAME,
                "compiler_flags": COMPILER_FLAGS,
                "diff_label": func_name,
            }
        ).encode("ascii")
        with urllib.request.urlopen("https://decomp.me/api/scratch", post_data) as f:
            resp = f.read()
            json_data: dict[str, str] = json.loads(resp)
            if "slug" in json_data:
                slug = json_data["slug"]
                token = json_data.get("claim_token")
                if token:
                    print(f"https://decomp.me/scratch/{slug}/claim?token={token}")
                else:
                    print(f"https://decomp.me/scratch/{slug}")
            else:
                error = json_data.get("error", resp)
                print(f"Server error: {error}")
    except Exception as e:
        print(e)


if __name__ == "__main__":
    main()
