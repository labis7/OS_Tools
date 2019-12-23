#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>

# define MSGSIZE 16
char * msg1 = "Buenos Dias ! #1" ;
char * msg2 = "Buenos Dias ! #2" ;
char * msg3 = "Buenos Dias ! #3" ;
main () {
char inbuf [ MSGSIZE ];
int p [2] , i =0 , rsize =0;
pid_t pid ;
if ( pipe ( p ) == -1) { perror ( " pipe call " ) ; exit (1) ;}
switch ( pid = fork () ) {
case -1: perror ( " fork call " ) ; exit (2) ;
case 0: 
printf("writing\n");	
write ( p [1] , msg1 , MSGSIZE ) ;
// if child then write !
printf("writing\n");
write ( p [1] , msg2 , MSGSIZE ) ;
printf("writing\n");
write ( p [1] , msg3 , MSGSIZE ) ;

printf("dONE\n");
break ;
default : for ( i =0; i <5; i ++) {
// if parent then read !
write ( p [1] , msg1 , MSGSIZE ) ;
write ( p [1] , msg2 , MSGSIZE ) ;
sleep(1);
rsize = read ( p [0] , inbuf , MSGSIZE ) ;
printf("read done!\n");
printf ( " %.*s \n " , rsize , inbuf ) ;
}
wait ( NULL ) ;
}
exit (0) ;
}
