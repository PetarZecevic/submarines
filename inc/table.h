#ifndef TABLE_H
#define TABLE_H

#include <stdbool.h>
#include "config.h"

/*
Structure representing one field one the table.
Field will hold one coordinate and will be used a part of the submarine.
Empty fields are denoted by WATER symbol.
*/
typedef struct Field
{
	int row;
	int column;
}Field;

/*
Table will hold submarines, during the game.
*/
typedef char Table[ROW][COL];

/*
Initialize table(ROWxCOL) with empty values represented by WATER.
*/
extern void InitTable(Table t);

/*
Table columns are denoted by 'A'-'C', and rows by '1'-'3'.
*/
extern void PrintTable(const Table t);

/*
Check if submarine1 is correctly placed on the table t.
*/
extern bool CheckSubmarine1(const Table t,int row,int col);

/*
Check if submarine 2 if correctly placed on the table t, we assume first field is correctly placed, 
second field must be adjacent to first field, adjacent in row or column.
First field's row is row1, and column is col1.
Second field's row is row2, and column is col2.
*/
extern bool CheckSubmarine2(const Table t,int row1,int col1,int row2, int col2);

// Check if coordinate is valid, according to predefined dimension of the table.
extern bool CoordValid(char r, char c);

/*
Check if submarine's field is located at coordinate(checkRow, checkColumn)
*/
extern bool CheckSubmarineHit(Field submarine[], int submarineLength, int checkRow, int checkColumn);

#endif // TABLE_H
