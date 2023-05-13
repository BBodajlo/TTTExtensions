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
#include <time.h>
#include <pthread.h>
#include "tttfunctions.h"

int testCode = 0;
//Functions

// Used for checking the first 4 bytes of a message
// The first 4 bytes should be a command name
// Will only accept them if they are capitialized
// Uses the enums in the header to compare
int checkValidMessage(char *msg)
{
    msg[5] = '\0';
    for(int i = 0; i < NUM_OF_MESSAGE_TYPES; i++)
    {
        printf("Message: %s, Message Type: %s\n", msg, messageTypes[i]);
        if(strcmp(msg, messageTypes[i]) == 0)
        {
            return TRUE;
        }
    }
    return FALSE;
}


// Functions for the message structs

// Fuctions for the Play Message

// Adding a given name to the play message struct
void playerAddName(char *n)
{
   // msg->name = calloc(strlen(n)+1, 1); // Zero out memory for the field
   // strcpy(msg->name, n);
    // TODO(?): add a '\0' to the end of the name (might not need since strcpy works)
    
}


// GAME FUNCTIONS 
char checkTurn(char* board){
    int numX = 0;
    int numO = 0;

    for(int i = 0; i < 9; i++){
        if(board[i] == 'X'){
            numX++;
        }
        else if(board[i] == 'O'){
            numO++;
        }
    }

    if(numX == numO){
        return 'X';
    }
    else{
        return 'O';
    }
}

int checkValidMove(int row, int col, char* board){

    if(row < 1 || row > 3 || col < 1 || col > 3){
        return FALSE;
    }
    if(board[(row-1)*3 + (col-1)] != '.'){ // .s separate blanks
        return FALSE;
    }
    return TRUE;

}

int makeMove(int row, int col, char letter, char* board){
    if(checkValidMove(row, col, board) == TRUE && checkTurn(board) == letter){
        board[(row-1)*3 + (col-1)] = letter;
        return TRUE;
    }
    return FALSE;
}

char checkWin(char* board){
    // Check rows
    for(int i = 0; i < 3; i++){
        if(board[i * 3] == board[i * 3 + 1] && board[i * 3 + 1] == board[i * 3 + 2] && board[i * 3] != '.'){
            return board[i * 3];
        }
    }

    // Check columns
    for(int i = 0; i < 3; i++){
        if(board[i * 3] == board[(i+1)*3] && board[(i+1)*3] == board[(i+2)*3] && board[i * 3] != '.'){
            return board[i * 3];
        }
    }

    // Check diagonals
    if(board[0] == board[4] && board[4] == board[8] && board[0] != '.'){
        return board[0];
    }

    if(board[2] == board[4] && board[4] == board[6] && board[2] != '.'){
        return board[2];
    }

    // Check for no moves left
    for(int i = 0; i < 9; i++){
        if(board[i] == '.'){
            return 'N'; // No winner yet
        }
    }

    return 'D';   
}

//Functions for game struct

//Returns TRUE when player added
//Returns FALSE when the game is full
int gameAddPlayer(currentConnections_t *player, game_t *game)
{
    
    if(game->player1 == NULL)
    {
        game->player1 = player;
        return TRUE;
    }
    else if(game->player2 == NULL)
    {
        game->player2 = player;
        return TRUE;
    }
    else{
        puts("Game is full");
        return FALSE;
    }
    
}

//Check if the game has two players
int gameIsFull(game_t *game)
{
    if(game->player1 == NULL|| game->player2 == NULL)
    {
        return(FALSE);
    }
    return(TRUE);
}

//Check if a player is in the game
int gameIsPlayer(currentConnections_t *player, game_t *game)
{
    if(game->player1 == player || game->player2 == player)
    {
        return(TRUE);
    }
    return(FALSE);
}

//Functions for currentConnection Struct

//Add a connection called when first accepting a socket
//Initializes names and next to NULL
void addConn(struct sockaddr *conn,  currentConnections_t *currentConns)
{   
    puts("here1");
    pthread_mutex_lock(&currentConns->lock);
    currentConnections_t *ptr = currentConns;
    if(currentConns == NULL)
    {
        puts("is null");
    }
    else
    {
        puts("is not null");
    }
    while(ptr->next != NULL)
    {
        ptr = ptr->next;
    }
    //currentConnections_t *newConn = (currentConnections_t*)malloc(sizeof(currentConnections_t));
    ptr->next = (currentConnections_t*)malloc(sizeof(currentConnections_t));
    
    ptr->next->conn = conn;
    ptr->next->name = NULL;
    ptr->next->role = NULL;
    ptr->next->next = NULL;
    pthread_mutex_unlock(&currentConns->lock);
}

//Assigns a connection a name
//Called when the Play message is read
void addConnName(char *name, currentConnections_t *thisConn)
{
    /* thisConn->name = (char *)malloc(strlen(name)+1);
    memset(thisConn->name, 0, strlen(thisConn->name)+1); */
    thisConn->name = (char *)calloc(strlen(name) + 1, sizeof(char));
    strcpy(thisConn->name, name);
}

