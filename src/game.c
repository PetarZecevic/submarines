#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "game.h"
#include "message.h"
#include "interface.h"

#define ONE_SECOND 1000*1000 //micro-seconds

char errorMessage[DEFAULT_BUFLEN];

// Sleeps program for given ammount of seconds
void WaitFor(int seconds)
{
	usleep(seconds * ONE_SECOND);
}

void PrintGameState(GameState gameState)
{
	printf("GAME STATE: ");
	switch(gameState)
	{
		case INIT:
			printf("Place submarines in your table to start the match\n");
			break;
		case SET_COORD:
			printf("Placing submarines in player's table\n");
		case READY_TO_PLAY:
			printf("Submarines placed, you are ready to start the match\n");
			break;
		case PLAYING:
			printf("Match in progress\n");
			break;
		case PAUSE:
			printf("Match paused\n");
			break;
		case FIN:
			printf("Game finished\n");
			break;
		default:
			break;
	}
	return;
}

void PrintGameplayInfo(const GameStatus* gameStatus)
{
	printf("\tGame info\n");
	// Player Info
	printf("Submarine 1:\n");
	printf("field1 ( %c , %c )\n", (gameStatus->submarine1[0].row + '1') , (gameStatus->submarine1[0].column + 'A'));
	printf("fields left: %d\n\n", gameStatus->fieldsLeftSubmarine1);
	printf("Submarine 2:\n");
	printf("field1 ( %c , %c )\n", (gameStatus->submarine2[0].row + '1') , (gameStatus->submarine2[0].column + 'A'));
	printf("field2 ( %c , %c )\n", (gameStatus->submarine2[1].row + '1') , (gameStatus->submarine2[1].column + 'A'));
	printf("fields left: %d\n\n", gameStatus->fieldsLeftSubmarine2);
	// Enemy info
	printf("Sinked enemy's submarine 1: %d\n", gameStatus->wonSubmarine1);
	printf("Sinked enemy's submarine 2: %d\n\n", gameStatus->wonSubmarine2);
	
	if(gameStatus->engineState == PASSIVE_PLAY)
	{
		printf("Waiting for enemy's move...\n");
	}
	else if(gameStatus->engineState == ACTIVE_PLAY)
	{
		printf("Your turn, guess coordinate of enemy's submarines\n");
	}
	else
	{
		printf("\n");
	}
}

void InitGameStatus(GameStatus* gameStatus)
{
	InitTable(gameStatus->playerTable);
	InitTable(gameStatus->enemyTable);
	gameStatus->gameState = INIT;
	gameStatus->engineState = NOT_PLAY;
	gameStatus->fieldsLeftSubmarine1 = 1;
	gameStatus->fieldsLeftSubmarine2 = 2;
	gameStatus->wonSubmarine1 = 0;
	gameStatus->wonSubmarine2 = 0;
	gameStatus->missedFields = 0;
}

EngineState GetRoleFromServer(int playerSocket)
{
	EngineState retVal;
	char role[2];
	if(WrapperSend(playerSocket, READY, strlen(READY), errorMessage))
	{
		memset(role, 0, 2);
		if(WrapperRecv(playerSocket, role, 2, errorMessage))
		{
			if(strncmp(role, ACTIVE, 1) == 0)
			{
				retVal = ACTIVE_PLAY;
			}
			else if(strncmp(role, PASSIVE, 1) == 0)
			{
				retVal = PASSIVE_PLAY;
			}
			else
			{
				retVal = NOT_PLAY;
			}
		}
		else
		{
			retVal = NOT_PLAY;
		}
	}
	else
	{
		retVal = NOT_PLAY;
	}

	return retVal;
}

bool CreateFeedback(int r, int c, char feedbackMessage[], GameStatus* gameStatus)
{	
	bool lost = false;
	// Check if enemy got hit your submarine.
	if(gameStatus->playerTable[r][c] == SUBMARINE)
	{
		// HIT
		gameStatus->playerTable[r][c] = HIT;
		strcpy(feedbackMessage, LOST_FIELD);
		if(CheckSubmarineHit(gameStatus->submarine1, 1, r, c))
		{
			gameStatus->fieldsLeftSubmarine1--;
			strcpy(feedbackMessage, LOST_SUBMARINE_1);
		}
		else if(CheckSubmarineHit(gameStatus->submarine2, 2, r, c))
		{
			gameStatus->fieldsLeftSubmarine2--;
			if(gameStatus->fieldsLeftSubmarine2 == 0)
				strcpy(feedbackMessage, LOST_SUBMARINE_2);
		}
		
		if((gameStatus->fieldsLeftSubmarine1 + gameStatus->fieldsLeftSubmarine2) == 0)
		{
			// Lost game.
			strcpy(feedbackMessage, LOST_MATCH);
			lost = true;
		}
	}
	else
	{	// MISS
		strcpy(feedbackMessage, MISSED_FIELD);
		if(gameStatus->playerTable[r][c] == WATER)
		{
			gameStatus->playerTable[r][c] = MISS;
			gameStatus->missedFields++;
			if(gameStatus->missedFields == 6)
			{
				strcpy(feedbackMessage, LOST_MATCH_M);
				lost = true;	
			}
		}
		gameStatus->engineState = ACTIVE_PLAY;
	}

	return lost;
}

