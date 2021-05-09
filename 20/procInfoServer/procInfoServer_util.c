/*
 *
 * Copyright 2012, 2013. Antonino N. Mione
 *
 */
/*
 * procInfoServer_util.c
 */

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
// #include <malloc.h>
#include <regex.h>
#include <pwd.h>

#include "procInfoServer.h"
#include "procInfoServer_util.h"

/*
 * Return pointer for temporary file names (output of ps -aef command)
 */
char *tmpFile = 0;

/*
 * function: readline - This function reads data from a file up to a newline
 *              and returns the line, resetting the file to the character
 *              after the newline.
 *
 * parameters: infile (FILE *) - This is the file handle for the file from
 *                which to read the data.
 *             outbuffer (char *) - This is a buffer into which the data is
 *                to be written.
 *             length (int *) - This is the length of the buffer before the
 *                call and the count of character written to the buffer
 *                after the call.
 *
 * return: int - This is a boolean that is either true(1) if data was 
 *            read or false(0) if the function failed.
 *
 */
int readline(FILE *infile, char *outbuffer, int *length)
{
   static int charsread = 0;        /* The number of characters read */
   char rbuffer[max_file_line_len]; /* This is a buffer to hold the line */
   int buf_idx = 0;                 /* Temporary index for searching 
                                       for line termination. */
   int rcount;                      /* */
   int res = false;                 /* Result to be returned */

   /*
    * Read a bunch of characters. Hopefully, lines are shorter than
    * this. Note that we read objects of size '1' and read a
    * 'line-length' worth of these characters.
    */
   rcount = fread(rbuffer, 1, max_file_line_len, infile);
   if (rcount > 0)
   {
      /* 
       * The read was successful. The code received 'rcount' characters.
       */
      while ((rbuffer[buf_idx] != '\n') &&
             (buf_idx < rcount))
      {
         /*
          * Copy the data until there is no room or the code reaches
          * a newline character.
          */
         outbuffer[buf_idx] = rbuffer[buf_idx];
         buf_idx++;
      }
      if ((buf_idx < rcount) && (rbuffer[buf_idx] == '\n'))
      {
         /*
          * Make sure the newline is copied into the output
          * buffer. Then make sure the file input pointer
          * is set to the next character in the file for the
          * following read.
          */
         outbuffer[buf_idx] = '\n';
         fseek(infile, -(rcount - buf_idx - 1), SEEK_CUR);

         /*
          * Assure that the buffer is terminated with a null and
          * set the length and return value.
          */
         outbuffer[buf_idx+1] = '\0';
         *length = buf_idx;
	 if (buf_idx < 3)
	 {
	    /*
	     * Line is too short to be valid. Skip it and return false.
 	     */
            outbuffer[0] = '\0';
            *length = 0;
	    res = false;
	 }
	 else
	 {
	    res = true;
	 }
      }
      else 
      {
         /*
          * The read call failed. Place a null at the beginning of
          * the provided buffer and save 0 for the line length.
          */
         outbuffer[0] = '\0';
         *length = 0;
      }
   }
   else
   {
      res = false;
   }
   return res;
}

/*
 * function: get_procs - This function collects process information using
 *               the 'system()' call with the 'ps' command. It saves the
 *               output into a temporary file for later reading and parsing.
 *
 * parameters: None.
 *
 * return: int
 *
 */
int get_procs(void)
{
   char sys_string[max_line_len];
   int ps_command_res;

   /*
    * Generate a temporary file name.
    */
      tmpFile = tempnam("/tmp",".tmpprocs");
   //   tmpFile = tempnam(".",".tmpprocs");
  
   printf ("Tmpfile=|%s|\n", tmpFile);
   /*
    * Create the command string filling in the name of the temporary file.
    */
   //   sprintf (sys_string, "ps -aefo user:16,pid,ppid,state,stime,tname,time,cmd >%s", tmpFile);
   sprintf (sys_string, "ps -aef >%s", tmpFile);
   ps_command_res = system(sys_string);
  
   /*
    * Check the result of the system call. Note that if the result
    * is zero, it means there was no error and the call worked. In that
    * case, we return 1 (true). Otherwise, we return 0 (false)
    */
   if (ps_command_res == 0)
   {
      ps_command_res = 1;
   }
   else
   {
      ps_command_res = 0;
   }
   return ps_command_res;
}

