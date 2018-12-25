#include <stdio.h>
#include "table.h"

void InitTable(Table t)
{
	for(int i = 0; i < ROW; i++)
    {
        for(int j = 0; j < COL; j++)
        {
            t[i][j] = WATER;
        }
    }
}

void PrintTable(const Table t)
{
    printf("  ");
	char ch = 'A';
	for(; ch < ('A' + 3); ch++)
	{
		printf("%c ", ch);
	}
	printf("\n");
	for(int r = 1; r < 4; r++)
	{
		printf("%d ", r);
		for(int i = 0; i < 3; i++)
		{
			printf("%c ", t[r-1][i]);
		}
		printf("\n");	
	}
}
bool CheckSubmarine1(const Table t,int row,int col)
{
	bool allow;
	if(row>ROW-1 || col>COL-1 || row<0 || col<0)
	{
		printf("Values of rows and columns must be inside the range\n");
		allow=false;
	}
	else if(t[row][col]==WATER)
	{
		allow=true;
	}
	else
	{
		printf("This position has already been taken.\n");
		allow=false;
	}
	return allow;
}


bool CheckSubmarine2(const Table t, int row1,int col1,int row2,int col2)
{
	bool success = true;
	if(row2>ROW-1 || col2>COL-1 || row2<0 || col2<0)
	{
		printf("Values of rows and columns must be inside the range\n");
		success = false;
	}
	else
	{
		if(t[row2][col2]==WATER)
		{	
			if( ((row2==row1+1) && (col2==col1)) || ((row2==row1-1) && (col2==col1)) ||
				((row2==row1) && (col2==col1+1)) || ((row2==row1) && (col2==col1-1)))
			{
				success = true;
			}
			else
			{
				printf("Fields must be adjacent by row or column!\n");
				success = false;
			}
		}
		else
		{
			printf("Position has already been taken.\n");
			success = false;
		}
	}
	return success;
}

bool CoordValid(char r, char c)
{
	return (r < ROW) && (r >= 0) && (c < COL) && (c >= 0);
}

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
