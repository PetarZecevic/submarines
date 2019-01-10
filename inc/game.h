#ifndef GAME_H
#define GAME_H

#include "table.h"

typedef enum {INIT = 0, SET_COORD, READY_TO_PLAY, PLAYING, PAUSE, FIN}GameState;

typedef enum {ACTIVE_PLAY, PASSIVE_PLAY, NOT_PLAY}EngineState;

// Structure that holds all data needed for gameplay.
typedef struct{
	Table playerTable;
	Field submarine1[1];
	Field submarine2[2];
	Table enemyTable;
	int fieldsLeftSubmarine1;
	int fieldsLeftSubmarine2;
	int wonSubmarine1;
	int wonSubmarine2;
	int missedFields;
	GameState gameState;
	EngineState engineState;
}GameStatus;

// Global buffer for storing messages about errors that ocurr during the game.
extern char errorMessage[DEFAULT_BUFLEN];

/*
Display current state of game so that player knows which option to select.
*/
extern void PrintGameState(GameState gameState);

/*
Display info that contains, submarine type 1 coordinates and fields left, 
submarine type 2 coordinates and fields left.
How many enemy's submarines are sinked.
*/
extern void PrintGameplayInfo(const GameStatus* gameStatus);

/*
Prepare game data:
    - Empty player and enemy table.
    - Init game state.
    - Init engine state.
    - Recover number of submarine's fields.
    - Reset sinked enemy submarines. 
*/
extern void InitGameStatus(GameStatus* gameStatus);

/*
Sends server message that player is ready and receives message that tells if player is active or passive in current gameplay.
Parameter playerSocket is used as communication socket with server.
It's assumed that player is already connected to server.
State is returned, if error ocurred, NOT_PLAY state is returned.
*/
extern EngineState GetRoleFromServer(int playerSocket);

/*
Checks given coordinate(r,c) in player's table, to send server a feedback through feedbackMessage about impact.
Impact:
	- Lost field of submarine on that coordinate
	- Lost submarine1
	- Lost submarine2
	- Lost Game
	- Didn't lost field(MISS)
After check is done, player's table is updated through gameStatus structure.
Function returns true if player lost game, otherwise false.
*/
extern bool CreateFeedback(int r, int c, char feedbackMessage[], GameStatus* gameStatus);

/*
Interpret feedback message from server.
That is message that describes result of player hitting enemy's submarine.
Possible results are:
	- Missed enemy's submarine field
	- Sinked enemy's submarine1
	- Sinked enemy's submarine2
	- Sinked one field submarine(doesn't know which one)
We use r and c, as coordinate to update enemy table.
Coordinate is the coordinate that player send's server to hit enemy's submarine.
*/
extern bool InterpretFeedback(int r, int c, char feedbackMessage[], GameStatus* gameStatus);


/*
Function that executes communication with server, during the gameplay.
If player is active, player can hit enemy submarine coordinate or pause the game.
If player is passive he checks if enemy hits his submarine field.
Both cases above are executed through communication with server, that arbitrages the game.
*/
extern void GameEngine(int playerSocket, GameStatus* gameStatus);

/*
Function that executes FSM(finite state machine) that represents game logic.
Depending on state, different functionalities are executed.
Parameter playerSocket is needed for communication with server during the game.
It's assumed that player is already connected to server.
Game is played with one opponent.
For more info about the game check docs directory.
*/
extern void GameLoop(int playerSocket);

#endif // GAME_H