//Assigns a connection a role (X or 0)
//Used when creating a game
void addConnRole(char *role, currentConnections_t *thisConn)
{
    thisConn->role = calloc(sizeof(role), 0);
    strcpy(thisConn->role, role);
}

//Gets the connection node from the currenConns struct
//associated with the connection given
struct currentConnections* getConn(struct sockaddr *conn, currentConnections_t *currentConns)
{
    pthread_mutex_lock(&currentConns->lock);
    currentConnections_t *ptr = currentConns;
    while(ptr != NULL)
    {
        printf("Comparing %p to %p", ptr->conn, conn);
        if(ptr->conn == conn)
        {
            puts("Returning a valid conn from getConn");
            pthread_mutex_unlock(&currentConns->lock);
            return ptr;
        }
        ptr = ptr->next;
    }
    puts("Didn't find a connection in getConn");
    pthread_mutex_unlock(&currentConns->lock);
    //free(ptr);
}

void freeCurrenconns(currentConnections_t *conns)
{

    if(conns == NULL)
    {
        return;
    }

    while(conns != NULL)
    {
        
        currentConnections_t *temp = conns;
        free(temp->name);
        //free(temp->role);
        conns = conns->next;
        free(temp);
    }
    //free(conns);
    
}

//just need to free the board and the game struct
//the connections are handled by another free function
void freeGame(game_t *game)
{
    if(game != NULL)
    {
        free(game->board);
        free(game);
    }

    
}

//Goes through the currenConns and make sure
//there isn't a name already in use when adding a name through the
//play message
int nameInUse(char *name, currentConnections_t *currentConns)
{
    pthread_mutex_lock(&currentConns->lock);
    currentConnections_t *ptr = currentConns;
    while(ptr != NULL)
    {
        //puts("here");
        if(ptr->name != NULL && strcmp(name, ptr->name) == 0)
        {
            puts("someone has this name already");
            return TRUE;
        }
        ptr = ptr->next;
    }
    free(ptr);
    pthread_mutex_unlock(&currentConns->lock);
    return FALSE;
    
}

//Used to the the supposed number after each message; can be used for every message type
//So PLAY|5|NAME| will return 5
//It will then have the read pointer at the N, as it skips the | after the number
int getNumOfBytesToRead(int sock, char *buf)
{   
    char *numOfBytes = malloc(sizeof(char));
    int intSpot = 0;
    while(1)
    {
        int bytes = read(sock, buf, 1);
        printf("numtoreadfun: %s\n", buf);
        if(bytes == 0)
        {
            free(numOfBytes);
            return 0;
        }
        else
        {
            if(buf[0] != '|' && isdigit(buf[0]) != 0)
            {
                if(intSpot == 0)
                {
                    numOfBytes[0] = buf[0];
                    intSpot++;
                }
                else{
                    numOfBytes = (char *)realloc(numOfBytes, intSpot+1);
                    printf("numofBytes : %c\n", numOfBytes[0]);
                    numOfBytes[intSpot] = buf[0];
                    intSpot++;
                }
            }
            else if(buf[0] == '|')
            {
                numOfBytes = (char *)realloc(numOfBytes, intSpot+1);
                numOfBytes[intSpot] = '\0';
                printf("numofBytes : %c\n", numOfBytes[1]);
                printf("numofBytes : %s\n", numOfBytes);
                int returnValue = atoi(numOfBytes);
                free(numOfBytes);
                return returnValue;
            }
            else{
                free(numOfBytes);
                return 0;
            }
        }
    }
}


