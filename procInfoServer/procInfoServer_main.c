/*
 *
 * Copyright 2012, 2013. Antonino N. Mione
 *
 */
/*
 * procInfoServer_main.c
 */

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
// #include <malloc.h>
#include <fcntl.h>

#include "procInfoServer.h"
#include "procInfoServer_auth.h"
#include "procInfoServer_signals.h"
#include "procInfoServer_commands.h"

/*
 * The server will run on this port.
 */
#if defined(SERVER_PORT)
const int default_server_port = SERVER_PORT;
int server_port = SERVER_PORT;
#else
const int default_server_port = 7060;
int server_port = 7060;
#endif


/*
 * Global Variables
 */
int authenticated = 1;    /* The currently connected user has authenticated */
int dropConnection = 0;   /* Drop connection in next loop */
int exit_program = 0;     /* Exit program on next loop */

/*
 * function: main - This is the main function. It sets up the port and
 *              begins to listen on it.
 *
 * parameters: argc (int) - Count of argument strings from command line.
 *             argv (char **) - Pointer to list of argument strings.
 *
 * return (int) - 0 if successful, non-zer if an error occured.
 *
 */
int main(int argc, char **argv)
{
   char hostname[max_hname_len];    /* Buffer to hold the hostname. */
   int hname_len = max_hname_len;   /* Length of host buffer.       */
   int result;                      /* Result of gethostname() call. */
   int sock;                        /* Socket number                 */
   struct sockaddr_in servaddr;     /* Structure for bind            */
   char cmd_line[max_line_len];     /* Buffer to hold command line.  */
   int cmd_line_len = max_line_len; /* Length of command buffer.     */
   int desc;                        /* descriptor from accept()      */


   /*
    * If there is an argument, use that as the port.
    */
   if (argc > 1) {
     sscanf(argv[1], "%d", &server_port);
   }

   /*
    * Read all of the records from .authFile into an array for
    * authentication checks.
    */
   //   read_auth_records();

   /*
    * This registers a routine to catch signals.
    */
   register_signals();

   /*
    * Get the server host's hostname.
    */
   result = gethostname(&hostname[0], hname_len);
   if (!result)
   {
     // printf ("Host: %s\n", hostname);
   }

   /*
    * Set up a socket to use a tcp (stream) connection to the client.
    */ 
   sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   if (sock > 0)
   {
      /*
       * The socket creation worked.
       */
      /*
       * Create a sockaddr structure to hold the IP address and port
       * to which the code will listen. INADDR_ANY is used to accept
       * connections for any IP address this machine responds to.
       * The port argument is hardcoded to 7077 (see above constant).
       */
      memset(&servaddr, 0, sizeof(servaddr));
      servaddr.sin_family      = AF_INET;
      servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
      servaddr.sin_port        = htons(server_port);

      /*
       * Attempt to bind the port. This will fail if another application
       * is listening on the port. Sometimes, if the code is not exited
       * correctly, the server will not immediately be able to restart
       * as the bind will fail. Wait about 60 seconds when restarting
       * this server after a failure.
       */
      if (bind(sock, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
      {
         printf ("Bind failed!\n");
      }
      else
      {
         /*
          * The bind worked. Now the server will listen on the port.
          */
         while (1)
         {
            /*
             * Loop listening for a client connection. If the return is
             * less than 0, we haven't seen a connection yet. Then sleep
             * 1 second and try again.
             */
            while (listen(sock, 5) < 0)
            {
               sleep(1);
            }
            {
               /* 
                * The while loop dropped through so there is a client
                * connection waiting.
                * This loop accepts the connection.
                */
               socklen_t slen;  /* This is the length of a sockaddr struct */
               pid_t chldPid;   /* This is the PID of the created child */

               int testfileno;  /* This is a descriptor returned from open */

               /*
                * Try to create a special marker file (.exit)
                */
               testfileno = open(".exit", O_RDONLY);

               if (testfileno > 0)
               {
                  close(testfileno);
                  system("rm .exit");
		  close(sock);      /* Close the network connection */
                  exit (0);
               }

               /*
                * Set the current length of the sockaddr structure that
                * will be used in the accept call.
                */
               slen = sizeof(servaddr);

               /*
                * Try to accept the waiting connection. This step, if 
                * successful, completes openning of the TCP connection
                * between client and server.
                */
               desc = accept(sock, (struct sockaddr *) &servaddr, &slen);
               if (desc < 0)
               {
                  /*
                   * The open failed. Report the error and try to listen
                   * again.
                   */
                  int status;

                  printf ("accept failed! errno=%d\n", errno);
                  continue;
               }
               else
               {
                  /* 
                   * The accept worked.
                   */
                  printf ("accept!!\n");
               }
                
               if ((chldPid = fork()) != 0)
               {
                 /*
                  * This is the parent. Go back and listen
                  */
                 continue;
               }
               else
               {
                 /*
                  * This is the child process. Close the original socket
                  * since that is for the parent to use when listenning for
                  * new connections.
                  */
                  close(sock);
                  /*
                   * The process_commands() call will take care of
                   * the entire dialog between client and server.
                   * When this exits, the client has issued a 'quit' 
                   * request.
                   */
                  process_commands(desc);
                  /*
                   * Client issued 'quit' so close the descriptor.
                   */
                  close(desc);

                  /*
                   * If the exit_program flag was lit, this process
                   * should exit.
                   */
                  if (exit_program)
                  {
                     close(sock);
                     exit(0);
                  }
               }
            }
         }
      }
   }
   /*
    * The server is finished. Close the socket it was using for listenning,
    * indicate the socket was closed, and terminate the program.
    */
   close (sock);
}
