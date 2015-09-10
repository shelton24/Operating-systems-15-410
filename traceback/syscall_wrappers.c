/** @file syscall_wrappers.c
 *  @brief Syscall wrappers 
 *
 *  This file contains the syscall wrappers required by traceback.c
 *
 *  @author Shelton Dsouza (sdsouza)
 *  @bug No known bugs
 */

/* -- Includes -- */

#include <syscall_wrappers.h>

#include <stdlib.h> /* Wrapper for malloc */
#include <signal.h> /* Wrapper for sigaction,sigaddset,... */
#include <unistd.h> /* Wrapper for write */
#include <stdio.h>  /* Wrapper for Fileno */

/* Error handling function */
void unix_error(char *message) 
{
  fprintf(stderr, "MSG: %s\n", message);
}

/* Wrapper for Malloc*/
void *Malloc(size_t size) 
{
  void *p;

  if ((p  = malloc(size)) == NULL)
    unix_error("Traceback: Malloc error");
  return p;

}

/* Wrapper for free */
void Free(void *ptr) 
{
    free(ptr);
}


/* Wrapper for making a set empty */
int Sigemptyset(sigset_t *set)
{
  int rc;
  if ((rc = sigemptyset(set)) < 0)
    unix_error("Traceback: Sigemptyset error");
  return rc;
}

/* Wrapper for installing a handler */
int Sigaction(int signum, const struct sigaction *act,struct sigaction *oldact)
{
  int rc;
  if ((rc= sigaction(signum,act,oldact)) < 0)
    unix_error("Traceback: Sigaction error");
  return rc;
}

/* Wrapper for setting the process mask */
int Sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{ 
  int rc;
  if ((rc = sigprocmask(how, set, oldset)) < 0)
    unix_error("Traceback: Sigprocmask error");
  return rc;
}

/* Wrapper for filling a set with all signals */
int Sigfillset(sigset_t *set)
{ 
  int rc; 
  if ((rc = sigfillset(set) < 0))
    unix_error("Traceback: Sigfillset error");
  return rc;
}


/* Wrapper for adding a signal to a set */
int Sigaddset(sigset_t *set, int signum)
{ 
  int rc;
  if ((rc = sigaddset(set, signum)) < 0)
    unix_error("Traceback: Sigaddset error");
  return rc;
}


/* Wrapper for deleting a signal from a set */
int Sigdelset(sigset_t *set, int signum)
{
  int rc;
  if ((rc = sigdelset(set, signum)) < 0)
    unix_error("Traceback: Sigdelset error");
  return rc;
}


/* Wrapper for checking if a signal is member of a set */
int Sigismember(const sigset_t *set, int signum)
{
  int rc;
  if ((rc = sigismember(set, signum)) < 0)
    unix_error("Traceback: Sigismember error");
  return rc;
}

/* Wrapper for obtaining file descriptor */
int Fileno(FILE *fp)
{
  int fd;
  if((fd = fileno(fp)) < 0)
    unix_error("Traceback: FIleno error");
  return fd;
}

/* Wrapper for write */
int Write(int fd, const void *buf, size_t count) 
{
  int rc;
  if ((rc = write(fd, buf, count)) < 0)
    unix_error("Traceback: Write error");
  return rc;
}




