/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Modified for AArch64 QEMU virt (ReTron OS) at 2024.
 *
 *----------------------------------------------------------------------
 */

/*
 *	kernel.ld (QEMU virt - AArch64)
 *	Linker Script
 */

OUTPUT_FORMAT("elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
ENTRY(_start)

/* QEMU virt loads kernel at 0x40000000 */
KERNEL_BASE = 0x40000000;

SECTIONS
{
	. = KERNEL_BASE;

	/* Boot code must come first */
	.text.boot : {
		*(.text.boot)
	}

	/* Exception vector table - must be 2KB aligned */
	. = ALIGN(0x800);
	.vectors : {
		*(.vectors)
	}

	/* Code section */
	.text : {
		*(.text)
		*(.text.*)
	}

	/* Read-only data */
	.rodata : {
		*(.rodata)
		*(.rodata.*)
	}

	/* Data section */
	. = ALIGN(4096);
	.data : {
		*(.data)
		*(.data.*)
	}

	/* BSS section */
	. = ALIGN(4096);
	__bss_start = .;
	.bss : {
		*(.bss)
		*(.bss.*)
		*(COMMON)
	}
	__bss_end = .;

	/* Stack section */
	. = ALIGN(16);
	.stack : {
		*(.stack)
	}

	/* Heap starts after BSS */
	. = ALIGN(4096);
	__heap_start = .;

	/* Discard unwanted sections */
	/DISCARD/ : {
		*(.comment)
		*(.note*)
		*(.eh_frame*)
	}
}
