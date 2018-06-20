//////////////////////////////////
//A representation of a Sudoku puzzle. Also contains a recursive backtracking 
// algorithm capable of solving valid Sudoku puzzles using Constraint 
// Propogation and a Most Constrained/MRV Heuristic. 
// See readme.md and results.docx for more info
// 
//
///Author: Dustin Fast (dustin.fast@outlook.com), 2017

#pragma once
#include <iostream>
#include <string>
#include <Windows.h>
#include <fstream>
#include <queue>

using namespace std;

struct Cell //Representation of a Sudoku cell
{
	Cell() {}
	Cell(int val) : nVal(val) {}

	int nVal = -1;
	int nConstraintCount = 0;
	int constraintsOnCell[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };	//Constraints on this cell. Index aligns with SudokuPuzzle::m_nDomain.. 
																//-1 denotes a constrained value. ex: if constraints[2] == -1 then 3 is not a valid assignment

	//if constraint is not already constrained for this cell, constrains it and increments constraint count
	void setConstraint(int constraint) 
	{
		if (constraintsOnCell[constraint - 1] == 0) 
		{
			constraintsOnCell[constraint - 1] = -1;
			nConstraintCount++;
		}
	}

	//Returns -1 if there are no more remaining unconstrained values, else returns the last remaining value
	//Note: This function assumes it is only called when the cell has 8 or more constraints on it.
	int getLastRemainingValue()
	{
		for (int i = 0; i < 9; i++)
			if (constraintsOnCell[i] == 0)
				return i + 1;
		return -1;
	}
	
};

class SudokuPuzzle //Representation of a Sudoku puzzle with tiles of type Cell
{
public:
	SudokuPuzzle(string filename);

	bool doSolve(); //Public function to call recursive backtracking algorithm function.
	void printPuzzle(); //Outputs the puzzle to the console, in its current state.
	void printConstraintCounts(); //Outputs the num constraints of each unassinged cell. For debug purposes

private:
	bool solveSudoku();
	void copyUnit(int row, int col, Cell from[9][9], Cell to[9][9]); //copies members of row/cols's unit from a 9x9 grid to a 9x9 grid.
	void setInitialCellConstraints();
	bool propogateConstraint(int row, int col, int val); //sets the constraint flag for val of each cell in m_grid[row][col]'s unit. Returns true iff no values are fully constrained.
	bool getNextEmptyCell(int &row, int &col);
	bool getNextEmptyCellByConstraintCount(int &row, int &col);
	bool isValidAssignment(int row, int col, int val);	//Checks each unit (i.e row, column, and 3x3 box) for the existence of a given val

	Cell m_grid[9][9];		// The 9x9 puzzle grid.
	Cell m_origGrid[9][9];	// A copy of the original grid, to be used in denoting original numbers when printing
	//const int m_domain[9] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };		// An array of the nine possible cell values, to be used with Cell::valConstraints
};

//Constructor - populates the puzzle grid from the specified csv file representing the grid
SudokuPuzzle::SudokuPuzzle(string filename) 
{
	ifstream fsIn;
	fsIn.open(filename);
	if (fsIn.fail())
		cout << "\nError: File could not be opened. Behavior will be unpredicatable.\n";

	string strRow;
	int nRowIndex = 0;
	while (getline(fsIn, strRow, '\n')) //iterate file by line
	{
		string str = "";
		int nColIndex = 0;
		for (unsigned int i = 0; i < strRow.length(); i++) //essentially explode strRow on comma delimeter
		{
			if (strRow[i] != ',')
			{
				m_grid[nRowIndex][nColIndex] = Cell(atoi(&strRow[i]));
				m_origGrid[nRowIndex][nColIndex] = Cell(atoi(&strRow[i]));
				nColIndex++;
			}
		}
		nRowIndex++;
	}
	fsIn.close();
	
	//Set initial cell constraints
	setInitialCellConstraints();
}

// Public function to start the recursive backtracking algororithm to solve the puzzle
// Returns true iff puzzle was solved
bool SudokuPuzzle::doSolve()
{
	//Do 
	if (solveSudoku())
		return true;
	else
		return false;
}

