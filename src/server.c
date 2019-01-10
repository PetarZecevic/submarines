/* 
    ********************************************************************
    Odsek:          Elektrotehnika i racunarstvo
    Departman:      Racunarstvo i automatika
    Katedra:        Racunarska tehnika i racunarske komunikacije (RT-RK)
    Predmet:        Osnovi Racunarskih Mreza 1
    Godina studija: Treca (III)
    Skolska godina: 2018/2019
    Semestar:       Zimski (V)
    
    Ime fajla:      server.c
    Opis:           TCP/IP server
    
    Platforma:      Raspberry Pi 2 - Model B
    OS:             Raspbian
    ********************************************************************
*/

#include <stdio.h>
#include <string.h>    //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include <pthread.h> // mutex, thread
#include <semaphore.h> // semaphore
#include <stdbool.h>

#include "config.h"
#include "message.h"

#define PLAYER1 1
#define PLAYER2 2

typedef struct
{
    int sock;
    int id;
    char coordMessage[COORD_LENGTH];
    char feedbackMessage[FEEDBACK_LENGTH];
    char errorMessage[DEFAULT_BUFLEN];
    char readyMessage[2];
    char role[2];
    char name[DEFAULT_BUFLEN];
}Player;

// Global data.
int activePlayer;
int playersConnected = 0;
int playersReady = 0;
int roleSend = 0;
int errorFlag;
char communicationBuffer[COORD_LENGTH + FEEDBACK_LENGTH];
// Mutex for global data.
pthread_mutex_t dataAccess;
// Semaphores that express thread's activity.
sem_t activity[2];

// Condition determines action on the semaphore, post or wait.
void Synchronization(bool condition, sem_t* waitSemaphore, sem_t* notifySemaphore)
{	
	if(!condition)
	{
		sem_wait(waitSemaphore);
	}
	else
	{
		sem_post(notifySemaphore);
	}
}

/*
Handles error case.
Set's errorFlag to 1, and notifies other thread to continue execution.
*/
void ErrorCode(sem_t* notifySem)
{
    pthread_mutex_lock(&dataAccess);
    errorFlag = 1;
    pthread_mutex_unlock(&dataAccess);
    sem_post(notifySem);
}

/*
Match can start when both players are ready and have been assigned roles.
*/
bool PrepareMatch(Player* player, int threadId, int otherThreadId)
{
    bool success;
    //Receive message that player is ready.
    success = WrapperRecv(player->sock, player->readyMessage, 2, player->errorMessage);
    if(success)
    {
        printf("Message from %s: %s\n", player->name, player->readyMessage);
        pthread_mutex_lock(&dataAccess);
        playersReady++;
        pthread_mutex_unlock(&dataAccess);
    }
    else
    {
        printf("%s error: %s\n", player->name, player->errorMessage);
        ErrorCode(&activity[otherThreadId]);
        return false;
    }

    // Wait while other player is not ready.
    Synchronization((playersReady == 2), &activity[threadId], &activity[otherThreadId]);
    if(errorFlag > 0)
    {
        return false;
    }
    // Send message about role.
    (player->id == activePlayer) ? strcpy(player->role, ACTIVE) : strcpy(player->role, PASSIVE);
    success = WrapperSend(player->sock, player->role, 2, player->errorMessage);
    if(success)
    {
        pthread_mutex_lock(&dataAccess);
        roleSend++;
        pthread_mutex_unlock(&dataAccess);
    }
    else
    {
        printf("%s error: %s\n", player->name, player->errorMessage);
        ErrorCode(&activity[otherThreadId]);
        return false; 
    }
    // Wait while other player is not assigned role.
    Synchronization((roleSend == 2), &activity[threadId], &activity[otherThreadId]);
    // Error case.
    if(errorFlag > 0)
    {
        return false;
    }

    return success;
}