bool InterpretFeedback(int r, int c, char feedbackMessage[], GameStatus* gameStatus)
{
	bool wonGame = false;
	if(strncmp(feedbackMessage, LOST_FIELD, FEEDBACK_LENGTH) == 0)
	{
		gameStatus->enemyTable[r][c] = HIT;
	}
	else if(strncmp(feedbackMessage, LOST_SUBMARINE_1, FEEDBACK_LENGTH) == 0)
	{
		gameStatus->enemyTable[r][c] = HIT;
		gameStatus->wonSubmarine1++;
	}
	else if(strncmp(feedbackMessage, LOST_SUBMARINE_2, FEEDBACK_LENGTH) == 0)
	{
		gameStatus->enemyTable[r][c] = HIT;
		gameStatus->wonSubmarine2++;
	}
	else if(strncmp(feedbackMessage, MISSED_FIELD, FEEDBACK_LENGTH) == 0)
	{
		if(gameStatus->enemyTable[r][c] == WATER)
		{
			gameStatus->enemyTable[r][c] = MISS;
		}
		gameStatus->engineState = PASSIVE_PLAY;
	}
	else if(strncmp(feedbackMessage, LOST_MATCH, FEEDBACK_LENGTH) == 0)
	{


		gameStatus->enemyTable[r][c] = HIT;
		gameStatus->wonSubmarine1 = 1;
		gameStatus->wonSubmarine2 = 1;
		wonGame = true;
	}
	else if(strncmp(feedbackMessage, LOST_MATCH_M, FEEDBACK_LENGTH) == 0)
	{
		gameStatus->enemyTable[r][c] = MISS;
		gameStatus->wonSubmarine1 = 1;
		gameStatus->wonSubmarine2 = 1;
		wonGame = true;
	}
	return wonGame;
}

void GameEngine(int playerSocket, GameStatus* gameStatus)
{
	ClearScreen();
	PrintGameState(gameStatus->gameState);

	// Messages.
	char coordMessage[COORD_LENGTH] = {0};
	char feedbackMessage[FEEDBACK_LENGTH] = {0};
	char errorMessage[DEFAULT_BUFLEN] = {0};
	
	// Option from menu.
	char option;

	// Display gameplay screen.
	ClearScreen();
	PrintGameState(gameStatus->gameState);
	GameplayScreen(gameStatus->playerTable, gameStatus->enemyTable);
	PrintGameplayInfo(gameStatus);

    // Help variables for storing coordinates.
    char r, c;

	if(gameStatus->engineState == ACTIVE_PLAY)
	{
		GameplayOptions();
		option = GetUserOptionInput('1', '4');
		switch(option)
		{	
			case 1: // NEW MOVE
				GetRowColumnInput(&r, &c);
				if(CoordValid(r, c))
				{
					coordMessage[0] = r + '0';
					coordMessage[1] = c + '0';
					// Send move.
					if(WrapperSend(playerSocket, coordMessage, COORD_LENGTH, errorMessage))
					{
						memset(feedbackMessage, 0, FEEDBACK_LENGTH);
						// Receive result, hit or miss.
						if(WrapperRecv(playerSocket, feedbackMessage, FEEDBACK_LENGTH, errorMessage))
						{
							if(InterpretFeedback(r, c, feedbackMessage, gameStatus))
							{	
								gameStatus->engineState = NOT_PLAY;
								ClearScreen();
								printf("Last Gameplay State\n");
								GameplayScreen(gameStatus->playerTable, gameStatus->enemyTable);
								PrintGameplayInfo(gameStatus);
								printf("You won the match!\n");
								WaitFor(3);
								gameStatus->gameState = INIT;
							}
						}
						else
						{
							printf("%s\n", errorMessage);
							WaitFor(2);
							gameStatus->gameState = FIN;	
						}
					}
					else
					{
						printf("%s\n", errorMessage);
						WaitFor(2);
						gameStatus->gameState = FIN;
					}
				}
				else
				{
					printf("Coordinate not valid, try again!");
					WaitFor(1);
				}
				break;
			case 2: // PAUSE MATCH
				gameStatus->gameState = PAUSE;
				break;
			default:
				printf("Invalid option.\n");
				WaitFor(1);
				break;
		}
	}
	else if(gameStatus->engineState == PASSIVE_PLAY)
	{	// PASSIVE
		memset(coordMessage, 0, COORD_LENGTH);
		if(WrapperRecv(playerSocket, coordMessage, COORD_LENGTH, errorMessage))
		{
			if(strncmp(coordMessage, END_MATCH, COORD_LENGTH) == 0)
			{
				gameStatus->engineState = NOT_PLAY;
				ClearScreen();
				printf("Last Gameplay State\n");
				GameplayScreen(gameStatus->playerTable, gameStatus->enemyTable);
				PrintGameplayInfo(gameStatus);
				printf("You won the match, other player submitted!\n");
				WaitFor(3);
				gameStatus->gameState = INIT;
			}
			else
			{
				char r = coordMessage[0];
				char c = coordMessage[1];
				r -= '0';
				c -= '0';
				if(CreateFeedback(r, c, feedbackMessage, gameStatus))
				{
					gameStatus->engineState = NOT_PLAY;
					ClearScreen();
					printf("Last Gameplay State\n");
					GameplayScreen(gameStatus->playerTable, gameStatus->enemyTable);
					PrintGameplayInfo(gameStatus);
					printf("You lost the match!\n");
					WaitFor(3);
					gameStatus->gameState = INIT;
				}
				if(!WrapperSend(playerSocket, feedbackMessage, FEEDBACK_LENGTH, errorMessage))
				{
					printf("%s\n", errorMessage);
					WaitFor(2);
					gameStatus->gameState = FIN;
				}
			}
		}
		else
		{
			printf("%s\n", errorMessage);
			WaitFor(2);
			gameStatus->gameState = FIN;
		}
	}
}

