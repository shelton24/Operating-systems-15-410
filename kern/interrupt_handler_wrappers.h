/** @file syscall_wrappers.h
 *  @brief Wrappers for error handling
 *  
 *  This includes functions for masking signals,allocating
 *  memory, writing to a file...
 *
 *  @author Shelton Dsouza (sdsouza)
 */

#ifndef __INTERRUPT_HANDLER_WRAPPERS_H
#define __INTERRUPT_HANDLER_WRAPPERS_H


void timer_handler_wrapper();
void keyboard_handler_wrapper();

#endif