/* 
    ********************************************************************
    Odsek:          Elektrotehnika i racunarstvo
    Departman:      Racunarstvo i automatika
    Katedra:        Racunarska tehnika i racunarske komunikacije (RT-RK)
    Predmet:        Osnovi Racunarskih Mreza 1
    Godina studija: Treca (III)
    Skolska godina: 2017/2018
    Semestar:       Zimski (V)
    
    Ime fajla:      client.c
    Opis:           TCP/IP klijent
    
    Platforma:      Raspberry Pi 2 - Model B
    OS:             Raspbian
    ********************************************************************
*/

#include<stdio.h>      //printf
#include<string.h>     //strlen
#include<sys/socket.h> //socket
#include<arpa/inet.h>  //inet_addr
#include <fcntl.h>     //for open
#include <unistd.h>    //for close

#include "config.h"
#include "interface.h"
#include "table.h"
#include "message.h"

char errorMessage[DEFAULT_BUFLEN];

typedef enum {INIT = 0, SET_COORD, PLAYING, PAUSE, FIN}GameState;
typedef enum {ACTIVE_PLAY, PASSIVE_PLAY}EngineStates;

int GameLoop(int playerSocket);

void GameEngine(int playerSocket)
{
    // GAME-LOOP PLAYER: PSEUDO-COD
    /*
        wait_for_server_message;
        state = parse_server_message;
        game_active = true;
        while game_active:
            if state == active:
                print "Your turn, guess coordinate of opponent's submarine";
                get_coordinate_from_user;
                send_server_coordinate;
                wait_for_confirmation;
                miss_or_hit? = parse_server_message;
                if miss:
                    print "Miss.";
                    state = passive;
                else if hit:
                    print "Hit.";
                    state = active;
                else if won:
                    print "You won, game is over!";
                    game_active = false;
            else if state == passive:
                print "Waiting for opponent's move"
                wait_for_opponents_move;
                coordinate = parse_server_message;
                miss_or_hit? = check_coordinate_in_table;
                if hit:
                    all_submarines_lost?;
                    if positive_answer:
                        send_server_lost_message;
                        print "You lost, game is over!";
                        game_active = false;
                    else
                        send_server_hit_message;
                        print "Lost field.";
                        state = passive;
                else if miss:
                    send_server_miss_message;
                    print "Opponent missed.";
                    state = active;
                    

    */
}

void PrintStatus(bool status)
{
	if(status)
		printf("\nYou turn, guess coordinate of enemy's submarine\n");
	else
		printf("\nWaiting for enemy's move\n");
	return;
}

