/*
 * Copyright 2013. Antonino N. Mione
 *
 *
 * procInfoServer
 */

#ifndef __procInfoServer_h
#define __procInfoServer_h

/*
 * Set the size for return data buffer.
 */
#define big_buffersize 10000

/*
 * These are various constants for the size of hostnames, file names,
 * and line buffers.
 */

#define max_hname_len 80
#define max_fname_len 80
#define max_line_len 1024
#define max_file_line_len 1024
#define max_buffer_line_length 1024

/*
 * Read up to this many records from the .authFile
 */
#define max_auth_records 20

struct auth_record {
  char user_name[80];
  char cookie_string[80];
};

#define false 0
#define true 1

#define ERR (-1)

#endif /* #ifndef procInfoServer */

