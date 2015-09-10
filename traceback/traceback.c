/** @file traceback.c
 *  @brief The traceback function
 *
 *  This file contains the traceback function for the traceback library. It
 *  prints out a stack trace of the program it is called from. The stack 
 *  trace will display the function calls made to reach the current location
 *  in the program. 
 *  The following arguments are supported for a function :
 *
 *  1. Character, 2. Integer, 3. Float, 4. Double, 5. String, 6. String array,
 *
 *  7. Void pointer, 8. Unknown argument.
 *
 *  @author Shelton Dsouza (sdsouza)
 *  @bug No known bugs
 */

/* -- Includes -- */

/* For traceback library */
#include "traceback_internal.h" /* for function table info */

/* Assembly for getting the return address and stack base pointer */
#include "traceback_asm.h"

/* For syscall wrappers */
#include "syscall_wrappers.h"

#include <string.h>  /* Find length of a string, string copy */
#include <signal.h>  /* For sigaction, masking signals */
#include <setjmp.h>  /* For setjmp and longjmp */
#include <errno.h>   /* For write syscall error */
#include <ctype.h>   /* For isPrint() */
#include <stdlib.h>  /* For malloc and free */
#include <contracts.h> /* For asserts */

/* -- Macros -- */

/* Maximum buffer size for function name and arguments */
#define MAX_BUF_SIZE 8192  

/* Maximum characters permissible in a string */
#define STRING_MAX_CHARACTERS 25

/* Maximum number of strings to be displayed in astring array */
#define STRING_ARRAY_MAX_STRINGS 3

/* Save signal mask in sigsetjmp  and restore in siglongjmp*/
#define SAVE_SIGNAL_MASK 1

/* Used as second argument in siglongjmp to indicate to sigsetjmp that
 * we are returning from the SIGSEGV handler.
 */
#define SETJMP_RET_VALUE_HANDLER 1

/* Used to check if address is writtable */
#define DUMMY_VALUE 10

/* Get previous function's base pointer from the current ebp */
#define GET_OLD_EBP(bp) (*(unsigned int *)(bp))

/* Get stack address at some offset from ebp 
 * Used for fetching arguments
 */
#define GET_ARG_OFFSET(bp,offset) (((char *) bp) + offset)

/* Get integer argument at address bp */
#define GET_ARG(bp) (*(unsigned int *)(bp))

/* Get float argument at address bp */
#define GET_ARG_FLOAT(bp) (*(float *)(bp))

/* Get string address at some offset from ebp */
#define GET_NEXT_STRING_OFFSET(bp,offset) ((unsigned int*)bp + offset)

/* Get string at some offset from ebp */
#define GET_NEXT_STRING_ADDR(bp,offset) (*((unsigned int*)bp + offset))

/* Dereference address bp. Used to check if an address is valid */
#define DEREFERENCE_ADDR(bp) (*((unsigned int*)bp))

/* Get difference between addr1 and addr2 */
#define GET_ADDR_DIFF(addr1,addr2) ((unsigned int)addr1 - (unsigned int)addr2)

/* For syscall errors */
#define SYSCALL_RETURN_ERROR -1

#define SYSCALL_RETURN_SUCCESS 1

/* Function prototypes for internal helper routines */
int check_valid_frame(void *base_pointer);

int get_function_index(void *return_ptr);

int get_function_arguments(int func_index,int arg_num,void *base_pointer,
	                       char *buf,const char* arg_name,int arg_offset,
	                       int read_cnt);

int get_char_argument(void *base_pointer,int arg_offset,char *buf,
	                  const char *arg_name,int read_cnt);

int get_int_argument(void *base_pointer,int arg_offset,char *buf,
	                 const char *arg_name,int read_cnt);

int get_float_argument(void *base_pointer,int arg_offset,char *buf,
	                   const char *arg_name,int read_cnt);

int get_double_argument(void *base_pointer,int arg_offset,char *buf,
	                    const char *arg_name,int read_cnt);

int get_string_argument(void *base_pointer,int arg_offset,char *buf,
	                    const char *arg_name,int read_cnt);

int get_string_array(void *base_pointer,int arg_offset,char *buf,
	                 const char *arg_name,int read_cnt);

int get_void_pointer_argument(void *base_pointer,int arg_offset,char *buf,
	                          const char *arg_name,int read_cnt);

int get_unknown_argument(void *base_pointer,int arg_offset,char *buf,
	                     const char *arg_name,int read_cnt);

int build_string(char *str,char *buf,int read_cnt);

int is_string_printable(char *str);

int build_string_array(char *str,char *buf,int read_cnt);

int is_address_valid(void *addr);

int check_mem_addr_writtable(int *addr);

int install_SIGSEGV_handler(sigset_t *old_mask,struct sigaction *old_act_h);