/*
 * function: time2float - This is a utility routine that converts an
 *              ASCII string in the format of a time <m:ss.hh> and 
 *              transforms it into the number of seconds it represents.
 *
 * parameters: time_string (char *) - String that holds the time value
 *
 * return: float - converted value in seconds.
 *
 */
float time2float(char *time_string)
{
   /*
    * Local variables.
    */

   /*
    * This is the pattern to match a correctly formatted time string.
    */
   char *timePatternSource = "[  ]*([0-9]?[0-9]):([0-9][0-9])\\.([0-9][0-9])";
   char *timePattern2Source = "[  ]*([0-9]?[0-9]):([0-9][0-9]):([0-9][0-9])";
   char *timePattern3Source = "[  ]*([0-9]+)";
   size_t num_matches = 5;           /* Maximum matching fields to store  */
   regmatch_t matches[num_matches];  /* Stored matching string ranges     */
   regex_t comp_pattern;             /* Regular expression pattern struct */
   regex_t comp_pattern2;            /* Regular expression pattern struct */
   regex_t comp_pattern3;            /* Regular expression pattern struct */
   int eflags = 0;                   /* Error Flags                       */
   int retval;                       /* Return value                      */
   int hours;                        /* Hours from time string            */
   int mins;                         /* Minutes from time string          */
   int secs;                         /* Seconds from time string          */
   int hths;                         /* Hundreths of a second from time   */
   float req_time = 0.0;             /* Converted time value              */

   /*
    * Compile the regular expression held in timePatternSource.
    */
   retval = regcomp(&comp_pattern, timePatternSource, REG_EXTENDED);
   retval = regcomp(&comp_pattern2, timePattern2Source, REG_EXTENDED);
   retval = regcomp(&comp_pattern3, timePattern3Source, REG_EXTENDED);

   /*
    * If the RE compile worked.
    */
   if (retval == 0)
   {
      /*
       * Execute the pattern on the argument sent in by the user.
       */
      retval = regexec(&comp_pattern,
                       time_string,
                       num_matches,
                       matches,
                       eflags);
                   
      /*
       * A zero retval means the RE execution worked correctly.
       */
      if (retval == 0)
      {
         /*
          * Variables local to the 'if' scope.
          */
         char work_buffer[max_buffer_line_length];
         char temp_string[160];
         float line_time;

         /*
          * Make a copy of the time string.
          */
         strcpy(work_buffer, time_string);
         work_buffer[matches[1].rm_eo] = 0;
         work_buffer[matches[2].rm_eo] = 0;
         work_buffer[matches[3].rm_eo] = 0;
         sscanf (&work_buffer[matches[1].rm_so], "%d", &mins);
         sscanf (&work_buffer[matches[2].rm_so], "%d", &secs);
         sscanf (&work_buffer[matches[3].rm_so], "%d", &hths);
         req_time = mins * 60 + secs + ((float) hths / 100.0);
      }
      else
      {
         /*
          * Execute the pattern on the argument sent in by the user.
          */
         retval = regexec(&comp_pattern2,
                          time_string,
                          num_matches,
                          matches,
                          eflags);
          
         if (retval == 0)
         {

            /*
             * Variables local to the 'if' scope.
             */
            char work_buffer[max_buffer_line_length];
            char temp_string[160];
            float line_time;
              
            /*
             * Make a copy of the time string.
             */
            strcpy(work_buffer, time_string);
            work_buffer[matches[1].rm_eo] = 0;
            work_buffer[matches[2].rm_eo] = 0;
            work_buffer[matches[3].rm_eo] = 0;
            sscanf (&work_buffer[matches[1].rm_so], "%d", &hours);
            sscanf (&work_buffer[matches[2].rm_so], "%d", &mins);
            sscanf (&work_buffer[matches[3].rm_so], "%d", &secs);
            req_time = hours * 3600 + mins * 60 + secs;
         }
	 else
         {
         /*
          * Execute the pattern on the argument sent in by the user.
          */
            retval = regexec(&comp_pattern3,
                             time_string,
                             num_matches,
                             matches,
                             eflags);
          
	    if (retval == 0)
            {
               /*
                * Variables local to the 'if' scope.
                */
               char work_buffer[max_buffer_line_length];
               char temp_string[160];
               float line_time;
              
               /*
                * Make a copy of the time string.
                */
               strcpy(work_buffer, time_string);
               work_buffer[matches[1].rm_eo] = 0;
               sscanf (&work_buffer[matches[1].rm_so], "%d", &secs);
	       hours = 0;
	       mins = 0;
               req_time = secs;
            }
	 }
      }
      regfree(&comp_pattern);
      regfree(&comp_pattern2);
      regfree(&comp_pattern3);
   }
   return req_time;
}