#define NAMEMAXLEN 21 // +1 for the bar not being a part of the name
//void* readPlay(int sock, struct sockaddr *rem, socklen_t rem_len, currentConnections_t *currentConns, volatile int active, game_t *currentGame)
void* readPlay(void* readPlayArgs)
{
    
    playArgs_t *args = (playArgs_t*)readPlayArgs;
    int sock = args->sock;
    struct sockaddr_storage *rem = args->player;
    socklen_t rem_len = args->len;
    int active = args->active;
    game_t *currentGame = args->currentGame;
    currentConnections_t *currentConns = args->currentConnections;
    free(args);



    char buf[BUFSIZE + 1], host[HOSTSIZE], port[PORTSIZE];
    int bytes, error;

    //Getting the info of a player connection
    //error = getnameinfo(rem, rem_len, host, HOSTSIZE, port, PORTSIZE, NI_NUMERICSERV);
    error = 0;
    if (error) 
    {
        fprintf(stderr, "getnameinfo: %s\n", gai_strerror(error));
        strcpy(host, "??");
        strcpy(port, "??");
        return(FALSE);
    }
    printf("Connection from %s:%s\n", host, port);

    
    // Always want to read 5 bytes since every message in
    // the protocol starts with 5 characters (PLAY + |)
    //Will probably need to change how this functions in general
    while(active)
    {
        
        bytes = read(sock, buf, PROTOCOLSTARTLEN); //Get the first 5 bytes for the message protocol
        //buf[bytes] = '\0';
       // printf("[%s:%s] read %d bytes |%s|\n", host, port, bytes, buf);
        
        //printf("Enum: %s\n", messageTypes[PLAY]);
        //int error = write(sock, buf, BUFSIZE);
        //printf("%d\n", error);

        //If we read less than 5 bytes, it can't be a proper message in the protocol
        /* if(bytes != 5)
        {
            puts("Invalid message protocol");
            sendINVLMessage(sock, "Received non-play protocol message");
            read(sock,buf,BUFSIZE);
        } */
        if(bytes == 5) //We have a proper len start to the message
        {
            //Checking to see if first 5 bytes match one of the protocols
            if(checkValidMessage(buf) == TRUE) 
            {
                puts("Valid message yay!!!");
                printf("here1\n");
                currentConnections_t *thisConn = getConn((struct sockaddr*)rem, currentConns);
                printf("here\n");
                //Checking to see if the message is the PLAY message (PLAY|)
                if(strcmp(buf, messageTypes[PLAY]) == 0)
                {
                    //Gets the node in the struct of current conns associated with this one
                    

                    //If it doesn't have a name, we should continue reading the message
                    if(thisConn->name == NULL)
                    {
                        //The next "token" should be the number of bytes to read next
                        //This call will deduce if it is formatted correctly and return the number
                        int numToRead = getNumOfBytesToRead(sock, buf);
                        //There wasn't anything after the (PLAY|) part
                        if(numToRead == 0)
                        {
                            //TODO: Return error message on format
                            puts("Did not read a number from message");

                            sendINVLMessage(sock, "Did not read a number from message");
                            read(sock,buf,BUFSIZE);
                        }
                        else //we got something to read
                        {
                            //printf("Num to read %d\n", numToRead);

                            //Read the next bit which should be the name
                            //PLAY|5| This part -> NAME|
                            //The | after the 5 is handled before this already
                            bytes = read(sock, buf, numToRead);
                            
                            printf("buffer: %s\n", buf);
                            //This will confirm if the message ended with a | meaning a correct number
                            //was given in the message; this will allow names to contain anything up to the amount given
                            //Minus 2 for \n and carriage return
                            //Imposing a 20 character limit for the name (+1 for the | after len)
                            if(bytes > NAMEMAXLEN) 
                            {
                                sendINVLMessage(sock, "Name is too long");
                            }
                            else if(buf[(numToRead)-1] == '|') //&& bytes-1 == numToRead)
                            {
                               // printf("numtoread: %d\n", numToRead);
                                //printf("Buf: %c\n", buf[numToRead-1]);
                                printf("Whole buff: %s\n", buf);

                                //Replace the | with an ending string character
                                buf[numToRead-1] = '\0';
                                
                                //TODO Fix the nameInUse to actually find the name/make sure struct is working properly
                                //Need to check if any of connection has the name given
                                //Current could not get this to work/give me a proper reading
                                if(nameInUse(buf, currentConns) == FALSE)
                                {

                                    //Finally can add the connection's name to the struct
                                    addConnName(buf, thisConn);
                                    //printf("Conn's name = %s\n", thisConn->name);
                                    //Read the rest of the buffer to clear
                                    //read(sock,buf,BUFSIZE);
                                    write(sock, "WAIT|0|", 7);
                                    gameAddPlayer(thisConn, currentGame);
                             
                                    //Stop reading since we've read what we wanted to
                                    break;
                                }
                                else{
                                    //TODO name already in use INVL
                                    puts("Name already in use");
                                    sendINVLMessage(sock, "Name already in use");
                                    //read(sock,buf,BUFSIZE);
                                }
                            }
                            else{
                                //TODO: Return error on format
                                puts("Play message len isn't correct");
                                sendINVLMessage(sock, "Play message length isn't correct");
                                //printf("buf: %s", buf);
                                //printf("Buf: %c\n", buf[numToRead-1]);
                                //read(sock,buf,BUFSIZE);
                                //break;
                            }
                        }
                    }
                    else{
                        //TODO
                        printf("This conn already has a name: %s", thisConn->name);
                        sendINVLMessage(sock, "This connection already has a name");
                        puts("This conn already has a name!!!");
                        //read(sock,buf,BUFSIZE);
                        //break;   
                    }
                    //free(thisConn);
                }
                else
                {
                    puts("Invalid message protocol");
                    sendINVLMessage(sock, "Received non-play protocol message");
                    read(sock,buf,BUFSIZE);
                }
                
            }
            else
            {
                // TODO: Send and invalid message back saying command
                // Was not recongnized or something
                puts("Invalid message protocol");
                sendINVLMessage(sock, "Received non-play protocol message");
                read(sock,buf,BUFSIZE);
            }
        }
        else
        {
            sendINVLMessage(sock, "No valid input was given");
        }
        memset(buf, 0, BUFSIZE);
    }
    
}


