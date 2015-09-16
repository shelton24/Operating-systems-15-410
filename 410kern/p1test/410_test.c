/** @file 410_test.c
 *  @brief A test file for your drivers.
 */

/* -- Includes -- */

/* multiboot header file */
#include <multiboot.h> /* for boot_info */

/* x86 specific includes */
#include <asm.h> /* register manipulation */
#include <timer_defines.h>
#include <interrupt_defines.h>
#include <p1kern.h>

#include <stdio.h>
#include <assert.h>

/* --- Stuff and Nonsense --- */

void wait_char(char testc){
  char c;
  do{
    c = readchar();
  }while(c != testc);
}

int seconds = 0;

void tick(unsigned int numTicks)
{
    if (numTicks % 100 == 0)
      ++seconds;
}

/* --- Kernel entrypoint --- */
int
kernel_main(mbinfo_t *mbinfo, int argc, char **argv, char **envp)
{
    if(handler_install(tick) < 0) {
      panic("oh noes! where are my handlers");
    }

    enable_interrupts();

    clear_console();

    hide_cursor();

    set_term_color(FGND_GREEN | BGND_BLACK);

    set_cursor(12, 0);
    printf("H\b\b");
    //set_cursor(15, 34);
    printf("Hi");
    printf("\b\b");
    show_cursor();
    /*wait_char('c'); putbyte('d');
    wait_char('a'); putbyte('o');
    wait_char('t'); putbyte('g');
    set_cursor(16, 25);
    printf("That took %d seconds\n", seconds);
    printf("Yay kitties! - now what if you type \"dog\"?");*/
    while(1)
      continue;
}
