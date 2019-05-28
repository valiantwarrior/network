/*
 * 20134220 Jeong hyun, Woo
 * BasicTCPClient.c
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
#define IP "165.194.35.202" //nsl2.cau.ac.kr
#define PORT    24220

void printMenu();
void sigHandler(int sigNo);

int main(int argc, char** argv) {

    clock_t start,end,elapsed;
    struct sockaddr_in serveraddr;
    int option;
    int server_sockfd;
    int client_len;
    char buf[MAXLINE];
    char* tok;


    if((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("error");
        return 1;
    }

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(IP);
    serveraddr.sin_port = htons(PORT);
    

    client_len = sizeof(serveraddr);

    if((connect(server_sockfd, (struct sockaddr*)&serveraddr, client_len)) == -1) {
        perror("connect error");
        return 1;
    }

    /* when user terminate program by Ctrl + c, print 'Bye bye~~' at terminal */
    signal(SIGINT,(void*)sigHandler);
    

    for(;;){
        /* option select */
        printMenu();
        memset(buf,0x00,MAXLINE);
        read(0,buf,MAXLINE);
        option = atoi(buf);
        
        /* If user input command is not in 1 ~ 5, Ask new input command */
        if(option < 1 || option > 5){
            printf("Invalid option : 1 ~ 5\n");
            continue;
        }

        /* If user input command is in 1 ~ 5 */
        switch(option) {
            /* command 1 : echo with lower -> upper */
            case 1 :
                start = clock(); 
                /* send command to server, in this case, 1 is send to server */
                if(write(server_sockfd, buf, 1) <= 0) {
                    perror("write error");
                    return 1;
                }
                end = clock(); 
                elapsed = end - start; // round-trip response time that spent sending command to the server

                /* get sentence */
                printf("Input lowercase sentence: ");
                fflush(stdout);
                memset(buf,0x00,MAXLINE);
                read(0,buf,MAXLINE);
                
                /* send to server */
                start = clock();
                if(write(server_sockfd, buf, strlen(buf)) <= 0) {
                    perror("write error");
                    return 1;
                }

                /* receive from server */
                memset(buf, 0x00, MAXLINE);
                if(read(server_sockfd, buf, MAXLINE) <= 0) {
                    perror("read error");
                    return 1;
                }

                end = clock();
                elapsed += end - start; // round-trip response time that spent sending and receiving sentence to the server
                
                /* print result */
                fprintf(stdout,"\nReply from server: %s",buf);
                fprintf(stdout,"Response time : %ld ms\n\n",elapsed);
                break;

            /* command 2 : get client's IP address & Port number */
            case 2 :
                start = clock();

                /* send command to the server, in this case, 2 is sent to server */
                if(write(server_sockfd, buf, 1) <= 0) {
                    perror("write error");
                    return 1;
                }
                
                /* recevie client's ip & port from server */
                memset(buf,0x00, MAXLINE);
                if(read(server_sockfd,buf,MAXLINE) <= 0) {
                    perror("ip,port read error");
                    return 1;
                }
                end = clock();
                elapsed = end - start; // round-trip response time
                
                /* print result 
                 * receive format : xxx.xxx.xxx.xxx,PORT -> split string with delimeter ','*/
                tok = strtok(buf,",");
                fprintf(stdout,"\nReply from server: IP=%s",tok);
                fflush(stdout);
                tok = strtok(NULL,",");
                fprintf(stdout," PORT=%s\n",tok);
                fprintf(stdout,"Response time : %ld ms\n\n",elapsed);
                break;

            /* command 3 : get server's current time */
            case 3 :
                start = clock();
                /* send command to the server, in this case, 3 is sent to server */
                if(write(server_sockfd, buf, 1) <= 0) {
                    perror("write error");
                    return 1;
                }
                /* receive the server's current time */
                memset(buf,0x00, MAXLINE);
                if(read(server_sockfd,buf,MAXLINE) <= 0) {
                    perror("read error");
                    return 1;
                }
                end = clock();
                elapsed = end - start; // round-trip response time

                /* print result */
                fprintf(stdout,"\nReply from server: %s\n",buf);
                fprintf(stdout,"Response time : %ld ms\n\n",elapsed);
                break;
            
            /* command 4 : get the server's service count */
            case 4 :
                start = clock();
                /* send command to the server, in this case, 4 is sent to server */
                if(write(server_sockfd, buf, 1) <= 0) {
                    perror("write error");
                    return 1;
                }

                /* receive the server's service count */
                memset(buf,0x00,MAXLINE);
                if(read(server_sockfd,buf,MAXLINE) <= 0) {
                    perror("read error");
                    return 1;
                }
                end = clock();
                elapsed = end - start; // round-trip response time

                /* print result */
                fprintf(stdout,"\nReply from server: %s\n",buf);
                fprintf(stdout,"Response time : %ld ms\n\n",elapsed);
                break;

            default :
                break;
        }

        
        /* if command is 5 or Ctrl + c, send command to server and terminate program */
        if(option == 5 || option == 0) {
            if(write(server_sockfd, buf, 1) <= 0) {
                    perror("write error");
                    return 1;
                }
            close(server_sockfd);
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