void sighandler(int signum);

int restore_SIGSEGV_handler(sigset_t *old_mask,struct sigaction *old_act_h);

/* Global variables */
sigjmp_buf reg_state; /* For saving register state in setjmp and longjmp */
int setjmp_ret_value; /* Used for return value of setjmp */


/** @brief  Prints out the backtrace into the the file "fp"
 *
 *           The function does the following :
 *           1. Install the SIGSEGV handler to handle the segmentation
 *              faults due to accessing invalid addresses and save the current
 *              mask of the process so that it can be restored on return from
 *              traceback.
 *           2. Get the current base pointer. If the base pointer is 
 *              not a valid stack address, return with a FATAL error.
 *           3. Get the return address from the base pointer and check 
 *              if its a valid address. If not, return with a FATAL error.
 *           4. Comapre the previous function's base pointer with the current
 *              base pointer. If less, it's a condition for either a fatal
 *              error or termination of traceback. If the previous function's
 *              base pointer address is writtable, it's a FATAL condition else,
 *              its a condition for traceback to terminate as we have reached 
 *              a condition where we cannot backtrace further.
 *           5. If the result of step 4 is greater, make the current base 
 *              pointer = previous function's base pointer.
 *           6. Using the return address, find the function which it belongs to.
 *           7. Get the function name and arguments with the help of the 
 *              global function table and the current base pointer and write
 *              them into a buffer.
 *           8. Write the buffer contents into the file "fp".
 *           9  Continue steps 2 to 8 untill a termination condition is reached
 *              or a FATAl error is encountered.
 *          10. Restore the handler for SIGSEGV and the process mask before
 *              returning from traceback.
 *  @param  fp File to which the traceback is to be written to.
 *  @return Void.
 */
void traceback(FILE *fp)
{
  /* Obtain file descriptor associated with file fp */
  int fd;

  if((fd = Fileno(fp)) == -1)
  	return;
  
  sigset_t old_mask; /* for saving current signal mask */
  struct sigaction old_act_h; /* for saving old action for SIGSEGV handler */
  memset(&old_act_h,0,sizeof(old_act_h));

  /* install handler */
  if((install_SIGSEGV_handler(&old_mask,&old_act_h)) < 0)
  	return;

  void *base_pointer;     /* Points to current ebp */
  void *return_addr;      /* Return address */

  char *buf;              /* buffer for storing function name and args */
  const char *func_name;  /* function name */
  const char *arg_name;   /* argument name */

  int frame_invalid = 0;  /* Checks if stack frame is valid */
  int func_index = 0;     /* Function index for a given return address */
  int arg_offset;         /* Argument offset from ebp */
  int arg_num = 0;        /* Counter for argument number of a function */
  int read_cnt = 0;       /* Current position of buffer */

  /* Get inital ebp (traceback ebp) */
  base_pointer = get_initial_base_pointer();

  /* Allocate memory for buf */
  if ((buf = Malloc(MAX_BUF_SIZE)) == NULL)
  	return;

  /* Loop till base pointer is not NULL. I tried running a program that
   * called traceback in my local machine. I noticed that the ebp pushed
   * was NULL inside the main function. Thus, base_pointer becoming NULL
   * should be one of the termination conditions.
   */
  while(base_pointer != NULL)
  {
     
    /* Check to see if frame pointer is valid */
    if(check_valid_frame(base_pointer))
      return_addr = get_return_addr(base_pointer);

    else
    {
      frame_invalid = 1;
	  break;
	}

    /* Check to see if return address is valid */
    if(!check_valid_frame(return_addr)) 
    {
      frame_invalid = 1;
	  break;
	}

	/* old ebp > current ebp indicates well formed stack and backtrace
	 * should continue.
	 */
    if(((void *)GET_OLD_EBP(base_pointer)) > base_pointer)
	  base_pointer = (void *)GET_OLD_EBP(base_pointer);

    /* old ebp <= current ebp: 2 cases
     * 1. old ebp is writtable which means it is a fatal error. Maybe a 
     *    wrong ebp was pushed in one of the functions.
     * 2. old ebp is not writtable indicating termination condition. The 
     *    location is not writtable means that it can be the address of a
     *    code segment. I have seen in gdb that the ebp pushed just after
     *    entering main belongs to a code segment. I have checked this for
     *    multiple programs and seen that this is always the case.
     */
	else 
	{
	  /* Check if old_ebp is writtable */
      if(!check_mem_addr_writtable((int *)GET_OLD_EBP(base_pointer)))
      	/* old_ebp is not writtable indicating termination condition */
	    break;
      
      /*  old_ebp is writable indicating fatal error and traceback should
       *  stop.
       */
	  else 
	  {
	    frame_invalid = 1;
		break;
	  }

	}

    /* get function index from return address */
	func_index = get_function_index(return_addr);
    
    /* continue to next function if appropriate function is not found,
     * maybe because of the function size limit of 1 MB
     */
	if(func_index == -1) 
	{
	  snprintf(buf,MAX_BUF_SIZE,"Function %p(...), in\n",return_addr);

	  if(Write(fd,buf,strlen(buf)) < 0)
	  {
	  	Free(buf);
	    return;
	  }

	  memset(buf,0,MAX_BUF_SIZE);
	  continue;
	}

    /* Get function name */
	func_name = functions[func_index].name;
	/* get function name into the buffer. read_cnt gives number of
	 * bytes read.
	 */
	read_cnt = snprintf(buf,MAX_BUF_SIZE,"Function %s(",func_name);
    
    /* Initialize argument number for the function */
	arg_num = 0;
	/* Get argument name from function table */
	arg_name = functions[func_index].args[0].name;
	/* Get argument offset from ebp */
	arg_offset = functions[func_index].args[0].offset;

    /* Continue parasing till a function with zero length name is
     * encountered or number of args do not exceed 6.
     */
	while((strlen(arg_name) != 0) && arg_num < ARGS_MAX_NUM) 
	{

	  if(arg_num >= 1)
	    read_cnt += snprintf(buf+read_cnt,MAX_BUF_SIZE,", ");

	  /* Check if the stack address where arument is stored is valid */
	  if(is_address_valid(GET_ARG_OFFSET(base_pointer,arg_offset)))
	  {
	    read_cnt = get_function_arguments(func_index,arg_num,
		base_pointer,buf,arg_name,arg_offset,read_cnt);
	  }

      /* Stack address not accessible, hence terminate traceback with
       * fatal error
       */
	  else
	  {
	    snprintf(buf+read_cnt,MAX_BUF_SIZE,"), in\n");
	    frame_invalid = 1;
		break;
	  }

      /* Go to next argument */
	  arg_num++;
	  arg_name = functions[func_index].args[arg_num].name;
	  arg_offset = functions[func_index].args[arg_num].offset;

	}

    /* Print void if no arguments for a function */
	if(arg_num == 0)
	  read_cnt += snprintf(buf+read_cnt,MAX_BUF_SIZE,"void), in\n");

	else
	  read_cnt += snprintf(buf+read_cnt,MAX_BUF_SIZE,"), in\n");

    /* Write the function name with arguments to file */
	if(Write(fd,buf,strlen(buf)) < 0)
	{
	  Free(buf);
	  return;
	}

	memset(buf,0,MAX_BUF_SIZE);

  }

   /* Terminate traceback if frame is invalid */
  if(frame_invalid)
  {
  	read_cnt += snprintf(buf+read_cnt,MAX_BUF_SIZE,
  		                 "FATAL:Stack frame invalid/corrupt\n");
  }

  /* Free buffer */
  Free(buf);
  /* Restore signal handler for SIGSEGV and the original signal mask before
   * traceback was called
   */
  if((restore_SIGSEGV_handler(&old_mask,&old_act_h)) < 0)
  	return;
	
}

