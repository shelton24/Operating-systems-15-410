/** @file traceback_asm.S
 *  @brief Functions in assembly
 *
 *  This file contains functions for getting the base pointer of the
 *  traceback function and for fetching the return address from the base
 *  pointer.
 *
 *  @author Shelton Dsouza (sdsouza)
 */

 /* define the get_initial_base_pointer and get_return_addr labels 
  * so that they can be called from other files.
  */
 .global get_initial_base_pointer
 .global get_return_addr

 /* This method initializes the base pointer */
get_initial_base_pointer:
  movl %ebp,%eax  /* get ebp into eax */
  ret

 /*  This method gets the return address of the previous function */
get_return_addr:
  /* Get the base pointer passed as an argument into ecx */
  movl	4(%esp),%ecx
   /* get the return address from ecx  by performing *(ecx+4) */	 
  movl    4(%ecx),%eax 
  ret