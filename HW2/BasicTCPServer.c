/*
 * 20134220 Jeong hyun, Woo
 * BasicTCPCServer.c
*/



#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>

#define MAXBUF 2048
#define PORT 24220

void sigHandler(int sigNo);

int main(int argc, char** argv) {
    
    int server_sockfd, client_sockfd;
    int client_len, n, option, serviceCount = 0;
    char buf[MAXBUF];
    char ip[MAXBUF];
    time_t currentTime;
    struct tm tm;
    struct sockaddr_in clientaddr, serveraddr;

    client_len = sizeof(clientaddr);

    if((server_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        perror("socket error");
        exit(0);
    }
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(PORT);

    bind(server_sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    listen(server_sockfd,5);
    
    /* ctrl + c handler */
    signal(SIGINT,(void*)sigHandler);

    printf("The server is ready to receive on port %d\n",PORT);
    client_sockfd = accept(server_sockfd, (struct sockaddr*)&clientaddr, &client_len);

    
    
    while(1) {
        memset(buf, 0x00, MAXBUF);
    
        printf("Connection requested from ('%s', %d)\n", inet_ntoa(clientaddr.sin_addr), clientaddr.sin_port);
        
        /* If commands received from client are ctrl + c or 5, close client socket and wait for new client */
        if(read(client_sockfd,buf,MAXBUF) == 0 || atoi(buf) == 5) {
            
            printf("Command 5\n");
            close(client_sockfd);
            client_sockfd = accept(server_sockfd, (struct sockaddr*)&clientaddr, &client_len);
            continue;
        }
        else{
            option = atoi(buf);
        }
        fprintf(stdout,"Command %d\n",option);
        
        /* command 1 ~ 5 */
        switch(option) {

            case 1 :
                memset(buf,0x00,MAXBUF);
                /* receive sentence */
                if(read(client_sockfd, buf, MAXBUF) <= 0) {
                    perror("read error");
                    return 1;
                }
                /* lowercase to uppercase */
                for(int i =0; buf[i] != '\0'; i++) {
                    buf[i] = toupper(buf[i]);
                }
                /* send uppsercase-sentence */
                if(write(client_sockfd, buf, MAXBUF) <= 0) {
                    perror("write error");
                    return 1;
                }
                serviceCount++;
                break;
            
            case 2 :
                /*format : xxx.xxx.xxx.xxx,port*/
                memset(buf,0x00,MAXBUF);
                memset(ip,0x00,MAXBUF);
                sprintf(buf,"%d",clientaddr.sin_port);
                strcpy(ip,inet_ntoa(clientaddr.sin_addr));
                strcat(ip,",");
                strcat(ip,buf);
                
                /* send client's ip & port with format "xxx.xxx.xxx.xxx,PORT"*/
                if(write(client_sockfd, ip, MAXBUF) <= 0) {
                    perror("write error");
                    return 1;
                }
                serviceCount++;
                break;

            case 3 :
                memset(buf,0x00,MAXBUF);
                /* get server's current time */
                currentTime = time(NULL);
                tm = *localtime(&currentTime);
                /* time format : YYYY-MM-DD HH:MM:SS */
                sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
                /* send server's current time */
                if(write(client_sockfd, buf, MAXBUF) <= 0) {
                    perror("write error");
                    return 1;
                } 
                serviceCount++;
                break;
            
            case 4 :
                serviceCount++;
                /* send server's service count */
                sprintf(buf,"%d",serviceCount);
                if(write(client_sockfd, buf, MAXBUF) <= 0) {
                    perror("write error");
                    return 1;
                }
                break;
            default:
                break;
        }
        
    }
    close(server_sockfd);

    return 0;
}

void sigHandler(int sigNo) {
    printf("\nBye bye~\n");
    exit(1);
}