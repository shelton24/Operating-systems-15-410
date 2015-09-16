/** @file console_device_driver.c 
 *
 *  @brief Console Device Driver Interface
 *  This file contains the functions required to build the device driver
 *  for the console
 *
 *  @author Shelton Dsouza (sdsouza)
 *  @bug No known bugs
 */

#include <p1kern.h>    /* Contains function definitions written here */
#include <asm.h>       /* register manipulation */
#include <simics.h>    /* Sim breakpoints */
#include <string.h>
#include <console_device_driver.h>
#include <stdlib.h>

#define SUCCESS 1
#define FAILURE 0

#define CURSOR_HIDE_CONSTANT (CONSOLE_WIDTH * CONSOLE_HEIGHT)

#define CURSOR_HIDE_ROW CONSOLE_HEIGHT

#define CURSOR_HIDE_COL CONSOLE_WIDTH

#define CURSOR_HIDE_ADD(bp) (bp + CURSOR_HIDE_CONSTANT)

#define CURSOR_HIDE_SUB(bp) (bp - CURSOR_HIDE_CONSTANT)

#define GET_CURSOR_POS(row,col) (row * CONSOLE_WIDTH + col)

#define IO_PORT_WIDTH (1 << 8)

#define SCROLL_CONSOLE_BUFFER_LENGTH 2 * (CONSOLE_WIDTH * (CONSOLE_HEIGHT - 1))

#define GET_NEXT_ROW(addr) (addr + (2 * CONSOLE_WIDTH))

#define GET_LAST_ROW(addr) (addr + SCROLL_CONSOLE_BUFFER_LENGTH)

#define CONSOLE_END (CONSOLE_MEM_BASE + (2 * (CONSOLE_WIDTH * CONSOLE_HEIGHT)))


int term_color = FGND_WHITE;
int cursor_hidden = 0;
char *scroll_console_buffer;

int putbyte( char ch )
{

	int row,col;  /* For getting the curren cursor posotion */


	get_cursor(&row,&col); 

  if(!check_special_characters(ch,row,col))
  {
		print_char(ch,row,col);
  	inc_cursor_position(row,col);
  }

  return 0; 
}

int get_actul_index(int row,int col)
{
	int index;
	index = GET_CURSOR_POS(row,col);
	index = CURSOR_HIDE_SUB(index);
	return index;
}

void print_char(char ch,int row,int col)
{
	int index;
	if(cursor_hidden)
	{
		index = get_actul_index(row,col);
		row = index/80;
		col = index%80;
	}
	*(char *)(CONSOLE_MEM_BASE + 2*(row*CONSOLE_WIDTH + col)) = ch; 
	*(char *)(CONSOLE_MEM_BASE + 2*(row*CONSOLE_WIDTH + col) + 1) = term_color;
}

void 
putbytes( const char *s, int len )
{
	int i;

  for(i = 0; i < len; i++)
  {
  	putbyte(s[i]);
  }

}

int
set_term_color( int color )
{
	term_color = color;
  return 0;
}

void
get_term_color( int *color )
{
	*color = term_color;
}

int
set_cursor( int row, int col )
{
	int index = get_cursor_location(row,col);
	
	send_data_IO_port(index);

  return 0;
}

void
get_cursor( int *row, int *col )
{
	int temp;
	outb(CRTC_IDX_REG,CRTC_CURSOR_LSB_IDX);
	temp = inb(CRTC_DATA_REG);
	outb(CRTC_IDX_REG,CRTC_CURSOR_MSB_IDX);
	temp = (inb(CRTC_DATA_REG) << 8) | temp;
	*row = temp / CONSOLE_WIDTH;
	*col = temp % CONSOLE_WIDTH;
}

void
hide_cursor()
{
	cursor_hidden = 1;
	int row,col;
	get_cursor(&row,&col);
  int index = GET_CURSOR_POS(row,col);
  index = CURSOR_HIDE_ADD(index);
	adjust_cursor_position(index);

}

void
show_cursor()
{
	if(!cursor_hidden)
		return;
	cursor_hidden = 0;
	int row,col;
	int index;
	get_cursor(&row,&col);
	index = GET_CURSOR_POS(row,col);
	index = CURSOR_HIDE_SUB(index);
	adjust_cursor_position(index);
}