/** @brief  Get function index from the return address
 * 
 *          Compares the starting addresses obtained from the
 *          provided global function table with the return address
 *          and checks for the smallest positive difference between
 *          the two. The difference is then compared with the maximum
 *          function size constraint.  
 *  @param  return_addr The return address used for finding which
 *          function it belongs to.
 *  @return Int Returns function index is appropriate fit is found,
 *          else return -1.
 */
int get_function_index(void *return_addr)
{
  int func_index = 0;
  /* get the difference between the return address and starting address of
   * first function in the function table
   */
  void *temp = (void *)GET_ADDR_DIFF(return_addr,functions[0].addr);
  void *addr_diff;
  int i;

  /* loop through to find the smallest positive differece between return
   * address and the starting addresses of all the functions in the
   * function table.
   */
  for(i = 1; (strlen(functions[i].name) != 0) && (i < FUNCTS_MAX_NUM); i++) 
  {

    addr_diff = (void *)GET_ADDR_DIFF(return_addr,functions[i].addr);
	if((addr_diff < temp) && addr_diff > 0)
	{
	  temp = (void *)GET_ADDR_DIFF(return_addr,functions[i].addr);
	  func_index = i;
	}

  }

  /* Check for the maximum function size constraint of 1 MB */
  if((temp > 0) && ((unsigned int)temp < MAX_FUNCTION_SIZE_BYTES))
    return func_index;
  
  else
	return -1;

}

