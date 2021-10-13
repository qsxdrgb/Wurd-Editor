#include "StudentTextEditor.h"
#include "Undo.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;

TextEditor* createTextEditor(Undo* un)
{
	return new StudentTextEditor(un);
}

StudentTextEditor::StudentTextEditor(Undo* undo)
 : TextEditor(undo) {

	rowPos = 0;
	colPos = 0;
	itTracker = listOfWords.begin();
}

StudentTextEditor::~StudentTextEditor()
{
	//Nothing dynamically allocated
}

bool StudentTextEditor::load(std::string file) { //TODO: Strip carriage returns

	reset();
	ifstream infile(file);
	if (!infile)
		return false;

	string stringToRead;
	while (getline(infile, stringToRead))
	{
		if (!stringToRead.empty() && stringToRead.back() == '\r') //Strip carriage returns
			stringToRead.pop_back();
		listOfWords.push_back(stringToRead);
	}
	itTracker = listOfWords.begin();
	return true;
}

bool StudentTextEditor::save(std::string file) {
	
	ofstream outfile(file);
	if (!outfile)
		return false;

	std::list<std::string>::iterator it = listOfWords.begin();
	while (it != listOfWords.end())
	{
		outfile << *it << endl;
		it++;
	}
	return true;

}

void StudentTextEditor::reset() {

	listOfWords.clear();
	rowPos = 0;
	colPos = 0;

}

void StudentTextEditor::move(Dir dir) {

	switch (dir)
	{
	case UP:
		if (rowPos == 0) //Break if cursor is on top line
			break;
		rowPos--;
		itTracker--; //Iterator tracks which line we're on
		if (colPos > (*itTracker).size()) //If moving up causes the cursor to be too far right, reposition it to the end of the line
			colPos = (*itTracker).size();
		break;
	case DOWN:
		if (next(itTracker) == listOfWords.end()) //Break if cursor is on bottom line
			break;
		rowPos++;
		itTracker++;
		if (colPos > (*itTracker).size()) //If moving down causes the cursor to be too far right, reposition it to the end of the line
			colPos = (*itTracker).size();
		break;
	case LEFT:
		if (itTracker != listOfWords.begin() && colPos == 0) //If this isn't the first row and the cursor is already all the way left, move to the prev row
		{                                                    //and send cursor all the way to the right
			rowPos--;
			itTracker--;
			colPos = (*itTracker).size();
			break;
		}
		if (colPos != 0) //Otherwise move left unless the cursor is already all the way to the left
			colPos--;
		break;
	case RIGHT:
		if (next(itTracker) != listOfWords.end() && colPos == (*itTracker).size()) //If this isn't the last row and the cursor is already all the way right, 
		{                                                                         //move to the next row and send cursor all the way to the left
			rowPos++;
			itTracker++;
			colPos = 0;
			break;
		}
		if (colPos >= (*itTracker).size()) //Break if cursor is all the way to the right
			break;
		colPos++;
		break;
	case HOME:
		colPos = 0; //Send cursor all the way to the left
		break;
	case END:
		colPos = (*itTracker).size(); //Send cursor all the way to the right
		break;
	}
}

void StudentTextEditor::del() { 

	if (colPos >= (*itTracker).size() && next(itTracker) != listOfWords.end()) //If cursor's all the way to the right and not at the last line
	{		
		(*itTracker).append(*next(itTracker)); //Append the next line
		itTracker = listOfWords.erase(next(itTracker)); //Erase next line
		itTracker--;
		getUndo()->submit(Undo::Action::JOIN, rowPos, colPos);
		return;
	}

	char deletedChar = (*itTracker).at(colPos); //Store char to be deleted
	(*itTracker).erase(colPos, 1);
	getUndo()->submit(Undo::Action::DELETE, rowPos, colPos, deletedChar);
}

