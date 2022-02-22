#!/bin/sh

src="$(realpath "$1")"
curdir="$(pwd)"
if [ -z "$src" ]; then
    echo usage: \`genctx.sh \<src\>\`
    exit
fi

cd "$(dirname "$0")"

mwcc_compiler/3.0/mwcceppc.exe -E -i "$curdir/include" "$src" -o "$curdir/ctx.c"

echo 'void _restgpr_14(void);void _savegpr_14(void);' | cat - "$curdir/ctx.c" > /tmp/out && mv /tmp/out "$curdir/ctx.c"
echo 'void _restgpr_15(void);void _savegpr_15(void);' | cat - "$curdir/ctx.c" > /tmp/out && mv /tmp/out "$curdir/ctx.c"
echo 'void _restgpr_16(void);void _savegpr_16(void);' | cat - "$curdir/ctx.c" > /tmp/out && mv /tmp/out "$curdir/ctx.c"
echo 'void _restgpr_17(void);void _savegpr_17(void);' | cat - "$curdir/ctx.c" > /tmp/out && mv /tmp/out "$curdir/ctx.c"
echo 'void _restgpr_18(void);void _savegpr_18(void);' | cat - "$curdir/ctx.c" > /tmp/out && mv /tmp/out "$curdir/ctx.c"
echo 'void _restgpr_19(void);void _savegpr_19(void);' | cat - "$curdir/ctx.c" > /tmp/out && mv /tmp/out "$curdir/ctx.c"
echo 'void _restgpr_20(void);void _savegpr_20(void);' | cat - "$curdir/ctx.c" > /tmp/out && mv /tmp/out "$curdir/ctx.c"
echo 'void _restgpr_21(void);void _savegpr_21(void);' | cat - "$curdir/ctx.c" > /tmp/out && mv /tmp/out "$curdir/ctx.c"
echo 'void _restgpr_22(void);void _savegpr_22(void);' | cat - "$curdir/ctx.c" > /tmp/out && mv /tmp/out "$curdir/ctx.c"
echo 'void _restgpr_23(void);void _savegpr_23(void);' | cat - "$curdir/ctx.c" > /tmp/out && mv /tmp/out "$curdir/ctx.c"
echo 'void _restgpr_24(void);void _savegpr_24(void);' | cat - "$curdir/ctx.c" > /tmp/out && mv /tmp/out "$curdir/ctx.c"
echo 'void _restgpr_25(void);void _savegpr_25(void);' | cat - "$curdir/ctx.c" > /tmp/out && mv /tmp/out "$curdir/ctx.c"
echo 'void _restgpr_26(void);void _savegpr_26(void);' | cat - "$curdir/ctx.c" > /tmp/out && mv /tmp/out "$curdir/ctx.c"
echo 'void _restgpr_27(void);void _savegpr_27(void);' | cat - "$curdir/ctx.c" > /tmp/out && mv /tmp/out "$curdir/ctx.c"
echo 'void _restgpr_28(void);void _savegpr_28(void);' | cat - "$curdir/ctx.c" > /tmp/out && mv /tmp/out "$curdir/ctx.c"
echo 'void _restgpr_29(void);void _savegpr_29(void);' | cat - "$curdir/ctx.c" > /tmp/out && mv /tmp/out "$curdir/ctx.c"
echo 'void _restgpr_30(void);void _savegpr_30(void);' | cat - "$curdir/ctx.c" > /tmp/out && mv /tmp/out "$curdir/ctx.c"
echo 'void _restgpr_31(void);void _savegpr_31(void);' | cat - "$curdir/ctx.c" > /tmp/out && mv /tmp/out "$curdir/ctx.c"