// Recursive backtracking algorithm to solve the puzzle. 
// Returns true iff puzzle was solved
bool SudokuPuzzle::solveSudoku()
{
	//Assigns the next unassigned cell to (nRow, nCol)
	//After the last empty cell has been populated, getNextEmptyCell will
	//return false. This is our recursive base case and signifies that the 
	//puzzle has been solved.
	int nRow, nCol;
	if (!getNextEmptyCellByConstraintCount(nRow, nCol)) 
		return true;

	for (int val = 1; val <= 9; val++) //iterate cell domain. Should really iterate m_domain here instead.
	{
		if (m_grid[nRow][nCol].constraintsOnCell[val - 1] == -1) //don't evaluate this value if it's constrained for the variable
			continue;

		if (isValidAssignment(nRow, nCol, val))
		{
			//Make a copy of the current game grid, to restore after we leave a processed node.
			Cell tempGridCopy[9][9];
			copyUnit(nRow, nCol, m_grid, tempGridCopy);

			//modify value of current cell
			m_grid[nRow][nCol].nVal = val;

			//add this value to the constraints list of every cell in this unit and if a variable was found to 
			//to have no remaining valid assignments, don't evalue this path further.
			if (propogateConstraint(nRow, nCol, val))
			{
				//printConstraintCounts(); //debug
				//system("pause"); //debug

				if (solveSudoku())
					return true;
			}

			//Set grid cells back to what they were before the recursive call
			//Note this is never reached once we return true (above) for the first time.
			copyUnit(nRow, nCol, tempGridCopy, m_grid);
		}
	}
	return false;
}

//Gets the first unassinged cell (i.e cell == 0), searching from l to r, top to bottom.
// Returns true iff an unsassigned cell was found. 
// Populates parameters row and col with the row/col of the empty cell found.
bool SudokuPuzzle::getNextEmptyCell(int &row, int &col)
{
	for (row = 0; row < 9; row++)
		for (col = 0; col < 9; col++)
			if (m_grid[row][col].nVal == 0)
				return true; //if empty cell found
	//if empty cell not found
	return false;
}

//Gets the next unassinged cell (i.e cell == 0), searching from l to r, top to bottom, with the highest number of constraints on it (i.e. Most Constrained/MRV Heuristic)
// Returns true iff an unsassigned cell was found. 
// Populates parameters row and col with the row/col of the unassigned cell found.
bool SudokuPuzzle::getNextEmptyCellByConstraintCount(int &row, int &col)
{
	int nMostConstrained = -1;
	row = col = -1;
	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (m_grid[i][j].nVal == 0 && m_grid[i][j].nConstraintCount > nMostConstrained) //if we're on an unassigned cell with a higher constraint count than the last highest
			{
				row = i;
				col = j;
				nMostConstrained = m_grid[i][j].nConstraintCount;
			}
		}
	}

	if (row != -1) //if an unassigned cell was found
		return true;
	return false; //if unassigned cell not found
}

//Checks each row, column, and 3x3 box for the existence of a given val. 
//Returns false iff value is not found.
bool SudokuPuzzle::isValidAssignment(int row, int col, int val)	
{
	//check that we have an initialised value
	if (val == -1)
		return false;
	//Check col
	for (int i = 0; i < 9; i++) 
		if (m_grid[i][col].nVal == val)
			return false;

	//Check row
	for (int j = 0; j < 9; j++) 
		if (m_grid[row][j].nVal == val)
			return false;

	//Check box
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			if (m_grid[row - (row % 3) + i][col - (col % 3) + j].nVal == val)
				return false;

	return true;
}

//Outputs the puzzle to the console in its current state.
//Unassigned cells are printed as default color, else printed as blue
void SudokuPuzzle::printPuzzle()
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	cout << endl;
	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (m_grid[i][j].nVal != 0)
				SetConsoleTextAttribute(hConsole, 11); //set output color to blue	
			cout << m_grid[i][j].nVal << "  ";
			SetConsoleTextAttribute(hConsole, 7); //ensure output color goes back to default
		}
		cout << endl;
	}
}