/*
Function that thread executes, thread Id will be 0 or 1.
We know in advance that we will use two threads, and based 
on that assumption comunnication mechanism works.
Params are:
    - socket of server
    - Id of thread that executes function
    - Id of player that thread serves in game
Thread's communicate between themself using semaphores 
and global buffer.
*/
void* PlayerLogic(void* param)
{

    Player player;

    int* iparam = (int*) param;
    int socketServer = iparam[0];
    int threadId = iparam[1];
    player.id = iparam[2];
    int otherThreadId = (threadId + 1) % 2;

    struct sockaddr_in playerAddr;

    int c = sizeof(playerAddr);
    player.sock = accept(socketServer, (struct sockaddr *)&playerAddr, (socklen_t*)&c);
    if (player.sock < 0)
    {
        perror("accept failed");
        ErrorCode(&activity[otherThreadId]);
        return NULL;
    }
    puts("Connection accepted");

    pthread_mutex_lock(&dataAccess);
    playersConnected++;
    pthread_mutex_unlock(&dataAccess);

    // Wait for other player to connect, if he didn't connect.
    Synchronization((playersConnected == 2), &activity[threadId], &activity[otherThreadId]);
    // Error case.
    if(errorFlag > 0)
        return NULL;

    (player.id == PLAYER1) ? strncpy(player.name, "Player1", 8) : strncpy(player.name, "Player2", 8);
    
    bool gameOver = false;
    bool matchOver = false;
    bool success;

    while(!gameOver)
    { 
        if(!PrepareMatch(&player, threadId, otherThreadId))
            break;
        
        bool endGameFlag = false;
        memset(communicationBuffer, 0, 2);

        while(!matchOver)
        {
            // Active case.
            if(player.id == activePlayer)
            {
                success = WrapperRecv(player.sock, player.coordMessage, COORD_LENGTH, player.errorMessage);
                if(!success)
                {
                    printf("%s error: %s\n", player.name, player.errorMessage);
                    ErrorCode(&activity[otherThreadId]);
                    gameOver = true;
                    break;
                }
                
                // Send coordinate to global buffer.
                pthread_mutex_lock(&dataAccess);
                strncpy(communicationBuffer, player.coordMessage, COORD_LENGTH);
                pthread_mutex_unlock(&dataAccess);
                
                if(strncmp(player.coordMessage, END_MATCH, COORD_LENGTH) == 0)
                {
                    // Received end-match message instead of coordinate.
                    printf("%s: Match over, end game situation!\n", player.name);
                    endGameFlag = true;
                }

                // Notify other player's thread to get coordinate from buffer.   
                sem_post(&activity[otherThreadId]);
                // Wait for feeback message from other player's thread.
                sem_wait(&activity[threadId]);

                if(errorFlag > 0)
                {   
                    // Game and match over.
                    gameOver = true;
                    break;
                }
                else if(endGameFlag)
                {
                    // Match over.
                    break;
                }

                pthread_mutex_lock(&dataAccess);
                strncpy(player.feedbackMessage, communicationBuffer, FEEDBACK_LENGTH);
                pthread_mutex_unlock(&dataAccess);
                
                if(strncmp(player.feedbackMessage, LOST_MATCH, FEEDBACK_LENGTH) == 0)
                {
                    printf("%s: Match over, win-lose situation!\n", player.name);
                    matchOver = true;
                } 
                else if(strncmp(player.feedbackMessage, LOST_MATCH_M, FEEDBACK_LENGTH) == 0)
                {
                    printf("%s: Match over, miss situation!\n", player.name);
                    matchOver = true;
                }
                
                // Send feedback to active player.  
                success = WrapperSend(player.sock, player.feedbackMessage, FEEDBACK_LENGTH, player.errorMessage);
                if(!success)
                {
                    printf("%s error: %s\n", player.name, player.errorMessage);
                    ErrorCode(&activity[otherThreadId]);
                    gameOver = true;
                    break;   
                }
                sem_post(&activity[otherThreadId]);
            }
            // Passive case.
            else
            {
                sem_wait(&activity[threadId]);
                if(errorFlag > 0)
                {   
                    // Game and match over.
                    gameOver = true;
                    break;
                }

                pthread_mutex_lock(&dataAccess);
                strncpy(player.coordMessage, communicationBuffer, COORD_LENGTH);
                pthread_mutex_unlock(&dataAccess);
                
                if(strncmp(player.coordMessage, END_MATCH, COORD_LENGTH) == 0)
                {
                    printf("%s: Match over, end game situation!\n", player.name);
                    endGameFlag = true;
                }

                success = WrapperSend(player.sock, player.coordMessage, COORD_LENGTH, player.errorMessage);
                if(!success)
                {
                    printf("%s error: %s\n", player.name, player.errorMessage);
                    ErrorCode(&activity[otherThreadId]);
                    gameOver = true;
                    break;
                }

                if(endGameFlag)
                {   
                    // Match over.
                    sem_post(&activity[otherThreadId]);
                    break;
                }

                success = WrapperRecv(player.sock, player.feedbackMessage, FEEDBACK_LENGTH, player.errorMessage); 
                if(!success)
                {
                    printf("%s error: %s\n", player.name, player.errorMessage);
                    ErrorCode(&activity[otherThreadId]);
                    gameOver = true;
                    break;
                }

                pthread_mutex_lock(&dataAccess);
                strncpy(communicationBuffer, player.feedbackMessage, FEEDBACK_LENGTH);
                pthread_mutex_unlock(&dataAccess);

                if(strncmp(player.feedbackMessage, LOST_MATCH, FEEDBACK_LENGTH) == 0)
                {
                    printf("%s: Match over, win-lose situation!\n", player.name);
                    matchOver = true;
                }
                else if(strncmp(player.feedbackMessage, LOST_MATCH_M, FEEDBACK_LENGTH) == 0)
                {
                    printf("%s: Match over, miss situation!\n", player.name);
                    matchOver = true;
                }                             
                else if(strncmp(player.feedbackMessage, MISSED_FIELD, FEEDBACK_LENGTH) == 0)
                {
                    pthread_mutex_lock(&dataAccess);
                    activePlayer = player.id;
                    pthread_mutex_unlock(&dataAccess);
                }

                sem_post(&activity[otherThreadId]);
                sem_wait(&activity[threadId]);

                if(errorFlag > 0)
                {   
                    // Game and match over.
                    gameOver = true;
                    break;
                }
            }        
        }
        // Reset data.
        pthread_mutex_lock(&dataAccess);
        playersReady = 0;
        roleSend = 0;
        pthread_mutex_unlock(&dataAccess);
        // Start new match.
        matchOver = false;
    }

    return NULL;
}