/** @brief  Get arguments of a function
 *
 *          Gets the arguments of a function into the passed argument "buf".
 *          It checks the argument type using a switch case and then calls
 *          the appropriate function to build the argument.
 *  @param  func_index The functions index in the global function table
 *          for which arguments have to be fetched.
 *  @param  arg_num The argument number of the function whose index is 
 *          func_index.
 *  @param  base_pointer Used as the base address to fetch arguments.
 *  @param  buf Buffer in which fetched arguments are to be stored.
 *  @param  arg_name Argument name obtained from function table.
 *  @param  arg_offset Offset from base pointer where argument is stored.
 *  @param  read_cnt used as offset from the buffer starting address to 
 *          write arguments into the buffer 
 *  @return Int Returns the number of bytes written into the buffer         
 */
int get_function_arguments(int func_index,int arg_num,void *base_pointer,
	char *buf,const char* arg_name,int arg_offset,int read_cnt)
{
  /* Jump to case label depending on which the argument is. For eg, if the
   * argument is of type "int", the control will be transferred to the
   * case TYPE_INT and a call to get_int_argument will be made to fetch 
   * the integer.
   */
  switch(functions[func_index].args[arg_num].type) 
  {
    case TYPE_CHAR: 
	  read_cnt = get_char_argument(base_pointer,arg_offset,buf,
	  	                           arg_name,read_cnt);
	  break;

	case TYPE_INT: 
	  read_cnt = get_int_argument(base_pointer,arg_offset,buf,
	                              arg_name,read_cnt);
	  break;

	case TYPE_FLOAT: 
	  read_cnt = get_float_argument(base_pointer,arg_offset,buf,
	  	                            arg_name,read_cnt);
	  break;

	case TYPE_DOUBLE: 
	  read_cnt = get_double_argument(base_pointer,arg_offset,buf,
	  	                             arg_name,read_cnt);
	  break;

	case TYPE_STRING:
	  read_cnt = get_string_argument(base_pointer,arg_offset,buf,
	  	                             arg_name,read_cnt);
	  break;

	case TYPE_STRING_ARRAY: 
	  read_cnt = get_string_array(base_pointer,arg_offset,buf,
	  	                          arg_name,read_cnt);
	  break;

	case TYPE_VOIDSTAR: 
	  read_cnt = get_void_pointer_argument(base_pointer,arg_offset,buf,
	  	                                   arg_name,read_cnt);
	  break;

	case TYPE_UNKNOWN:
	  read_cnt = get_unknown_argument(base_pointer,arg_offset,buf,
	                                  arg_name,read_cnt);
	  break; 
  }

  return read_cnt;

}

/** @brief  Get character

            Get character from  *(base_pointer+offset). If the
 *          character is not printable, print it in the octal form.
 *  @param  base_pointer Used as the base address to fetch arguments.
 *  @param  buf Buffer in which fetched arguments are to be stored.
 *  @param  arg_name Argument name obtained from function table.
 *  @param  arg_offset Offset from base pointer where argument is stored.
 *  @param  read_cnt used as offset from the buffer starting address to 
 *          write arguments into the buffer 
 *  @return Int Returns the number of bytes written into the buffer.
 */
int get_char_argument(void *base_pointer,int arg_offset,char *buf,
	                  const char *arg_name,int read_cnt)
{
  int character;
  /* Get the character from ebp+offset */
  character = GET_ARG(GET_ARG_OFFSET(base_pointer,arg_offset));
  /* Check if character is printable. If not, print it in octal form */
  if(isprint(character))
    read_cnt += snprintf(buf+read_cnt,MAX_BUF_SIZE,"char %s='%c'",
	  	                 arg_name,character);
  else
    read_cnt += snprintf(buf+read_cnt,MAX_BUF_SIZE,"char %s='\\%o'",
	    	             arg_name,character);
  return read_cnt;
}

/** @brief  Get integer

            Get integer from  *(base_pointer+offset). 
 *  @param  base_pointer Used as the base address to fetch arguments.
 *  @param  buf Buffer in which fetched arguments are to be stored.
 *  @param  arg_name Argument name obtained from function table.
 *  @param  arg_offset Offset from base pointer where argument is stored.
 *  @param  read_cnt used as offset from the buffer starting address to 
 *          write arguments into the buffer 
 *  @return Int Returns the number of bytes written into the buffer.
 */
int get_int_argument(void *base_pointer,int arg_offset,char *buf,
	                 const char *arg_name,int read_cnt)
{
  /* Get the integer from ebp+offset into buffer */
  read_cnt += snprintf(buf+read_cnt,MAX_BUF_SIZE,"int %s=%d",
	    	           arg_name,
	    	           GET_ARG(GET_ARG_OFFSET(base_pointer,arg_offset)));
  return read_cnt;
}

/** @brief  Get float

            Get float argument from  *(base_pointer+offset). 
 *  @param  base_pointer Used as the base address to fetch arguments.
 *  @param  buf Buffer in which fetched arguments are to be stored.
 *  @param  arg_name Argument name obtained from function table.
 *  @param  arg_offset Offset from base pointer where argument is stored.
 *  @param  read_cnt used as offset from the buffer starting address to 
 *          write arguments into the buffer 
 *  @return Int Returns the number of bytes written into the buffer.
 */
