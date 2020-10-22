# Linker order for every file, passed to the Metrowerks linker.

O_FILES :=                                  \
    $(BUILD_DIR)/asm/.init.o                \
    $(BUILD_DIR)/asm/extab_.o               \
    $(BUILD_DIR)/asm/extabindex_.o          \
    $(BUILD_DIR)/asm/.text.o                \
    $(BUILD_DIR)/asm/.ctors.o               \
    $(BUILD_DIR)/asm/.dtors.o               \
    $(BUILD_DIR)/asm/.rodata.o              \
    $(BUILD_DIR)/asm/.data.o                \
    $(BUILD_DIR)/asm/.bss.o                 \
    $(BUILD_DIR)/asm/.sdata.o               \
    $(BUILD_DIR)/asm/.sbss.o                \
    $(BUILD_DIR)/asm/.sdata2.o              \
    $(BUILD_DIR)/asm/.sbss2.o               \