int beginMessage(int sock1, int sock2, currentConnections_t *player1, currentConnections_t *player2, game_t *game)
{
    //For choosing who is X and who is O
    srand(time(NULL));
    int randomNum = rand()%2; //Will be 0 or 1;

    //printf("rand = %d\n", randomNum);
    //printf("name: %s\n", player1->name);
    //printf("player 2 name in begin: %s\n", player2->name);
    //printf("name len: %ld\n", strlen(player1->name)+1);
    //char len = strlen(player1->name)+1;
    //char *len;
    char *beginStart = (char *)malloc(sizeof(char)*BUFSIZE);
    if(testCode == 0)
    {
        if(randomNum == 0)
        {
            player1->role = "X";
            player2->role = "O";
        // printf("role: %s\n", player1->role);
        // printf("player 2 role in begin: %s\n", player2->role);
        }
        else
        {
            player1->role = "O";
            player2->role = "X";
            //printf("role: %s\n", player1->role);
        // printf("player 2 role in begin: %s\n", player2->role);
        }
    }
    else if(testCode == 1)
    {
        player1->role = "X";
        player2->role = "O";
    }

    //Send to player 1 first (Getting player1->role and player2->name)

        /* strcpy(beginStart, "BEGN|");
        sprintf(len, "%ld", (strlen(player2->name)+4)); // +4 for 3 bars and the role (X or O)
        strcat(beginStart, len);
        strcat(beginStart, "|");
        strcat(beginStart, player1->role);
        strcat(beginStart, "|");
        strcat(beginStart, player2->name);
        strcat(beginStart, "|");
        printf("final: %s\n", beginStart); */
        sprintf(beginStart, "%s|%ld|%s|%s|", "BEGN", (strlen(player2->name)+3),
        player1->role, player2->name);
        write(sock1, beginStart, strlen(beginStart));
        memset(beginStart, 0, BUFSIZE);

        
        //Send to player 1 next (Getting player2->role and player1->name)
        /* strcpy(beginStart, "BEGN|");
        sprintf(len, "%ld", (strlen(player1->name)+4)); // +4 for 3 bars and the role (X or O)
        strcat(beginStart, len);
        strcat(beginStart, "|");
        strcat(beginStart, player2->role);
        strcat(beginStart, "|");
        strcat(beginStart, player1->name);
        strcat(beginStart, "|");
        printf("final: %s\n", beginStart); */
        sprintf(beginStart, "%s|%ld|%s|%s|", "BEGN", (strlen(player1->name)+3),
        player2->role, player1->name);
        write(sock2, beginStart, strlen(beginStart));
        
    free(beginStart);

}

//Will read the buffer with the given amount
//Returns buffer and the number of bytes read
int readBuffer(int sock, char *socketBuffer, int *b, int amountToRead)
{
    char buf[BUFSIZE + 1], host[HOSTSIZE], port[PORTSIZE];
    int bytes, error;

    //Getting the info of a player connection
    //error = getnameinfo(rem, rem_len, host, HOSTSIZE, port, PORTSIZE, NI_NUMERICSERV);
    
   
    bytes = read(sock, buf, amountToRead); 

    if(bytes != 0)
    {
        strcpy(socketBuffer, buf);
        *b = bytes;
        printf("From readBuffer: %s\n", buf);
        return(TRUE);
    }
    else if(bytes == -1)
    {
        puts("Got nothing in readBuffer");
        return(FALSE);
    }
    else
    {
        puts("Error in readBuffer");
        return(FALSE);
    }
   
}

//Returns the enum correlated to the protocol message
//Returns -1 if it isn't a protocol message
int findMessage(char *buffer)
{
    char protocolMessage[6];
    memcpy(protocolMessage, buffer, PROTOCOLSTARTLEN);
    protocolMessage[5] = '\0';
    puts("in find message");
    printf("readMessage: %s\n", protocolMessage);
    if(strcmp(protocolMessage, messageTypes[MOVE]) == 0)
    {
        puts("leaving find message");
        return(MOVE);
    }
    else if(strcmp(protocolMessage, messageTypes[RSGN]) == 0)
    {
        return(RSGN);
    }
    else if(strcmp(protocolMessage, messageTypes[DRAW]) == 0)
    {
        return(DRAW);
    }
    else{
        return -1;
    }
}

//Will set the gameboard to to all .'s 
//Board is size 10 for the null terminator
void initializeGameBoard(game_t *game)
{
    char board[10];
    for(int i = 0; i < 9; i++)
    {
        board[i] = '.';
    }
    board[9] = '\0';
    game->board = (char *)malloc(sizeof(char)*10); //10 for string terminator
    strcpy(game->board, board);
}

