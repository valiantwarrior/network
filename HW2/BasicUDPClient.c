/*
 * 20134220 Jeong hyun, Woo
 * BasicUDPClient.c
*/




#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>


#define MAXLINE 2048
#define IP      "165.194.35.202" // nsl2.cau.ac.kr
#define PORT    24220

void printMenu();
void sigHandler(int sigNo);

int main(int argc, char** argv) {
    clock_t start,end,elapsed;
    int option;
    int sockfd;
    char buf[MAXLINE];
    char *tok;
    struct sockaddr_in addr;
    socklen_t addrlen;

    if((sockfd = socket(AF_INET, SOCK_DGRAM,0)) == -1) {
        return 1;
    }
    memset((void*)&addr, 0x00, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(IP);
    addr.sin_port = htons(PORT);

    addrlen = sizeof(addr);

    /* KeyboradInterrupt handler(Ctrl + C) */
    signal(SIGINT,(void*)sigHandler);

    for(;;) {
        printMenu();
        memset(buf,0x00,MAXLINE);
        read(0,buf,MAXLINE);    // get command
        option = atoi(buf);

        /* if command is not in 1~5, ask new command input */
        if(option < 1 || option > 5) {
            printf("Invalid option : 1 ~ 5\n");
            continue;
        }
        
        /* command 1 ~ 5 */
        switch(option) {
            /* command 1 : echo with uppercase */
            case 1:
                start = clock();
                /* send command to the server */
                sendto(sockfd, (void*)&buf, 1,0,(struct sockaddr*)&addr,addrlen);
                end = clock();
                elapsed = end-start; // round-trip time spent sending command to the server

                printf("Input lowercase sentence: ");
                fflush(stdout);

                /* get user's input(sentence) */
                memset(buf,0x00,MAXLINE);
                read(0,buf,MAXLINE);
                start = clock();

                /* send sentence to the server and receive uppercase-sentence from the server */
                sendto(sockfd, (void*)&buf, sizeof(buf),0,(struct sockaddr*)&addr,addrlen);
                memset(buf,0x00,MAXLINE);
                recvfrom(sockfd, (void*)&buf, sizeof(buf),0,(struct sockaddr*)&addr,&addrlen);

                end = clock(); 
                elapsed += end-start; // round-trip time spent sending and receiving sentece.
                /* print result */
                fprintf(stdout, "\nReply from server: %s\n",buf);
                fprintf(stdout,"Response time : %ld ms\n\n",elapsed);
                break;

            /* command 2 : get client's IP & port*/
            case 2:
                start = clock();
                
                /* send command to the server */
                sendto(sockfd, (void*)&buf, 1,0,(struct sockaddr*)&addr,addrlen);
                memset(buf,0x00,MAXLINE);
                /* receive client's ip & port info.*/
                recvfrom(sockfd, (void*)&buf, sizeof(buf),0,(struct sockaddr*)&addr,&addrlen);
                
                end = clock();
                elapsed = end - start; // round-trip time

                /* format : xxx.xxx.xxx.xxx,PORT -> split string with delimeter ','
                 * print result */
                tok = strtok(buf,",");
                fprintf(stdout,"\nReply from server: IP=%s",tok);
                fflush(stdout);
                tok = strtok(NULL,",");
                fprintf(stdout," PORT=%s\n",tok);
                fprintf(stdout,"Response time : %ld ms\n\n",elapsed);
                break;

            /* command 3 : get the server's current time */
            case 3:
                start = clock();
                /* send command to the server */
                sendto(sockfd, (void*)&buf, 1,0,(struct sockaddr*)&addr,addrlen);
                
                /* receive the server's current time */
                memset(buf,0x00,MAXLINE);
                recvfrom(sockfd, (void*)&buf, sizeof(buf),0,(struct sockaddr*)&addr,&addrlen);
                end = clock();
                elapsed = end - start; // round-trip time

                /* print result */
                fprintf(stdout,"\nReply from server: %s\n",buf);
                fprintf(stdout,"Response time : %ld ms\n\n",elapsed);
                break;
            
            /* command 4 : get the server's service count */
            case 4:
                start = clock();
                /* send command to the server */
                sendto(sockfd, (void*)&buf, 1,0,(struct sockaddr*)&addr,addrlen);
                /* receive the server's service count */
                memset(buf,0x00,MAXLINE);
                recvfrom(sockfd, (void*)&buf, sizeof(buf),0,(struct sockaddr*)&addr,&addrlen);
                end = clock();
                elapsed = end - start; // round-trip time
                
                /* print result */
                fprintf(stdout,"\nReply from server: %s\n",buf);
                fprintf(stdout,"Response time : %ld ms\n\n",elapsed);
                break;

            default:
                break;
        }

        /* if command is 5 or Ctrl + C, terminate program */
        if(option == 5 || option == 0) {
            close(sockfd);
            printf("Bye bye~~\n");
            break;
        }

    }
    
    return 0;
}

void sigHandler(int sigNo) {
    printf("\nBye bye~~\n");
    exit(1);
}
void printMenu() {
    printf("<Menu>\n");
    printf("1) convert text to UPPER-case\n");
    printf("2) get my IP address and port number\n");
    printf("3) get server time\n");
    printf("4) get server service count\n");
    printf("5) exit\n");
    printf("Input option: ");
    fflush(stdout);
}