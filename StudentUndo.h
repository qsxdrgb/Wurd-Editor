#ifndef STUDENTUNDO_H_
#define STUDENTUNDO_H_

#include "Undo.h"
#include <stack>
#include <string>

class StudentUndo : public Undo {
public:

	void submit(Action action, int row, int col, char ch = 0);
	Action get(int& row, int& col, int& count, std::string& text);
	void clear();

private:
	struct undoInfo
	{
		undoInfo(Action op, int row, int col, char ch);

		Action operation;
		int rowNum;
		int colNum;
		char charTracker;
		std::string batchedText;
	};
	
	std::stack<undoInfo> undoInfoStack;
};

#endif // STUDENTUNDO_H_