//Outputs original puzzle vals as blue and constraint counts of unassinged cols as white.
//For debug use
void SudokuPuzzle::printConstraintCounts()
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	cout << endl;
	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (m_grid[i][j].nVal == 0)
				cout << (m_grid[i][j].nConstraintCount) << "  ";
			else
			{
				SetConsoleTextAttribute(hConsole, 11); //set output color to blue	
				cout << m_grid[i][j].nVal << "  ";
			}
			SetConsoleTextAttribute(hConsole, 7); //ensure output color goes back to default

		}
		cout << endl;
	}

	/*cout << "cell (8, 8) constraints: ";
	for (int i = 0; i < 9; i++)
		cout << m_grid[8][8].constraintsOnVal[i] << "  ";*/
}

//Sets the constraints on each cell and populate that cell's Constraints list
void SudokuPuzzle::setInitialCellConstraints()
{
	for (int i = 0; i < 9; i++) //iterate rows
	{
		for (int j = 0; j < 9; j++) //iterate cols
		{
			if (m_grid[i][j].nVal == 0)
			{
				//row
				for (int k = 0; k < 9; k++)
				{
					if (m_grid[i][k].nVal != 0)
					{
						m_grid[i][j].setConstraint(m_grid[i][k].nVal);
					}
				}

				//col
				for (int k = 0; k < 9; k++)
				{
					if (m_grid[k][j].nVal != 0)
					{
						m_grid[i][j].setConstraint(m_grid[k][j].nVal);
					}
				}

				//box
				for (int k = 0; k < 3; k++)
				{
					for (int l = 0; l < 3; l++)
					{
						if (m_grid[i - (i % 3) + k][j - (j % 3) + l].nVal != 0)
						{
							m_grid[i][j].setConstraint(m_grid[i - (i % 3) + k][j - (j % 3) + l].nVal);
						}
					}
				}
			}
		}
	}
}

//Sets the constraint flag for val of each cell in m_grid[row][col]'s unit.
//If a cell is found to have 8 (or 9, to handle already maxed out cells) constraints on it, the last remaining value is assigned if valid, 
//if not valid then return false because this path is a dead end.
bool SudokuPuzzle::propogateConstraint(int row, int col, int val)
{
	//Set col constraints
	for (int i = 0; i < 9; i++)
	{
		m_grid[i][col].setConstraint(val);
		if (m_grid[i][col].nVal == 0 && m_grid[i][col].nConstraintCount >= 8)
			if (isValidAssignment(i, col, m_grid[i][col].getLastRemainingValue()))
				m_grid[i][col].nVal = m_grid[i][col].getLastRemainingValue();
			else
				return false;
	}

	//set row constraints,m;.
	for (int j = 0; j < 9; j++)
	{
		m_grid[row][j].setConstraint(val);
		if (m_grid[row][j].nVal == 0 && m_grid[row][j].nConstraintCount >= 8)
			if (isValidAssignment(row, j, m_grid[row][j].getLastRemainingValue()))
				m_grid[row][j].nVal = m_grid[row][j].getLastRemainingValue();
			else
				return false;
	}
	//set box member constraints
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
		{
			int nRow = row - (row % 3) + i;
			int nCol = col - (col % 3) + j;

			m_grid[nRow][nCol].setConstraint(val);
			if (m_grid[nRow][nCol].nVal == 0 && m_grid[nRow][nCol].nConstraintCount >= 8)
			if (isValidAssignment(nRow, nCol, m_grid[nRow][nCol].getLastRemainingValue()))
				m_grid[nRow][nCol].nVal = m_grid[nRow][nCol].getLastRemainingValue();
			else
				return false;
		}
	return true;
}

//Copies the unit of the cell specified at (row, col) from the given cell array to the given cell array.
void SudokuPuzzle::copyUnit(int row, int col, Cell from[9][9], Cell to[9][9])
{
	//col
	for (int i = 0; i < 9; i++)
		to[i][col] = from[i][col];

	//row
	for (int j = 0; j < 9; j++)
		to[row][j] = from[row][j];

	//box
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			to[row - (row % 3) + i][col - (col % 3) + j] = from[row - (row % 3) + i][col - (col % 3) + j];

}