//Called when the server reads MOVE|
//It will get the number of bytes of the move message that was sent
//It will then determine if the message was formatted correctly
//If it passes all checks, it will return TRUE
//It will return FALSE if the message was wrong, or the move was invalid
int readMove(int sock, currentConnections_t *player, game_t *game, int *turnRow, int *turnCol)
{
    char buf[BUFSIZE];

    //Already have read the MOVE message, need the number next
    int numToRead = getNumOfBytesToRead(sock, buf);
    printf("NUM to read: %d", numToRead);
    //Should be the rest of the message
    //So bytes should be 7 (Move message + \n character)
    int bytes = read(sock, buf, numToRead); //+1 for \n 

    //Checking if message is the correct length as specified or less than the max allowed size
    //Should always only read 6 bytes since it is X|1,1|
    //Bytes-1 represents -1 for the \n
    //If not 7, then they didn't give a correctly formatted message and less specific error checking is needed
    //bytes-1 == numToRead && 
    if((bytes == 6 )|| bytes > BUFSIZE)
    {
        //Checking if the message contained the correct player role; they stated they were either X or O correctly
        if(buf[0] == player->role[0])
        {
            //This portion should contain a comma at this spot always
            printf("buffer: %s\n", buf);
            if(buf[3] == ',')
            {
                int row;
                int col;
                //Both coordinates should be a digit
                if(isdigit(buf[2]) > 0 && isdigit(buf[4]) > 0)
                {
                    //Converting the chars into ints from the message
                     row = buf[2]-'0'; 
                     col = buf[4]-'0';
                }
                else
                {
                    sendINVLMessage(sock, "Row and/or col were not numbers");
                    puts("Row and/or col were not numbers");
                    return(FALSE);
                }
                
                //row and col need to be digits and need to be 1, 2, or 3
                if(row > 0 && row < 4 && col > 0 && col < 4)
                {
                    //the last character needs to be a | for it to be a valid message
                    //read to make move
                    if(buf[5] == '|')
                    {
                        //Ready to make a move as it passes all checks
                        //Will return true if the move is valid and give the row and col
                        //to the main function outside
                        if(makeMove(row, col, player->role[0], game->board) == TRUE)
                        {
                            puts("Player moved successfully");
                            *turnCol = col;
                            *turnRow = row;
                            return(TRUE);
                        }
                        else
                        {
                            sendINVLMessage(sock, "That spot is already occupied");
                            puts("move was invalid");
                        }
                    }
                    else
                    {
                        sendINVLMessage(sock, "Message didn't end with a |");
                        puts("message didn't end with a |");
                    }
                }
                else
                {
                    sendINVLMessage(sock, "Row and/or Col were not digits in valid range (1, 2, or 3)");
                    puts("row and or col were not digits/in valid range");
                }
            }
            else
            {
                sendINVLMessage(sock, "Player didn't format message correctly");
                puts("Player didn't format message correctly");
            }
            
        }
        else
        {
            //TODO Error
            printf("buffer %s\n", buf);
            sendINVLMessage(sock, "Player didn't specify their role correctly");
            puts("Player didn't specify their role correctly");
        }
    }
    else
    {
        //TODO Error
        printf("bytes: %d", bytes);
        printf("numtoread: %d", numToRead);
        sendINVLMessage(sock, "Invalid message length");
        puts("Invalid message length");
    }
    
    return(FALSE);
}

//Will turn a movdMessage struct into its string representation
//Need to free the variable that it is being called to equal
//EX: MOVD|16|X|1,2|
char* toStringMOVD(movdMessage *movdMsg)
{
    char *msg = (char *)malloc(sizeof(char)*BUFSIZE);
    char len[3]; //Should never be greater than 999 (really 256)
    strcpy(msg, movdMsg->movd);
    strcat(msg, "|");
    sprintf(len, "%d", MOVDMESSAGELEN);
    strcat(msg, len);
    strcat(msg, "|");
    strcat(msg, movdMsg->role);
    strcat(msg, "|");
    strcat(msg, movdMsg->position);
    strcat(msg, "|");
    strcat(msg, movdMsg->board);
    strcat(msg, "|");
    //printf("final message %s\n", msg);
    return(msg);
}

//Function to send the MOVD message to both players when a move is made
//Will make a movdMessage struct and call its toString function
void sendMOVDMessage(int sock1, int sock2, char *board, char *role, int row, int col)
{
    movdMessage *movdMsg = (movdMessage *)malloc(sizeof(movdMessage));
    movdMsg->movd = "MOVD";
    movdMsg->role = role;
    char pos[4];
    sprintf(pos, "%d,%d", row, col); //formats the row and col into "row,col" in a char
    movdMsg->position = pos;
    movdMsg->board = board;

    char *finalMsg = toStringMOVD(movdMsg);
    write(sock1, finalMsg, BUFSIZE);
    write(sock2, finalMsg, BUFSIZE);
    free(finalMsg);
    free(movdMsg);

}

