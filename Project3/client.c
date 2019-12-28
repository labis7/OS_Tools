#include <stdio.h>  
#include <string.h>   //strlen  
#include <stdlib.h>  
#include <fcntl.h>
#include <errno.h>  
#include <unistd.h>   //close  
#include <arpa/inet.h>    //close  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <netdb.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros

#define BUFFSIZE 512
#define TRUE   1  
#define FALSE  0  

void perror_exit( char *message )
{
	perror ( message ) ;
	exit ( EXIT_FAILURE ) ;
}

char * name_from_address(struct in_addr addr)
{
	struct hostent *rem ; int asize = sizeof(addr.s_addr) ;
	if (( rem = gethostbyaddr(&addr.s_addr , asize , AF_INET ) ) )
		return rem -> h_name ; /* reverse lookup success */
	return inet_ntoa(addr) ; /* fallback to a . b . c . d form */
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
	int sock; // socket and message length

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
	
	//n_read = read( sock , buffer , BUFFSIZE );
	//if ( write_all(STDOUT_FILENO , buffer , n_read ) < n_read )
	//	perror_exit("fwrite");



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


//////////////////////////// Setup & Accept UDP ////////////////////////////////////////
    
 	//Parent will handle it.	
	if(fork()!=0)
	{
		int n , UPD_sock ; unsigned int serverlen , clientlen ;
		char buf[256] , *clientname ;
		struct sockaddr_in server , client ;
		struct sockaddr *serverptr = ( struct sockaddr *)&server ;
		struct sockaddr *clientptr = ( struct sockaddr *)&client ;
		

		if (( UPD_sock = socket( AF_INET , SOCK_DGRAM , 0) ) < 0)
		perror_exit( "socket " ) ;
		/* Bind socket to address */
		server.sin_family = AF_INET ;
		/* Internet domain */
		server.sin_addr.s_addr = htonl( INADDR_ANY ) ;
		server.sin_port = htons(8889) ;
		/* A u t o s e l e c t a port */
		serverlen = sizeof( server ) ;
		if ( bind( UPD_sock , serverptr , serverlen ) < 0)
			perror_exit( "bind " ) ;
		/* 
		if ( getsockname( sock , serverptr , &serverlen ) < 0)
			perror_exit( " getsockname " ) ;
		*/
		printf( "Socket port : %d\n" , ntohs(server.sin_port ) ) ;
		while (1) 
		{ 
			clientlen = sizeof( client ) ;
			/* Receive message */
			if (( n = recvfrom( UPD_sock , buf , sizeof ( buf ) , 0 , clientptr , &clientlen ) ) <0)
				perror ( " recvfrom " ) ;
			if (n == 0)
			{ //then close sock
				close(sock);
				printf("UDP Conection is now completed!\n");
				break;
			}
			else
			{
				buf[ sizeof( buf ) -1]= '\0'; /* force str t e r m i n a t i o n */
				/* Try to discover client ’s name */
				clientname = name_from_address(client.sin_addr) ;
				printf( "Received from %s : %s \n" , clientname , buf ) ;
			}
		}
		wait();
 		return 0;
	}



 	



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// CHILD  /////////////////////////////////

    ////////// File Opening /////////////
	FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(argv[4], "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    
	while ((read = getline(&line, &len, fp)) != -1) 
    {
        //printf("Retrieved line of length %zu:\n", read);
        //printf("Sending command \"%s\" via TCP . . .\n", line);
        sleep(0.1);   ///////////////////////////////////////////IMPORTANT #TODO:Find the reason//
        write(sock, line, read);
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