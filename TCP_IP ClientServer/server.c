

#include <stdio.h>	/* standard C i/o facilities */
#include <stdlib.h>
#include <unistd.h>	/* Unix System Calls */
#include <sys/types.h>	/* system data type definitions */
#include <sys/socket.h> /* socket specific definitions */
#include <netinet/in.h> /* INET constants and stuff */
#include <arpa/inet.h>	/* IP address conversion stuff */

/* Server main routine - this is an iterative server

   1. create a socket
   2. bind the socket and print out the port number assigned
   3. put the socket into passive mode (listen)
   4. do forever
        get next connection
        handle the connection 
      enddo
*/


int main(int argc, char* argv[]){
	/* checks if only 1 argument (the port number) is entered */
	if(argc != 2){
		printf("Usage: server <port number>\n");
		exit(0);
	}

	
	/* converts port number from string to int */
	int port = atoi(argv[1]);
	if(port < 1024 || port > 65535){
		printf("Port number not valid\n");
		exit(0);
	}
/*	printf("\nArgc: %d\nPort: %d\n", argc, port); */

  int ld,sd;			
  struct sockaddr_in skaddr;	
  struct sockaddr_in from;	
  int addrlen,length;
  
  int n;

/* create a socket 
       IP protocol family (PF_INET) 
       TCP protocol (SOCK_STREAM)
*/
  
  if ((ld = socket( PF_INET, SOCK_STREAM, 0 )) < 0) {
    perror("Problem creating socket\n");
    exit(1);
  }
  
/* establish our address 
   address family is AF_INET
   our IP address is INADDR_ANY (any of our IP addresses)
   the port number is assigned by the kernel 
*/

  skaddr.sin_family = AF_INET;
  skaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  skaddr.sin_port = htons(port);

  if (bind(ld, (struct sockaddr *) &skaddr, sizeof(skaddr))<0) {
    perror("Problem binding\n");
    exit(0);
  }
  
/* find out what port we were assigned and print it out */

  length = sizeof( skaddr );
  if (getsockname(ld, (struct sockaddr *) &skaddr, (socklen_t*) &length)<0) {
    perror("Error getsockname\n");
    exit(1);
  }
  printf("The Server passive socket port number is %d\n",ntohs(skaddr.sin_port));

/* put the socket into passive mode (waiting for connections) */

  if (listen(ld,5) < 0 ) {
    perror("Error calling listen\n");
    exit(1);
  }
  
/* now process incoming connections forever ... */


  printf("Ready for a connection...\n");
  addrlen=sizeof(skaddr);
  if ( (sd = accept( ld, (struct sockaddr*) &from, (socklen_t*) &addrlen)) < 0) {
    perror("Problem with accept call\n");
    exit(1);
  }

  printf("Got a connection - processing...\n");

  /* Determine and print out the address of the new
     server socket */

  length = sizeof( skaddr );
  if (getsockname(sd, (struct sockaddr *) &skaddr, (socklen_t*) &length)<0) {
    perror("Error getsockname\n");
    exit(1);
  }
  printf("The active server port number is %d\n",ntohs(skaddr.sin_port));
  printf("The active server IP ADDRESS is %s\n",inet_ntoa(skaddr.sin_addr));

  /* print out the address of the client  */

  printf("The client port number is %d\n",ntohs(from.sin_port));
  printf("The client IP ADDRESS is %s\n",inet_ntoa(from.sin_addr));
    
  /* read and send to stdout until the client closes the connection */
  char *sizebuf = calloc(10, sizeof(char));
	/* prints error if memory was not allocated */
	if(sizebuf == NULL){
		printf("Failed to allocate enough memory\n");
		exit(1);
	}
  int size = 0, i;

  printf("\nBegin data/message\n");

  /* first gets the size data and stores into sizebuf */
  while((n=read(sd, sizebuf, 10))>0){
  	if(n < 0){
  		printf("Error reading\n");
  		exit(1);
  	}
  	/* size data is converted to an int. Array lines[size] will hold the 
  			actual line of size bytes */
		size = atoi(sizebuf);

		char *lines = calloc(size, sizeof(char));
		/* checks to see if memory was successfully allocated.
				This dynamic allocation prevents buffer overflows */
		if(lines == NULL){
			printf("Failed to allocate enough memory\n");
			exit(1);
		}

		/* stronger error checking is possible because we already know the 
				incoming line size */
		if((n=read(sd, lines, size))!=size){
			printf("Error reading\n");
			exit(1);
		}

		/* prints out the lines in reverse order */
		for(i = size; i >= 0; i--){
			printf("%c" , lines[i-1]);
		}
  }

	/* prints did not receive message if the program
			did not receive any line data */
	if(size == 0){
		printf("\n\nDid not receive any line data\n");
	}
	else{
		printf("\n\nEnd data/message\n");
	}

  printf("Done with connection - closing\n\n\n");
  close(sd);
  
  return 0;
}
