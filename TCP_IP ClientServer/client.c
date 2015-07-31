

#include <stdio.h>	/* standard C i/o facilities */
#include <stdlib.h>	/* needed for atoi() */
#include <unistd.h>	/* Unix System Calls */
#include <sys/types.h>	/* system data type definitions */
#include <sys/socket.h> /* socket specific definitions */
#include <netinet/in.h> /* INET constants and stuff */
#include <arpa/inet.h>	/* IP address conversion stuff */

/* define the message we will send */

/* char *message = "Networking Rules !!!\n"; */


/* client program:

   The following must passed in on the command line:
   
      name of the server (argv[1])
      port number of the server (argv[2])
*/

int main( int argc, char **argv ) {

  int sk;
  struct sockaddr_in skaddr;

  /* first - check to make sure there are 3 command line parameters
     (argc=4 since the program name is argv[0]) 
  */

  if (argc!=4) {
    printf("Usage: client <server name> <server port> <filename>\n");
    exit(0);
  }

	if(atoi(argv[2]) < 1024 || atoi(argv[2]) > 65535){
		printf("Port number not valid\n");
		exit(0);
	}
	
  /* create a socket 
     IP protocol family (PF_INET) 
     TCP protocol (SOCK_STREAM)
  */
  
  if ((sk = socket( PF_INET, SOCK_STREAM, 0 )) < 0) {
    printf("Problem creating socket\n");
    exit(1);
  }

  /* fill in an address structure that will be used to specify
     the address of the server we want to connect to

     address family is IP  (AF_INET)

     server IP address is found by calling gethostbyname with the
     name of the server (entered on the command line)

     server port number is argv[2] (entered on the command line)
  */

  skaddr.sin_family = AF_INET;

#ifndef SUN
  if (inet_aton(argv[1],&skaddr.sin_addr)==0) {
    printf("Invalid IP address: %s\n",argv[1]);
    exit(1);
  }
#else
  /*inet_aton is missing on Solaris - you need to use inet_addr! */
  /* inet_addr is not as nice, the return value is -1 if it fails
     (so we need to assume that is not the right address !)
  */

  skaddr.sin_addr.s_addr = inet_addr(argv[1]);
  if (skaddr.sin_addr.s_addr ==-1) {
    printf("Invalid IP address: %s\n",argv[1]);
    exit(1);
  }
#endif
 
  skaddr.sin_port = htons(atoi(argv[2]));
  

  /* attempt to establish a connection with the server */


  if (connect(sk,(struct sockaddr *) &skaddr,sizeof(skaddr)) < 0 ) {
    printf("Problem connecting socket\n");
    exit(1);
  }

  /* Send a string and finish*/
  
  /* pointer to file */
  FILE *fileptr;
  int i, filesize = 0;
  
  /* attempts to open file for reading */
  if ((fileptr = fopen(argv[3], "r")) == NULL){
  	printf("Error opening file\n");
  	exit(1);
  }
  
  /* calculates size of file */
  while(getc(fileptr) != EOF){
  	filesize++;
  }
  
  /* rewinds filepointer to beginning of file */
  rewind(fileptr);
  
  char *holder = calloc(filesize, sizeof(char));
  /* check to prevent buffer overflow */
  if(holder == NULL){
  	printf("Error allocating enough memory\n");
  	exit(1);
  }
  
  /* transfers contents in file to array holder */
  /* size of array matches filesize */
  for(i = 0; i < filesize; i++){
  	holder[i] = getc(fileptr);
  }
  
  /* closes pointer to file since we have the contents
  		in the holder array		*/
  fclose(fileptr);
  
  
  int count = 0, tracker = 0;
  char temp = holder[0];
  while(count < filesize){
  	tracker = 0;
  	
  	/* buffer to hold size of lines */
		char *buf = calloc(10, sizeof(char));
		/* check to prevent buffer overflow */
		if(buf == NULL){
			printf("Error allocating enough memory\n");
			exit(1);
		}
		
  	/* case if simultaneous '\n' are detected 
  	 		the \n is stored in array and sent as a
  			standalone line */
  	if(temp == '\n'){
  		count++;
  		tracker++;
  		
  		char *lines = calloc(1, sizeof(char));
  		/* check to prevent buffer overflow */
  		if(lines == NULL){
  			printf("Error allocating enough memory\n");
  			exit(1);
  		}
  		
  		lines[0] = temp;
  		if(count < filesize){
  			temp = holder[count];
  		}
  		
  		/* first size of the line is sent using sprintf to convert
  				int to a string. The server converts back to int using
  				atoi */			
			sprintf(buf, "%d", tracker);
			if(write(sk, buf, 10)!=10){
				printf("Error writing\n");
				exit(1);
			}
			/* after server has size of line, it allocated enough space and
					then the client sends the actual line */
			if(write(sk, lines, tracker)!=tracker){
				printf("Error writing\n");
				exit(1);
			}
  	}
  	/* case where we do not have simultaneous '\n' works the same as
  			above code */
  	else{
  		/* counts size of line and stores in tracker.
  				This dynamic allocation prevents buffer overflows */
			for(i = count; temp != '\n' && count < filesize; i++){
				count++;
				tracker++;
				temp = holder[i];
			}
			count -= tracker;
			temp = holder[count];
			
			/* lines has exactly enough space to accomodate every line.
					This way space is not wasted and buffer overflows are prevented*/
			char *lines = calloc(tracker, sizeof(char));
			/* check to prevent buffer overflows */
			if(lines == NULL){
				printf("Error allocating enough memory\n");
				exit(1);
			}
			
			/* stores every line into lines array */ 
			int j = 0;
			for(i = count; temp != '\n' && count < filesize; i++){
				lines[j] = holder[i];
				temp = holder[i];
				count++;
				j++;
			}
			
			if(count < filesize){
				temp = holder[count];
			}
		
			sprintf(buf, "%d", tracker);
			/* if size is more than 9999999999, then it will overflow buffer
					therefore data is not sent */
			if(tracker > 9999999999){
				printf("Line size too large\n");
				printf("Data was not sent to prevent buffer overflows\n");
				exit(1);
			}
			
			/* sends the size of line, then the line itself 
					(same as above) */	
			if(write(sk, buf, 10)!=10){
				printf("Error writing\n");
				exit(1);
			}
			if(write(sk, lines, tracker)!=tracker){
				printf("Error writing\n");
				exit(1);
			}
  	}
  }

	/* closes connection after every line is sent */
	close(sk);

  return(0);
}
