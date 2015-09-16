/** @file syscall_wrappers.h
 *  @brief Wrappers for error handling
 *  
 *  This includes functions for masking signals,allocating
 *  memory, writing to a file...
 *
 *  @author Shelton Dsouza (sdsouza)
 */

#ifndef __CONSOLE_DEVICE_DRIVER_H
#define __CONSOLE_DEVICE_DRIVER_H

void print_char(char ch,int row,int col);

int check_special_characters(char ch,int row,int col);

void inc_cursor_position(int row,int col);

void move_cursor_next_line(int row);

void move_cursor_line_start(int row);

void dcr_cursor_position(int row,int col);

void adjust_cursor_position(int index);

int get_cursor_location(int row,int col);

void send_data_IO_port(int index);

void remove_characters();

void scroll_console();

void clear_console_row(unsigned int addr);

int get_actul_index(int row,int col);

#endif