void GameLoop(int playerSocket)
{	
	// Holds game data.
	GameStatus gameStatus;
	// Game flags.
	bool gameFinished = false;
	// Option from menies.
	char option;
	// Init game status.
	InitGameStatus(&gameStatus);
	
	// Loop.
	while(!gameFinished)
	{
		switch(gameStatus.gameState)
		{
		
			case INIT:
				// Init game.
				InitGameStatus(&gameStatus);
				ClearScreen();
				PrintGameState(gameStatus.gameState);
				MainMenuScreen();
				option = GetUserOptionInput('1', '5');
				switch (option)
				{
					case 1: // SET
						gameStatus.gameState = SET_COORD;
						break;
					case 5: // EXIT
						gameStatus.gameState = FIN;
						break;
					default: // OTHER INPUT
						printf("Invalid option.\n");
						WaitFor(1);
						break;
				}
				break;
			case SET_COORD:
				ClearScreen();
				SetCoordinates(gameStatus.playerTable, gameStatus.submarine1, gameStatus.submarine2);
				ClearScreen();
				printf("Submarines placed\n");
				PrintTable(gameStatus.playerTable);
				WaitFor(3);
				gameStatus.gameState = READY_TO_PLAY;
				break;
			case READY_TO_PLAY:
				ClearScreen();
				PrintGameState(gameStatus.gameState);
				MainMenuScreen();
				option = GetUserOptionInput('1', '5');
				switch(option)
				{
					case 1: // SET
						gameStatus.gameState = SET_COORD;
						break;
					case 2: // START NEW MATCH
						ClearScreen();
						printf("Waiting for other player to start the match...\n");
						gameStatus.engineState = GetRoleFromServer(playerSocket);
						if(gameStatus.engineState == NOT_PLAY)
						{
							printf("%s\n", errorMessage);
							WaitFor(2);
							gameStatus.gameState = FIN;
						}
						else
							gameStatus.gameState = PLAYING;
						break;
					case 4: // CONTINUE
						printf("You can not continue the match, because you have not played yet.\n");
						WaitFor(1);
						break;
					case 5: // EXIT
						gameStatus.gameState = FIN;
						break;
					default: // OTHER INPUT
						printf("Invalid option.\n");
						WaitFor(1);
						break;
				}
				break;
			case PLAYING:
				GameEngine(playerSocket, &gameStatus);
				break;
			case PAUSE:
				ClearScreen();
				PrintGameState(gameStatus.gameState);
				MainMenuScreen();
				option = GetUserOptionInput('1', '5');
				switch (option)
				{
					case 3: // END MATCH
						gameStatus.gameState = INIT;
						char endMatchMessage[COORD_LENGTH];
						strcpy(endMatchMessage, END_MATCH);
						if(!WrapperSend(playerSocket, endMatchMessage, COORD_LENGTH, errorMessage))
						{
							// Error case.
							ClearScreen();
							printf("%s\n", errorMessage);
							WaitFor(2);
							gameStatus.gameState = FIN;
						}
						else
						{
							ClearScreen();
							printf("Last Gameplay State\n");
							PrintGameplayInfo(&gameStatus);
							printf("You lost the match, because you left!\n");
							WaitFor(3);
						}
						break;
					case 4: // CONTINUE
						gameStatus.gameState = PLAYING;
						break;
					case 5: // EXIT
						gameStatus.gameState = FIN;
						break;
					default: // OTHER INPUT
						printf("Invalid option.\n");
						WaitFor(1);
						break;
				}
				break;
			case FIN:
				ClearScreen();
				PrintGameState(gameStatus.gameState);
				printf("Last Gameplay State\n\n");
				GameplayScreen(gameStatus.playerTable, gameStatus.enemyTable);
				printf("End of game, goodbye!\n");
				gameFinished = true;
				break;
		}
	}
}
