/*
 *  $Id: zybo_z7.ld 1814 2023-04-13 06:28:01Z ertl-honda $
 */

_STACK_SIZE = DEFINED(_STACK_SIZE) ? _STACK_SIZE : 16384;
_TASK_STACK_SIZE = _STACK_SIZE * 32;
_HEAP_SIZE = DEFINED(_HEAP_SIZE) ? _HEAP_SIZE : 16384;

_ABORT_STACK_SIZE = DEFINED(_ABORT_STACK_SIZE) ? _ABORT_STACK_SIZE : 1024;
_SUPERVISOR_STACK_SIZE = DEFINED(_SUPERVISOR_STACK_SIZE) ? _SUPERVISOR_STACK_SIZE : 4096;
_IRQ_STACK_SIZE = DEFINED(_IRQ_STACK_SIZE) ? _IRQ_STACK_SIZE : 0;
_FIQ_STACK_SIZE = DEFINED(_FIQ_STACK_SIZE) ? _FIQ_STACK_SIZE : 0;
_UNDEF_STACK_SIZE = DEFINED(_UNDEF_STACK_SIZE) ? _UNDEF_STACK_SIZE : 0;

OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm") 
OUTPUT_ARCH(arm)
ENTRY(start);

MEMORY {
	RAM (xrw) : ORIGIN = 0x00100000, LENGTH = 511M
}

SECTIONS {
	.text : {
		__start_text = .;
		KEEP(*(.vector))
		*(.text .text.*)
		*(.glue_7t)
		*(.glue_7)
		__end_kernel = .;
	} > RAM
	_etext = .;
	PROVIDE(etext = .);

	.rodata ALIGN(4) : {
		__start_rodata = .;
		KEEP(*(.rodata .rodata.*))
		__end_rodata = .;
	} > RAM
	.preinit_array ALIGN(4) : {
		PROVIDE_HIDDEN (__preinit_array_start = .);
		KEEP (*(.preinit_array))
		PROVIDE_HIDDEN (__preinit_array_end = .);
	} > RAM
	.init_array ALIGN(4) : {
		PROVIDE_HIDDEN (__init_array_start = .);
		KEEP (*(SORT(.init_array.*)))
		KEEP (*(.init_array))
		PROVIDE_HIDDEN (__init_array_end = .);
	} > RAM
	.fini_array ALIGN(4) : {
		PROVIDE_HIDDEN (__fini_array_start = .);
		KEEP (*(SORT(.fini_array.*)))
		KEEP (*(.fini_array))
		PROVIDE_HIDDEN (__fini_array_end = .);
	} > RAM
	.ARM.exidx ALIGN(4) : {
		__exidx_start = .;
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
		__exidx_end = .;
	} > RAM

	.data : ALIGN(4) {
		__start_data = .;
		*(.data .data.*)
		__end_data = .;
	} > RAM
	__start_idata = LOADADDR(.data);
	. = ALIGN(4);
	_edata = .;
	PROVIDE(edata = .);

	.bss ALIGN(4) : {
		__start_bss = .;
		*(.bss .bss.*)
		*(COMMON)
		__end_bss = .;
	} > RAM

	.mmu_tbl (ALIGN(16384)) : {
	__mmu_tbl_start = .;
	*(.mmu_tbl)
	__mmu_tbl_end = .;
	} > RAM


	.heap (NOLOAD) : {
	. = ALIGN(16);
	_heap = .;
	HeapBase = .;
	_heap_start = .;
	. += _HEAP_SIZE;
	_heap_end = .;
	HeapLimit = .;
	} > RAM

	.stack (NOLOAD) : {
   . = ALIGN(16);
   _stack_end = .;
   . += _TASK_STACK_SIZE;
   . = ALIGN(16);
   _stack = .;
   __stack = _stack;
   . = ALIGN(16);
   _irq_stack_end = .;
   . += _IRQ_STACK_SIZE;
   . = ALIGN(16);
   __irq_stack = .;
   _supervisor_stack_end = .;
   . += _SUPERVISOR_STACK_SIZE;
   . = ALIGN(16);
   __supervisor_stack = .;
   _abort_stack_end = .;
   . += _ABORT_STACK_SIZE;
   . = ALIGN(16);
   __abort_stack = .;
   _fiq_stack_end = .;
   . += _FIQ_STACK_SIZE;
   . = ALIGN(16);
   __fiq_stack = .;
   _undef_stack_end = .;
   . += _UNDEF_STACK_SIZE;
   . = ALIGN(16);
   __undef_stack = .;
	}>RAM

	. = ALIGN(4);
	_end = .;
	PROVIDE(end = .);
	.comment 0 : { *(.comment) }	

	/*
	 *  DWARF debug sections.
	 *  Symbols in the DWARF debugging sections are relative to
	 *  the beginning of the section so we begin them at 0.
	 */

	/* DWARF 1 */
	.debug			0 : { *(.debug) }
	.line			0 : { *(.line) }

	/* GNU DWARF 1 extensions */
	.debug_srcinfo	0 : { *(.debug_srcinfo) }
	.debug_sfnames	0 : { *(.debug_sfnames) }

	/* DWARF 1.1 and DWARF 2 */
	.debug_aranges	0 : { *(.debug_aranges) }
	.debug_pubnames	0 : { *(.debug_pubnames) }

	/* DWARF 2 */
	.debug_info		0 : { *(.debug_info) }
	.debug_abbrev	0 : { *(.debug_abbrev) }
	.debug_line		0 : { *(.debug_line) }
	.debug_frame	0 : { *(.debug_frame) }
	.debug_str		0 : { *(.debug_str) }
	.debug_loc		0 : { *(.debug_loc) }
	.debug_macinfo	0 : { *(.debug_macinfo) }

	/* SGI/MIPS DWARF 2 extensions */
	.debug_weaknames 0 : { *(.debug_weaknames) }
	.debug_funcnames 0 : { *(.debug_funcnames) }
	.debug_typenames 0 : { *(.debug_typenames) }
	.debug_varnames  0 : { *(.debug_varnames) }
}	
