/** @file syscall_wrappers.h
 *  @brief Wrappers for error handling
 *  
 *  This includes functions for masking signals,allocating
 *  memory, writing to a file...
 *
 *  @author Shelton Dsouza (sdsouza)
 */

#ifndef __INSTALL_HANDLERS_H
#define __INSTALL_HANDLERS_H

typedef struct {
  char *buf;                 /* Buffer array */         
  int num_slots;                     /* Maximum number of slots */
  volatile int front;        /* buf[(front+1)%num_slots] is first item */
  volatile int rear;         /* buf[rear%num_slts] is last item */
} sbuf_t;

typedef sbuf_t* SharedBuffer;

void sbuf_init(int num_slots);
void sbuf_insert(int item);
int sbuf_remove();
int convert_aug_char(kh_type aug_char);
void install_timer_handler(void *tickback);
void timer_C_handler();
void install_keyboard_handler();
void keyboard_C_handler();

#endif

