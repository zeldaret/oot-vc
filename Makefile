WINDOWS := $(shell which wine ; echo $$?)
UNAME_S := $(shell uname -s)

#-------------------------------------------------------------------------------
# Files
#-------------------------------------------------------------------------------

TARGET_COL := wii

TARGET := 00000001

BUILD_DIR := build

SRC_DIRS := $(shell find src -type d)
ASM_DIRS := $(shell find asm -type d -not -path "asm/non_matchings*") $(shell find data -type d)

C_FILES       := $(foreach dir,$(SRC_DIRS), $(wildcard $(dir)/*.c))
S_FILES       := $(foreach dir,$(ASM_DIRS), $(wildcard $(dir)/*.s))

# Inputs
LDSCRIPT := $(BUILD_DIR)/ldscript.lcf

# Outputs
DOL     := $(BUILD_DIR)/00000001.app
ELF     := $(DOL:.app=.elf)
MAP     := $(BUILD_DIR)/00000001.map

O_FILES := $(addprefix $(BUILD_DIR)/,$(S_FILES:.s=.o) $(C_FILES:.c=.o))

GLOBAL_ASM_C_FILES != grep -rl 'GLOBAL_ASM(' $(C_FILES)
GLOBAL_ASM_O_FILES = $(addprefix $(BUILD_DIR)/,$(GLOBAL_ASM_C_FILES:.c=.o))

#-------------------------------------------------------------------------------
# Tools
#-------------------------------------------------------------------------------

# Programs
ifeq ($(WINDOWS),1)
	WINE :=
else
	WINE := wine
endif

# Hack for OSX
ifeq ($(UNAME_S),Darwin)
	CPP     := cpp-10 -P
	SHA1SUM := shasum -a 1
else
	CPP     := cpp -P
	SHA1SUM := sha1sum
endif

AS      := $(DEVKITPPC)/bin/powerpc-eabi-as
OBJCOPY := $(DEVKITPPC)/bin/powerpc-eabi-objcopy
CC      := $(WINE) tools/mwcc_compiler/3.0/mwcceppc.exe
CC_2.7	:= $(WINE) tools/mwcc_compiler/2.7/mwcceppc.exe
LD      := $(WINE) tools/mwcc_compiler/3.0/mwldeppc.exe
ELF2DOL := tools/elf2dol
PYTHON  := python3

ASM_PROCESSOR_DIR := tools/asm_processor
ASM_PROCESSOR := $(ASM_PROCESSOR_DIR)/compile.sh

ASFLAGS := -mgekko -I include
LDFLAGS := -map $(MAP) -fp hard -nodefaults -w off
CFLAGS  := -Cpp_exceptions off -proc gekko -fp hard -O4,p -nodefaults -msgstyle gcc -i include

# elf2dol needs to know these in order to calculate sbss correctly.
SDATA_PDHR := 9
SBSS_PDHR := 10

#-------------------------------------------------------------------------------
# Recipes
#-------------------------------------------------------------------------------

### Default target ###

default: all

all: dirs $(DOL)

ALL_DIRS := build $(BUILD_DIR) $(addprefix $(BUILD_DIR)/,$(SRC_DIRS) $(ASM_DIRS))

.PHONY: tools

# Make sure build directory exists before compiling anything
dirs:
	$(shell mkdir -p $(ALL_DIRS))

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

$(ELF): $(O_FILES) $(GLOBAL_ASM_O_FILES) $(LDSCRIPT)
	$(RM) -rf $(ASM_PROCESSOR_DIR)/tmp
	$(LD) $(LDFLAGS) -o $@ -lcf $(LDSCRIPT) $(O_FILES)

$(BUILD_DIR)/src/MetroTRK/%.o : CC := $(CC_2.7)

$(GLOBAL_ASM_O_FILES) : BUILD_C := $(ASM_PROCESSOR) "$(CC) $(CFLAGS) $(OPT_FLAGS)" "$(AS) $(ASFLAGS)"

BUILD_C ?= $(CC) $(CFLAGS) $(OPT_FLAGS) -c -o

$(BUILD_DIR)/%.o: %.s
	$(AS) $(ASFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: %.c
	$(BUILD_C) $@ $<
	$(OBJCOPY) --remove-section .mwcats.text --remove-section .comment $@
