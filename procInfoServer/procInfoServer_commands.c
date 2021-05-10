/*
 *
 * Copyright 2012, 2013. Antonino N. Mione
 *
 */
/*
 * procInfoServer_commands.c
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

#include "procInfoServer.h"
#include "procInfoServer_util.h"
#include "procInfoServer_auth.h"

extern int authenticated;
extern int dropConnection;
extern int exit_program;

/*
 * These are error strings to send back over the connection to the client.
 */
const char *auth_error="511 Network Authentication Required\n";
const char *bad_request_error="400 Bad Request\n";
const char *no_content_error="204 No Content\n";
const char *success_return="200 Ok\n";


/*
 * This enum is for the different types of information that the client
 * may request to be extracted from the ps listing.
 */
enum EXT_TYPE {EXT_TYPE_IMAGE, EXT_TYPE_CPU, EXT_TYPE_USER, EXT_TYPE_UID};

extern char *tmpFile;

/*
 * function: extract - This dumps the current processes to a temporary
 *              file and filters it by the requested criteria. It returns
 *              a buffer with the extracted process information.
 *
 * parameters: ext_type (enum EXT_TYPE) - This is the type of matching
 *                to be done on the process data.
 *             argument (char *) - This is the item for which to scan
 *             retbuffer (char **) - This is a pointer to a location
 *                in which this function can store the pointer to the allocated
 *                return buffer.
 *             retbuf_length - (int *) - This is a location in which to store
 *                the length of the returned buffer.
 *
 * return (void) - No return value.
 */