void 
clear_console()
{
	int index = 0;
	remove_characters();
	if(cursor_hidden)
		index = CONSOLE_WIDTH * CONSOLE_HEIGHT;
	adjust_cursor_position(index);
}

void remove_characters()
{
	int i;
	for(i = CONSOLE_MEM_BASE; i < CONSOLE_END; i+=2)
	{
		*(char *)i = 0x00;
	}
}

void
draw_char( int row, int col, int ch, int color )
{
	*(char *)(CONSOLE_MEM_BASE + 2*(row*CONSOLE_WIDTH + col)) = ch; 
	*(char *)(CONSOLE_MEM_BASE + 2*(row*CONSOLE_WIDTH + col) + 1) = color;
}

char
get_char( int row, int col )
{
	char ch;

	ch = *(char *)(CONSOLE_MEM_BASE + 2*(row*CONSOLE_WIDTH + col)); 
  return ch; 

}

int check_special_characters(char ch,int row,int col)
{
	int special_char_flag;

	switch(ch)
	{
		case '\n':
			move_cursor_next_line(row);
			special_char_flag = SUCCESS;
			break;

		case '\r':
      move_cursor_line_start(row);
			special_char_flag = SUCCESS;
			break;

		case '\b':
		  print_char(' ',row,col - 1);
		  dcr_cursor_position(row,col);
			special_char_flag = SUCCESS;
			break;

		default:
			special_char_flag = FAILURE;

	}

	return special_char_flag;
}

void inc_cursor_position(int row,int col)
{
	int index = GET_CURSOR_POS(row,col) + 1;
	adjust_cursor_position(index);

}

void move_cursor_next_line(int row)
{
	int index = ((row+1) * CONSOLE_WIDTH);
	adjust_cursor_position(index);
}

void move_cursor_line_start(int row)
{
	int index = (row * CONSOLE_WIDTH);
	adjust_cursor_position(index);
}

void dcr_cursor_position(int row,int col)
{
	int index = (row * CONSOLE_WIDTH) + col - 1;
	adjust_cursor_position(index);
}

void adjust_cursor_position(int index)
{
	if(cursor_hidden)
	{
		if(index >= 2 * (CONSOLE_WIDTH * CONSOLE_HEIGHT))
		{
			scroll_console();
			index = SCROLL_CONSOLE_BUFFER_LENGTH + CONSOLE_WIDTH;
		}
	}

	else
	{
		if(index >= (CONSOLE_WIDTH * CONSOLE_HEIGHT))
		{
			scroll_console();
			index = CONSOLE_WIDTH * (CONSOLE_HEIGHT - 1);
		}
	}

	send_data_IO_port(index);
}

int get_cursor_location(int row,int col)
{
	int index = GET_CURSOR_POS(row,col);

	if(!cursor_hidden)
		return index;
	else
		return CURSOR_HIDE_ADD(index);
}

void send_data_IO_port(int index)
{
	outb(CRTC_IDX_REG,CRTC_CURSOR_LSB_IDX);
	outb(CRTC_DATA_REG,index % IO_PORT_WIDTH);
	outb(CRTC_IDX_REG,CRTC_CURSOR_MSB_IDX);
	outb(CRTC_DATA_REG,index / IO_PORT_WIDTH);
}

void scroll_console()
{
	scroll_console_buffer = malloc(SCROLL_CONSOLE_BUFFER_LENGTH);

	char *mem_area_second_row = (char *) GET_NEXT_ROW(CONSOLE_MEM_BASE);
	char *mem_area_start = (char *)CONSOLE_MEM_BASE;

	memcpy(scroll_console_buffer,mem_area_second_row,
		     SCROLL_CONSOLE_BUFFER_LENGTH);
	memcpy(mem_area_start,scroll_console_buffer,
		     SCROLL_CONSOLE_BUFFER_LENGTH);
	free(scroll_console_buffer);
	
	unsigned int mem_area_lastrow = GET_LAST_ROW(CONSOLE_MEM_BASE);
	clear_console_row(mem_area_lastrow);
}

void clear_console_row(unsigned int addr)
{
	unsigned int i;
	for(i = addr; i < addr + CONSOLE_WIDTH; i+=2)
	{
		*(char *)i = 0x00;
	}
}
