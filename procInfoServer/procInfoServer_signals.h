/*
 *
 * Copyright 2012. Antonino N. Mione
 *
 */
/*
 * procInfoServer_signals.h
 */


#ifndef __procInfoServer_signals_h
#define __procInfoServer_signals_h

#include <signal.h>

void catch_chld(int signal_num, siginfo_t *info, void *ptr);
void register_signals(void);

#endif /* __procInfoServer_signals_h */

