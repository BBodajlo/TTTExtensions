#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <pthread.h>
#include "tttfunctions.h"

#define BUFFERLEN 256
//int testCode = 0;
int connect_inet(char *host, char* port)
{
    struct addrinfo hints, *info_list, *info;
    int sock;
    int error;

    // look up remote host
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // in practice, this means give us IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // indicate we want a streaming socket

    error = getaddrinfo(host, port, &hints, &info_list);
    if (error) {
        fprintf(stderr, "error looking up %s:%s: %s\n", host, port,
        gai_strerror(error));
    return -1;
    }

    for (info = info_list; info != NULL; info = info->ai_next) 
    {
        sock = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
        if (sock < 0) 
        {
            continue;
        }

        error = connect(sock, info->ai_addr, info->ai_addrlen);
        if (error) 
        {
            close(sock);
            continue;
        }
        
        break;
    }
    freeaddrinfo(info_list);

    if (info == NULL) 
    {
        fprintf(stderr, "Unable to connect to %s:%s\n", host, port);
        return -1;
    }

    return sock;
}

int readOver(char *buf)
{
    if(strstr(buf, "OVER|") != NULL)
    {
        return(TRUE);
    }
    return(FALSE);
}

void* readFromServer(void *args)
{
    readInputArgs_t *arguments = (readInputArgs_t*)args;
    int sock = arguments->sock;
    free(arguments);
    char startMsg[BUFFERLEN];
    read(sock, startMsg, 49);
    printf("%s\n", startMsg);
    while(TRUE)
    {
        char recvBuf[BUFFERLEN];
        //printf("This is bytes: %d\n", bytes);
        int bytesToRead = 0;

        read(sock, recvBuf, PROTOCOLSTARTLEN);
        //printf("%s\n", recvBuf);
        
        if(findMessage(recvBuf) == WAIT)
        {
            puts("WAIT");
            read(sock, recvBuf, 2);
        }
        else if(findMessage(recvBuf) == MOVE)
        {
            puts("OKAY");
        }
        else if(findMessage(recvBuf) == BEGN)
        {
            bytesToRead = getNumOfBytesToRead(sock, recvBuf);
            read(sock, recvBuf, bytesToRead);
            //printf("This is buffer from begn: %s\n", recvBuf);
            printf("Your role is: %c\n", recvBuf[0]);
            puts("");
            

        }
        if(readOver(recvBuf) == TRUE)
        {
            puts("Server has ended the game");
            break;
        }
        memset(recvBuf,0,BUFFERLEN);
    }


}



// Main function
// This is where all of the logic/decision making will go for the client side
int main(int argc, char** argv)
{

    int sock; //The socket id got from connecting to server
    int bytes; // Number of bytes the client has read from std input

    char buf[BUFFERLEN];
    

    // The client needs to have a host and port as args
    // ./ttt localhost 50000
    if(argc < 3)
    {
        printf("Host and port were not specified");
        exit(EXIT_FAILURE);
    }

    // The main function to connect to a server
    sock = connect_inet(argv[1], argv[2]);
    // If the socket was unable to connect (safety feature)
    if(sock < 0)
    {
        exit(EXIT_FAILURE);
    }

    pthread_t readInput;
    readInputArgs_t *inputArgs = (readInputArgs_t*)malloc(sizeof(readInputArgs_t));
    inputArgs->sock = sock;
    int p2ThreadError = pthread_create(&readInput, NULL, readFromServer, (void*)inputArgs);
    if(p2ThreadError != 0)
    {
        perror("pthread_create failed");
        return -1;
    }
    pthread_detach(readInput);

    if(argc > 3)
    {
        if(strcmp(argv[3], "Regular") == 0)
        {
            write(sock, "PLAY|5|JOHN|MOVE|6|X|3,3|MOVE|6|X|2,2|DRAW|2|S|MOVE|6|X|1,1|", 61);
        }
        else if(strcmp(argv[3], "Draw") == 0)
        {
            write(sock, "PLAY|5|JOHN|MOVE|6|X|3,3|MOVE|6|X|2,2|DRAW|2|S|MOVE|6|X|1,1|", 61);
        }
        else if(strcmp(argv[3], "Tie") == 0)
        {
            write(sock, "PLAY|5|JOHN|MOVE|6|X|1,1|MOVE|6|X|1,3|MOVE|6|X|2,1|MOVE|6|X|2,2|MOVE|6|X|3,2|", 77);
        }
        else if(strcmp(argv[3], "Resign") == 0)
        {
        write(sock, "PLAY|5|JOHN|MOVE|6|X|1,1|RSGN|0|MOVE|6|X|2,1|MOVE|6|X|2,2|MOVE|6|X|3,2|", 71);
        }
    }
    
    //write(sock, "PLAY|5|JOHN|MOVE|6|X|3,3|MOVE|6|X|2,2|RSGN|0|MOVE|6|X|1,1|", 59);
     //write(sock, "PLAY|5|JOHN|", 61);
    // Main logic loop to just continally send messages to the server

    //Going to send the play message
    char name[BUFSIZE];
    read(STDIN_FILENO, name, BUFFERLEN);
    sendPLAYMessage(sock, name);


    while((bytes = read(STDIN_FILENO, buf, BUFFERLEN)) > 0)
    {
       
        //printf("%d\n", bytes);
        int error = write(sock, buf, bytes);
        if(error == -1)
        {
            printf("Error writing to server! %s\n", strerror(errno));
        }
        
    }
    close(sock);
    return EXIT_SUCCESS;
}

