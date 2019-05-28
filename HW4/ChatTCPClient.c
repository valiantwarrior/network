#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#define COMMAND_LENGTH 10
#define NICKNAME_LENGTH 65
#define MAXLINE 2048
#define IP "165.194.35.202"
#define PORT 24220


typedef int SOCKET;
typedef int BOOL;

/* special strings for state */
#define CLIENT_QUIT "$!#ZCR!$$$@##!#!"
#define SERVER_QUIT "!#$!FFDSSVVVSDG!@!#$!!@##"
#define VILLAIN_DETECTED "3##21$!2@3991"

#define TRUE 1
#define FALSE 0

void sigHandler(int sigNo);
BOOL nickname_validation(char* nickname);
void* thread_read_func(void* sockfd);


int main(int argc, char** argv)
{
    struct sockaddr_in serveraddr;
    SOCKET serverSocket;
    
    char buf[MAXLINE];
    char nickname[NICKNAME_LENGTH];
    char command[COMMAND_LENGTH];
    char message[MAXLINE-NICKNAME_LENGTH-COMMAND_LENGTH];
   
    pthread_t thread_id;
    int validation;
    int number_of_client_in_server;
    int readn;
    socklen_t addrlen;

    /* SIGINT handler */
    signal(SIGINT,(void*)sigHandler);

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(PORT);
    serveraddr.sin_addr.s_addr = inet_addr(IP);

    /* check argument : ./client <nickname> */
    if(argc != 2) {
        printf("Wrong argument\n");
        return 0;
    }
    /* nickname form check.*/
    if(nickname_validation(argv[1]) == FALSE) {
        return 0;
    }
    addrlen = sizeof(serveraddr);
    connect(serverSocket, (struct sockaddr*)&serveraddr, addrlen);
    memset(buf,0x00,MAXLINE);
    
    /*1. get number of client in server
      if 8 clients in server, quit program with message. */
    read(serverSocket,buf,MAXLINE);
    number_of_client_in_server = atoi(buf);
    if(number_of_client_in_server == 8) {
        printf("full.cannot connect\n");
        close(serverSocket);
        return 0;
    }
    
    /*2. If clients are under 8, send nickname and if there is same nickname in server.*/
    memset(buf,0x00,MAXLINE);
    strcpy(buf,argv[1]);
    write(serverSocket,buf,MAXLINE);

    /*3. If there is no <nickname>, server send 0. If <nickname> already exist, server send 1*/
    memset(buf,0x00,MAXLINE);
    read(serverSocket,buf,MAXLINE);
    validation = atoi(buf);
    
    /*Validation == 0 means, <nickname> is unique. So establishing connection*/
    if(validation==0) {
        
        
        /*4. Send nickname again to save client information in server */
        memset(buf,0x00,MAXLINE);
        strcpy(buf,argv[1]);
        write(serverSocket,buf,MAXLINE);
        
        /* Receive welcome message from server */
        read(serverSocket,buf,MAXLINE);
        printf("%s\n",buf);

        /* Background thread for read from server */
        pthread_create(&thread_id,NULL,thread_read_func,&serverSocket);
        pthread_detach(thread_id);

        memset(buf,0x00,MAXLINE);
        /* Get input */
        while( (readn = read(0,buf,MAXLINE) > 0 ) ){
            /* If input is start with \, check if this is valid command */
            if(buf[0] == '\\') {
                memset(command,0x00,COMMAND_LENGTH);
                memset(nickname,0x00,NICKNAME_LENGTH);
                memset(message,0x00,MAXLINE-NICKNAME_LENGTH-COMMAND_LENGTH);
                sscanf(buf,"%s %s %s",command, nickname, message);
                
                /* Send command to server */
                /*
                <Command list>
                \quit
                \stats
                \w <nickname> <message>
                \w change <nickname>
                \ver
                \list
                */
             
                if(strcmp(buf,"\\quit\n") == 0) {
                    write(serverSocket,buf,MAXLINE);
                }
       
                else if(strcmp(buf,"\\stats\n") == 0) {
                    write(serverSocket,buf,MAXLINE);   
                }
                
                else if( strcmp(command,"\\w") == 0  ) {
                    write(serverSocket,buf,MAXLINE);
                }

                else if( strcmp(command,"\\change") == 0 ) {
                    /* Before send this command, check wheter <nickname> is valid form. */
                    if(nickname_validation(nickname) == TRUE) {
                        write(serverSocket,buf,MAXLINE);
                    }
                    else {
                        continue;
                    }
                }

                else if( strcmp(buf, "\\ver\n") == 0) {
                    write(serverSocket,buf,MAXLINE);
                }

                else if( strcmp(buf, "\\list\n") == 0) {
                    write(serverSocket,buf,MAXLINE);
                }
                /* Other case : Ignore */
                else{
                    printf("COMMAND LIST :\n \\list\n \\w <nickname> <message>\n \\quit\n \\ver\n \\change <new nickname>\n \\stats\n");
                    memset(buf,0x00,MAXLINE);
                    continue;
                }
            }
            /* Input is not command, send to server */
            else {
                write(serverSocket,buf,MAXLINE);
            }
            memset(buf,0x00,MAXLINE);
        }
        close(serverSocket);
    }
    /* Validation == 1 means there is same <nickname> in server, so disconnect from server */
    else {
        printf("Duplicated Nickname. Cannot connect.\n");
        close(serverSocket);
    }
    return 0;
}
/* Background thread for read from server */
void* thread_read_func(void* sockfd) {
    SOCKET serverSocket = *((SOCKET*)sockfd);
    char buf[MAXLINE];
    int readn;
    
    while(1) {
        memset(buf,0x00,MAXLINE);
        read(serverSocket,buf,MAXLINE);
        /*If i sent "i hate professor", server remove my info and send VILLAIN_DETECTED. When I receive it, terminate program. */
        if(strcmp(buf,VILLAIN_DETECTED) == 0) {
            printf("\x1b[31m!VILLAIN! Bye~\n");
            close(serverSocket);
            exit(0);
        }
        /*If i sent '\quit', server remove my info and send CLIENT_QUIT. When I receive it, terminate program. */
        if(strcmp(buf,CLIENT_QUIT) == 0) {
            printf("Bye~\n");
            close(serverSocket);
            exit(0);
        }
        /*If server terminate, server send SERVER_QUIT. When I receive it, terminate my program too.*/
        if(strcmp(buf,SERVER_QUIT) == 0) {
            printf("Server terminated. Bye~\n");
            close(serverSocket);
            exit(0);
        }
        /* Other cases, print other client's messages */
        printf("%s",buf);
        printf("\x1b[0m");
        fflush(stdout);
    }
    close(serverSocket);
    return 0;
}


BOOL nickname_validation(char* nickname) {
    char test[NICKNAME_LENGTH];

    /*nickname length <= 64 && Only use "alphabet, number, -"" */
    if(strlen(nickname) > NICKNAME_LENGTH - 1) {
        printf("Nickname length limit : 64.\n");
        return FALSE;
    }
    else {
        sscanf(nickname,"%[a-zA-Z0-9-]",test);
        if(strcmp(nickname,test) == 0) {
            return TRUE;
        }
        else {
            printf("Nickname : number,alphabet,- allow.\n");
            return FALSE;
        }
    }
    
}

void sigHandler(int sigNo) {
    printf("\nBye~\n");
    exit(0);
}