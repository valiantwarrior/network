#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <stdint.h>
#include <ctype.h>
#include <signal.h>

#define IPADDR "165.194.35.202"
#define PORTNUM 24220

#define NICKNAME_LENGTH 65
#define MAX_LINE 2048
#define COMMAND_LENGTH 10

#define MAX_CLIENT 8
#define TRUE 1
#define FALSE 0



#define NOTICE_CLIENT_CONNECTED 10
#define NOTICE_CLIENT_DISCONNECTED 11
#define REGULAR 12
#define NOTICE_SERVER_TERMINATION 888

#define VILLAIN_DETECTED "3##21$!2@3991"
#define CLIENT_QUIT "$!#ZCR!$$$@##!#!"
#define SERVER_QUIT "!#$!FFDSSVVVSDG!@!#$!!@##"

typedef int MSG_TYPE;
typedef int SOCKET;
typedef int BOOL;

/* client data node*/
struct client_info {
    char nickname[NICKNAME_LENGTH];
    struct sockaddr_in clientaddr;
    SOCKET sockfd;
    int sendn;
    int receiven;
    struct client_info* prev;
    struct client_info* next;
};

/* client data is stored in linked-list*/
typedef struct client_info CLIENT;
CLIENT* client_info_head;
CLIENT* client_info_tail;

pthread_mutex_t lock;

void init_serverinfo(int* p_serverSocket, struct sockaddr_in* p_serveraddr, int addrlen);
char* init_clientinfo(SOCKET connectionSocket, struct sockaddr_in clientaddr, int addrlen);
CLIENT* append_clientinfo(SOCKET connectionSocket, struct sockaddr_in clientaddr, int addrlen);
void remove_clientinfo(CLIENT* target);
void* thread_main_func(void* socket);
void send_to_all(char* message, CLIENT* sender, MSG_TYPE type);
void send_to_one(char* message, CLIENT* sender, CLIENT* receiver);
void sigHandler(int sigNo);

CLIENT* is_exist(char* nickname);
BOOL is_duplicated_nickname(char* nickname);
BOOL dectect_villain(char* buf);
int get_client_number();
char* get_client_info();