//Will request a response to a draw message from the opposite player
//Will not leave until a proper draw response is had (R or A)
//Will return ACCEPT or REJECT
int readDraw(int requestedPlayer, int otherPlayer)
{
    char buff[BUFSIZE];
    int bytes;

    //Will continually loop until a proper response is found
    while(TRUE) //SHOULD BE WHILE ACTIVE MAYBE
    {
        //Need to send draw suggestion to the other player
        write(otherPlayer, "DRAW|2|S|", 10);

        //Need to read from other player
        read(otherPlayer, buff, PROTOCOLSTARTLEN);

        //Finding the draw message
        if(findMessage(buff) == DRAW)
        {
            int numToRead = getNumOfBytesToRead(otherPlayer, buff);

            //Need to make sure they send the correct length (always 2 for draw)
            if(numToRead == 2)
            {
                bytes = read(otherPlayer, buff, numToRead);
                
                //Message was correct length, the message accpeted the draw, and it ends with a |
                if(bytes == numToRead && buff[0] == 'A' && buff[1] == '|') //+1 for new line character
                {
                    return(ACCEPT);
                }
                else if(bytes == numToRead && buff[0] == 'R' && buff[1] == '|') //+1 for new line character
                {
                    return(REJECT);
                }
                else
                {
                    sendINVLMessage(otherPlayer, "Either proposal or message was invalid");
                    puts("Either proposal or message was invalid");
                }
            }
            else
            {
                sendINVLMessage(otherPlayer, "Not a proper draw response message length");
                puts("Not a proper draw response message length");
            }
        }
        else
        {
            sendINVLMessage(otherPlayer, "Please use the DRAW command");
            puts("Please use the DRAW command");
        }
    }
}

//Converts an OVER struct to its string form to send to players
//Needs to be freed after use
char* toStringOVER(overMessage *overMsg)
{
    char *msg = (char *)malloc(sizeof(char)*BUFSIZE);
    
    //Honestly I don't know why its +4, but that makes the len correct
    sprintf(msg, "%s|%ld|%c|%s|", overMsg->over, strlen(overMsg->outComeReason)+3,
    overMsg->outcome, overMsg->outComeReason);
    return(msg);
}

//Used to format a proper OVER message
//Takes in the two sockets and the outcome of the game for each
//It also takes in a general reason as to why the game ended (draw, who won, resigned ...)
void sendOVERMessage(int player1, int player2, char player1Outcome, char player2Outcome, char *reason)
{
    overMessage *msg = (overMessage *)malloc(sizeof(overMessage));

    msg->over = "OVER";
    
    //Player1 outcome first
    msg->outcome = player1Outcome;
    msg->outComeReason = reason;
    char *finalMessage1 = toStringOVER(msg);
    write(player1, finalMessage1, BUFSIZE);
    free(finalMessage1);

    //Player 2 outcome next
    msg->outcome = player2Outcome;
    msg->outComeReason = reason;
    char *finalMessage2 = toStringOVER(msg);
    write(player2, finalMessage2, BUFSIZE);
    free(finalMessage2);

    free(msg);

}


//Returns the string version of an invalid message
char* toStringINVL(invalidMessage *invlMessage)
{
    char *msg = (char *)malloc(sizeof(char)*BUFSIZE);

    sprintf(msg, "%s|%ld|%s|", invlMessage->invl, strlen(invlMessage->invalidReason)+1, invlMessage->invalidReason);
    return(msg);
    
}

//Used to send invalid message back to the player
//Takes in the socket and the message to send to them
void sendINVLMessage(int player1, char *reason)
{
    invalidMessage *msg = (invalidMessage *)malloc(sizeof(invalidMessage));

    msg->invl = "INVL";
    msg->invalidReason = reason;

    char *finalMessage = toStringINVL(msg);
    write(player1, finalMessage, BUFSIZE);
    free(finalMessage);
    free(msg);


}

