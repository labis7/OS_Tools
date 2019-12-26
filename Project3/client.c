	#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>// internet sockets
#include <netdb.h> 	//gethostbyname

#define BUFFSIZE 256

void perror_exit( char *message )
{
	perror ( message ) ;
	exit ( EXIT_FAILURE ) ;
}

/* Write () repeatedly until ’ size ’ bytes are written */
int write_all(int fd , void *buff , size_t size )
{
	int sent , n ;
	for ( sent = 0; sent < size ; sent += n ) {
		if (( n = write ( fd , buff + sent , size - sent ) ) == -1)
			return -1; /* error */
	}
	return sent ;
}

int main ( int argc , char *argv[]) 
{
	struct sockaddr_in servadd ; // The address of server
	struct hostent *hp ; // to resolve server ip
	int sock , n_read ; // socket and message length
	char buffer[BUFFSIZE]; // to receive message

	if ( argc != 5 ) 
	{
		puts("Usage: rls <hostname> <Server Port> <Receive Port> <FILE>");
		exit(1);
	}
	int S_PORT = atoi(argv[2]); 
	int R_PORT = atoi(argv[3]);
	/* Step 1: Get a socket */
	if (( sock = socket( AF_INET , SOCK_STREAM , 0) ) == -1 )
	perror_exit("socket") ;
	/* Step 2: lookup server ’s address and connect there */
	if (( hp = gethostbyname(argv[1])) == NULL )
	{
		herror ("gethostbyname");
		exit(1);
	}
	memcpy(&servadd.sin_addr , hp -> h_addr , hp -> h_length );
	servadd.sin_port = htons(S_PORT); /* set port number */
	servadd.sin_family = AF_INET;
	/* set socket type */
	if ( connect( sock , ( struct sockaddr *)&servadd , sizeof(servadd) ) !=0)
	perror_exit("connect");
	
	n_read = read( sock , buffer , BUFFSIZE );
	if ( write_all(STDOUT_FILENO , buffer , n_read ) < n_read )
		perror_exit("fwrite");

	


	////////// File Opening /////////////
	FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(argv[4], "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);
	


	/*
	while(1)
	{


		
		sleep(2);
		if(write_all(sock, "labiz", 5) == -1)
			perror_exit("write");
		n_read = read( sock , buffer , BUFFSIZE );
		if ( write_all(STDOUT_FILENO , buffer , n_read ) < n_read )
			perror_exit("fwrite");
	}
		*/

	while ((read = getline(&line, &len, fp)) != -1) 
    {
        //printf("Retrieved line of length %zu:\n", read);
        write_all(sock, line, read);
        //printf("%s", line);
    }


	fclose(fp);
	if (line)
       	free(line);

	//break;
	
	

	/*
	// Step 3: send directory name + newline 
	if ( write_all(sock , argv[2] , strlen(argv[2])) == -1)
		perror_exit("write") ;
	if ( write_all( sock , "\n", 1) == -1 )
		perror_exit ("write");
	// Step 4: read back results and send them to stdout 
	while(( n_read = read( sock , buffer , BUFFSIZE ) ) > 0 )
		if ( write_all(STDOUT_FILENO , buffer , n_read ) < n_read )
			perror_exit("fwrite");
	*/

	close(sock);
	return 0;
}