/*
 * 20134220 Jeong hyun, Woo
 * BasicUDPServer.c
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
    int serviceCount = 0;
    int sockfd;
    socklen_t addrlen;
    struct sockaddr_in addr, cliaddr;
    char buf[MAXBUF];
    char ip[MAXBUF];
    int option;
    time_t currentTime;
    struct tm tm;

    /* ctrl + c handler */
    signal(SIGINT,(void*)sigHandler);

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        return 1;
    }
    memset((void*)&addr, 0x00, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    addrlen = sizeof(cliaddr);
    if((bind(sockfd,(struct sockaddr*)&addr,addrlen)) == -1) {
        return 1;
    }
    printf("The server is ready to receive on port %d\n",PORT);

    for(;;){
        memset(buf, 0x00, MAXBUF);
        /* receive command from client */
        recvfrom(sockfd,(void*)&buf,sizeof(buf),0,(struct sockaddr*)&cliaddr,&addrlen);
        printf("Connection requested from ('%s', %d)\n", inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);       
        option = atoi(buf);
        printf("Command %d\n",option);

        /* command 1 ~ 5 */
        switch(option) {
            case 1:
                memset(buf,0x00,MAXBUF);
                /* receive sentence */
                recvfrom(sockfd,(void*)&buf,sizeof(buf),0,(struct sockaddr*)&cliaddr,&addrlen);
                /* lowercase to uppercase */
                for(int i = 0; buf[i] != '\0' ; i++) {
                    buf[i] = toupper(buf[i]);
                }
                /* send uppercase-sentence */
                sendto(sockfd, (void*)buf, sizeof(buf), 0, (struct sockaddr*)&cliaddr,addrlen);
                serviceCount++;
                break;

            case 2:
                memset(buf, 0x00, MAXBUF);
                memset(ip,0x00,MAXBUF);
                /* format : xxx.xxx.xxx.xxx,PORT */
                sprintf(buf,"%d",cliaddr.sin_port);
                strcpy(ip,inet_ntoa(cliaddr.sin_addr));
                strcat(ip,",");
                strcat(ip,buf);
                /* send client's ip & port info with format "xxx.xxx.xxx.xxx,PORT" */
                sendto(sockfd, (void*)ip, sizeof(ip), 0, (struct sockaddr*)&cliaddr,addrlen);
                serviceCount++;
                break;
            
            case 3:
                memset(buf,0x00,MAXBUF);
                /* get server's current time */
                currentTime = time(NULL);
                tm = *localtime(&currentTime);
                /* send server's current time with format "YYYY-MM-DD HH:MM:SS" */
                sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
                sendto(sockfd, (void*)buf, sizeof(buf), 0, (struct sockaddr*)&cliaddr,addrlen);
                serviceCount++;
                break;

            case 4:
                serviceCount++;
                /* send server's service count */
                sprintf(buf,"%d",serviceCount);
                sendto(sockfd, (void*)buf, sizeof(buf), 0, (struct sockaddr*)&cliaddr,addrlen);
                break;
                
            default :
                break;
        }

    }

    close(sockfd);
    return 0;
}

void sigHandler(int sigNo) {
    printf("\nBye bye~\n");
    exit(1);
}