/*
 * function: match_user - This function matches the username with a 
 *              substring within the provided line. It returns true
 *              if the user name is found.
 *
 * parameters: line (char *) - Line holding process information
 *             user (char *) - user name string to locate
 *
 * return: boolean - true if the user name is found, false otherwise.
 *
 */
int match_user(char *line, char *user)
{
   char *found_at;
   int retval = false;
   char dupLine[1024];
   char *end = 0;
   int uid = 0;
   int idx = 0;

   for (idx = 0; idx < strlen(line); idx++) {
     if (line[idx] != ' ') {
       break;
     }
   }

   strncpy(dupLine, &line[idx], 1024);

   // Isolate first field
   end = index(dupLine, ' ');
   if (end > 0)
     {
       *end = 0;
     }

   // Try to parse first field as an integer
   if (sscanf(dupLine, "%d", &uid) > 0) {
     // Translate uid to a username.
     struct passwd *userpwent = getpwuid(uid);
     if (strcmp(userpwent->pw_name, user) == 0) {
       retval = true;
     }
   } else {
     // Compare token as a string
     found_at = strstr(dupLine, user);
     if (found_at == dupLine) {
       retval = true;
     }
   }
   return retval;
}

/*
 * function: match_uid - This function matches the uid with a 
 *              substring within the provided line. It returns true
 *              if the uid is found.
 *
 * parameters: line (char *) - Line holding process information
 *             req_uid (int) - Integer user id
 *
 * return: boolean - true if the user name is found, false otherwise.
 *
 */
int match_uid(char *line, int req_uid)
{
  /*
   * UID pattern. The uid should match at the beginning of the line, have
   * optional leading white space, be composed of 1-5 numeric digits, and be
   * followed by at least 1 whitespace character.
   */
   char *uidPatternSource = "^[  ]*([0-9]{1,5})[         ]+";
   size_t num_matches = 3;
   regmatch_t matches[num_matches];
   regex_t comp_pattern;
   int eflags = 0;
   int retval;
   int line_uid;

   /*
    * Build the pattern. make sure to use extended regular expression
    * support since constructs like {} require those features.
    */
   retval = regcomp(&comp_pattern, uidPatternSource, REG_EXTENDED);

   if (retval == 0)
   {
      /*
       * The regular expression compile worked.
       */
      retval = regexec(&comp_pattern,
                       line,
                       num_matches,
                       matches,
                       eflags);
                   
      if (retval == 0)
      {
         /*
          * The pattern matched!
          */
         char work_buffer[max_buffer_line_length];
         char temp_buf[160];
         int line_uid;

         /*
          * This code uses the start and end character offsets 
          * (the .rm_so and .rm_eo fields) to compute
          * the location within the string that matched the pattern.
          * As long as one or more characters matched, it extracts those
          * characters and converts that (numeric) string into an integer.
          */
         if (matches[0].rm_eo - matches[0].rm_so)
         {
            strncpy(temp_buf,
                    line+matches[0].rm_so,
                    matches[0].rm_eo - matches[0].rm_so);
            line_uid = atoi(temp_buf);
         }
         else
         {
            line_uid = 0;
         }
          
         /*
          * This compares the integer derived from the pattern match with
          * the uid argument passed into this function.
          */
         if (req_uid == line_uid)
         {
            retval = true;
         }
         else
         {
            retval = false;
         }

         /*
          * This frees the structure created by regcomp().
          */
         regfree(&comp_pattern);
      }
      else
      {
         /*
          * The pattern did not match the line.
          */
         retval = false;
      }
   }
   else
   {
      /*
       * The regular expression compile failed.
       */
      retval = false;
   }
   return retval;
}
