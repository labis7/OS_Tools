#include <stdio.h>  
#include <string.h>   //strlen  
#include <stdlib.h>  
#include <signal.h>
#include <fcntl.h>
#include <errno.h>  
#include <unistd.h>   //close  
#include <arpa/inet.h>    //close  
#include <sys/types.h>  
#include <sys/wait.h>
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <netdb.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros  
     
#define MSGSIZE 106 // 100 MAX for command      + 4 bytes int + 1 space char + 1(safety - maybe '\0')   
#define TRUE   1  
#define FALSE  0  
 
void run_commands(int read_fd);
int finish = 0;

void signalHandler(int signal)
{
    printf("Cought signal %d!\n",signal);
    if (signal == SIGUSR1) 
    {
        printf("Child ended\n");
        //sigaction(SIGCHLD,&new_action, NULL);
        //wait(NULL);
    }
    if(signal == SIGUSR2)
    {
        printf("Closing TCP/UDP sockets and file descriptors . . .\n");
        //
        //sigaction(SIGSTOP,&new_action, NULL);
        finish =1 ;
    }
    if(signal == SIGPIPE){
        printf("SIGPIPE signal Ignored\n");
        //sigaction(SIGPIPE,&new_action, NULL);
    }
}


     
int main(int argc , char *argv[])   
{   
    finish = 0 ;
    //Setting up actions in case of signal.

    struct sigaction new_action;

    /* Set up the structure to specify the new action. */
    new_action.sa_handler = signalHandler;
    //sigemptyset (&new_action.sa_mask);
    new_action.sa_flags = 0;

    sigaction(SIGUSR1,&new_action, NULL); 
    sigaction(SIGUSR2,&new_action, NULL);
    sigaction(SIGPIPE,&new_action, NULL);
    


    if ( argc != 3 ) 
    {
        puts("Usage: rls <Port> <Number Of Children>");
        exit(1);
    }
    
    int PORT = atoi(argv[1]); 
    int ch_num = atoi(argv[2]);
    
    
    int fd[2];
    pipe(fd);
    pid_t pid_list[1000];

    int pid;
    for(int i =0; i < ch_num; i++)
    {
        pid = fork();
        pid_list[i]=pid; //add child to the pid list
        if(pid == 0)//we let only parent in this loop
            break;
    }    
    if(pid == 0)
    {
        close(fd[1]);//Only parent writing
        run_commands(fd[0]);
    }

    close(fd[0]);//Parent only writes.


    int opt = TRUE;   
    int lsocket , addrlen , new_socket , csocket[30] , max_clients = 30 , activity, i , valread , sd;   
    int max_sd;   
    struct sockaddr_in myaddr;   
        
    char buffer[MSGSIZE], comm[MSGSIZE];  //data buffer of 1K  

    //set of socket descriptors  
    fd_set read_fd_set;   
         
  
     
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
         

    if (listen(lsocket, 10) < 0)   
    {   
        perror("listen");   
        exit(EXIT_FAILURE);   
    }   
         
    //accept the incoming connection  
    addrlen = sizeof(myaddr);   
    puts("Waiting for connections ...");   
    //FILE *pipe_fp;

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
      
        if (finish == 1)
        {
            for(int i =0; i < ch_num; i++)
            {
                //the SIGTERM signal, which will terminate a process, and free its allocated resources
                kill(pid_list[i],SIGTERM);  
                printf("Child with PID:%d ended!\n", pid_list[i]);            
            }
            //Parent free up//
            close(fd[1]); //close 'write side' of the pipe
            for (i = 0; i < max_clients; i++)      
                close(csocket[i]); //close all connected client sockets
            close(lsocket); //closing listening socket
            ////////////////// printf("Waiting Children. . .\n"); int status;
            pid_t wpid;
            int status;
            while ((wpid = wait(&status)) > 0);// waits for all children
            printf("Terminating\n");
            exit(0);
        }

        activity = select( max_sd + 1 , &read_fd_set , NULL , NULL , NULL);  //Waiting...no timeout 
       
        if (finish == 1)
        {
            for(int i =0; i < ch_num; i++)
            {
                //the SIGTERM signal, which will terminate a process, and free its allocated resources
                kill(pid_list[i],SIGTERM); 
                printf("Child with PID:%d ended!\n", pid_list[i]);            
            }
            //Parent free up//
            close(fd[1]); //close 'write side' of the pipe
            for (i = 0; i < max_clients; i++)      
                close(csocket[i]); //close all connected client sockets
            close(lsocket); //closing listening socket
            ////////////////// printf("Waiting Children. . .\n"); int status;
            pid_t wpid;
            int status;
            while ((wpid = wait(&status)) > 0);// waits for all children
            printf("Terminating\n");
            exit(0);
        }


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
                if ((valread = read( sd , buffer, 512)) == 0)   
                {   
                    //Somebody disconnected , get his details and print  
                    getpeername(sd , (struct sockaddr*)&myaddr ,(socklen_t*)&addrlen);   
                    printf("Host disconnected , ip %s , port %d \n" ,inet_ntoa(myaddr.sin_addr) , ntohs(myaddr.sin_port));   
                         
                    //Close the socket and mark as 0 in list for reuse  
                    close( sd );   
                    csocket[i] = 0;   
                }   
                     
                //Else .we have incoming data 
                else 
                {   

                    //set the string terminating NULL byte on the end  
                    //of the data read  
                    buffer[valread] = '\0';  
                    //getline(&line, &len, buffer);
                    //printf("New IO activity!:");


                    //sleep(1) fix this
                    char *ptr = strtok(buffer,"\n");
                    while(ptr != NULL){ //Break down possible 2-3 line commands
                        //printf("Sending to Pipe the command:%s\n", ptr);
                        strcpy(comm, ptr );
                        if (write(fd[1] , comm, MSGSIZE) == -1)
                        {    perror("Error in Writing"); 
                             exit(2) ;
                        } 
                        
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
    FILE *pipe_fp;
    int fd = read_fd;
    char command[MSGSIZE];


    //////// UDP socket setup for transmit  /////
    int sock;

    struct hostent *rem;
    struct sockaddr_in server , client ;
    unsigned int serverlen = sizeof(server) ;
    struct sockaddr *serverptr = (struct sockaddr *)&server ;
    struct sockaddr *clientptr = (struct sockaddr *)&client ;

    if (( sock = socket(AF_INET , SOCK_DGRAM , 0) ) < 0)
    {
        perror("socket") ; 
        exit(1) ;
    }
    /* Find server ’s IP address */
    char *sk="localhost";
    if (( rem = gethostbyname(sk) ) == NULL ) 
    {
        herror( "gethostbyname" ) ;
        exit(1) ;
    }


    // Setup server ’s IP address and port 
    server.sin_family = AF_INET ;
    // Internet domain 
    memcpy(&server.sin_addr , rem -> h_addr , rem -> h_length ) ;
    
    //PORT WILL BE completed later

    // Setup my address 
    client.sin_family = AF_INET ;
    // Internet domain 
    client.sin_addr.s_addr = htonl(INADDR_ANY); // Any address 
    client.sin_port = htons(0) ;


    if ( bind (sock , clientptr , sizeof(client)) < 0) 
    {
        perror("bind"); 
        exit(1);
    }

    

    /////////////////////////////////

    while(1)
    {
        //Children Waiting for new activity on PIPE
        //int fd = read_fd;
        if ( read(fd , command , MSGSIZE) < 0) 
        {
            perror ("Problem in reading.") ;
            exit(5) ;
        }
        //printf("Received Message: %s\n", buff);
        //fflush(stdout);
        //("THread:%d finished\n", getpid());
        //sleep(1);
        char tmpcomm[MSGSIZE];
        strcpy(tmpcomm,command);
        bzero(command,sizeof(command));

        char *ptr;
        //printf("Reading from pipe the command:%s\n",ptr );
        char *info = strtok(tmpcomm,";"); //tmpcomm must be kept intact!!! There are many ptrs on that string.
        ptr = strtok(NULL,";"); //Now take the actual command

        //take the info
        char *inf = strtok(info," ");
        char S_PORT[5],num[5];
        strcpy(num,inf);
        inf = strtok(NULL," ");
        strcpy(S_PORT,inf);
        server.sin_port = htons(atoi(S_PORT)) ;
        //

           
        //printf("Received Port:%s  Line Num:%s  Command:%s \n",S_PORT,num,ptr );
        
        
        char temp[32];
        char fresult[512],result[512]={0x0};
        //Choose only the first Command(including Pipes)
        char *ptr1 = strtok(ptr,";"); //Not safe way to do it !!! WARNING !!!
        
        if(strcmp(ptr1,"end\0") == 0) //Case that child need to be terminated(after closing its resources)
        {
            //send signal to parent
            kill(getppid(),SIGUSR1);
            printf("Child with PID %d ended\n",getpid());
            fflush(stdout);
            // Free //

            close(sock);
            pclose(pipe_fp);
            exit(0);
            //////////
        }
        if(strcmp(ptr1,"timeToStop\0") == 0)
        {
            kill(getppid(), SIGUSR2);
            sleep(0.5);
            close(sock);
            pclose(pipe_fp);
            exit(0);//just in case
        }


        char tempcomm[MSGSIZE]={0x0};
        if(strstr(ptr1,"|")){    //Warning! NOT SAFE
            //Commands Without pipeline
            char *ptr2 = strtok(ptr1, "|"); //Warning! NOT SAFE
            while(ptr2 != NULL)
            {
                //if the command is in the list of the 5 AND if its able to run 
                if((strstr(ptr2, "ls") != NULL)||(strstr(ptr2, "cat") != NULL)||(strstr(ptr2, "cut") != NULL)||(strstr(ptr2, "grep") != NULL)||(strstr(ptr2, "tr") != NULL))
                {
                    //run the command until here
                    strcat(tempcomm, ptr2);
                    pipe_fp = popen(tempcomm,"r");  
                    //printf("Testing Command:%s\n", tempcomm);  
                    while (fgets(temp, sizeof(temp), pipe_fp) != NULL)//IF the command is wrong, that part will be skipped
                        ;         
                    if(pclose(pipe_fp)>0) //means error 
                    {
                       if(strlen(command)>2)     //but there are some commands before that.
                       {
                            command[strlen(command)-2]='\0'; //remove the extra pipe and run
                            break; 
                       }
                       else
                            break;// means that it fails from the start
                    } 

                    //printf("\n%s Passed Succesfully\n", tempcomm);
                    //ELSE
                    strcat(command, ptr2);//Keep the command and move on.
                    //strcat(command, "|");
                    //for the next iteration
                    strcat(tempcomm, "|");
                }
                else
                {
                    if(strlen(command)>2) ///This the only case where the command after pipe is wrong
                        command[strlen(command)-2]='\0';//delete the "|" that was put before
                    else
                        break;
                }
                ptr2 = strtok(NULL,"|");
                if(ptr2 != NULL)//if there is more, add the missing pipe
                    strcat(command, "|");
            }
        }
        else //No pipe
        {
            if((strstr(ptr1, "ls") != NULL)||(strstr(ptr1, "cat") != NULL)||(strstr(ptr1, "cut") != NULL)||(strstr(ptr1, "grep") != NULL)||(strstr(ptr1, "tr") != NULL))
            {
                pipe_fp = popen(ptr1,"r");
                 
                while (fgets(temp, sizeof(temp), pipe_fp) != NULL)//IF the command is wrong, that part will be skipped
                    ;    
                //The pclose() function returns -1 if wait4 returns an error, or some other error is detected.
                if( pclose(pipe_fp) > 0) 
                    strcpy(command, "\0"); //Not needed
                else
                   strcpy(command, ptr1);        
            }
            else            
                strcpy(command, "\0");//See error below
        }                        
        
        //Re-open it for the final read
        pipe_fp = popen(command,"r");
        


        if((!(strlen(command)>2))||(strlen(command)>100))
            strcpy(result,"Error");
        else
        {
            ////////////////////////// EXECUTE & SEND ////////////////////////////////
            while (fgets(temp, sizeof(temp), pipe_fp) != NULL)//IF the command is wrong, that part will be skipped
            {
                if((strlen(temp)+strlen(result))>505) //then split it into multiple UDP packets
                {
                    strcpy(fresult,num);//First line : INFO
                    strcat(fresult,"\n");
                    strcat(fresult,result);//The rest lines are results
                    //printf("Sending:%s\n",fresult );
                    if ( sendto( sock , fresult , strlen(fresult)+1 , 0 , serverptr , serverlen ) < 0)
                    {
                        perror("sendto");
                        exit(1);
                    }
                    bzero(result,sizeof(result));
                }
                strcat(result, temp);
            }
            ////////////////////////////////////////////////////////////////////////////
        }
        pclose(pipe_fp);
        
        strcpy(fresult, num);
        strcat(fresult,"\n");
        strcat(fresult, result);  

        //printf("Sending  \"%s\" via UDP!\n", fresult);
        if ( sendto( sock , fresult , strlen(fresult)+1 , 0 , serverptr , serverlen ) < 0)
         {
            perror("sendto");
            exit(1);
        }


    }

    close(sock);
    /*
    if ((pipe_fp = popen(command ,"r")) == NULL )
        perror_exit("popen") ; 
    while ((c = getc(pipe_fp)) != EOF)
        putc(c , sock);
                              
   pclose(pipe_fp);
    */

    exit(0);

    //snprintf( command , BUFSIZ , "ls %s" , dirname ) ;
}
