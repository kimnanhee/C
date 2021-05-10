/*
 *
 * Copyright 2012, 2013. Antonino N. Mione
 *
 */
/*
 * procInfoServer_signals.c
 */

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
// #include <malloc.h>
#include "procInfoServer.h"
#include "procInfoServer_signals.h"

struct sigaction act;

/*
 * function: catch_chld - This function is used to handle signals.
 *    Specifically, it catches the signal sent when a child dies
 *    so that it can  call wait() and allow the child to properly terminate.
 *
 * parameters: signal_num (int) - The number of the generated signal.
 *             info (siginfo_t *) - The populated information structure
 *                 giving details about the signal that occurred.
 *             ptr (void *) - Not used
 *             
 *
 * return: None.
 *
 */
void catch_chld(int signal_num, siginfo_t *info, void *ptr)
{
   int status;

   /*
    * Note which signal occurred, wait for the child, and indicate the
    * returned status from the child.
    */
   wait(&status);
}

/*
 * function: register_signals - This function registers handler routines
 *    for signals that the application wants to catch. Currently, it is
 *    hardcoded to register a signal handler for SIGCHLD only.
 *
 * parameters: None.
 *
 * return: None.
 *
 */
void register_signals(void)
{
   act.sa_sigaction = catch_chld;
   act.sa_flags = SA_SIGINFO;

   sigaction(SIGCHLD, &act, NULL);
}
