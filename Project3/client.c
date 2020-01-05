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
		char buf[512] , *clientname ;
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
				buf[ sizeof( buf ) -1]= '\0'; /* force str t e r m i n a t i o n */
				/* Try to discover client ’s name */
				clientname = name_from_address(client.sin_addr) ;
				//printf( "Received from %s : %s \n" , clientname , buf ) ;
				char buffer[512];
				strcpy(buffer,buf);
				char *ptr = strtok(buf, "\n");
				int filenum = atoi(ptr);
				char filename[128];
				sprintf(filename,"output.%s.%d",argv[3],filenum);
				ptr = strtok(NULL,"\n");
				char *position=strstr(buffer,"\n"); 
				int pos = position - buffer;
				//printf("Buffer:%s\n", buffer+pos+1);//+1 is the new line('\n')
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
					//fprintf( fileptr, "\0");
			    }

				else	
					fprintf(fileptr, buffer+pos+1);
			    fclose(fileptr);
			}
		}
		//wait();
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
        //printf("Retrieved line of length %zu:\n", read);
        //printf("Sending command \"%s\" via TCP . . .\n", line);
        sleep(0.2);   ///////////////////////////////////////////IMPORTANT #TODO:Find the reason//
        char temp[105]={0x0};
        //strcpy(temp)
        snprintf(temp,105,"%d %d;%s",i,R_PORT,line);
        bzero(line,sizeof(line));
        snprintf(line,strlen(temp),"%s",temp);
        //printf("Sending %s, size:%d\n",line ,strlen(line) );
        write_all(sock, line , strlen(line));
        //printf("%s", line);
        if(counter == 10 )
        {
        	counter = 0;
        	sleep(5);
        }
    }

    sleep(0.1);//Wait before closing
	fclose(fp);
	if (line)
       	free(line);


	close(sock);
	return 0;
}