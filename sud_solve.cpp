// Sudoku puzzle solver - solves any sudoku puzzle in approx .01 seconds.
// See README.md and results.docx for more information
//
/// Author: Dustin Fast (dustin.fast@outlook.com), 2017

#include <iostream>
#include "ctime"
#include "sud_solve.h"

using namespace std;

const string WELCOME_MSG =
"\nConstraint Satisfaction Demonstration\n"
"-------------------------------------------------------------\n"
"Solves the given Sudoku puzzle via recursive backtracking with constraint propagation.\n";

int main()
{

	string strInput = "";

	while (strInput == "")
	{
		cout << WELCOME_MSG;
		cout << "Enter a Sudoku file ('x' to exit): ";
		cin >> strInput; 
		if (strInput != "x")
		{
			//ensure file exists before we pass it to the constructor
			ifstream fsIn;
			fsIn.open(strInput);
			if (fsIn.fail())
			{
				cout << "Error: File could not be opened.\n";
				strInput = "";
				continue;
			}
			fsIn.close();

			//Init puzzle object and "start" the timer.
			SudokuPuzzle puzzle(strInput);
			clock_t clkBegin = clock(); 

			cout << "\n\nAttempting to find solution for puzzle:";
			puzzle.printPuzzle(); //output puzzle to console
			cout << endl;
			//puzzle.printConstraintCounts(); //debug only

			//Run the solver
			if (puzzle.doSolve() == true)
			{
				cout << "\n\nDONE - Solution Found:";
				puzzle.printPuzzle();	//output puzzle state to console on solver success
			}
			else
			{
				cout << "\n\nDONE - No solution exists for this puzzle.\n" << endl;
			}
			
			clock_t clkEnd = clock();  //"stop" the timer
			cout << endl;
			cout << "Elapsed Time: " << double(clkEnd - clkBegin) / CLOCKS_PER_SEC << endl;
			strInput = "";
			system("pause");
		}
		else
			break;
	}

	return 0;
}