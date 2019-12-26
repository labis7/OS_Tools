#include <stdio.h>  
#include <string.h>   //strlen  
#include <stdlib.h>  
#include <errno.h>  
#include <unistd.h>   //close  
#include <arpa/inet.h>    //close  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros  
     
#define TRUE   1  
#define FALSE  0  
 
void run_commands(int read_fd);
     
int main(int argc , char *argv[])   
{   
    int fd,fd2;
    char *fifo = "myfifo"; 
    if ( argc != 3 ) 
    {
        puts("Usage: rls <Port> <Number Of Children>");
        exit(1);
    }
    int PORT = atoi(argv[1]); 
    int ch_num = atoi(argv[2]);

    if ( mkfifo( fifo , 0666) == -1 ) {
        if ( errno != EEXIST ) {
            perror("mkfifo");
            exit(1);
        }
        if ( ( fd = open( fifo , O_WRONLY ) ) < 0) {
            perror("fifo open/write problem") ;
            exit(3);
        }
        if ( ( fd2 = open( fifo , O_RONLY ) ) < 0) {
            perror("fifo open/read problem");
            exit(3);
        }

    }

    int pid;
    for(int i =0; i < ch_num; i++)
    {
        pid = fork();
        if(pid == 0)
            break;
    }    
    if(pid == 0)
    {
        run_commands(fd2);
    }
    char * myfifo = "myfifo"; 
    int opt = TRUE;   
    int lsocket , addrlen , new_socket , csocket[30] , max_clients = 30 , activity, i , valread , sd;   
    int max_sd;   
    struct sockaddr_in myaddr;   
        
    char buffer[1025];  //data buffer of 1K  
         

    
    printf("%d\n", PORT);

    //set of socket descriptors  
    fd_set read_fd_set;   
         
    //a message  
    char *message = "ECHO Daemon v1.0 \r\n";   
     
    //initialise all csocket[] to 0 so not checked  
    for (i = 0; i < max_clients; i++)   
    {   
        csocket[i] = 0;   
    }   
         
    //create a master socket  
    if( (lsocket = socket(AF_INET , SOCK_STREAM , 0)) == 0)   
    {   
        perror("socket failed");   
        exit(EXIT_FAILURE);   
    }   
     
    //set master socket to allow multiple connections ,  
    //this is just a good habit, it will work without this  
    if( setsockopt(lsocket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )   
    {   
        perror("setsockopt");   
        exit(EXIT_FAILURE);   
    }   
     
    //type of socket created  
    myaddr.sin_family = AF_INET;   
    myaddr.sin_addr.s_addr = INADDR_ANY;   
    myaddr.sin_port = htons( PORT); // Requested port   
         
    //bind the socket to localhost port  
    if (bind(lsocket, (struct sockaddr *)&myaddr, sizeof(myaddr))<0)   
    {   
        perror("bind failed");   
        exit(EXIT_FAILURE);   
    }   
    printf("Listener on port %d \n", PORT);   
         
    //try to specify maximum of 3 pending connections for the master socket  
    if (listen(lsocket, 3) < 0)   
    {   
        perror("listen");   
        exit(EXIT_FAILURE);   
    }   
         
    //accept the incoming connection  
    addrlen = sizeof(myaddr);   
    puts("Waiting for connections ...");   
         
    while(1)   
    {   
        //clear the socket set  
        FD_ZERO(&read_fd_set);   


        //add master socket to set  
        FD_SET(lsocket, &read_fd_set);   
        max_sd = lsocket;  //max socket descriptor 
             
        //add child sockets to set  
        for ( i = 0 ; i < max_clients ; i++)   
        {   
            //socket descriptor  
            sd = csocket[i];   
                 
            //if valid socket descriptor then add to read list  
            if(sd > 0)   
                FD_SET( sd , &read_fd_set);   
                 
            //highest file descriptor number, need it for the select function  
            if(sd > max_sd)   
                max_sd = sd;   
        }   
      
         
        activity = select( max_sd + 1 , &read_fd_set , NULL , NULL , NULL);  //Waiting...no timeout 
        //printf("NEW ACTIVITY DETECTED\n");
        if ((activity < 0) && (errno!=EINTR))   
        {   
            printf("select error");   
        }   
             
        //If something happened on the listening socket ,  
        //then its an incoming connection  
        if (FD_ISSET(lsocket, &read_fd_set)) //check only listening socket  
        {   
            if ((new_socket = accept(lsocket, (struct sockaddr *)&myaddr, (socklen_t*)&addrlen))<0)   
            {   
                perror("accept");   
                exit(EXIT_FAILURE);   
            }   
             
            //inform user of socket number - used in send and receive commands  
            printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , new_socket , inet_ntoa(myaddr.sin_addr) , ntohs(myaddr.sin_port));   
           
            
            //send new connection greeting message  
            if( send(new_socket, message, strlen(message), 0) != strlen(message) )   
            {   
                perror("send");   
            }   
                 
            puts("Welcome message sent successfully");   
            

            //add new socket to array of sockets  
            for (i = 0; i < max_clients; i++)   
            {   
                  
                if( csocket[i] == 0 ) //Finding the 1st empty slot
                {   
                    csocket[i] = new_socket;   
                    printf("Adding to list of sockets as %d\n" , i);   
                         
                    break;   
                }   
            }   
        }  //END_IF : New connection  
             
        //else its some IO operation on some other socket 
        for (i = 0; i < max_clients; i++)   
        {   
            sd = csocket[i];   
                 
            if (FD_ISSET( sd , &read_fd_set))   
            {   
                //Check if it was for closing , and also read the  
                //incoming message  
                if ((valread = read( sd , buffer, 1024)) == 0)   
                {   
                    //Somebody disconnected , get his details and print  
                    getpeername(sd , (struct sockaddr*)&myaddr ,(socklen_t*)&addrlen);   
                    printf("Host disconnected , ip %s , port %d \n" ,inet_ntoa(myaddr.sin_addr) , ntohs(myaddr.sin_port));   
                         
                    //Close the socket and mark as 0 in list for reuse  
                    close( sd );   
                    csocket[i] = 0;   
                }   
                     
                //Echo back the message that came in  
                else 
                {   
                    //char *line = NULL;
                    //size_t len = 0;
                    //ssize_t read;

                    //set the string terminating NULL byte on the end  
                    //of the data read  
                    buffer[valread] = '\0';  
                    //getline(&line, &len, buffer);
                    //printf("New IO activity!:");
                    char *ptr = strtok(buffer,"\n");
                    while(ptr != NULL){
                        printf("New Command: %s",ptr);
                        printf("\n");
                        //send(sd , buffer , strlen(buffer) , 0 );   
                        ptr = strtok(NULL,"\n");
                    }
                }   
            }   
        }   
    }   
         
    return 0;   
}


void run_commands(int read_fd)
{
    int fd = read_fd;
    
}
