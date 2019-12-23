/* i n e t _ s t r _ c l i e n t . c : Internet stream sockets client */
#include <stdio.h>
#include <sys/types.h>
/* sockets */
#include <sys/socket.h>
/* sockets */
#include <netinet/in.h>
/* internet sockets */
#include <unistd.h>
/* read , write , close */
#include <netdb.h>
/* g e t h o s t b y a d d r */
#include <stdlib.h>
/* exit */
#include <string.h>
/* strlen */
void perror_exit( char *message ) ;
void main( int argc , char *argv[]) {
int port , sock , i ;
char buf[256];
struct sockaddr_in server ;
struct sockaddr *serverptr = ( struct sockaddr*) &server ;
struct hostent *rem ;
if( argc != 3) {
printf("Please give host name and port number\n");
exit(1) ;}


/* Create socket */
if(( sock = socket( AF_INET , SOCK_STREAM , 0) ) < 0)
perror_exit("socket") ;
/* Find server address */
if(( rem = gethostbyname( argv[1]) ) == NULL ) {
herror("gethostbyname") ; exit(1) ;
}
port = atoi( argv[2]) ; /* Convert port number to integer */
server.sin_family = AF_INET;
/* Internet domain */
memcpy(&server.sin_addr , rem -> h_addr , rem -> h_length ) ;
server.sin_port = htons( port ) ;
/* Server port */
/* I n itiate c o n n e c t i o n */
if ( connect( sock , serverptr , sizeof( server ) ) < 0)
perror_exit("connect") ;
printf("Connecting to %s port %d\n" , argv[1] , port ) ;


do 
{
	printf("Give input string:") ;
	fgets( buf , sizeof(buf) , stdin ) ; /* Read from stdin */
	for( i =0; buf[i] != '\0'; i++) { /* For every char */
		/* Send i - th c h a r a c t e r */
		if(write( sock , buf + i , 1) < 0)
		perror_exit("write") ;
		/* receive i - th c h a r a c t e r t r a n s f o r m e d */
		if(read ( sock , buf + i , 1) < 0)
		perror_exit("read");
	}
	printf("Received string : %s", buf );
} while(strcmp( buf , "END\n") != 0) ; /* Finish on " end " */
close(sock) ;
/* Close socket and exit */
}


void perror_exit(char *message )
{
	perror(message) ;
	exit(EXIT_FAILURE) ;
}