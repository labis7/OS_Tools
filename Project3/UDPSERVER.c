#include <sys/types.h>

#include <sys/socket.h>

#include <netinet/in.h>

#include <netdb.h>

#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
void perror_exit( char *message ) ;

char * name_from_address(struct in_addr addr)
{
	struct hostent *rem ; int asize = sizeof(addr.s_addr) ;
	if (( rem = gethostbyaddr(&addr.s_addr , asize , AF_INET ) ) )
		return rem -> h_name ; /* reverse lookup success */
	return inet_ntoa(addr) ; /* fallback to a . b . c . d form */
}

void main()
{
	int n , sock ; unsigned int serverlen , clientlen ;
	char buf[256] , *clientname ;
	struct sockaddr_in server , client ;
	struct sockaddr *serverptr = ( struct sockaddr *)&server ;
	struct sockaddr *clientptr = ( struct sockaddr *)&client ;
	

	if (( sock = socket( AF_INET , SOCK_DGRAM , 0) ) < 0)
	perror_exit( "socket " ) ;
	/* Bind socket to address */
	server.sin_family = AF_INET ;
	/* Internet domain */
	server.sin_addr.s_addr = htonl( INADDR_ANY ) ;
	server.sin_port = htons(8889) ;
	/* A u t o s e l e c t a port */
	serverlen = sizeof( server ) ;
	if ( bind( sock , serverptr , serverlen ) < 0)
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
		if (( n = recvfrom( sock , buf , sizeof ( buf ) , 0 , clientptr , &clientlen ) ) <0)
			perror ( " recvfrom " ) ;
		buf[ sizeof( buf ) -1]= '\0'; /* force str t e r m i n a t i o n */
		/* Try to discover client ’s name */
		clientname = name_from_address(client.sin_addr) ;
		printf( "Received from %s : %s \n" , clientname , buf ) ;
		/* 
		if ( sendto( sock , buf , n , 0 , clientptr , clientlen ) <0)
			perror_exit ( " sendto " ) ;
		*/
	}
}

void perror_exit( char *message )
{
	perror(message ) ;
	exit( EXIT_FAILURE ) ;
}
	
