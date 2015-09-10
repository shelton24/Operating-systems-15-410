/** @file syscall_wrappers.h
 *  @brief Wrappers for error handling
 *  
 *  This includes functions for masking signals,allocating
 *  memory, writing to a file...
 *
 *  @author Shelton Dsouza (sdsouza)
 */

#ifndef __SYSCALL_WRAPPERS_H
#define __SYSCALL_WRAPPERS_H

/* -- Includes -- */

#include <signal.h>
#include <stdio.h>

/* Function prototypes */

void unix_error(char* msg); /* Wrapper for error handling message */

void *Malloc(size_t size);  /* Allocating memory */

void Free(void *ptr); /* Free ptr */

int Sigemptyset(sigset_t *set); /* Make an empty set */

/* Install handler */
int Sigaction(int signum, const struct sigaction *act,
	           struct sigaction *oldact);

/* Changing the signal mask of a process */
int Sigprocmask(int how, const sigset_t *set, sigset_t *oldset);

/* Fill set with all signals */
int Sigfillset(sigset_t *set);

/* Add signal signum to set */
int Sigaddset(sigset_t *set, int signum);

/* Delete signal signum from set */
int Sigdelset(sigset_t *set, int signum);

/* Verify if signal signum is a member of set */
int Sigismember(const sigset_t *set, int signum);

/* Obtain descriptor associated with file fp */
int Fileno(FILE *fp);

/* Write count bytes to file whose descriptor is fd */
int Write(int fd, const void *buf, size_t count);

#endif

#ifdef DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...) ((void) 0)
#endif