void* gameLoop(void *arguments)
{

    arguments_t *args = (arguments_t*) arguments;
    int sock1 = args->sock1;
    int sock2 = args->sock2;
    struct sockaddr_storage *player1 = args->player1;
    struct sockaddr_storage *player2 = args->player2;
    socklen_t player_len = *args->len;
    currentConnections_t **currentConnsPointer = args->currentConnections;
    currentConnections_t *currentConns = *currentConnsPointer;
    game_t *currentGame = args->currentGame;
    free(args);
    
    //Initialize the game board
    //game_t *currentGame = malloc(sizeof(game_t));
    //currentGame->player1 = NULL;
    //currentGame->player2 = NULL;
    //TODO: MAKE THIS A CHAR ARRAY WITH 9 DOTS
    //currentGame->board = NULL;

    //initialized board to all .'s
    //initializeGameBoard(currentGame);

    char buf[BUFSIZE];
    int bytes = 0;
    
    int active = 1;


        //Prompt both players to enter their name
        
        write(sock2, "Enter the play message with a name to join a game", 49);

        //Create a thread to get the playerMessage from player1
        /* pthread_t player1PlayThread;
        playArgs_t *readPlay1Args = (playArgs_t*)malloc(sizeof(playArgs_t));
        readPlay1Args->sock = sock1;
        readPlay1Args->player = player1;
        readPlay1Args->len = player_len;
        readPlay1Args->currentConnections = currentConns;
        readPlay1Args->active = active;
        readPlay1Args->currentGame = currentGame; */

        //pthread_create(&player1PlayThread, NULL, readPlay, (void*)readPlay1Args);
        //readPlay(sock1, (struct sockaddr *)player1, player_len, currentConns, active, currentGame);

        //Create a thread to get the playMessage from player2
        /* pthread_t player2PlayThread;
        playArgs_t *readPlay2Args = (playArgs_t*)malloc(sizeof(playArgs_t));
        readPlay2Args->sock = sock2;
        readPlay2Args->player = player2;
        readPlay2Args->len = player_len;
        readPlay2Args->currentConnections = currentConns;
        readPlay2Args->active = active;
        readPlay2Args->currentGame = currentGame;
        
        pthread_create(&player2PlayThread, NULL, readPlay, (void*)readPlay2Args); */
        
        
        //readPlay(sock2, (struct sockaddr *)player2, player_len, currentConns, active, currentGame);

        

        //Just for testing

        

        //Wait for both player threads to finish
        /* pthread_join(player1PlayThread, NULL);
        pthread_join(player2PlayThread, NULL); */


        printf("Player 1: %s\n", currentGame->player1->name);
        printf("Player 2: %s\n", currentGame->player2->name);
        //Technically at this point, both sockets should have given the PLAY message and their name
        //So we should decide who is X and who is O
        //Will indeed decide who is X and who is O
        beginMessage(sock1, sock2, currentGame->player1, currentGame->player2, currentGame);
    

        //Function to read the entire buffer from both
        //Then decide which function to call

        //Vars for the below game loop
        int socketTurn;
        int notSocketTurn;
        currentConnections_t *playerTurn; //Keep track of which player's turn it is
        int switchTurn; //Used to determine if the turn needs to be switched
        int turnRow = 0; //= to 0 for testing so things don't break
        int turnCol = 0;
        int drawOutcome;
        char player1Outcome;
        char player2Outcome;
        //Who ever was determined to be X will go first
        if(currentGame->player1->role[0] == 'X')
        {
            socketTurn = sock1;
            playerTurn = getConn((struct sockaddr *)player1, currentConns);
        }
        if(currentGame->player2->role[0] == 'X')
        {
            socketTurn = sock2;
            playerTurn = getConn((struct sockaddr *)player2, currentConns);
        }

        //This will be where the main gameloop occurs
        while(TRUE)
        {
            

            //Used for outcome message decisions
            if(socketTurn == sock1)
            {
                notSocketTurn = sock2;
            }
            else
            {
                notSocketTurn = sock1;
            }

            //For some reason this will send twice as it loops through twice
           // write(socketTurn, "It is your turn", 15); //Place holder for testing

            //Whoever's turn it is, lets read from them
            //readBuffer will return the Message portion of the stream (MOVE|, DRAW|...)
            readBuffer(socketTurn, buf, &bytes, PROTOCOLSTARTLEN);

            printf("buffer in main: %s\n", buf);

            //Decide what message was read from the stream
            if(findMessage(buf) == MOVE)
            {
                //ReadMove will return True if the move was valid so switchTurn will be true
                //Returns false if move was invalid so switchTurn will be false
                puts("here");
                switchTurn = readMove(socketTurn, playerTurn, currentGame, &turnRow, &turnCol);
                puts("here2");
            }
            else if(findMessage(buf) == DRAW)
            {
                //Checking so see of the draw message was formatted correctly
                bytes = getNumOfBytesToRead(socketTurn, buf);
                if(bytes == 2) //Should always be 2 for draw
                {
                    bytes = read(socketTurn, buf, bytes);
                    //Contents of the message should always be S and | since they are suggesting one
                    if(bytes == 2 && buf[0] == 'S' && buf[1] == '|')
                    {
                        //Deciding who to get a returning draw response from to accept or reject
                        if(socketTurn == sock1)
                        {
                            drawOutcome = readDraw(socketTurn, sock2);
                            
                        }
                        else
                        {
                             drawOutcome = readDraw(socketTurn, sock1);
                        }

                        //The opponent accepted the draw proposal
                        if(drawOutcome == ACCEPT)
                        {
                            //Reason as to why the game ended
                            char *over = "Game ended in a draw";
                            //Will send the over message to both players and will end the game
                            sendOVERMessage(sock1, sock2, 'D', 'D', over);
                            break;
                        }
                        else if(drawOutcome == REJECT)
                        {
                            //Tell the original player that the draw request was denied
                            //Will then tell the original player it is their turn again and
                            //wait for another response
                            if(socketTurn == sock1)
                            {
                                write(sock1, "DRAW|2|R|", 10);
                            }
                            else
                            {
                                write(sock2, "DRAW|2|R|", 10);
                            }
                        }
                        else
                        {
                            puts("SOMETHING WENT WRONG WITH DRAW");
                        }
                    }
                    else
                    {
                        sendINVLMessage(socketTurn, "Draw formatted incorrectly");
                        puts("Draw formatted incorrectly");
                    }
                }
                else{
                    sendINVLMessage(socketTurn, "Draw formatted incorrectly");
                    puts("Draw formatted incorrectly");
                }
                puts("DRAWING!");
            }
            else if(findMessage(buf) == RSGN)
            {
                bytes = read(socketTurn, buf, 2); //2 since it only needs the 0|
                //RSGN|0| should be 3 bytes read (0 | \n)
                //Who's ever turn it is (the one who resigned) will have lost and the other won
                if(buf[0] == '0' && buf [1] == '|')
                {
                    char reason[100]; //place holder for rn;
                    sprintf(reason, "%s has resigned!", playerTurn->name);
                    sendOVERMessage(socketTurn, notSocketTurn, 'L', 'W', reason);
                    break;
                }
                else
                {
                    sendINVLMessage(socketTurn, "Resign was formatted incorrectly");
                    puts("Resign was formatted incorrectly");
                }
            }
            else
            {
                sendINVLMessage(socketTurn, "Not a valid message");
                puts("Not valid message");
            }

            //After an above message was read, will determine if the turn needs to be switched
            if(switchTurn == TRUE)
            {
                switchTurn = FALSE;
                printf("Wins?: %c\n",checkWin(currentGame->board)); 
                if(checkWin(currentGame->board) == 'N')
                {
                    //Write board to each player
                    sendMOVDMessage(sock1, sock2, currentGame->board, playerTurn->role, turnRow, turnCol);

                    if(socketTurn == sock1)
                    {
                        socketTurn = sock2;
                        playerTurn = getConn((struct sockaddr *)player2, currentConns);
                    }
                    else{
                        socketTurn = sock1;
                        playerTurn = getConn((struct sockaddr *)player1, currentConns);
                    }
                    memset(buf, 0, BUFSIZE);

                }
                else if(checkWin(currentGame->board) == 'X' || checkWin(currentGame->board) == 'O')
                {
                    sendMOVDMessage(sock1, sock2, currentGame->board, playerTurn->role, turnRow, turnCol);
                    char reason[100]; //place holder for rn;
                    sprintf(reason, "%s has won!", playerTurn->name);
                    sendOVERMessage(socketTurn, notSocketTurn, 'W', 'L', reason);
                    break;
                }
                else if(checkWin(currentGame->board) == 'D')
                {
                    sendMOVDMessage(sock1, sock2, currentGame->board, playerTurn->role, turnRow, turnCol);
                    char *over = "Game ended in a draw";
                    sendOVERMessage(sock1, sock2, 'D', 'D', over);
                    break;
                }
                
            }
            
        }
        
        /* getConn((struct sockaddr *)player1, currentConns)->name = "NOMORE";
        getConn((struct sockaddr *)player2, currentConns)->name = "NOMORE"; */
        puts("Before remove lock");
        
         puts("In remove lock");
        removeConn(&currentConns, (currentConnections_t *)getConn((struct sockaddr *)player2, currentConns), currentConns);
        int i = removeConn(&currentConns, (currentConnections_t *)getConn((struct sockaddr *)player1, currentConns), currentConns);

        if(i == -1){
            *currentConnsPointer = NULL;
        }

        puts("after remove lock");
        //free(player1);
        //free(player2);
        freeGame(currentGame);
        
        
        close(sock1);
        close(sock2);
        //break;  

        //readBuffer(sock2, (struct sockaddr *)&player1, player_len, player2Buffer, &bytes);
        //printf("%s\n", player2Buffer);  


        //POTENTIALLY HAVE GLOBAL VARIABLES TO REPRESENT STATES LIKE IF TWO PEOPLE ARE IN A GAME
        //HAVE VAR SAYING STARTGAME = 1 AND THEN HAVE A CORRESPONDING FUNCTION CALL AFTER THE READS
        
    //}
}

int removeConn(currentConnections_t **head, currentConnections_t *nodeToRemove, currentConnections_t *currentConns)
{
    currentConnections_t *current = *head;
    currentConnections_t *prev = NULL;
    pthread_mutex_lock(&currentConns->lock);

    // Traverse the linked list to find the node to remove
    while (current != NULL && current != nodeToRemove) {
        prev = current;
        current = current->next;
    }

    // If the node to remove is not found, return
    if (current == NULL) {
        pthread_mutex_unlock(&currentConns->lock);
        return 0;
    }

    // If the node to remove is the head of the list, update the head pointer
    if (prev == NULL) {
        *head = current->next;
    } else {
        prev->next = current->next;
    }

    // Free the memory used by the node to remove
    free(nodeToRemove->conn);
    free(nodeToRemove->name);
    //free(nodeToRemove->role);
    free(nodeToRemove);

    //printf("Head: %s\n", *head);
    if(*head == NULL)
    {
        printf("here null\n");
        pthread_mutex_unlock(&currentConns->lock);
        return -1;
    }
    pthread_mutex_unlock(&currentConns->lock);
    return 0;
    
}
    