int main(int argc, char const *argv[])
{
    
    SOCKET serverSocket, connectionSocket;
    socklen_t addrlen;
    int readn;
    char buf[MAX_LINE];
    int validation;
    pthread_t thread_id;
    struct sockaddr_in serveraddr, clientaddr;
    int current_client_number = 0;

    
    
    client_info_head = (CLIENT*)malloc(sizeof(CLIENT));
    client_info_tail = (CLIENT*)malloc(sizeof(CLIENT));
    if(pthread_mutex_init(&lock, NULL) != 0 ) {
        return 1;
    }

    init_serverinfo(&serverSocket, &serveraddr, sizeof(serveraddr));
    bind(serverSocket, (struct sockaddr*)&serveraddr,sizeof(serveraddr));
    listen(serverSocket,5);

    signal(SIGINT,(void*)sigHandler);

    while(1) {
        addrlen = sizeof(clientaddr);
        connectionSocket = accept(serverSocket,(struct sockaddr*)&clientaddr, &addrlen);

        /* Server sends the number of clients currently connected to the client attempting a new connection */       
        current_client_number = get_client_number();
        memset(buf,0x00,MAX_LINE);
        sprintf(buf,"%d",current_client_number);
        write(connectionSocket,buf,1);

        /* If current number of clients is 8, the connection is rejected and close socket.*/
        if(current_client_number == MAX_CLIENT) {
            close(connectionSocket);
            continue;
        }
        /* If current number of clients is under 8, */
        else {
            /* Client will send nickname to server, */
            read(connectionSocket,buf,MAX_LINE);
            /* And if received nickname is duplicated, send 1(If client receive 1, client will exist)
               + close socket */
            if(is_exist(buf) != NULL) {
                memset(buf,0x00,MAX_LINE);
                validation = 1;
                sprintf(buf,"%d",validation);
                write(connectionSocket,buf,1);
                close(connectionSocket);
            }
            /* If nickname is not duplicated, send 0(if client receive 0, client will continue the connection */
            else {
                memset(buf,0x00,MAX_LINE);
                validation = 0;
                sprintf(buf,"%d",validation);
                write(connectionSocket,buf,1);
                /* Thread for communication with Client <nickname> */
                pthread_create(&thread_id, NULL, thread_main_func, (void*)&connectionSocket);
                pthread_detach(thread_id);
            }
            memset(buf,0x00,MAX_LINE);
        }

    }
    
    
    free(client_info_head);
    free(client_info_tail);
    
    close(serverSocket);

    return 0;
}
/* Get number of clients currently connected */
int get_client_number() {
    int current_client_number = 0;
    CLIENT* temp;
   
    if(client_info_head->next == NULL)
        return 0;
    else {
        for(temp = client_info_head->next; temp->next != NULL; temp = temp->next) {
            current_client_number ++;
        }
    }
   

    return current_client_number;
}
/* Get all clients info in string form which is stored in linked-list*/
char* get_client_info() {
    char* buf;
    char info[128];
    CLIENT* target;
    buf = malloc(sizeof(char)*MAX_LINE);
    memset(buf,0x00,MAX_LINE);
    /* Need Mutex */
   
    if(client_info_head->next == NULL) {
        sprintf(buf,"No client.\n");
    }
    else {
        for(target = client_info_head->next; target->next != NULL ; target = target->next) {
            memset(info,0x00,128);
            sprintf(info,"<%s %s %d>\n",target->nickname,inet_ntoa(target->clientaddr.sin_addr),ntohs(target->clientaddr.sin_port));
            strcat(buf,info);
        }
    }
    
    return buf;
}

/* Initializing serverSocket, etc. */
void init_serverinfo(SOCKET* p_serverSocket, struct sockaddr_in*  p_serveraddr, int addrlen) {
    *p_serverSocket = socket(AF_INET,SOCK_STREAM,0);
    memset((void*)p_serveraddr, 0x00, addrlen);
    p_serveraddr->sin_family = AF_INET;
    p_serveraddr->sin_addr.s_addr = htonl(INADDR_ANY);
    p_serveraddr->sin_port = htons(PORTNUM);
}

/* Initializing the info of the newly connected client and stores it in the list */
/* ReturnVal = Node(CLIENT) pointer */
CLIENT* append_clientinfo(SOCKET connectionSocket, struct sockaddr_in clientaddr, int addrlen){
    
    CLIENT* newClient = (CLIENT*)malloc(sizeof(CLIENT));
    char* nickname = malloc(sizeof(char) * NICKNAME_LENGTH);
    
    memset(nickname,0x00,NICKNAME_LENGTH);
    newClient->sockfd = connectionSocket;
    newClient->sendn = 0;
    newClient->receiven = 0;
    
    read(newClient->sockfd,nickname,NICKNAME_LENGTH);
    
    memcpy((void*)&(newClient->clientaddr), &clientaddr, addrlen);
    memcpy(newClient->nickname, nickname, strlen(nickname));
    
    memcpy((void*)&(newClient->clientaddr), &clientaddr, addrlen);
    if(client_info_head->next == NULL && client_info_tail->prev == NULL) {
        client_info_head->next = newClient;
        newClient->prev = client_info_head;
        client_info_tail->prev = newClient;
        newClient->next = client_info_tail;
    }
    else {
        client_info_tail->prev->next = newClient;
        newClient->prev = client_info_tail->prev;
        newClient->next = client_info_tail;
        client_info_tail->prev = newClient;
    }
    
    free(nickname);

    return newClient;
}