int get_float_argument(void *base_pointer,int arg_offset,char *buf,
	                   const char *arg_name,int read_cnt)
{ 
  /* Get the float value from ebp+offset into buffer */
  read_cnt += snprintf(buf+read_cnt,MAX_BUF_SIZE,"float %s=%f",
	    	           arg_name,
	    	           GET_ARG_FLOAT(GET_ARG_OFFSET(base_pointer,
	    	           	                            arg_offset)));
  return read_cnt;
}

/** @brief  Get double

            Get double argument from  *(base_pointer+offset). 
 *  @param  base_pointer Used as the base address to fetch arguments.
 *  @param  buf Buffer in which fetched arguments are to be stored.
 *  @param  arg_name Argument name obtained from function table.
 *  @param  arg_offset Offset from base pointer where argument is stored.
 *  @param  read_cnt used as offset from the buffer starting address to 
 *          write arguments into the buffer 
 *  @return Int Returns the number of bytes written into the buffer.
 */
int get_double_argument(void *base_pointer,int arg_offset,char *buf,
	                   const char *arg_name,int read_cnt)
{
  /* Get the double value from ebp+offset into buffer */
  read_cnt += snprintf(buf+read_cnt,MAX_BUF_SIZE,"double %s=%f",
	    	           arg_name,
	    	           GET_ARG_FLOAT(GET_ARG_OFFSET(base_pointer,
	    	           	                            arg_offset)));
  return read_cnt;
}

/** @brief  Get string

            Get starting address of string from  *(base_pointer+offset) and
 *          then build the string by calling build_string.
 *  @param  base_pointer Used as the base address to fetch the string address
 *  @param  buf Buffer in which fetched arguments are to be stored.
 *  @param  arg_name Argument name obtained from function table.
 *  @param  arg_offset Offset from base pointer where argument address
 *          is stored.
 *  @param  read_cnt used as offset from the buffer starting address to 
 *          write arguments into the buffer 
 *  @return Int Returns the number of bytes written into the buffer.
 */
int get_string_argument(void *base_pointer,int arg_offset,char *buf,
	                    const char *arg_name,int read_cnt)
{
  char *string;
  /* Get the string address from ebp+offset*/
  string = (char *)GET_ARG(GET_ARG_OFFSET(base_pointer,arg_offset));
  read_cnt += snprintf(buf+read_cnt,MAX_BUF_SIZE,"char *%s=",
	    		       arg_name);
  /* Build string */
  read_cnt = build_string(string,buf,read_cnt);
  return read_cnt;
}

/** @brief  Get string array

            Get starting address of string array from 
 *          *(base_pointer+offset) and call build_string_array for 
 *          fetching the string elements of the array.
 *  @param  base_pointer Used as the base address to fetch the string address
 *  @param  buf Buffer in which fetched arguments are to be stored.
 *  @param  arg_name Argument name obtained from function table.
 *  @param  arg_offset Offset from base pointer where argument address
 *          is stored.
 *  @param  read_cnt used as offset from the buffer starting address to 
 *          write arguments into the buffer 
 *  @return Int Returns the number of bytes written into the buffer.
 */
int get_string_array(void *base_pointer,int arg_offset,char *buf,
	                 const char *arg_name,int read_cnt)
{
  char *string;
  /* Get the address of the string array from ebp+offset */ 
  string = (char *)GET_ARG(GET_ARG_OFFSET(base_pointer,arg_offset));
  read_cnt += snprintf(buf+read_cnt,MAX_BUF_SIZE,"char **%s=",arg_name);
  /* Check if address of the string array is valid */
  if(is_address_valid(string))
    read_cnt = build_string_array(string,buf,read_cnt);
  /* Address not valid/accessible, hence the strings in the array cannot be
   * printed. So, print its address.
   */
  else
  	read_cnt += snprintf(buf+read_cnt,MAX_BUF_SIZE,"{%p}",string);

  return read_cnt;	
}

/** @brief  Get void pointer

            Get void pointer argument from  *(base_pointer+offset). 
 *  @param  base_pointer Used as the base address to fetch arguments.
 *  @param  buf Buffer in which fetched arguments are to be stored.
 *  @param  arg_name Argument name obtained from function table.
 *  @param  arg_offset Offset from base pointer where argument is stored.
 *  @param  read_cnt used as offset from the buffer starting address to 
 *          write arguments into the buffer 
 *  @return Int Returns the number of bytes written into the buffer.
 */