void StudentTextEditor::backspace() { 

	if (colPos == 0 && rowPos != 0) //If cursor's all the way to the left and not at the first line
	{
		colPos = (*prev(itTracker)).size(); //Adjust cursor
		rowPos--;
		(*prev(itTracker)).append(*itTracker);
		itTracker = listOfWords.erase(itTracker);
		itTracker--;
		getUndo()->submit(Undo::Action::JOIN, rowPos, colPos);
		return;
	}

	if (colPos == 0 && rowPos == 0) //If cursor's top left, do nothing
		return;

	colPos--;
	char deletedChar = (*itTracker).at(colPos); //Store char to be deleted
	(*itTracker).erase(colPos, 1);

	getUndo()->submit(Undo::Action::DELETE, rowPos, colPos, deletedChar);
}

void StudentTextEditor::insert(char ch) { 

	if (ch == '\t') //If the char is a tab, convert it to four spaces
	{
		(*itTracker).insert(colPos, 4, ' ');
		colPos += 4;
	}
	
	else
	{
		(*itTracker).insert(colPos, 1, ch); //Insert char one time at the cursor
		colPos++;
	}

	getUndo()->submit(Undo::Action::INSERT, rowPos, colPos, ch);
}

void StudentTextEditor::enter() { 
	
	itTracker = listOfWords.insert(next(itTracker), (*itTracker).substr(colPos)); //Insert a word on the next line starting from the cursor 
	(*prev(itTracker)).erase(colPos); //Get rid of the inserted chars on the original line
	getUndo()->submit(Undo::Action::SPLIT, rowPos, colPos);
	rowPos++; //Adjust cursor
	colPos = 0; //Adjust cursor
}

void StudentTextEditor::getPos(int& row, int& col) const {

	row = rowPos;
	col = colPos;
}

int StudentTextEditor::getLines(int startRow, int numRows, std::vector<std::string>& lines) const {

	if (startRow < 0 || numRows < 0 || startRow > listOfWords.size())
		return -1;

	std::list<std::string>::const_iterator it = itTracker;
	
	int difference = rowPos - startRow;

	//Move iterator to startRow depending on where it currently is in relation to startRow
	while (difference > 0 && rowPos > startRow && it != listOfWords.begin())
	{
		it--;
		difference--;
	}

	while (difference < 0 && rowPos < startRow && it != listOfWords.end())
	{
		it++;
		difference++;
	}
	
	lines.clear();

	for (int rows = 0; rows < numRows && it != listOfWords.end(); rows++)
	{
		lines.push_back(*it);
		it++;
	}

	return lines.size();

}

void StudentTextEditor::undo() {

	int tempRow;
	Undo::Action tempAction = getUndo()->get(tempRow, colPos, count, text);

	if (tempAction == Undo::Action::ERROR)
		return;

	//Readjust row position and iterator, since the cursor might be rows away from the row that needs undoing

	while (tempRow > rowPos && next(itTracker) != listOfWords.end()) 
	{
		rowPos++;
		itTracker++;
	}
	
	while (tempRow < rowPos && itTracker != listOfWords.begin())
	{
		rowPos--;
		itTracker--;
	}

	switch (tempAction)
	{	
	case Undo::Action::INSERT:
		(*itTracker).insert(colPos, text);
		break;
	case Undo::Action::DELETE:
		(*itTracker).erase(colPos, count);
		break;
	case Undo::Action::JOIN:
		(*itTracker).append(*next(itTracker)); //Append the next line
		itTracker = listOfWords.erase(next(itTracker)); //Erase next line
		itTracker--;
		break;
	case Undo::Action::SPLIT:
		itTracker = listOfWords.insert(next(itTracker), (*itTracker).substr(colPos)); //Insert a word on the next line starting from the cursor 
		(*prev(itTracker)).erase(colPos); //Get rid of the inserted chars on the original line
		rowPos++; //Adjust cursor
		colPos = 0; //Adjust cursor
		break;
	}

}
