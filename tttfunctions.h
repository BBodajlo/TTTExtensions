#ifndef LIBRARYHEADERFILE_INCLUDED
#define LIBRARYHEADERFILE_INCLUDED

//Enum for messages
#define NUM_OF_MESSAGE_TYPES 9
#define BUFSIZE 256
#define HOSTSIZE 100
#define PORTSIZE 10
#define PROTOCOLSTARTLEN 5
#define MOVDMESSAGELEN 16
extern int testCode;
enum messageTypes{PLAY, WAIT, BEGN, MOVE, MOVD, INVL, RSGN, DRAW, OVER};
enum drawOutcomes{ACCEPT, REJECT};
//Mapping Enums to Strings
static const char *const messageTypes[] =
{
    [PLAY] = "PLAY|",
    [WAIT] = "WAIT|",
    [BEGN] = "BEGN|",
    [MOVE] = "MOVE|",
    [MOVD] = "MOVD|",
    [INVL] = "INVL|",
    [RSGN] = "RSGN|",
    [DRAW] = "DRAW|",
    [OVER] = "OVER|"
};
// Structs for the different types of server messages

typedef struct currentConnections{
    struct sockaddr *conn;
    char *name;
    char *role; // Would be X or O
    struct currentConnections *next;
    pthread_mutex_t lock;
}currentConnections_t;

typedef struct game{
    char *board;
    currentConnections_t *player1; 
    currentConnections_t *player2;
}game_t;

typedef struct arguments{
    int sock1;
    int sock2;
    struct sockaddr_storage *player1;
    struct sockaddr_storage *player2;
    socklen_t *len;
    currentConnections_t **currentConnections;
    game_t *currentGame;
}arguments_t;

typedef struct playArgs{
    int sock;
    struct sockaddr_storage *player;
    socklen_t len;
    currentConnections_t *currentConnections;
    int active;
    game_t *currentGame;
}playArgs_t;

//Enum for true and false
enum trueFalse{FALSE, TRUE};

// Play Message
typedef struct{
    char *play; // This should say PLAY (may take out)
    char *name; // Name declared by client
}playMessage;

//Wait Message
typedef struct{
    char *wait;
}waitMessage;

//Begin Message
typedef struct{
    char *begn;
    char *role; // Either X or O
    char *name;
}beginMessageS;

//Move Message
typedef struct{
    char *move;
    char *role; // Either X or O
    char *position; // Two integers separated by comma: 1,2
}moveMessage;

//Movd Message
typedef struct{
    char *movd;
    char *role; // Either X or O
    char *position; // Two integers separated by comma: 1,2
    char *board;
}movdMessage;

// INVL Message
typedef struct{
    char *invl;
    char *invalidReason; // Maybe make this an enum of error messages???
}invalidMessage;

//Rsgn message
typedef struct{
    char *rsgn;
}resignMessage;

//Draw Message
typedef struct{
    char *draw;
    char *message; // Either S suggest, A accept, R reject
}drawMessage;

//Over message
typedef struct{
    char *over;
    char outcome; // Either W win, L loss, D draw
    char *outComeReason; //Potentially an ENUM for all cases (?)
}overMessage;

void playerAddName(char*); // Will add the name to a struct of the play message
int checkValidMessage(char*);
char checkTurn(char*);
int checkValidMove(int, int, char*);
int makeMove(int, int, char, char*);
char checkWin(char*);
int gameAddPlayer(currentConnections_t*, game_t *);
int gameIsFull(game_t *);
int gameIsPlayer(currentConnections_t *, game_t *);
void addConn(struct sockaddr *, currentConnections_t*);
void addConnName(char *, currentConnections_t *);
void addConnRole(char *, currentConnections_t *);
struct currentConnections* getConn(struct sockaddr *, currentConnections_t *);
void freeCurrenconns(currentConnections_t *);
void freeGame(game_t *);
int nameInUse(char *, currentConnections_t *);
int getNumOfBytesToRead(int , char *);
//void* readPlay(int , struct sockaddr *, socklen_t , currentConnections_t *, volatile int,  game_t *);
void* readPlay(void*);
int beginMessage(int , int , currentConnections_t *, currentConnections_t *, game_t *);
int readBuffer(int, char *, int *, int);
int findMessage(char *);
void initializeGameBoard(game_t *);
int readMove(int , currentConnections_t *, game_t *, int *, int *);
char* toStringMOVD(movdMessage *);
void sendMOVDMessage(int , int , char *, char *, int , int );
int readDraw(int , int );
void sendOVERMessage(int , int , char , char , char *);
char* toStringINVL(invalidMessage *);
void sendINVLMessage(int , char *);
void* gameLoop(void*);
int removeConn(currentConnections_t **,  currentConnections_t *, currentConnections_t *);
#endif

