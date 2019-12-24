/* rlsd . c - a remote ls server - with paranoia */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#define PORTNUM 15000
/* rlsd listens on this port */
void perror_exit (char *msg);
void sanitize (char *str);
int read_file(int fd);

int main ( int argc , char *argv[])
{
	/*
	struct sockaddr_in myaddr ; // build our address here
	int lsock;
	int yes=1;
	// use popen to run ls
	char comm[BUFSIZ];     // from client
	char command[BUFSIZ];// for popen ()
	// * create a TCP socket 
	if (( lsock = socket ( AF_INET , SOCK_STREAM , 0) ) < 0)
		perror_exit ("socket") ;
	// * bind address to socket . 
	myaddr.sin_addr.s_addr = htonl( INADDR_ANY );
	myaddr.sin_port = htons( PORTNUM ); // port to bind socket 
	myaddr.sin_family = AF_INET ; // internet addr family 

	if(setsockopt(lsock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) 
	{
        perror("setsockopt");
        exit(1);
    }
      
	if ( bind( lsock ,( struct sockaddr *)&myaddr , sizeof(myaddr) ) )
		perror_exit("bind") ;
	// * listen for connections with Qsize =5 * 

	*/
	int lsock;
    struct sockaddr_in name;
    int yes = 1;
 
    /* Create the socket. */
    lsock = socket (AF_INET, SOCK_STREAM, 0);
    if (lsock < 0)
    {
            perror ("socket");
            exit (EXIT_FAILURE);
    }
 
    // tento radek zpusobi, ze pri opakovanem restartu serveru, bude volani
    // funkce bind() uspesne, kdo neveri, at ho zakomentuje :))
    if(setsockopt(lsock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
            perror("setsockopt");
            exit(1);
    }

    /* Give the socket a name. */
    name.sin_family = AF_INET;
    name.sin_port = htons (PORTNUM);
    name.sin_addr.s_addr = htonl (INADDR_ANY);
    if (bind (lsock, (struct sockaddr *) &name, sizeof (name)) < 0)
    {
            perror ("bind");
            exit (EXIT_FAILURE);
    }
	if ( listen(lsock , 1) < 0 )
		perror_exit("listen");



	fd_set active_fd_set, read_fd_set;
	int i;
	struct sockaddr_in clientname;
	size_t size;

	FD_ZERO(&active_fd_set);
	FD_SET(lsock, &active_fd_set);

	while(1)  // main loop : accept - read - write */ 
	{ 
		read_fd_set = active_fd_set;
		if(select(FD_SETSIZE, &read_fd_set, NULL,NULL,NULL) < 0 )
		{
			perror("select");
			exit(EXIT_FAILURE);
		}


		for(i = 0 ; i < FD_SETSIZE ; ++i)
		{
			if(FD_ISSET(i, &read_fd_set))
			{
				if(i == lsock)
				{
					int newsock; //this will be used for the next client, the original one(lsock) will be used for commun.
					size = sizeof(clientname);
					newsock = accept(lsock, NULL, NULL);

					if(newsock < 0)
					{
						perror("accept");
						exit(EXIT_FAILURE);
					}	
					//fprintf 
					FD_SET(newsock, &active_fd_set);


				}
				else
				{
					if(read_file(i) < 0 )//using the same lsock.(a new one has already created(for listening))
					{
						close(i); //NOT SURE YET, CLIENT MAY WANT TO REPEAT on same sock(it will)
						FD_CLR(i, &active_fd_set);
					}
				}
			}
		}

		/*
		// accept connection , ignore client address 
		if (( csock = accept(lsock , NULL , NULL ) ) < 0 )
			perror_exit("accept");
		
		// open socket as buffered stream 
		if (( sock_fp = fdopen(csock , "r+" ) ) == NULL )
			perror_exit("fdopen");

		// read dirname and build ls command line 
		if ( fgets(dirname , BUFSIZ , sock_fp) == NULL )
			perror_exit("reading dirname");
		sanitize(dirname);// clear wild characters
		snprintf(command , BUFSIZ , "ls %s" , dirname ) ;// Invoke ls through popen
		 		//creating a pipe between the calling program and the executed command, 
		//where the return value is the readable end of the pipe
		if (( pipe_fp = popen(command, "r")) == NULL )  
		// transfer data from ls to socket 
		while( ( c = getc(pipe_fp) ) != EOF )
			putc(c , sock_fp ) ;
		pclose(pipe_fp) ;
		fclose(sock_fp) ;
		*/


	}
	return 0;
}

int read_file(int fd)
{

	int c,csock ; // listening and client sockets
	FILE *sock_fp ;		// stream for socket IO
	FILE *pipe_fp ;
	// use popen to run ls
	char comm[BUFSIZ];     // from client
	char command[BUFSIZ];// for popen ()

	// open socket as buffered stream 
	if (( sock_fp = fdopen(fd , "r+" ) ) == NULL )
		perror_exit("fdopen");

	// read dirname and build ls command line 
	if ( fgets(comm , BUFSIZ , sock_fp) == NULL )
		perror_exit("reading dirname");
	sanitize(comm);// clear wild characters
	snprintf(command , BUFSIZ , "ls %s" , comm ) ;// Invoke ls through popen
	 		//creating a pipe between the calling program and the executed command, 
	//where the return value is the readable end of the pipe
	if (( pipe_fp = popen(command, "r")) == NULL )  
	// transfer data from ls to socket 
	while( ( c = getc(pipe_fp) ) != EOF )
	{
		putc(c , sock_fp ) ;
		printf("skrr");
	}
	pclose(pipe_fp) ;
	fclose(sock_fp) ;	
}


/* it would be very bad if someone passed us an dirname like
* "; rm *" and we naively created a command " ls ; rm *".
* So .. we remove everything but slashes and alphanumerics.
*/
void sanitize ( char *str )
{
	char *src , *dest;
	for ( src = dest = str ; *src ; src++ )
		if ( *src == '/' || isalnum(*src) )
			*dest++ = *src;
	*dest = '\0';
}
/* Print error message and exit */
void perror_exit(char *message )
{
	perror(message) ;
	exit(EXIT_FAILURE) ;
}
