#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
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


// Main function
// This is where all of the logic/decision making will go for the client side
int main(int argc, char** argv)
{

    int sock; //The socket id got from connecting to server
    int bytes; // Number of bytes the client has read from std input

    char buf[BUFFERLEN];
    char recvBuf[BUFFERLEN];

    // The client needs to have a host and port as args
    // ./ttt localhost 50000
    if(argc < 3)
    {
        printf("Host and port and test were not specified");
        exit(EXIT_FAILURE);
    }

    // The main function to connect to a server
    sock = connect_inet(argv[1], argv[2]);
    // If the socket was unable to connect (safety feature)
    if(sock < 0)
    {
        exit(EXIT_FAILURE);
    }
    printf("here");
    int id = fork();

    if(id == 0)
    {
        while((bytes = read(sock, recvBuf, BUFFERLEN)) > 0)
        {
            //printf("This is bytes: %d\n", bytes);
            printf("%s\n", recvBuf);
            if(readOver(recvBuf) == TRUE)
            {
                puts("Server has ended the game");
                break;
            }
            memset(recvBuf,0,BUFFERLEN);
        }
    }
    if(argc > 3)
    {
        if(strcmp(argv[3], "Regular") == 0)
        {
            write(sock, "PLAY|5|TEDD|MOVE|6|O|1,3|MOVE|6|O|2,3|DRAW|2|R|MOVE|6|O|1,2|", 61);
        }
        else if(strcmp(argv[3], "Draw") == 0)
        {
            write(sock, "PLAY|5|TEDD|MOVE|6|O|1,3|MOVE|6|O|2,3|DRAW|2|A|MOVE|6|O|1,2|", 61);
        }
        else if(strcmp(argv[3], "Tie") == 0)
        {
        write(sock, "PLAY|5|TEDD|MOVE|6|O|1,2|MOVE|6|O|2,3|MOVE|6|O|3,1|MOVE|6|O|3,3|", 64);
        }
        else if(strcmp(argv[3], "Resign") == 0)
        {
        write(sock, "PLAY|5|TEDD|MOVE|6|O|1,2|MOVE|6|O|2,3|MOVE|6|O|3,1|MOVE|6|O|3,3|", 64);
        }
    }
   
    //write(sock, "PLAY|5|TEDD|", 12);
    // Main logic loop to just continally send messages to the server
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
    if(id != 0)
    {
        wait(0);
    }
    return EXIT_SUCCESS;
}

