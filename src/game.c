#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "game.h"
#include "message.h"
#include "interface.h"

char errorMessage[DEFAULT_BUFLEN];

void PrintGameState(GameState gameState)
{
	printf("GAME STATE: ");
	switch(gameState)
	{
		case INIT:
			printf("Place submarines in your table to start the game\n");
			break;
		case SET_COORD:
			printf("Submarines placed, you are ready to start the game\n");
			break;
		case PLAYING:
			printf("Gameplay in progress\n");
			break;
		case PAUSE:
			printf("Game paused\n");
			break;
		case FIN:
			printf("Game finished\n");
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
	printf("fields left: %d\n", gameStatus->fieldsLeftSubmarine1);
	printf("Submarine 2:\n");
	printf("field1 ( %c , %c )\n", (gameStatus->submarine2[0].row + '1') , (gameStatus->submarine2[0].column + 'A'));
	printf("field2 ( %c , %c )\n", (gameStatus->submarine2[1].row + '1') , (gameStatus->submarine2[1].column + 'A'));
	printf("fields left: %d\n", gameStatus->fieldsLeftSubmarine2);
	// Enemy info
	printf("Sinked enemy's submarine 1: %d\n", gameStatus->wonSubmarine1);
	printf("Sinked enemy's submarine 2: %d\n", gameStatus->wonSubmarine2);
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
}

bool GetRoleFromServer(int playerSocket, EngineState* state)
{
	bool success = true;
	char role[2];
	if(WrapperSend(playerSocket, READY, strlen(READY), errorMessage))
	{
		memset(role, 0, 2);
		if(WrapperRecv(playerSocket, role, 2, errorMessage))
		{
			if(strncmp(role, ACTIVE, 1) == 0)
			{
				*state = ACTIVE_PLAY;
			}
			else if(strncmp(role, PASSIVE, 1) == 0)
			{
				*state = PASSIVE_PLAY;
			}
		}
		else
		{
			printf("%s\n", errorMessage);
			success = false;
		}
	}
	else
	{
		printf("%s\n", errorMessage);
		success = false;
	}
	
	return success;
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
			strcpy(feedbackMessage, LOST_GAME);
			lost = true;
		}
	}
	else
	{	// MISS
		strcpy(feedbackMessage, MISSED_FIELD);
		if(gameStatus->playerTable[r][c] == WATER)
		{
			gameStatus->playerTable[r][c] = MISS;
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
	else if(strncmp(feedbackMessage, LOST_GAME, FEEDBACK_LENGTH) == 0)
	{
		gameStatus->enemyTable[r][c] = HIT;
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
	
	// Flag for role logic.
	static bool startedGame = false;

	if(!startedGame)
	{
		printf("Waiting for other player to start the game...\n");	
		if(!GetRoleFromServer(playerSocket, &(gameStatus->engineState)))
		{
			gameStatus->gameState = FIN;
			return;
		}
		startedGame = true;
	}

	// Display gameplay screen.
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
								printf("You won the game!\n");
								gameStatus->gameState = FIN;
							}
						}
						else
						{
							printf("%s\n", errorMessage);
							gameStatus->gameState = FIN;	
						}
					}
					else
					{
						printf("%s\n", errorMessage);
						gameStatus->gameState = FIN;
					}
				}
				else
				{
					printf("Coordinate not valid, try again!");
					usleep(1000*1000);
				}
				break;
			case 2: // PAUSE GAME
				gameStatus->gameState = PAUSE;
				break;
			default:
				printf("Invalid option.\n");
				usleep(1000*1000);
				break;
		}
	}
	else if(gameStatus->engineState == PASSIVE_PLAY)
	{	// PASSIVE
		memset(coordMessage, 0, COORD_LENGTH);
		if(WrapperRecv(playerSocket, coordMessage, COORD_LENGTH, errorMessage))
		{
			char r = coordMessage[0];
			char c = coordMessage[1];
			r -= '0';
			c -= '0';
			if(CreateFeedback(r, c, feedbackMessage, gameStatus))
			{
				printf("You lost the game!\n");
				gameStatus->gameState = FIN;
			}
			if(!WrapperSend(playerSocket, feedbackMessage, FEEDBACK_LENGTH, errorMessage))
			{
				printf("%s\n", errorMessage);
				gameStatus->gameState = FIN;
			}
		}
		else
		{
			printf("%s\n", errorMessage);
			gameStatus->gameState = FIN;
		}
	}
}

void GameLoop(int playerSocket)
{	
	// Holds game data.
	GameStatus gameStatus;
	// Init game.
	InitGameStatus(&gameStatus);
	// Game flags.
	bool gameFinished = false, alreadySet = false;
	// Option from menies.
	char option;
	// Loop.
	while(!gameFinished)
	{
		switch(gameStatus.gameState)
		{
		
			case INIT:
				ClearScreen();
				PrintGameState(gameStatus.gameState);
				MainMenuScreen();
				option = GetUserOptionInput('1', '4');
				switch (option)
				{
					case 1: // SET
						gameStatus.gameState = SET_COORD;
						break;
					case 3: // END
						gameStatus.gameState = FIN;
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
					SetCoordinates(gameStatus.playerTable, gameStatus.submarine1, gameStatus.submarine2);
					ClearScreen();
				}
				alreadySet = true;
				PrintGameState(gameStatus.gameState);
				MainMenuScreen();
				option = GetUserOptionInput('1', '4');
				switch(option)
				{
					case 1: // SET
						gameStatus.gameState = SET_COORD;
						alreadySet = false;
						break;
					case 2: // PLAY
						gameStatus.gameState = PLAYING;
						break;
					case 3: // END
						gameStatus.gameState = FIN;
						break;
					case 4: // CONTINUE
						gameStatus.gameState = SET_COORD;
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
				GameEngine(playerSocket, &gameStatus);
				break;
			case PAUSE:
				ClearScreen();
				PrintGameState(gameStatus.gameState);
				MainMenuScreen();
				option = GetUserOptionInput('1', '4');
				switch (option)
				{
					case 3: // END
						gameStatus.gameState = FIN;
						break;
					case 4: // CONTINUE
						gameStatus.gameState = PLAYING;
						break;
					default: // OTHER INPUT
						printf("Invalid option.\n");
						usleep(1000*1000);
						break;
				}
				break;
			case FIN:
				ClearScreen();
				PrintGameState(gameStatus.gameState);
				GameplayScreen(gameStatus.playerTable, gameStatus.enemyTable);
				printf("Last Gameplay State\n");
				printf("End of game, goodbye!\n");
				gameFinished = true;
				break;
		}
	}
}