#include "StudentUndo.h"
#include <iostream>

using namespace std;

Undo* createUndo()
{
	return new StudentUndo;
}

void StudentUndo::submit(const Action action, int row, int col, char ch) {
	
	if (undoInfoStack.empty() && action == DELETE) //If a delete is called with an empty stack, just push it on
	{
		undoInfo toPush(action, row, col, ch);
		toPush.batchedText.push_back(ch);
		undoInfoStack.push(toPush);
		return;
	}

	if (!undoInfoStack.empty() && undoInfoStack.top().operation == DELETE && action == DELETE && row == undoInfoStack.top().rowNum) //This considers batching of dels/backs
	{
		if (col == undoInfoStack.top().colNum) //Same column means delete, insert at back
		{
			undoInfoStack.top().batchedText.push_back(ch);
		}
		else if (col == undoInfoStack.top().colNum - 1 ) //Back by 1 column means backspace, insert at front
		{
			undoInfoStack.top().batchedText.insert(0, 1, ch);
			undoInfoStack.top().colNum--;
		}
		else //Not batchable
		{
			undoInfo toPush(action, row, col, ch);
			toPush.batchedText.push_back(ch);
			undoInfoStack.push(toPush);
		}
		return;
	}
	else //Non batchable stuff (except for insert, that batching is done in get)
	{
		undoInfo toPush(action, row, col, ch);
		toPush.batchedText.push_back(ch);
		undoInfoStack.push(toPush);
	}
	
}

StudentUndo::Action StudentUndo::get(int &row, int &col, int& count, std::string& text) {
    

	if (undoInfoStack.empty())
		return Undo::Action::ERROR;

	text = "";
	count = 1;
	row = undoInfoStack.top().rowNum;
	col = undoInfoStack.top().colNum;
	switch (undoInfoStack.top().operation)
	{
	case INSERT:
		col--;
		undoInfoStack.pop();	
		//Batch insertions by comparing columns
		while (!undoInfoStack.empty() && undoInfoStack.top().operation == INSERT && col == undoInfoStack.top().colNum && row == undoInfoStack.top().rowNum)
		{
			count++;
			col--;
			undoInfoStack.pop();
		}
		return Undo::Action::DELETE;
	case DELETE:
		text.append(undoInfoStack.top().batchedText);
		undoInfoStack.pop();
		return Undo::Action::INSERT;
	case SPLIT:
		undoInfoStack.pop();
		return Undo::Action::JOIN;
	case JOIN:
		undoInfoStack.pop();
		return Undo::Action::SPLIT;
	}
	return Undo::Action::ERROR;
}

void StudentUndo::clear() {
	while (!undoInfoStack.empty())
		undoInfoStack.pop();
}

StudentUndo::undoInfo::undoInfo(Action op, int row, int col, char ch)
{
	operation = op;
	rowNum = row;
	colNum = col;
	charTracker = ch;
}
