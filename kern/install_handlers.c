/** @file install_handlers.c 
 *
 *  @brief Installing handlers for interrupts
 *  This file
 *  
 *
 *  @author Shelton Dsouza (sdsouza)
 *  @bug No known bugs
 */

#include <p1kern.h>    /* Contains function definitions written here */
#include <interrupt_defines.h>
#include <seg.h> 
#include <stdlib.h>
#include <timer_defines.h> /* Timer interrupts paramters */
#include <keyhelp.h>
#include <install_handlers.h>
#include <interrupt_handler_wrappers.h>
#include <asm.h>       /* register manipulation */
#include <simics.h>    /* Sim breakpoints */

#define BUFFER_MAX_SLOTS 100
#define GET_IDT_ADDR(bp,index) ((unsigned int)bp + (index * 8))
#define PUT_VAL_IDT_LSB(bp,val) (*(unsigned int *)bp = val)
#define PUT_VAL_IDT_MSB(bp,val) (*((unsigned int *)bp + 1) = val)
#define GET_LOWER_NIBBLE(bp) (bp & 0x0000FFFF)
#define GET_UPPER_NIBBLE(bp) (bp >> (1 << 4))

#define PACK_LSB_IDT_ENTRY(val1,val2) ((val1 << (1 << 4)) | val2)
#define PACK_MSB_IDT_ENTRY(val1,val2) ((val1 << (1 << 4)) | val2)

#define TIMER_INTERRUPT_INTERVAL 0.01

void (*fptr)(unsigned int);
SharedBuffer sbuf;
unsigned int numTicks = 0;

int handler_install(void (*tickback)(unsigned int))
{
	 lprintf("Tick install address:%p",tickback);
	 install_timer_handler(tickback);
	 install_keyboard_handler();
	 sbuf_init(BUFFER_MAX_SLOTS);
 	 return 0;
}

void sbuf_init(int num_slots)
{
  sbuf = calloc(1,sizeof(sbuf_t));
  sbuf->buf = calloc(num_slots,sizeof(int)); 
  sbuf->num_slots = num_slots;         /* Buffer holds max of n items */
  sbuf->front = sbuf->rear = 0;        
}

void sbuf_insert(int item)
{
	sbuf->buf[sbuf->rear++ % (sbuf->num_slots)] = item;
}

int sbuf_remove()
{
	int item;

	item = sbuf->buf[sbuf->front++ % (sbuf->num_slots)];

	return item;
}

int readchar(void)
{
	int item;
	kh_type augmented_ch;

	item = sbuf_remove();
	augmented_ch = process_scancode(item);

	item = convert_aug_char(augmented_ch);

	return item;

}

int convert_aug_char(kh_type aug_char)
{
	if(KH_HASDATA(aug_char))
	{
		if(!KH_ISMAKE(aug_char))
			return KH_GETCHAR(aug_char);
	}

	return -1;
}

void install_timer_handler(void *tickback)
{
	 fptr = tickback;
	 void *idt_base_addr = idt_base();
 	 unsigned int timer_IDT_addr = GET_IDT_ADDR(idt_base_addr,TIMER_IDT_ENTRY);
 	 unsigned int timer_handler_addr = (unsigned int)&timer_handler_wrapper;
 	 unsigned int timer_handler_lowaddr = GET_LOWER_NIBBLE(timer_handler_addr);
 	 unsigned int timer_handler_highaddr = GET_UPPER_NIBBLE(timer_handler_addr);
 	 unsigned int make_LSB_entry = PACK_LSB_IDT_ENTRY(SEGSEL_KERNEL_CS,timer_handler_lowaddr);
 	 unsigned int make_MSB_entry = PACK_MSB_IDT_ENTRY(timer_handler_highaddr,0x00008F00);
 	 PUT_VAL_IDT_LSB(timer_IDT_addr,make_LSB_entry);
 	 PUT_VAL_IDT_MSB(timer_IDT_addr,make_MSB_entry);

 	 outb(TIMER_MODE_IO_PORT,TIMER_SQUARE_WAVE);
 	 unsigned int num_cycles = TIMER_INTERRUPT_INTERVAL * TIMER_RATE;
 	 unsigned int num_cycles_lsb = num_cycles & (0x0F);
 	 unsigned int num_cycles_msb = num_cycles & (0xF0);
 	 outb(TIMER_PERIOD_IO_PORT,num_cycles_lsb);
 	 outb(TIMER_PERIOD_IO_PORT,num_cycles_msb);
}

void timer_C_handler()
{
	//lprintf("timer handler: %d",numTicks);
	fptr(numTicks++);
	outb(INT_CTL_PORT,INT_ACK_CURRENT);
}

void install_keyboard_handler()
{
	 void *idt_base_addr = idt_base();
 	 unsigned int keyboard_IDT_addr = GET_IDT_ADDR(idt_base_addr,KEY_IDT_ENTRY);
 	 unsigned int keyboard_handler_addr = (unsigned int)&keyboard_handler_wrapper;
 	 unsigned int keyboard_handler_lowaddr = GET_LOWER_NIBBLE(keyboard_handler_addr);
 	 unsigned int keyboard_handler_highaddr = GET_UPPER_NIBBLE(keyboard_handler_addr);
 	 unsigned int make_LSB_entry = PACK_LSB_IDT_ENTRY(SEGSEL_KERNEL_CS,keyboard_handler_lowaddr);
 	 unsigned int make_MSB_entry = PACK_MSB_IDT_ENTRY(keyboard_handler_highaddr,0x00008F00);
 	 PUT_VAL_IDT_LSB(keyboard_IDT_addr,make_LSB_entry);
 	 PUT_VAL_IDT_MSB(keyboard_IDT_addr,make_MSB_entry);

}

void keyboard_C_handler()
{
	lprintf("Keyboard handler");
	int ch = inb(KEYBOARD_PORT);
	sbuf_insert(ch);
	outb(INT_CTL_PORT,INT_ACK_CURRENT);
}