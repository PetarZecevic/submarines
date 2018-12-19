#ifndef INTERFACE_H
#define INTERFACE_H

#include "table.h"

/*
Prints main menu with options.
*/
extern void MainMenuScreen(void);

/*
Screen with player's and enemy's board that player will see during gameplay.
*/
extern void GameplayScreen(const Table playerTable,const Table enemyTable);

/*
Options that player can select during gameplay.
*/
extern void GameplayOptions(void);

/*
Clears content of the current screen, for the next screen.
*/
extern void ClearScreen(void);

/*
 It is assumed that user wiil input row from the range '1'-'3'.
 It is also assumed that user inputs column from range 'A'-'C'.
 Row and column arguments are passed by reference.
 They will hold result of user's input after function ends.
 If input was out of proposed range, row will hold -1, and column also.
 If row input was correct,we transform input range '1'-'3' to 0-2.
 If column input was correct, we transform input range 'A'-'C' to 0-2.
*/
extern void GetRowColumnInput(char* row, char* column);

/*
Player set's coordinates for his two submarines.
It is assumed that submarine1 has one field and submarine2 has two fields.
*/
extern void SetCoordinates(Table playerTable, Field submarine1[], Field submarine2[]);

/*
Prompts user to input char value from range (lowerBound - upperBound)
If multiple chars were entered, or value was out of range, function returns -1.
Otherwise it returns scaled input value by this mapping:
	lowerBound -> 1,
	lowerBound+1 -> 2,
	...
	upperBound -> 1 + (upperBound-lowerBound).
*/
extern char GetUserOptionInput(char lowerBound, char upperBound);

#endif // INTERFACE_H
