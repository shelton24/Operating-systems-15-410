/** @file traceback_asm.h
 *  @brief provides the definitions for the traceback_asm.S file
 *
 *  @author Shelton Dsouza (sdsouza)
 */

#ifndef __TRACEBACK_ASM_H
#define __TRACEBACK_ASM_H

/* Get return address from the base pointer */
void *get_return_addr(void *bp);

/* Get base pointer (of traceback function) */
void *get_initial_base_pointer();

#endif /* __TRACEBACK_ASM_H */
