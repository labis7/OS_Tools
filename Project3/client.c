#include <stdio.h>  
#include <string.h>   
#include <signal.h> 
#include <stdlib.h>  
#include <fcntl.h>
#include <errno.h>  
#include <unistd.h>   
#include <arpa/inet.h>    
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

void  ALARMhandler(int sig)
{
  
  exit(0);
}


int main ( int argc , char *argv[]) 
{
	signal(SIGALRM, ALARMhandler);     //init the handler   

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

//////////////////////////// Setup & Accept UDP ////////////////////////////////////////
    
 	//Parent will handle it.	
	if(fork()!=0)
	{
		int n , UPD_sock ; unsigned int serverlen , clientlen ;
		char buf[512] ;
		struct sockaddr_in server , client ;
		struct sockaddr *serverptr = ( struct sockaddr *)&server ;
		struct sockaddr *clientptr = ( struct sockaddr *)&client ;
		

		if (( UPD_sock = socket( AF_INET , SOCK_DGRAM , 0) ) < 0)
		perror_exit( "socket " ) ;
		/* Bind socket to address */
		server.sin_family = AF_INET ;
		/* Internet domain */
		server.sin_addr.s_addr = htonl( INADDR_ANY ) ;
		server.sin_port = htons(R_PORT) ;
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
				//
				buf[ sizeof( buf ) -1]= '\0'; 
				char buffer[512];
				strcpy(buffer,buf); //copy,because we will use strtok and buf must be intact
				char *ptr = strtok(buf, "\n");
				int filenum = atoi(ptr);
				char filename[128];
				sprintf(filename,"output.%s.%d",argv[3],filenum);
				ptr = strtok(NULL,"\n");
				//find the offset and seek in order to get the data result
				char *position=strstr(buffer,"\n"); // the '\n' is the key where we will split the file info from actual data
				int pos = position - buffer; // memory_pos_we_want_to_seek - starting_mem_of_the_string
				FILE *fileptr=fopen(filename,"a");
				if(fileptr == NULL)
			    {
			        /* File not created hence exit */
			        printf("Unable to create file.\n");
			        exit(EXIT_FAILURE);
			    }
			    //ptr is the result(or just part of it)
			    if(strcmp(ptr, "Error\0")==0)
			    {
					//write empty file
			    }
				else	
					fputs(buffer+pos+1, fileptr); //add the offset we found before +1 to ignore '\n'

			    fclose(fileptr);
			    alarm(7);
			}
		}
		
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

    int i = 0;
    int counter=0;

	while ((read = getline(&line, &len, fp)) != -1) 
    {
    	i++;
    	counter++;
        sleep(0.2); 
        char temp[105]={0x0}; //init
        snprintf(temp,105,"%d %d;%s",i,R_PORT,line); // concat
        snprintf(line,strlen(temp),"%s",temp);
        //printf("Sending %s, size:%d\n",line ,strlen(line) );
        write_all(sock, line , strlen(line));

        if(counter == 10 )
        {
        	counter = 0;
        	sleep(5);
        }
    }

    //free
	fclose(fp);
	if (line)
       	free(line);


	close(sock);
	return 0;
}