WINDOWS := $(shell which wine ; echo $$?)

#-------------------------------------------------------------------------------
# Files
#-------------------------------------------------------------------------------

TARGET_COL := wii

TARGET := 00000001

BUILD_DIR := build/

SRC_DIRS := $(shell find src/ -type f -name '*.c')
ASM_DIRS := $(shell find asm/ -type f -name '*.s')

# Inputs
LDSCRIPT := $(BUILD_DIR)/ldscript.lcf

# Outputs
DOL     := $(BUILD_DIR)/00000001.app
ELF     := $(DOL:.app=.elf)
MAP     := $(BUILD_DIR)/00000001.map

# Object files in link order
include obj_files.mk

#-------------------------------------------------------------------------------
# Tools
#-------------------------------------------------------------------------------

MWCC_VERSION := 3.0

# Programs
ifeq ($(WINDOWS),1)
	WINE :=
else
	WINE := wine
endif

AS      := $(DEVKITPPC)/bin/powerpc-eabi-as
OBJCOPY := $(DEVKITPPC)/bin/powerpc-eabi-objcopy
CPP     := cpp -P
CC      := $(WINE) tools/mwcc_compiler/$(MWCC_VERSION)/mwcceppc.exe
LD      := $(WINE) tools/mwcc_compiler/$(MWCC_VERSION)/mwldeppc.exe
ELF2DOL := tools/elf2dol
SHA1SUM := sha1sum
PYTHON  := python3

POSTPROC := tools/postprocess.py

# Options
INCLUDES := -I include -I include/dolphin -I src -I src/msl -I src/msl/ppc_eabi -I src/runtime

ASFLAGS := -mgekko -I include
LDFLAGS := -map $(MAP) -fp hard -nodefaults -w off
CFLAGS  := -Cpp_exceptions off -proc gekko -fp hard -O4,p -nodefaults -msgstyle gcc $(INCLUDES)

# for postprocess.py
PROCFLAGS := -fprologue-fixup=old_stack

# elf2dol needs to know these in order to calculate sbss correctly.
SDATA_PDHR := 9
SBSS_PDHR := 10

#-------------------------------------------------------------------------------
# Recipes
#-------------------------------------------------------------------------------

### Default target ###

default: all

all: $(DOL)

ALL_DIRS := build $(BUILD_DIR) $(addprefix $(BUILD_DIR)/,$(SRC_DIRS) $(ASM_DIRS))

# Make sure build directory exists before compiling anything
DUMMY != mkdir -p $(ALL_DIRS)

.PHONY: tools

$(LDSCRIPT): ldscript.lcf
	$(CPP) -MMD -MP -MT $@ -MF $@.d -I include/ -I . -DBUILD_DIR=$(BUILD_DIR) -o $@ $<

$(DOL): $(ELF) | tools
	$(ELF2DOL) $< $@ $(SDATA_PDHR) $(SBSS_PDHR) $(TARGET_COL)
	$(SHA1SUM) -c $(TARGET).sha1

clean:
	rm -f -d -r build
	$(MAKE) -C tools clean

tools:
	$(MAKE) -C tools

$(ELF): $(O_FILES) $(LDSCRIPT)
	$(LD) $(LDFLAGS) -o $@ -lcf $(LDSCRIPT) $(O_FILES)
# The Metrowerks linker doesn't generate physical addresses in the ELF program headers. This fixes it somehow.
#	$(OBJCOPY) $@ $@

$(BUILD_DIR)/%.o: %.s
	$(AS) $(ASFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
# TODO: See if this is necessary after actually adding some C code
#   $(PYTHON) $(POSTPROC) $(PROCFLAGS) $@