int main(int argc , char *argv[])
{
    int playerSocket;
    struct sockaddr_in server;

    //Create socket
    playerSocket = socket(AF_INET , SOCK_STREAM , 0);
    if (playerSocket == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    server.sin_addr.s_addr = inet_addr(SERVER_IP);
    server.sin_family = AF_INET;
    server.sin_port = htons(DEFAULT_PORT);

    //Connect to remote server
    if (connect(playerSocket , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
	        return 1;
    }
    puts("Connected\n");	
	
	GameLoop(playerSocket);
    //playGame(playerSocket);
    close(playerSocket);
    return 0;
}

/*
Check if submarine's field is located at coordinate(checkRow, checkColumn)
*/
bool CheckSubmarineHit(Field submarine[], int submarineLength, int checkRow, int checkColumn)
{
	bool hit = false;
	for(int i = 0; i < submarineLength; i++)
	{
		if(submarine[i].row == checkRow && submarine[i].column == checkColumn)
		{
			hit = true;
			break;
		}
	}
	return hit;
}

int GameLoop(int playerSocket)
{
	// Tables neccesary for game.
	Table playerTable;
	Table enemyTable;
	// Player's submarines.
	Field submarine1[1];
	Field submarine2[2];
	// Init game.
	InitTable(playerTable);
	InitTable(enemyTable);
	
	GameState gameState = INIT;

	// Messages
	char coordMessage[COORD_LENGTH] = {0};
	char confirmMessage[CONFIRM_LENGTH] = {0};
	char errorMessage[DEFAULT_BUFLEN] = {0};

	int submarine1LostFields = 0;
	int submarine2LostFields = 0;

	// Game flags.
	bool gameFinished = false, alreadySet = false, startedGame = false, active = true;
	// Option from menies;
	char option;

	while(!gameFinished)
	{
		switch(gameState)
		{
		
			case INIT:
				ClearScreen();
				MainMenuScreen();
				option = GetUserOptionInput('1', '4');
				switch (option)
				{
					case 1: // SET
						gameState = SET_COORD;
						break;
					case 3: // END
						gameState = FIN;
						break;
					default: // OTHER INPUT
						printf("Invalid option.\n");
						usleep(1000*1000);
						break;
				}
				break;
			case SET_COORD:
				ClearScreen();
				if(!alreadySet)
				{
					SetCoordinates(playerTable, submarine1, submarine2);
					ClearScreen();
				}
				alreadySet = true;
				MainMenuScreen();
				option = GetUserOptionInput('1', '4');
				switch(option)
				{
					case 1: // SET
						gameState = SET_COORD;
						alreadySet = false;
						break;
					case 2: // PLAY
						gameState = PLAYING;
						break;
					case 3: // END
						gameState = FIN;
						break;
					case 4: // CONTINUE
						gameState = SET_COORD;
						printf("You can not continue the game, because you have not played yet.\n");
						usleep(1000*1000);
						break;
					default: // OTHER INPUT
						printf("Invalid option.\n");
						usleep(1000*1000);
						break;
				}
				break;
			case PLAYING:
				if(!startedGame)
				{	
					char role[2];
					if(WrapperSend(playerSocket, READY, strlen(READY), errorMessage))
					{
						memset(role, 0, 2);
						if(WrapperRecv(playerSocket, role, 2, errorMessage))
						{
							if(strncmp(role, ACTIVE, 1) == 0)
							{
								active = true;
							}
							else
							{
								active = false;
							}
							startedGame = true;
						}
						else
						{
							printf("%s\n", errorMessage);
							gameState = FIN;	
						}
					}
					else
					{
						printf("%s\n", errorMessage);
						gameState = FIN;
					}
				}
				ClearScreen();
				GameplayScreen(playerTable, enemyTable);
				PrintStatus(active);
				if(active)
				{
					GameplayOptions();
					option = GetUserOptionInput('1', '4');
					switch(option)
					{	
						case 1: // NEW MOVE
							gameState = PLAYING;
							char r, c;
							GetRowColumnInput(&r, &c);
							if(CoordValid(r, c))
							{
								coordMessage[0] = r + '0';
								coordMessage[1] = c + '0';
								// Send move.
								if(WrapperSend(playerSocket, coordMessage, COORD_LENGTH, errorMessage))
								{
									memset(confirmMessage, 0, CONFIRM_LENGTH);
									// Receive result, hit or miss.
									if(WrapperRecv(playerSocket, confirmMessage, CONFIRM_LENGTH, errorMessage))
									{
										if(strncmp(confirmMessage, LOST_FIELD, CONFIRM_LENGTH) == 0 ||
											strncmp(confirmMessage, LOST_SUBMARINE_1, CONFIRM_LENGTH) == 0 ||
											strncmp(confirmMessage, LOST_SUBMARINE_2, CONFIRM_LENGTH) == 0)
										{
											enemyTable[(int)r][(int)c] = HIT;
										}
										else if(strncmp(confirmMessage, MISSED_FIELD, CONFIRM_LENGTH) == 0)
										{
											if(enemyTable[(int)r][(int)c] == WATER)
											{
												enemyTable[(int)r][(int)c] = MISS;
											}
											active = false;
										}
										else if(strncmp(confirmMessage, LOST_GAME, CONFIRM_LENGTH) == 0)
										{
											enemyTable[(int)r][(int)c] = HIT;
											ClearScreen();
											GameplayScreen(playerTable, enemyTable);
											printf("You won game is over!\n");
											usleep(1000*1000);
											gameState = FIN;
										}
									}
									else
									{
										printf("%s\n", errorMessage);
										gameState = FIN;	
									}
								}
								else
								{
									printf("%s\n", errorMessage);
									gameState = FIN;
								}
							}
							else
							{
								printf("Coordinate not valid, try again!");
								usleep(1000*1000);
							}
							break;
						case 2: // PAUSE GAME
							gameState = PAUSE;
							break;
						default:
							printf("Invalid option.\n");
							usleep(1000*1000);
							break;
					}
				}
				else
				{	// PASSIVE
					memset(coordMessage, 0, COORD_LENGTH);
					if(WrapperRecv(playerSocket, coordMessage, COORD_LENGTH, errorMessage))
					{
						char r = coordMessage[0];
						char c = coordMessage[1];
						r -= '0';
						c -= '0';
						// Check if enemy got hit your submarine.
						if(playerTable[(int)r][(int)c] == SUBMARINE)
						{
							// HIT
							playerTable[(int)r][(int)c] = HIT;
							strcpy(confirmMessage, LOST_FIELD);
							if(CheckSubmarineHit(submarine1, 1, (int)r, (int)c))
							{
								submarine1LostFields++;
								strcpy(confirmMessage, LOST_SUBMARINE_1);
							}
							else if(CheckSubmarineHit(submarine2, 2, (int)r, (int)c))
							{
								submarine2LostFields++;
								if(submarine2LostFields == 2)
									strcpy(confirmMessage, LOST_SUBMARINE_2);
							}
							if((submarine1LostFields + submarine2LostFields) == 3)
							{
								ClearScreen();
								GameplayScreen(playerTable, enemyTable);
								// Lost game.
								strcpy(confirmMessage, LOST_GAME);
								if(!WrapperSend(playerSocket, confirmMessage, CONFIRM_LENGTH, errorMessage))
								{
									printf("%s\n", errorMessage);
								}
								printf("\nYou lost the game, game is over!\n");	
								gameState = FIN;
							}
							else
							{	
								// Lost one field.
								if(!WrapperSend(playerSocket, confirmMessage, CONFIRM_LENGTH, errorMessage))
								{
									printf("%s\n", errorMessage);
									gameState = FIN;
								}
							}
						}
						else
						{	// MISS
							active = true;
							if(playerTable[(int)r][(int)c] == WATER)
							{
								playerTable[(int)r][(int)c] = MISS;
							}
							strcpy(confirmMessage, MISSED_FIELD);
							if(WrapperSend(playerSocket, confirmMessage, CONFIRM_LENGTH, errorMessage))
							{
								printf("\nEnemy missed.\n");
							}
							else
							{
								printf("%s\n", errorMessage);
								gameState = FIN;
							}
						}
					}
					else
					{
						printf("%s\n", errorMessage);
						gameState = FIN;
					}
				}
				break;
			case PAUSE:
				ClearScreen();
				MainMenuScreen();
				option = GetUserOptionInput('1', '4');
				switch (option)
				{
					case 3: // END
						gameState = FIN;
						break;
					case 4: // CONTINUE
						gameState = PLAYING;
						break;
					default: // OTHER INPUT
						printf("Invalid option.\n");
						usleep(1000*1000);
						break;
				}
				break;
			case FIN:
				printf("End of game, goodbye!\n");
				gameFinished = true;
				break;
		}
	}
	return 0;
}
