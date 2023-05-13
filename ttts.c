#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netdb.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>
#include "tttfunctions.h"
#include <time.h>
#include <pthread.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#define SERVER_QUEUE 4


//Structs

//Keep track of current connections
//Each connection should have a name
//TODO: Make function to search all names and
//Make sure to add unique name
//TODO: Need to make sure these connections are still active
//and if they aren't remove them




//Globals
volatile int active = 1;
currentConnections_t **currentConns = NULL;
currentConnections_t *tempConn = NULL;
//int testCode = 0;
//prototypes
int openServer(char*, int);

//Signal handler function
void handler(int signum)
{
    active = 0;
}
// Signal handler parameters
void install_handlers(void)
{
    struct sigaction act;
    act.sa_handler = handler;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGTERM, &act, NULL);
}


// Main function to actually open the server on the port specified
int openServer(char* serverPort, int serverQueue)
{
    struct addrinfo hint, *info_list, *info; // Structs to receive info from sockets
    int error;
    int sock; // Socket number for the server

    // Options for the types of connection for the server
    memset(&hint, 0, sizeof(struct addrinfo));
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;

    // (?) Getting info about the current enviornment that the server is going
    // to be set up on; stores ways for server to bind to a port
    error = getaddrinfo(NULL, serverPort, &hint, &info_list);
    if(error)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
        return -1;
    }

    // Main loop logic for the server to try and bind to a port
    for(info = info_list; info != NULL; info = info ->ai_next)
    {
        sock = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
        
        // if a method of socket making did not work, try another
        if(sock == -1)
        {
            continue;
        }

        // Actually bind the socket to a port so that it can accept other connections
        error = bind(sock, info->ai_addr, info->ai_addrlen);
        if(error)
        {
            close(sock);
            continue;
        }

        // Make the socket listen for other connections on its binded port
        error = listen(sock, serverQueue);
        if(error)
        {
            close(sock);
            continue;
        }

        break;

    }

    // Freeing the list of ways for the server to make to a socket
    struct sockaddr_storage addr;
    socklen_t addr_len = sizeof(addr);
    getsockname(sock, (struct sockaddr *)&addr, &addr_len);
    struct sockaddr_in *s = (struct sockaddr_in *)&addr;
    printf("Server IP address: %s\n", inet_ntoa(s->sin_addr));
    freeaddrinfo(info_list);

    // Ultimately the server was never set up on any port
    if(info == NULL)
    {
        fprintf(stderr, "Could not bind\n");
        return -1;
    }

    return sock;
}






