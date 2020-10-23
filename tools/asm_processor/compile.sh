#!/bin/bash

CC="$1"
shift
AS="$1"
shift

# When running windows exes through wsl1 you can't give them linux paths
mkdir tools/asm_processor/tmp
temp="$(mktemp --tmpdir=tools/asm_processor/tmp)"

tools/asm_processor/asm_processor.py "$2" --assembler "$AS" > "$temp.c" &&
$CC "$temp.c" -c -o "$temp.o" &&
tools/asm_processor/asm_processor.py "$2" --post-process "$temp.o" --assembler "$AS" --asm-prelude include/macros.inc
powerpc-eabi-objcopy --remove-section .mwcats.text "$temp.o" "$temp.2.o"
powerpc-eabi-objcopy --remove-section .comment "$temp.2.o" "$1"
rm -rf tools/asm_processor/tmp
