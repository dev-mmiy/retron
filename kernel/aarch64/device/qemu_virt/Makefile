#
#----------------------------------------------------------------------
#    T-Kernel 2.0 Software Package
#
#    Modified for AArch64 QEMU virt (ReTron OS) at 2024.
#
#----------------------------------------------------------------------
#
#	Makefile (QEMU virt - AArch64)
#	Kernel Build
#

# Cross compiler
CROSS_COMPILE ?= aarch64-linux-gnu-

CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)as
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump

# Directories
DEVICE_DIR = .
CPU_DIR = ../../cpu

# Target
TARGET = kernel
ELF = $(TARGET).elf
BIN = $(TARGET).bin

# Source files
ASM_SRCS = \
	icrt0.S \
	$(CPU_DIR)/cpu_support.S

C_SRCS = \
	kernel_main.c \
	devinit.c \
	tkdev_init.c

# Object files
OBJS = $(ASM_SRCS:.S=.o) $(C_SRCS:.c=.o)

# Include paths
INCLUDES = \
	-I$(DEVICE_DIR) \
	-I$(CPU_DIR) \
	-I../../../include \
	-I../../../../include

# Compiler flags
CFLAGS = -Wall -Wextra -O2 -ffreestanding -nostdlib -nostartfiles
CFLAGS += -march=armv8-a -mgeneral-regs-only
CFLAGS += $(INCLUDES)
CFLAGS += -DUSE_MMU=0 -DUSE_DBGSPT=0

# Assembler flags
ASFLAGS = -march=armv8-a
ASFLAGS += $(INCLUDES)

# Linker flags
LDFLAGS = -nostdlib -T kernel.ld

# Default target
all: $(BIN)

# Link
$(ELF): $(OBJS) kernel.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
	$(OBJDUMP) -d $@ > $(TARGET).lst

# Binary
$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

# Compile C files
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Assemble .S files
%.o: %.S
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean
clean:
	rm -f $(OBJS) $(ELF) $(BIN) $(TARGET).lst

# Run in QEMU
run: $(ELF)
	qemu-system-aarch64 -M virt -cpu cortex-a53 -nographic -kernel $(ELF)

# Debug in QEMU
debug: $(ELF)
	qemu-system-aarch64 -M virt -cpu cortex-a53 -nographic -kernel $(ELF) -S -s

.PHONY: all clean run debug