void extract(enum EXT_TYPE ext_type,
             char *argument,
             char **retbuffer,
             int *retbuf_length)
{
   FILE *rfile;                           /* Handle for temporary file.     */
   int buffer_line_length;                /* Length of returned buffer      */
   char rbuffer[max_buffer_line_length];  /* Buffer returned from readline  */
   int total_length = 0;                  /*        */
   char work_buffer[max_buffer_line_length];  /* A temporary buffer. */
  
   rfile = fopen(tmpFile, "r");

   /*
    * Check if the code could open the file for reading.
    */
   if (rfile == 0)
   {
      /*
       * The file could not be open. Stuff in zeros for the buffer
       * pointer and return length to indicate this to the caller.
       */
      *retbuffer = 0;
      *retbuf_length = 0;
      return;
   }
   else
   {
      /*
       * The open call worked. Allocate a large buffer for the filtered
       * text from the file.
       */
      *retbuffer = (char *) malloc(big_buffersize);
      if (*retbuffer)
      {
         memset(*retbuffer, 0, max_buffer_line_length);
      }
      char *buf_pointer = *retbuffer;

      /*
       * Verify the buffer was allocated.
       */
      if (*retbuffer)
      {
         /*
          * The buffer space was successfully allocated.
          */
         char *found_at;

	 /*
	  * Clear the work buffer.
	  */
	 memset(work_buffer, 0, max_buffer_line_length);

         /*
          * Process file input based on the requested extraction type.
          */
         switch (ext_type)
         {
            /*
             * This code extracts based on matching a program name.
             */
            case EXT_TYPE_IMAGE:
            {
               while ((readline(rfile, rbuffer, &buffer_line_length) > 0) &&
                      (total_length < big_buffersize))
               {
                  found_at = strstr(rbuffer, argument);
                  if (found_at != 0)
                  {
                     strcat (buf_pointer, rbuffer);
                     total_length += strlen(rbuffer);
                  }
               }
               *retbuf_length = total_length;
               break;
            }
            /*
             * This code extracts based on processes with CPU times exceeding
             * than the indicated value.
             */
            case EXT_TYPE_CPU:
            {
               int line_count = 0;
               size_t num_matches = 5;
               int mins;
               int secs;
               int hths;
               float req_time = 0.0;
               float line_time = 0.0;

               int retval;

               /*
                * Convert the ASCII time string (<m:ss.hh>) into the
                * number of seconds it represents.
                */
               req_time = time2float(argument);

               /*
                * As long as data is left in the buffer of process information,
                * we read a line and test it for a match of the time
                * criteria.
                */
               while ((readline(rfile, rbuffer, &buffer_line_length) > 0) &&
                      (total_length < big_buffersize))
               {
                  line_count++;

                  /*
                   * Extract and convert the time string from the 'ps' output
                   * line.
                   */
                  strncpy(work_buffer,&rbuffer[39],8);
                  line_time = time2float(work_buffer);
                  if (line_time > req_time)
                  {
                     /*
                      * The time in the string from 'ps' output is larger
                      * than the CPU parameter passed, so include this
                      * in the returned user buffer.
                      */
                     strcat (buf_pointer, rbuffer);
		     //                     strcat (buf_pointer, "\n");
                     total_length += (strlen(rbuffer) + 1);
                  }
                  buffer_line_length = max_buffer_line_length;

                }
                /* 
                 * Set this length so the caller knows how much data
                 * is currently written to the return buffer.
                 */
                *retbuf_length = total_length;
      
                break;
              }
            /*
             * This code extracts based on processes with the indicated
             * username (at the beginning of the process information line.)
             */
            case EXT_TYPE_USER:
            {
               int success = 0;

               /*
                * As long as data is left in the buffer of process information,
                * we read a line and test it for a match of the username
                * criteria.
                */
               while ((readline(rfile, rbuffer, &buffer_line_length) > 0) &&
                      (total_length < big_buffersize))
               {
                 /*
                  * Try to match the user name on the line.
                  */
                  strncpy(work_buffer,rbuffer,max_buffer_line_length);
                  success = match_user(work_buffer, argument);
                  if ( success )
                  {
                     
                     /*
                      * The user name matched. So include the line in the
                      * buffer to be returned to the requestor.
                      */
                     strcat (buf_pointer, work_buffer);
                     strcat (buf_pointer, "\n");
                     total_length += (strlen(work_buffer) + 1);
                  }
                  buffer_line_length = max_buffer_line_length;

               }
               /* 
                * Set this length so the caller knows how much data
                * is currently written to the return buffer.
                */
               *retbuf_length = total_length;

               break;

            }
            case EXT_TYPE_UID:
            {
               int success = 0;

               /*
                * As long as data is left in the buffer of process information,
                * we read a line and test it for a match of the uid
                * criteria.
                */
               while ((readline(rfile, rbuffer, &buffer_line_length) > 0) &&
                      (total_length < big_buffersize))
               {
                  /*
                   * Try to match the user name on the line.
                   */
                  strncpy(work_buffer,rbuffer,max_buffer_line_length);
                  success = match_uid(work_buffer, atoi(argument));
                  if ( success )
                  {
                      
                     /*
                      * The uid matched. So include the line in the
                      * buffer to be returned to the requestor.
                      */
                     strcat (buf_pointer, work_buffer);
                     strcat (buf_pointer, "\n");
                     total_length += (strlen(work_buffer) + 1);
                  }
                  buffer_line_length = max_buffer_line_length;

               }
               /* 
                * Set this length so the caller knows how much data
                * is currently written to the return buffer.
                */
               *retbuf_length = total_length;

               break;

            }
            default:
               break;
            }
        }
   }     
   fclose(rfile);

   /*
    * Clean up the temporary file. Make sure the pointer to the file
    * structure is valid and the process has access (this should be
    * the case as this process created the file.)
    */
   if (tmpFile != 0)
   {
      if (access(tmpFile, F_OK) != ERR)
      {
         if (access(tmpFile, W_OK) != ERR)
         {
            unlink(tmpFile);
         }
      }
   }
   return;
}


/*
 * function: process_line - This function processes a single command
 *              issued by the client program.
 *
 * parameters: cmd (char *) - 
 *             cmd_len (int) - 
 *             retbuffer (char **) - 
 *             retbuf_length (int *) - 
 *
 * return (void) - No return value.
 */