int get_void_pointer_argument(void *base_pointer, int arg_offset, char *buf,
	                          const char *arg_name, int read_cnt)
{
  /* Get the pointer value from ebp+offset into buffer */
  read_cnt += snprintf(buf+read_cnt,MAX_BUF_SIZE,"void *%s=0v%x",
	    	           arg_name,
	    	           GET_ARG(GET_ARG_OFFSET(base_pointer,arg_offset)));
  return read_cnt;
}

/** @brief  Get unknown argument

            Get the unknown argument from  *(base_pointer+offset). 
 *  @param  base_pointer Used as the base address to fetch arguments.
 *  @param  buf Buffer in which fetched arguments are to be stored.
 *  @param  arg_name Argument name obtained from function table.
 *  @param  arg_offset Offset from base pointer where argument is stored.
 *  @param  read_cnt used as offset from the buffer starting address to 
 *          write arguments into the buffer 
 *  @return Int Returns the number of bytes written into the buffer.
 */
int get_unknown_argument(void *base_pointer,int arg_offset,char *buf,
	                     const char *arg_name,int read_cnt)
{
  /* Get the value of unknown argument from ebp+offset into buffer */
  read_cnt += snprintf(buf+read_cnt,MAX_BUF_SIZE,"UNKNOWN %s=%p", arg_name,
	    	           (void *)GET_ARG(GET_ARG_OFFSET(base_pointer,
	    	           arg_offset)));
  return read_cnt;
}

/** @brief  Build the string

            Read the string "str" into the buffer. It first checks if the
 *          string is printable. If not, it puts the string's address into
 *          the buffer. If printable, the constraint for maximum characters
 *          in a string is checked and accordingly the string is inserted
 *          into the buffer. While transferring the string from its location
 *          to the buffer, if we encounter a segmentation fault due to invalid
 *          address/memory, it is handled using the SIGSEGV handler, setjmp 
 *          and longjmp and the string is represented by its address location.           
 *  @param  base_pointer Used as the base address to fetch arguments.
 *  @param  buf Buffer in which fetched arguments are to be stored.
 *  @param  arg_name Argument name obtained from function table.
 *  @param  arg_offset Offset from base pointer where argument is stored.
 *  @param  read_cnt used as offset from the buffer starting address to 
 *          write arguments into the buffer 
 *  @return Int Returns the number of bytes written into the buffer.
 */
int build_string(char *str,char *buf,int read_cnt)
{
  /* Save all the registers including eip using setjmp here so that,
   * we can jump here in case, printing out the string results in a 
   * segmentation fault due to accessing invalid address/memory.
   * The current process signal mask will also be saved here as a non-zero
   * value is given as the second argument to sigsetjmp.
   */
  setjmp_ret_value = sigsetjmp(reg_state,SAVE_SIGNAL_MASK);

  if(!setjmp_ret_value)
  {
  	/* Come here if it's for the first time */
    if(is_string_printable(str))
    {
      /* Check if length is less than 25 characters */	
	  if(strlen(str) <= STRING_MAX_CHARACTERS)
	    read_cnt += snprintf(buf+read_cnt,MAX_BUF_SIZE,"\"%s\"",str);
	  else
	  {
	  	/* If the string's length is more than 25 characters, print out
	  	 * only the first 25 characters.
	  	 */
	   /* define a buffer of size 26 to add terminating character*/
	    char temp_buf[STRING_MAX_CHARACTERS + 1];
		memset(temp_buf,0,sizeof(temp_buf));
		/* copy first 25 chracters */
		strncpy(temp_buf,str,STRING_MAX_CHARACTERS);
		/* add terminating character */
		temp_buf[STRING_MAX_CHARACTERS] = '\0';
		read_cnt += snprintf(buf+read_cnt,MAX_BUF_SIZE,
			                 "\"%s...\"",temp_buf);
	  }
	}
	else
	 /* Come here if the string was not printable, thus print its address */
	  read_cnt += snprintf(buf+read_cnt,MAX_BUF_SIZE,"%p",str);
  }
  else
  	/* Come here if a segmentation fault was encountered while fetching the
  	 * string from its address to the buffer; print the string's address 
  	 * in this case.
  	 */
    read_cnt += snprintf(buf+read_cnt,MAX_BUF_SIZE,"%p",str);

  return read_cnt;

}

/** @brief  Check if string is printable
 *
 *          Checks if each character in a string is printable using the 
 *          isPrint function defined in ctypes.h
 *  @param  str check if each character in the string "str" is printable
 *  @return Int Returns 1 if all character are printable, else 0.
 */
int is_string_printable(char *str)
{
  int i;
  int flag_printable = 1;
  
  for(i = 0; i < (strlen(str)); i++)
  {
    if(!isprint(str[i]))
    {
      /* Reset the flag even if one character is not printable and break */
	  flag_printable = 0;
	  break;
	}

  }

  return flag_printable;

}

