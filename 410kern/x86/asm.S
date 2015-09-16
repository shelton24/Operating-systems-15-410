/* x86/asm.S */
/* Copyright (c) 2014 Carnegie Mellon University */

#include <x86/seg.h>

.global lgdt
lgdt:
    movl 4(%esp), %eax      /* Swap the order of the arguments passed in */
    movl 8(%esp), %ecx
    pushl %eax
    pushw %cx
    lgdt (%esp)             /* Load the new GDTR... */
    ljmp $SEGSEL_KERNEL_CS, $lgdt_reload    /* Do a longjmp to cause a GDT reload */
lgdt_reload:
    movl $SEGSEL_KERNEL_DS, %eax    /* Load up the data segment selectors */
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    addl $6, %esp           /* And return */
    ret

.global lidt
lidt:
    movl 4(%esp), %eax      /* Swap the order of the arguments passed in */
    movl 8(%esp), %ecx
    pushl %eax
    pushw %cx
    lidt (%esp)             /* Load the IDTR */
    addl $6, %esp           /* And return */
    ret

# asm() to .S translation contributed by asubrama, F'08
.global idt_base
idt_base:
    subl $6, %esp       /* get 6 bytes for sidt */
    sidt (%esp)         /* sidt loads into the address */
    movl 2(%esp), %eax  /* we ignore the lower 16 bits (limit bits) */
    addl $6, %esp       /* reclaim stack */
    ret                 /* return the higher 32 bits */

.global disable_interrupts
disable_interrupts:
	cli
	ret

.global enable_interrupts
enable_interrupts:
	sti
	ret

.global rdtsc
rdtsc:
    rdtsc
    ret

.global inb
inb:
	xorl %eax, %eax
	movw 4(%esp), %dx
	in %dx, %al
	ret

.global inw
inw:
	xorl %eax, %eax
	movw 4(%esp), %dx
	in %dx, %ax
	ret

.global ind
ind:
	xorl %eax, %eax
	movw 4(%esp), %dx
	in %dx, %eax
	ret

.global outb
outb:
	movw 4(%esp), %dx
	movb 8(%esp), %al
	out %al, %dx
	ret

.global outw
outw:
	movw 4(%esp), %dx
	movw 8(%esp), %ax
	out %ax, %dx
	ret

.global outd
outd:
	movw 4(%esp), %dx
	movl 8(%esp), %eax
	out %eax, %dx
	ret

.global get_cr0
get_cr0:
    movl %cr0, %eax
    ret

.global get_cr2
get_cr2:
    movl %cr2, %eax
    ret

.global get_cr3
get_cr3:
    movl %cr3, %eax
    ret

.global get_cr4
get_cr4:
    movl %cr4, %eax
    ret

# cr0 is special: change must be followed
# by a "far jump" (thanks, nwf).

.global set_cr0
set_cr0:
    movl 4(%esp), %eax   # fetch param
    movl %eax, %cr0      # set %cr0
    pushw %cs            # 16 bits of %cs
    pushl $set_cr0_cont  # 32 bits of %eip
    ljmp *(%esp)         # load %cs:%eip from stack
set_cr0_cont:
    addl $6, %esp        # undo pushes
    ret

.global set_cr2
set_cr2:
    movl 4(%esp), %eax
    movl %eax, %cr2
    ret

.global set_cr3
set_cr3:
    movl 4(%esp), %eax
    movl %eax, %cr3
    ret

.global set_cr4
set_cr4:
    movl 4(%esp), %eax
    movl %eax, %cr4
    ret

.global init_tss

# esp0 is not a register.  But we can pretend!
.global get_esp0
get_esp0:
    leal init_tss, %eax
    movl 4(%eax), %eax
    ret

.global set_esp0
set_esp0:
    leal init_tss, %eax
    movl 4(%esp), %ecx
    movl %ecx, 4(%eax)
    ret

.global iodelay
iodelay:
    pushl %eax
    inb $0x80,%al
    inb $0x80,%al
    popl %eax
    ret