void process_line(char *cmd, int cmd_len, char **retbuffer, int *retbuf_length)
{
   char *spPos;
   char *nlPos;
   char argPart[max_line_len];
   char cPart[max_line_len];
   char userCookieString[max_line_len];
   char cookieString[max_line_len];
   unsigned int cpyCount = 0;
   int success;

   /*
    * Locate the end of line (there should be a newline ('\n':10) at the end
    * of the usable buffer. If not, there should be a return ('\r':13) present.
    * Overwrite these with a null ('\0').
    */
   nlPos = rindex(cmd, 10);
   if (nlPos)
   {
      *nlPos = '\0';
   }
   nlPos = rindex(cmd, 13);
   if (nlPos)
   {
      *nlPos = '\0';
   }

   /*
    * Find the first space character in the line. This should delimit the
    * the command from the argument.
    */
   spPos = index(cmd, 32);
   /*
    * Calculate the length of the command. If it is shorter than 
    * the maximum line length, then it is the difference between the 
    * location of the space and the beginning of the buffer. Otherwise,
    * the command occupies the whole line.
    */
   cpyCount = ((unsigned int)(spPos - cmd) < max_line_len) ?
      (spPos - cmd) :
      max_line_len;
   
   /*
    * Allocate an adequately sized output buffer.
    */
   *retbuffer = (char *) malloc(big_buffersize);

   /*
    * Check to verify the malloc worked.
    */
   if (*retbuffer)
   {
      /*
       * Isolate the command word for comparison with accepted commands.
       */
      strncpy(cPart, cmd, cpyCount);
      cPart[cpyCount] = 0;
      /*
       * This section handles 'auth' if the string matches.
       */
      
      if (strncmp(cPart, "auth", max_line_len) == 0)
      {
        /*
         * The remainder of the line (after the auth command) is the
         * cookie string or hashed password that will be passed to
         * check_auth() in orde to be checked.
         */
         cpyCount = (((cmd+strlen(cmd)) - spPos) < max_line_len) ?
           ((cmd+strlen(cmd)) - spPos) : max_line_len;
         strncpy(userCookieString, spPos + 1, cpyCount);
         authenticated = check_auth(userCookieString);

         /*
          * Print the status of the check_auth() call to show user
          * whether or not they 'logged in'.
          */
         sprintf(*retbuffer, "Authenticated: %d\n", authenticated);
         *retbuf_length = strlen (*retbuffer);
      }
      else
      {
         /*
          * This section handles quitting from the session or 
          * shutting the server application down.
          */
         if ((strncmp(cPart, "quitSession", max_line_len) == 0) ||
             (strncmp(cPart, "quit", max_line_len) == 0) ||
             (strncmp(cPart, "squit", max_line_len) == 0))
         {
            authenticated = 0;
            dropConnection = true;
            if (strncmp(cPart, "squit", max_line_len) == 0)
            {
               /*
                * If the command is 'squit', the user is asking the entire
                * server to shut down. This creates a 'flag' file (.exit)
                * so the main server process will notices this and exit.
                */
               exit_program = true;
               system("touch .exit");
               system("chmod 777 .exit");
            }
	    memset(*retbuffer, 0, big_buffersize);
            strcpy(*retbuffer, "Bye!\n");
	    retbuffer[4] = 0;
            *retbuf_length = strlen ("Bye!\n");
         }
         else
         {
            /*
             * The user must be authenticated to do anything else.
             * If they have not succeeded, place an error into the
             * buffer to be returned to them over the network connection.
             */
            if (!authenticated)
            {
               printf ("%s\n", auth_error);
               if (*retbuf_length > strlen (auth_error))
               {
                  strcpy (*retbuffer, auth_error);
                  *retbuf_length = strlen (auth_error);
               }
            }
            else
            {
               /*
                * This section handles the 'procs' command.
                */
               if (strncmp(cPart, "procs", max_line_len) == 0)
               {
                  /*
                   * get_procs() will return all of the system's process
                   * information into a large global buffer to be scanned.
                   */
                  success = get_procs();
                       
                  if (success)
                  {
                     /*
                      * Check that there is text to be matched. If not,
                      * indicate there was a problem with the command.
                      */
		     if (((signed int) strlen(cmd) - (signed int) cpyCount - 1) < 0)
                     {
                        printf ("Error, argument needed to 'procs'\n");
                        return;
                     }
                     else
                     {
                        /*
                         * Isolate the remainder of the commandline.
                         * Extract the remainder of the line as the argument.
                         * Send the argument to extract() which will match
                         * based on the requested type (EXT_TYPE_IMAGE means
                         * it should search for program names.
                         */
                        strncpy(argPart, cmd+cpyCount+1, strlen(cmd) - cpyCount - 1);
                        argPart[strlen(cmd)-cpyCount-1] = 0;
                        free(*retbuffer);
                        extract(EXT_TYPE_IMAGE, argPart, retbuffer, retbuf_length); 
                     }
                  }
               }
               else
               {
                  /*
                   * This section matches based on CPU time.
                   */
                  if (strncmp(cPart, "cpu", max_line_len) == 0)
                  {
                     /*
                      * get_procs() will return all of the system's process
                      * information into a large global buffer to be scanned.
                      */
                     success = get_procs();
                     if (success)
                     {
                        /*
                         * Check that there is text to be matched. If not,
                         * indicate there was a problem with the command.
                         */
                        if (((signed int) strlen(cmd) - (signed int) cpyCount - 1) < 0)
                        {
                            printf ("Error, argument needed to 'procs'\n");
                            return;
                         }
                         else
                         {
                            /*
                             * Isolate the remainder of the commandline.
                             * Extract the remainder of the line as the
                             * argument.
                             * Send the argument to extract() which will match
                             * based on the requested type (EXT_TYPE_CPU means
                             * it should search for cpu usage greater than that
                             * provided in the argument.
                             */
                            strncpy(argPart,
                                    cmd+cpyCount+1,
                                    strlen(cmd) - cpyCount - 1);
                            argPart[strlen(cmd)-cpyCount-1] = 0;
                            free(*retbuffer);
                            extract(EXT_TYPE_CPU,
                                    argPart,
                                    retbuffer, retbuf_length); 
                         }
                      }
                   }
                   else
                   {
                      if (strncmp(cPart, "user", max_line_len) == 0)
                      {
                         success = get_procs();
                         if (success)
                         {
                            if (((signed int) strlen(cmd) - (signed int) cpyCount - 1) < 0)
                            {
                               /*
                                * Check that there is text to be matched.
                                * If not, just reutrn.
                                */
                               return;
                            }
                            else
                            {
                               /*
                                * Isolate the remainder of the commandline.
                                * Extract the remainder of the line as the
                                * argument.
                                * Send the argument to extract() which will
                                * match based on the requested type
                                * (EXT_TYPE_USER means it should search
                                * for processes run by a matching username.)
                                */
                               strncpy(argPart,
                                       cmd+cpyCount+1,
                                       strlen(cmd) - cpyCount - 1);
                               argPart[strlen(cmd)-cpyCount-1] = 0;
                               free(*retbuffer);
                               extract(EXT_TYPE_USER,
                                       argPart,
                                       retbuffer, retbuf_length); 
                            }
                         }
                      }
                      else
                      {
                         if (strncmp(cPart, "uid", max_line_len) == 0)
                         {
                            success = get_procs();
                            if (success)
                            {
                               if (((signed int) strlen(cmd) - (signed int) cpyCount - 1) < 0)
                               {
                                   printf ("Error, argument needed to 'procs'\n");
                                   return;
                                }
                                else
                                {
                                   /*
                                    * Isolate the remainder of the commandline.
                                    * Extract the remainder of the line as the
                                    * argument.
                                    * Send the argument to extract() which
                                    * will match based on the requested
                                    * type (EXT_TYPE_CPU means it should
                                    * search for processes with a matching
                                    * uid.)
                                    */
                                   strncpy(argPart,
                                           cmd+cpyCount+1,
                                           strlen(cmd) - cpyCount - 1);
                                   argPart[strlen(cmd)-cpyCount-1] = 0;
                                   free(*retbuffer);
                                   extract(EXT_TYPE_UID,
                                           argPart,
                                           retbuffer, retbuf_length); 
                                }
                             }
                             else
                             {
                             }
                          }
                          else
                          {
                             /*
                              * The command was not recognized by the server.
                              * Format a buffer with the appropriate error
                              * to be returned to the user over the network
                              * connection.
                              */
			     printf ("%s\n", bad_request_error);
                             strcpy(*retbuffer, bad_request_error);
                             *retbuf_length = strlen(bad_request_error);
                          }
                       }
                    }
                 }
              }
          }
      }
   }
   else
   {
      // Internal error
      printf ("Internal error\n");
   }

   return;
}

/*
 * function: process_commands - This function manages the entire interaction
 *              for a single client connection.
 *
 * parameters: desc (int) - This is the descriptor for the client connection.
 *             
 * return (void) - No return value.
 */
void process_commands(int desc)
{
   char cmd_line[max_line_len];
   int cmd_line_len = max_line_len;
   int len_read;
   char *ret_buffer;
   int ret_length;

   while ((len_read = read(desc, cmd_line, cmd_line_len)) >= 0)
   {
      if (len_read > 0)
      {
         cmd_line[len_read] = 0;
         process_line(cmd_line, len_read, &ret_buffer, &ret_length);
         /* Report results */
         if (ret_length > 0)
           {
             if (ret_length > 50)
               {
                 write(desc, success_return, strlen(success_return));
               }
             write(desc, ret_buffer, ret_length);
             free(ret_buffer);
             // Reset length and buffer pointer
             *ret_buffer = 0;
             ret_length = 0;
           }
         else
           {
             write (desc, no_content_error, strlen(no_content_error));
           }

         if (dropConnection)
         {
            break;
         }
         cmd_line_len = max_line_len;
      }
      else
      {
         close(desc);
         return;
      }
   }
   exit_program = true;
}