/** @brief  Build string array
 *
 *          Takes the argument "str" as the base address of the string
 *          array. It then examines each of the string elements in the 
 *          array to check if the address is valid. If valid, it calls
 *          build_string else the address of the string location is put in buf.
 *  @param  str Base address of the string array.
 *  @param  buf Buffer in which fetched arguments are to be stored.
 *  @param  read_cnt used as offset from the buffer starting address to 
 *          write arguments into the buffer 
 *  @return Int Returns the number of bytes written into the buffer.
 */
int build_string_array(char *str,char *buf,int read_cnt)
{
  char *temp_str;
  int string_count = 0;

  read_cnt += snprintf(buf+read_cnt,MAX_BUF_SIZE,"{");

  /* Loop till we encounter more than 3 strings or a NULL pointer in the 
   * string array.
   */
  while(string_count != STRING_ARRAY_MAX_STRINGS)
  {
  	/* Check if address of the string located at address *(str + argc)
  	 * valid/accessible.
  	 */
  	if(is_address_valid(GET_NEXT_STRING_OFFSET(str,string_count)))
      temp_str = (char *)GET_NEXT_STRING_ADDR(str,string_count);

    else
    { /* If address not accessible, store the string address in the buffer */
      read_cnt += snprintf(buf+read_cnt,MAX_BUF_SIZE,"%p",
      	                   GET_NEXT_STRING_OFFSET(str,string_count));
    }

    /* Treat null pointer as an indication that the string array has come
     * to an end.
     */
	if(temp_str == NULL)
	  break; 

	if(string_count != 0)
	  read_cnt += snprintf(buf+read_cnt,MAX_BUF_SIZE,",");

    /* Build string */
	read_cnt = build_string(temp_str,buf,read_cnt);
	string_count++;

  }
  
  /* Check if the next string(4th string in the array) is accessible if there
   * are more than 3 strings
   */
  if(is_address_valid(GET_NEXT_STRING_OFFSET(str,string_count)))
    temp_str = (char *)GET_NEXT_STRING_ADDR(str,string_count);

  else
  	temp_str = NULL;

  if(string_count == STRING_ARRAY_MAX_STRINGS && temp_str != NULL)
  	/* There are more than 3 strings in the string array */
    read_cnt += snprintf(buf+read_cnt,MAX_BUF_SIZE,", ...");

  read_cnt += snprintf(buf+read_cnt,MAX_BUF_SIZE,"}");

  return read_cnt;

}

/** @brief  Check if stack frame is valid
 * 
 *          Checks if the stack frame is valid by dereferencing base pointer.
 *  @param  base_pointer Check if base_pointer address is valid or not.
 *  @return Int Returns 1 the frame is valid.
 */
int check_valid_frame(void *base_pointer)
{
  /* If address is valid, return 1 else 0 */
  if(is_address_valid(base_pointer))
  	return 1;
  else
	return 0;
}

/** @brief  Checks if a given address is valid by dereferencing it.
 *
 *          This function is used to detect corrupt retrurn addresses or
 *          corrupt stack addresses. This function helps to determine the FATAL
 *          error condition.
 *
 *  @param  addr Address for checking validity
 *  @return Int Returns 1 the address is valid.
 */
int is_address_valid(void *addr)
{
  int value;
  /* Save all the registers including eip using setjmp here so that,
   * we can jump here in case, accessing the memory location "addr"
   * results in a segmentation fault as its not accessible.
   * The current process signal mask will also be saved here as a non-zero
   * value is given as the second argument of sigsetjmp.
   */
  setjmp_ret_value = sigsetjmp(reg_state,SAVE_SIGNAL_MASK);
  if(!setjmp_ret_value)
    {
    	/* Come here if it's for the first time. Dereference addr to check
    	 * if its accessible.
    	 */
		value = DEREFERENCE_ADDR(addr);
		/* Initializing value to avoid compilation warning in C99 mode */
		value = value;
	}
  else
  	/* Come here if a segmentation fault was encountered while accessing 
  	 * memory location "addr"
  	 */
    return 0;

  return 1;
}

/** @brief  Checks if a given address is writtable
 *
 *          This function is used to check for a FATAL or a termination
 *          condition. 
 *          1. FATAL : Address is writtable. Maybe a stack address that 
 *             does not belong to a stack frame was pushed into the stack. 
 *             This means that the stack frame was corrupted and traceback
 *             cannot continue further.
 *          2. TERMINATION : Address is not writtable indicating termination
 *             condition. The location is not writtable means that it can be
 *             the address of a code segment. I have seen in gdb that the ebp
 *             pushed just after entering main belongs to a code segment. This
 *             should be an indication that we cannot continue with traceback 
 *             and it is a good termination point.
 *  @param  addr Address for checking is it is writtable
 *  @return Int Returns 1 the address can be written to else return 0.
 */
