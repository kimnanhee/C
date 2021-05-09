/*
 *
 * Copyright 2012. Antonino N. Mione
 *
 */
/*
 * procInfoServer_util.h
 */

#ifndef __procInfoServer_util_h
#define __procInfoServer_util_h

#define authfile_min_line_len 3

int readline(FILE *infile, char *outbuffer, int *length);
int get_procs(void);
float time2float(char *time_string);
int match_user(char *line, char *user);
int match_uid(char *line, int req_uid);


#endif /* __procInfoServer_util_h */