int main(int argc, char** argv)
{
    int socketServer;
    struct sockaddr_in server;
    
    //Create socket, TCP protocol.
    socketServer = socket(AF_INET , SOCK_STREAM , 0);
    if (socketServer == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(DEFAULT_PORT);

    //Bind
    if( bind(socketServer,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");

    //Listen
    listen(socketServer , 2);

    //Accept and incoming connection
    puts("Waiting for incoming connections...");

    // Player threads.
    pthread_t player1T;
    int args1[3] = {socketServer, 0, PLAYER1};

    pthread_t player2T;
    int args2[3] = {socketServer, 1, PLAYER2};

    // Init mutex and semaphore.
    pthread_mutex_init(&dataAccess, NULL);
    sem_init(&activity[0], 0, 0);
    sem_init(&activity[1], 0, 0);

    // Init global data.
    playersConnected = 0;
    playersReady = 0;
    roleSend = 0;

    // Set active player.
    activePlayer = PLAYER1;

    // Run threads.
    pthread_create(&player1T, NULL, PlayerLogic, (void*)args1);
    pthread_create(&player2T, NULL, PlayerLogic, (void*)args2);

    // Finish program.
    pthread_join(player1T, 0);
    pthread_join(player2T, 0);

    // Destroy mutex and semaphore.
    sem_destroy(&activity[0]);
    sem_destroy(&activity[1]);
    pthread_mutex_destroy(&dataAccess);

    return 0;
}