int check_mem_addr_writtable(int *addr)
{ 
  /* Save all the registers including eip using setjmp here so that,
   * we can jump here in case, writing into the memory location "addr"
   * results in a segmentation fault as its not writtable.
   * The current process signal mask will also be saved here as a non-zero
   * value is given as the second argument of sigsetjmp.
   */
  setjmp_ret_value = sigsetjmp(reg_state,SAVE_SIGNAL_MASK);
  if(!setjmp_ret_value)
  { 
    /* Come here if it's for the first time. Writing a dummy value of 10
     * at addr to check if its writtable
     */
    *addr = DUMMY_VALUE;
	return 1;
  }

  else 
  	/* Come here if a segmentation fault was encountered while writing to 
  	 * memory location "addr"
  	 */
    return 0;
}

/** @brief  Install SIGSEGV handler and save original mask
 *
 *          Install handler for SIGSEGV using sigaction. It saves the old 
 *          action associated with SIGSEGV so that it can restored on 
 *          returning from traceback. It also unblocks SIGSEGV from the current
 *          process mask to ensure SIGSEGV does not exist in the blocked set
 *          of the process.
 *  @param  old_mask Mask to save the blocked set of the process for
 *          restoring it later while returning from traceback.
 *  @param  old_act_h To save the old action associated with SIGSEGV. 
 *  @return Int returns -1 if any syscall error is encountered.
 */
int install_SIGSEGV_handler(sigset_t *old_mask,struct sigaction *old_act_h)
{
  int syscall_ret_error;

  struct sigaction act_h;
  memset(&act_h,0,sizeof(act_h));

  /* define mask while SIGSEGV handler will run. This mask contains all
   * signals so that no signal can interrupt while the SIGSEGV handler 
   * is running.
   */
  sigset_t handler_mask;
  /* Fill set with all signals */
  if((syscall_ret_error = Sigfillset(&handler_mask)) < 0)
  	return SYSCALL_RETURN_ERROR;
  /* save the old action associated with SIGSEGV */
  if((syscall_ret_error = Sigaction(SIGSEGV,NULL,old_act_h)) < 0)
  	return SYSCALL_RETURN_ERROR;
  /* sighandler is the handler that will run in case of a segmentation fault */
  act_h.sa_handler = sighandler;
  /* define mask to block all signals when handler runs */
  act_h.sa_mask = handler_mask;
  /* Install the SIGSEGV handler */
  if((syscall_ret_error = Sigaction(SIGSEGV,&act_h,NULL)) < 0)
  	return SYSCALL_RETURN_ERROR;

  /* Unblock SIGSEGV from the current signal mask in case it was blocked
   * so that SIGSEGV handler can run in case of a segfault
   */
  sigset_t mask;

  if((syscall_ret_error = Sigemptyset(&mask)) < 0)
  	return SYSCALL_RETURN_ERROR;

  if((syscall_ret_error = Sigaddset(&mask,SIGSEGV)) < 0)
  	return SYSCALL_RETURN_ERROR;

  /* Unblock SIGSEGV */
  if((syscall_ret_error = Sigprocmask(SIG_UNBLOCK,&mask,old_mask)) < 0)
  	return SYSCALL_RETURN_ERROR;

  return SYSCALL_RETURN_SUCCESS;
}

/** @brief  SIGSEGV handler
 *
 *          Handler for SIGSEGV. In case of a segmentation fault, it
 *          jumps to the location specified by reg_state. In the process,
 *          the registers as well as the signal mask are restored.
 *  @param  signum signal number associated with SIGSEGV
 *  @return Void.
 */
void sighandler(int signum)
{ 
  /* Long jump in case of a segmentation fault so that the situation can be
   * handled. All the registers including the signal mask will be restored.
   */
  siglongjmp(reg_state,SETJMP_RET_VALUE_HANDLER);
}

/** @brief  Restore SIGSEGV handler and original mask
 *
 *          Restore the  SIGSEGV handler and the process mask on 
 *          returning from traceback.
 *  @param  old_mask Mask to retrieve the blocked set of the process for
 *  @param  old_act_h To retrieve the old action associated with SIGSEGV. 
 *  @return Int returns -1 if any syscall error is encountered.
 */
int restore_SIGSEGV_handler(sigset_t *old_mask,struct sigaction *old_act_h)
{
  int syscall_ret_error;
  /* restore the original signal mask on return from traceback */
  if((syscall_ret_error = Sigprocmask(SIG_SETMASK,old_mask,NULL)) < 0)
  	return SYSCALL_RETURN_ERROR;
  /* restore the old action associated with SIGSEGV */
  if((syscall_ret_error = Sigaction(SIGSEGV,old_act_h,NULL)) < 0)
  	return SYSCALL_RETURN_ERROR;

  return SYSCALL_RETURN_SUCCESS;
}




