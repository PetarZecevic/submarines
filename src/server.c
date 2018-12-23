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

#include<stdio.h>
#include<string.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write

#include "config.h"
#include "message.h"

#define PLAYER1 1
#define PLAYER2 2

typedef struct Player
{
    int sock;
    int id;
    char coordMessage[COORD_LENGTH];
    char confirmMessage[CONFIRM_LENGTH];
    char errorMessage[DEFAULT_BUFLEN];
    char name[DEFAULT_BUFLEN];
}Player;

bool ActivePlayerLogic(Player* activePlayer, Player* passivePlayer, int* active, bool* gameActive)
{
    bool success = true;
    //wait_for_player1_coordinate;
    memset(activePlayer->coordMessage, 0, COORD_LENGTH);
    if(WrapperRecv(activePlayer->sock, activePlayer->coordMessage, COORD_LENGTH, activePlayer->errorMessage))
    {
        printf("%s send %s\n", activePlayer->name, activePlayer->coordMessage);
        //coord = parse_player1_message;
        //send_player2_coord_message;
        if(WrapperSend(passivePlayer->sock, activePlayer->coordMessage, COORD_LENGTH, passivePlayer->errorMessage))
        {
            memset(passivePlayer->confirmMessage, 0, CONFIRM_LENGTH);
            if(WrapperRecv(passivePlayer->sock, passivePlayer->confirmMessage, CONFIRM_LENGTH, passivePlayer->errorMessage))
            {
                printf("%s send %s\n", passivePlayer->name, passivePlayer->confirmMessage);
                //wait_for_player2_confirmation;
                //confirmation = parse_player2_message;
                //if confirmation == lost:
                if(strncmp(passivePlayer->confirmMessage, LOST_GAME, CONFIRM_LENGTH) == 0)
                {
                    //game_active = false;
                    *gameActive = false;
                }                                
                //else if confirmation == miss:
                else if(strncmp(passivePlayer->confirmMessage, MISSED_FIELD, CONFIRM_LENGTH) == 0)
                {
                    //active_player = player2;
                    *active = passivePlayer->id;
                }
                //else
                else
                {
                    //active_player = player1;
                    *active = activePlayer->id;
                }
                //send_player1_confirmation;
                if(!WrapperSend(activePlayer->sock, passivePlayer->confirmMessage, CONFIRM_LENGTH, activePlayer->errorMessage))
                {
                    // error section
                    printf("%s error: %s\n", activePlayer->name, activePlayer->errorMessage);
                    success = false;
                }
            }
            else
            {
                // error section
                printf("%s error: %s\n", passivePlayer->name, passivePlayer->errorMessage);
                success = false;
            }
        }
        else
        {
            // error section
            printf("%s error: %s\n", passivePlayer->name, passivePlayer->errorMessage);
            success = false;
        }
    }
    else
    {
        // error section
        printf("%s error: %s\n", activePlayer->name, activePlayer->errorMessage);
        success = false;
    }
    return success;
}

int main(int argc , char *argv[])
{
    int socketServer;
    struct sockaddr_in server;
    
    int socketPlayer1, socketPlayer2;
    struct sockaddr_in playerAddr1, playerAddr2;

    //Create socket
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
    int c = sizeof(struct sockaddr_in);

    //accept connection from player1
    socketPlayer1 = accept(socketServer, (struct sockaddr *)&playerAddr1, (socklen_t*)&c);
    if (socketPlayer1 < 0)
    {
        perror("accept failed");
        return 1;
    }
    puts("Connection accepted");
    //Init player1 structure
    Player player1;
    player1.sock = socketPlayer1;
    player1.id = PLAYER1;
    strcpy(player1.name, "Player1");

    //accept connection from player2
    socketPlayer2 = accept(socketServer, (struct sockaddr *)&playerAddr2, (socklen_t*)&c);
    if (socketPlayer2 < 0)
    {
        perror("accept failed");
        return 1;
    }
    puts("Connection accepted");
    // Init player2 structure
    Player player2;
    player2.sock = socketPlayer2;
    player2.id = PLAYER2;
    strcpy(player2.name, "Player2");

    char readyMessage[1];
    //Receive a message from player1
    if(WrapperRecv(player1.sock, readyMessage, 1, player1.errorMessage))
    {
        printf("Message from %s: %s\n", player1.name, readyMessage);
    }
    else
    {
        printf("%s error: %s\n", player1.name, player1.errorMessage);
        return 1;
    }

    //Receive a message from player2
    if(WrapperRecv(player2.sock, readyMessage, 1, player2.errorMessage))
    {
        printf("Message from %s: %s\n", player2.name, readyMessage);
    }
    else
    {
        printf("%s error: %s\n", player2.name, player2.errorMessage);
        return 1;
    }

    int activePlayer = PLAYER1;
    if(WrapperSend(player1.sock, ACTIVE, strlen(ACTIVE), player1.errorMessage) && 
        WrapperSend(player2.sock, PASSIVE, strlen(PASSIVE), player2.errorMessage))
    {
        bool gameActive = true;
        while(gameActive)
        {
            if(activePlayer == PLAYER1)
            {
                if(!ActivePlayerLogic(&player1, &player2, &activePlayer, &gameActive))
                {
                    gameActive = false;
                }
            }
            else if(activePlayer == PLAYER2)
            {
                if(!ActivePlayerLogic(&player2, &player1, &activePlayer, &gameActive))
                {
                    gameActive = false;
                }
            }
        }
    }
    else
    {
        printf("%s error: %s \n", player1.name, player1.errorMessage);
        printf("%s error: %s\n", player2.name, player2.errorMessage);
    }
    return 0;
}