// Main function
// Where logic handling will go
int main(int argc, char**argv)
{
    // Server needs to be started with a port number
    // ./ttts 50000
    if(argc < 2)
    {
        printf("No port was specified\n");
        exit(EXIT_FAILURE);
    }
    if(argc == 3 && strcmp(argv[2], "Test") == 0)
    {
        testCode = 1;
    }
    
    char *serverPort; // Port number for the server
    serverPort = argv[1];
    printf("Port: %s\n", serverPort);

    //Binding the server to a port and making it listen for connections
    int server = openServer(serverPort, SERVER_QUEUE);
    if(server < 0)
    {
        exit(EXIT_FAILURE);
    }


    
    struct ifaddrs *ifaddr, *ifa;
    char host[100];
    int family, s;
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(1);
    }
    // loop through all the network interfaces
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) {
            continue;
        }
        family = ifa->ifa_addr->sa_family;
        // check for IPv4 or IPv6 addresses that are not 0.0.0.0
        if (family == AF_INET || family == AF_INET6) {
            s = getnameinfo(ifa->ifa_addr, (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6),
                            host, 100, NULL, 0, NI_NUMERICHOST);
            if (s != 0) {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(1);
            }
            if (strcmp(host, "0.0.0.0") != 0) {
                printf("Connect to: %s\n", host);
            }
        }
    }
    freeifaddrs(ifaddr);










    puts("Looking for players to connect\n");

    //Wait until someone connects to the server

    while(active)
    {

        struct sockaddr_storage player1; // Will store the connection for player 1
        struct sockaddr_storage player2; // Will store the connection for player 2
        socklen_t player_len; 


        struct sockaddr_storage* player1malloc = NULL;
        struct sockaddr_storage* player2malloc = NULL;

        //Accept the first connection
        int sock1 = accept(server, (struct sockaddr *)&player1, &player_len);
        while(sock1 < 0)
        {
            perror("accept");
            sock1 = accept(server, (struct sockaddr *)&player1, &player_len);
            printf("sock1: %d\n", sock1);
        }
        player_len = sizeof(player1); // not sure what this is needed for other than the accept
        
        //Add the connection to the currentConns struct
        //Keeping all of the logic just incase it needs to move somewhere and needs tssshat functionality
        if(currentConns == NULL || *currentConns == NULL)
        {
                tempConn = (currentConnections_t*) malloc(sizeof(currentConnections_t));

                player1malloc = (struct sockaddr_storage*) malloc(sizeof(struct sockaddr_storage));
                memcpy(player1malloc, &player1, sizeof(struct sockaddr_storage));
                (tempConn)->conn = (struct sockaddr*) player1malloc;
                pthread_mutex_init(&(tempConn)->lock, NULL);
                (tempConn)->name = NULL;
                (tempConn)->next = NULL;

                currentConns = &tempConn;
        }
        else{
            player1malloc = (struct sockaddr_storage*) malloc(sizeof(struct sockaddr_storage));
            memcpy(player1malloc, &player1, sizeof(struct sockaddr_storage));
            
            addConn((struct sockaddr*) player1malloc, *currentConns);
        }

        puts("Got player 1"); //Got 1 connection

        //Prompt player 1 for playerMessage
        write(sock1, "Enter the play message with a name to join a game", 49);

        //Create a game for the two connections
        game_t *gameToPass = (game_t *)malloc(sizeof(game_t));
        //Initialize the game board
        gameToPass->player1 = NULL;
        gameToPass->player2 = NULL;
        gameToPass->board = NULL;

        //initialized board to all .'s
        initializeGameBoard(gameToPass);

        //Prompt the connection to enter their name while waiting for another player
        //Create a thread to get the playerMessage from player1
        pthread_t player1PlayThread;
        playArgs_t *readPlay1Args = (playArgs_t*)malloc(sizeof(playArgs_t));
        readPlay1Args->sock = sock1;
        readPlay1Args->player = player1malloc;
        readPlay1Args->len = player_len;
        readPlay1Args->currentConnections = *currentConns;
        readPlay1Args->active = active;
        readPlay1Args->currentGame = gameToPass;

        int p1ThreadError = pthread_create(&player1PlayThread, NULL, readPlay, (void*)readPlay1Args);
        if(p1ThreadError != 0)
        {
            perror("pthread_create failed");
            return -1;
        }


        //Wait for another player
        int sock2 = accept(server, (struct sockaddr *)&player2, &player_len);
        while(sock2 < 0)
        {
            perror("accept");
            sock2 = accept(server, (struct sockaddr *)&player1, &player_len);
        }
        player2malloc = (struct sockaddr_storage*) malloc(sizeof(struct sockaddr_storage));
        memcpy(player2malloc, &player2, sizeof(struct sockaddr_storage));
        addConn((struct sockaddr*) player2malloc, *currentConns); //add the second connection to the struct
        puts("Got player 2");
        
        //Prompt player 1 for playerMessage
        write(sock2, "Enter the play message with a name to join a game", 49);

        pthread_t player2PlayThread;
        playArgs_t *readPlay2Args = (playArgs_t*)malloc(sizeof(playArgs_t));
        readPlay2Args->sock = sock2;
        readPlay2Args->player = player2malloc;
        readPlay2Args->len = player_len;
        readPlay2Args->currentConnections = *currentConns;
        readPlay2Args->active = active;
        readPlay2Args->currentGame = gameToPass; //Isn't initialized yet, may cause error
        
        int p2ThreadError = pthread_create(&player2PlayThread, NULL, readPlay, (void*)readPlay2Args);
        if(p2ThreadError != 0)
        {
            perror("pthread_create failed");
            return -1;
        }
        
        if(sock1 < 0 || sock2 < 0)
        {  
            perror("accept");
            return -1;
        }
        



        
        int p1JoinError = pthread_join(player1PlayThread, NULL);
        int p2JoinError = pthread_join(player2PlayThread, NULL);

        if(p1JoinError != 0 || p2JoinError != 0)
        {
            perror("pthread_join failed");
            return -1;
        }

        //Create a thread for the game
        arguments_t *args = (arguments_t*)malloc(sizeof(arguments_t));
        args->sock1 = sock1;
        args->sock2 = sock2;
        args->player1 = player1malloc;
        printf("player1: %p\n", player1malloc);
        args->player2 = player2malloc;
        printf("player2: %p\n", player2malloc);
        args->len = &player_len;
        args->currentConnections = currentConns;
        args->currentGame = gameToPass;

        pthread_t thread;
        pthread_create(&thread, NULL, gameLoop, (void*)args);
        pthread_detach(thread);

    }        

    //THIS BREAK WILL NOT BE HERE EVENTUALLY

    //NEED TO FREE EVERYTHING!!!!!
    freeCurrenconns(*currentConns);
    //free(player1);
    //free(player2);
    //freeGame(currentGame);
    close(server);
    
    
    

    //write(sock, "Closing connection", 19);
    //close(sock);
    
}