/* Free 'target' client info and remove it from list */
void remove_clientinfo(CLIENT* target) {

    if(target->prev == client_info_head) {
        client_info_head->next = target->next;
        target->next->prev = client_info_head;
        target->next = NULL;
        target->prev = NULL;
    }
    else if(target->next == client_info_tail) {
        target->prev->next = client_info_tail;
        client_info_tail->prev = target->prev;
        target->prev = NULL;
        target->next = NULL;
    }
    else {
        target->prev->next = target->next;
        target->next->prev = target->prev;
        target->next = NULL;
        target->prev = NULL;
    }
    
    free(target);
}
/* For check duplicated nickname */
CLIENT* is_exist(char* nickname) {
   
    CLIENT* checker;
    
    if(client_info_head->next == NULL) {
        return NULL;
    }
    for (checker = client_info_head->next; checker->next != NULL; checker = checker->next) {
        if(checker->nickname != NULL) {
            if(strcmp(checker->nickname,nickname) == 0) {
                return checker;
            }
        }
    }
   
    return NULL;
}
/* For check duplicated nickname */
BOOL is_duplicated_nickname(char* nickname) {
    CLIENT* checker;
  
    if(client_info_head->next == NULL){
        return FALSE;
    }

    for(checker = client_info_head->next; checker->next != NULL ; checker = checker->next) {
        if(checker->nickname != NULL){
            if(strcmp(checker->nickname,nickname) == 0) {
                return TRUE;
            }
        }
    }
    
    return FALSE;
}

/* Detect "I hate professor" */
BOOL dectect_villain(char* buf) {
    char villain[] = "ihateprofessor";
    char temp[MAX_LINE];
    int index = 0;
    memcpy(temp,buf,MAX_LINE);
    
    for(int i = 0 ; temp[i] != '\0'; i++) {
        temp[i] = tolower(temp[i]);
        if(temp[i] != ' ') {
            temp[index] = temp[i];
            index++;
        }
    }
    temp[index] = '\0';
    if(strstr(temp,villain) != NULL) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}
/* Usage :
    1. For telling the client that the server is terminated
    2. For sending a chat message from one client to everyone
    3. For announcing new client has connected
    4. For announcing client has disconnected
*/
void send_to_all(char* message, CLIENT* sender, MSG_TYPE type) {
    
    CLIENT* target;
    char buf[MAX_LINE];

    /* Anounce for server termination */
    if(sender == NULL && type == NOTICE_SERVER_TERMINATION) {
        for(target = client_info_head->next; target->next != NULL ; target=target->next) {
            write(target->sockfd, message, MAX_LINE);
        }
        return;
    }
    
    memset(buf,0x00,MAX_LINE);
    
    /* Sending a chat message to everyone */
    if(type == REGULAR) {
        sprintf(buf,"%s> %s",sender->nickname,message);
    
        for(target = client_info_head->next; target->next != NULL; target = target->next) {
            if(target->sockfd == sender->sockfd) {
                continue;
            }
            write(target->sockfd, buf, MAX_LINE);
            target->receiven += 1;
        }
        sender->sendn += 1;
    }
    /* Announcing new client connected */
    else if(type == NOTICE_CLIENT_CONNECTED) {
        sprintf(buf,"[%s connected. There are %d users now.]\n",sender->nickname,get_client_number());
        for(target = client_info_head->next; target->next != NULL; target = target->next) {
            if(target->sockfd == sender->sockfd) {
                continue;
            }
            write(target->sockfd, buf, MAX_LINE);
        }
    }
    /* Announcing client disconnected */
    else if(type == NOTICE_CLIENT_DISCONNECTED) {
        
        sprintf(buf,"[%s is disconnected. There are %d users now.]\n",sender->nickname,get_client_number()-1);
        
        for(target = client_info_head->next; target->next != NULL; target = target->next) {
            if(target->sockfd == sender->sockfd) {
                continue;
            }
            write(target->sockfd, buf, MAX_LINE);
        }
    }
    else {
        return;
    }
    
}
/* Usage :
    1. For \stats
    2. For \list, \ver, \change
    3. For \w
*/
void send_to_one(char* message, CLIENT* sender, CLIENT* receiver) {
    CLIENT* from = sender;
    CLIENT* to = receiver;
    char buf[MAX_LINE];

    memset(buf,0x00,MAX_LINE);
    /*For \stats*/
    if(message == NULL && sender==receiver) {
        sprintf(buf,"Sent : %d\tReceived : %d\n",sender->sendn,sender->receiven);
        write(sender->sockfd,buf,MAX_LINE);    
    }
    /* For \list, \ver, \change */
    else if(sender==receiver) {
        write(sender->sockfd,message,MAX_LINE);
    }
    /* For \w */
    else {
        sprintf(buf,"\x1b[32m<From %s> %s\x1b[0m\n",sender->nickname,message);
        write(receiver->sockfd,buf,MAX_LINE);
        sender->sendn += 1;
        receiver->receiven += 1;
    }
    
}

