/** @file timer_handler_wrapper.S
 *  @brief Functions in assembly
 *
 *  This file contains the assembly wrappers for timer interrupt handler and
 *  keyboard interrupt handler
 *  
 *
 *  @author Shelton Dsouza (sdsouza)
 */

 /* define the timer_handler_wrapper and keyboard_handler_wrapper labels 
  * so that they can be called from other files.
  */

 .globl timer_handler_wrapper
 .globl keyboard_handler_wrapper

 
timer_handler_wrapper:
  pusha
  call timer_C_handler
  popa
  iret

 
keyboard_handler_wrapper:
  pusha
  call keyboard_C_handler
  popa
  iret