#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "config.h"
#include "interface.h"
#include "table.h"

void MainMenuScreen(void)
{
	printf("Options:\n");
	printf(" 1. Set Coordinates for your submarines\n");
	printf(" 2. Start new match\n");
	printf(" 3. End match\n");
	printf(" 4. Resume to match\n");
	printf(" 5. Exit game\n");
	return;
}

void GameplayScreen(const Table playerTable,const Table enemyTable)
{
	printf("Your table\n");
	PrintTable(playerTable);
	printf("\nEnemy table\n");
	PrintTable(enemyTable);
	return;
}

void GameplayOptions(void)
{
	printf("\nOptions:\n");
	printf("\t1. New move\n");
	printf("\t2. Main menu\n");
	return;
}

void ClearScreen(void)
{
    system("clear");
	return;
}

void GetRowColumnInput(char* row, char* column)
{
	char r, c;
	// Row.
	printf("Enter row(1-3): ");
	scanf("%c", &r);
	int i = 0;
	// Check if there was more than one char that is not '\n'.
	while(getchar() != '\n')
	{
		if(i < 2)
			i++;
	}
	if(i > 0)
		// Error in input.
		r = -1;
	else
		// Shift input to range 0-n
		r -= '1';

	// Column.
	printf("Enter column(A-C): ");
	scanf("%c", &c);
	i = 0;
	while(getchar() != '\n')
	{
		if(i < 2)
			i++;
	}
	if(i > 0)
		c = -1;
	else
		c -= 'A';

	// Update references.
	*row = r;
	*column = c;
	return;
}


void SetCoordinates(Table playerTable,Field submarine1[], Field submarine2[])
{
	// Clean player's table before setting submarines.
	InitTable(playerTable);
	// Temporary variables for user input.
	char row,col;
	// First Submarine.
	do
	{
		ClearScreen();
		PrintTable(playerTable);
		printf("First Submarine\n");
		GetRowColumnInput(&row, &col);
	}while(!CheckSubmarine1(playerTable, row, col));
	// Update
	playerTable[(int)row][(int)col] = SUBMARINE;
	submarine1[0].row = row;
	submarine1[0].column = col;

	// Second submarine.
	// First field.
	do
	{
		ClearScreen();
		PrintTable(playerTable);
		printf("Second submarine, first field\n");
		GetRowColumnInput(&row, &col);
	}while(!CheckSubmarine1(playerTable, row, col));
	// Update
	playerTable[(int)row][(int)col] = SUBMARINE;
	submarine2[0].row = row;
	submarine2[0].column = col;

	// Second field.
	do
	{
		ClearScreen();
		PrintTable(playerTable);
		printf("Second submarine, second field\n");
		GetRowColumnInput(&row, &col);
	}while(!CheckSubmarine2(playerTable, submarine2[0].row, submarine2[0].column, row, col));
	// Update
	playerTable[(int)row][(int)col] = SUBMARINE;
	submarine2[1].row = row;
	submarine2[1].column = col;

	return;
}

char GetUserOptionInput(char lowerBound, char upperBound)
{
	char input;
	printf("input:");
	scanf("%c", &input);
	int i = 0;
	while(getchar() != '\n')
	{
		if(i < 2)
			i++;
	}
	if(i > 0 || input < lowerBound || input > upperBound)
		input = -1;
	else
		input = (input+1) - lowerBound;
	return input;
}
