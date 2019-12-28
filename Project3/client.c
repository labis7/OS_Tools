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


//////////////////////////// setup UDP ////////////////////////////////////////
    

    if (fork() != 0) //FATHER will be accepting results
    {
    	printf("Father setting up\n");
    	fflush(stdout);
    	int opt = TRUE;   
    	int UDP_FD , myaddrlen , activity , valread ;   
    	int max_fd;   
    	struct sockaddr_in myaddr,client;  
    	struct sockaddr *myaddrptr = ( struct sockaddr *)&myaddr ; 
    	struct sockaddr *clientptr = ( struct sockaddr *)&client ;
        //char *clientname;
    	char buffer[BUFFSIZE];

    	fd_set rset;

    	if (( UDP_FD = socket(AF_INET , SOCK_DGRAM , 0) ) < 0) // Creating UDP Socket
			perror_exit("socket") ;

    	myaddr.sin_family = AF_INET; 
    	myaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    	myaddr.sin_port = htons(R_PORT);
    	myaddrlen = sizeof(myaddr);

    	if( setsockopt(UDP_FD, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )   
    	{   
        	perror("setsockopt");   
        	exit(EXIT_FAILURE);   
    	} 


    	//Binding
    	if ( bind(UDP_FD, myaddrptr , myaddrlen) < 0)
			perror_exit("bind");

		/*
		// Discover selected port 
		if(getsockname( sock , myaddrptr , &myaddrlen ) < 0)
			perror_exit( " getsockname " ) ;
    	
		*/
		socklen_t clientlen = sizeof(client);
    	
    	//max_fd = UDP_FD +1;
    	while(1)
    	{
    		FD_ZERO(&rset);
    		FD_SET(UDP_FD, &rset);
    		
    		max_fd = UDP_FD; 

    		/*
    		//add child sockets to set  
	        for ( i = 0 ; i < max_clients ; i++)   
	        {   
	            //socket descriptor  
	            UDP_FD = csocket[i];   
	                 
	            //if valid socket descriptor then add to read list  
	            if(UPD_fd > 0)   
	                FD_SET( UDP_FD , &rset);   
	                 
	            //highest file descriptor number, need it for the select function  
	            if(UDP_FD > max_fd)   
	                max_fd = UDP_fd;   
	        }  
			*/
			printf("Waiting for UDP Connection. . . \n");
    		activity = select(max_fd, &rset, NULL, NULL, NULL); 

    		if ((activity < 0) && (errno!=EINTR))   
        	{   
            	printf("select error");   
        	} 

        	//for (i = 0; i < max_clients; i++)   
        	//{   
            //	UPD_fd = csocket[i]; 
			if (FD_ISSET(UDP_FD, &rset))
    		{
    			bzero(buffer, sizeof(buffer)); //Clear buffer	
 	            if ((valread = recvfrom(UDP_FD, buffer, sizeof(buffer), 0,(struct sockaddr*)&clientptr, &clientlen)) == 0)   
                {   
                    //Somebody disconnected , get his details and print     
                         
                    //Close the socket and mark as 0 in list for reuse  
                    close(UDP_FD);   
                    //csocket[i] = 0;   
                } 
                else //New IO
       			{
       				buffer[valread] = '\0';

					//bzero(buffer, sizeof(buffer)); 
	            	printf("\nMessage from UDP client: "); 
	            	//n = recvfrom(UDP_fd, buffer, sizeof(buffer), 0,(struct sockaddr*)&clientptr, &clientlen); 
	            	puts(buffer); 
        		}
	        }
		        
		} 
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
        printf("Sending command \"%s\" via TCP . . .\n", line);
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