/* 1 thread <-> 1 client */
void* thread_main_func(void* socket) {
    
    SOCKET sockfd = *((SOCKET*)socket);
    int readn;
    struct sockaddr_in clientaddr;
    socklen_t addrlen;
    char buf[MAX_LINE];
    char* result;
    char command[COMMAND_LENGTH];
    char nickname[NICKNAME_LENGTH];
    char message[MAX_LINE-NICKNAME_LENGTH-COMMAND_LENGTH];
    CLIENT* current;
    CLIENT* target;

    addrlen = sizeof(clientaddr);
    getpeername(sockfd, (struct sockaddr*)&clientaddr, &addrlen);
    
    /*Init client info*/
    pthread_mutex_lock(&lock);
    current = append_clientinfo(sockfd, clientaddr,addrlen);
    pthread_mutex_unlock(&lock);
    
    /*
        When connecting to the C-Client, the problem is that "read" for nickname is not done properly in 
        append_clientinfo function.
        This code is  prepared for failing to read a nickname from client.
    */
    if(strlen(current->nickname)==0) {
	    memset(nickname,0x00,NICKNAME_LENGTH);
	    for(;;){
            read(current->sockfd,nickname,NICKNAME_LENGTH);
            if(strlen(nickname) > 0)
                break;
        }
        memcpy(current->nickname,nickname,NICKNAME_LENGTH);
    }
    printf("New client : %s {%s(%d)} Connected.(N : %d)\n",current->nickname,inet_ntoa(current->clientaddr.sin_addr),ntohs(current->clientaddr.sin_port),get_client_number());
    memset(buf,0x00,MAX_LINE);
    
    /* Send welcome message to newly connected client */
    sprintf(buf,"Welcome %s to cau-cse chat room at <%s,%d> You are %dth user.",current->nickname,IPADDR,PORTNUM,get_client_number());
    send_to_one(buf,current,current);

    /* Announcing to every clients that new client has connected */
    send_to_all(NULL,current,NOTICE_CLIENT_CONNECTED);

    memset(buf,0x00,MAX_LINE);  

    /* If client send '\quit' or terminate program with ctrl+c, exit this while loop*/
    /* Read from client */
    while( (readn = read(sockfd,buf,MAX_LINE)) > 0 ) {
        
        if(strlen(buf) <= 0) {
            continue;
        }
        
        printf("[From : %s] Read Data %s(%d) : %s",current->nickname,inet_ntoa(clientaddr.sin_addr),ntohs(clientaddr.sin_port),buf);
        fflush(stdout);

        
        
        /* message starting with '\' means that this message is command */
        if(buf[0] == '\\') {
            sscanf(buf,"%s %s %[^\n]", command,nickname,message);
            /* If server received \quit, server send "CLIENT_QUIT" code and exit while loop
               (when client receive "CLIENT_QUIT" code, client will soon exit program gracefully) */
            if(strcmp(command,"\\quit") == 0) {
                memset(buf,0x00,MAX_LINE);
                sprintf(buf,"%s",CLIENT_QUIT);
                send_to_one(buf,current,current);
                break;
            }
            /* \stats : send client's send,receive number */
            else if(strcmp(command,"\\stats") == 0) {
                send_to_one(NULL,current,current);
            }
            /* \w <nickname> <message> : whisper to <nickname> */
            else if(strcmp(command,"\\w") == 0 ) {
                /* if <nickname> exist, send <message> to <nickname> */
                if((target = is_exist(nickname)) && (strcmp(nickname,current->nickname)!=0)) {
                    send_to_one(message,current,target);
                }
                /* if <nickname> is client-self, notice to client */
                else if(strcmp(nickname,current->nickname) == 0) {
                    memset(buf,0x00,MAX_LINE);
                    sprintf(buf,"You are <%s>. Are you serious?\n",nickname);
                    send_to_one(buf,current,current);
                }
                /* if <nickname> is not exist, notice to client */
                else {
                    memset(buf,0x00,MAX_LINE);
                    sprintf(buf,"User <%s> does not exist :)\n",nickname);
                    send_to_one(buf,current,current);
                }
            }
            /* \change <nickname> */
            else if(strcmp(command,"\\change") == 0) {
                /* if <nickname> is already exist, notice to client*/
                if(is_exist(nickname)) {
                    memset(buf,0x00,MAX_LINE);
                    sprintf(buf,"Nickname <%s> is already exist.\n",nickname);
                    send_to_one(buf,current,current);
                }
                /* if <nickname> is not exist, change client nickname to <nickname>*/
                else {
                    memcpy(current->nickname,nickname,NICKNAME_LENGTH);
                    memset(buf,0x00,MAX_LINE);
                    sprintf(buf,"Your nickname changed : <%s>\n",nickname);
                    send_to_one(buf,current,current);
                }
            }
            /* \ver */
            else if(strcmp(command,"\\ver") == 0) {
                memset(buf,0x00,MAX_LINE);
                sprintf(buf,"Server 1.3v Client 1.1v\n");
                send_to_one(buf,current,current);
            }
            /* \list */
            else if(strcmp(command,"\\list") == 0) {
                memset(buf,0x00,MAX_LINE);
                result = get_client_info();
                send_to_one(result,current,current);
                free(result);
            }
            else {
                continue;
            }
        }
        /* In other cases, it is just chat message, so sending to everyone */
        else {
            send_to_all(buf,current,REGULAR);
        }

        /* Detecting whether "i hate professor" in client message */
        if(dectect_villain(buf) == TRUE) {
            /* Send "VILLAIN_DETECTED" code to this client */
            memset(buf,0x00,MAX_LINE);
            sprintf(buf,"%s",VILLAIN_DETECTED);
            send_to_one(buf,current,current);
            break;
        }

    }
    /* break the while loop means client send '\quit' or Ctrl+C*/
    /* announce to other client that this client is quit. */
    send_to_all(NULL,current,NOTICE_CLIENT_DISCONNECTED);
    printf("%s end\n",current->nickname);

    /*close socket & remove clientinfo*/
    pthread_mutex_lock(&lock);
    close(sockfd);
    remove_clientinfo(current);
    pthread_mutex_unlock(&lock);

    return 0;   
}

/*If server terminated with Ctrl+C, This clean-up routine will called*/
void sigHandler(int sigNo) {
    char buf[MAX_LINE];
    CLIENT* target;

    /* Announce to every clients that server terminated */
    /* And, close socket, free clientinfo */
    /* If client receive "SERVER_QUIT", client will terminate too */
    memset(buf,0x00,MAX_LINE);
    sprintf(buf,"%s",SERVER_QUIT);

    if(client_info_head->next != NULL){
        send_to_all(buf,NULL,NOTICE_SERVER_TERMINATION);
        for(target = client_info_head->next ; target->next != NULL ; target = target->next) {
            close(target->sockfd);
            free(target);
        }
    }
    free(client_info_head);
    free(client_info_tail);
    printf("\nBye bye~\n");
    exit(0);
}
