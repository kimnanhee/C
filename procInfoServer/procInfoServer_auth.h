/*
 *
 * Copyright 2012. Antonino N. Mione
 *
 */
/*
 * procInfoServer_auth.c
 */


#ifndef __procInfoServer_auth_h
#define __procInfoServer_auth_h


void read_auth_records(void);
char * get_cookie(char *user);
int check_auth (char *cookie);

#endif /* __procInfoServer_auth.h */
