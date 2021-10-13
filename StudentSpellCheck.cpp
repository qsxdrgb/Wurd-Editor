#include "StudentSpellCheck.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;
SpellCheck* createSpellCheck()
{
	return new StudentSpellCheck;
}

StudentSpellCheck::StudentSpellCheck()
{
	root = nullptr;
}

StudentSpellCheck::~StudentSpellCheck() {
	destructTrie(root);
}

bool StudentSpellCheck::load(std::string dictionaryFile) {

	ifstream dict(dictionaryFile);
	if (!dict)
		return false;

	if (root == nullptr)
	{
		root = new trieNode;
		string stringToRead;
		while (getline(dict, stringToRead))
			insertWord(root, stringToRead);
	}
	else
	{
		destructTrie(root);
		root = new trieNode;
		string stringToRead;
		while (getline(dict, stringToRead))
			insertWord(root, stringToRead);
	}
	return true;
}

bool StudentSpellCheck::spellCheck(std::string word, int max_suggestions, std::vector<std::string>& suggestions) {

	if (root == nullptr)
		return false;

	string tempString;
	char indexToChar;

	if (searchWord(root, word))
		return true;
	else
	{
		for (int i = 0; i < word.length(); i++) //Loop through each letter of the word
		{
			tempString = word;
			for (int j = 0; j < 27; j++) //For each char of the word, test all other possibilities
			{
				if (j == 26) //Convert the index to a char
					indexToChar = '\'';
				else
					indexToChar = j + 'a';

				tempString.at(i) = indexToChar;

				if (searchWord(root, tempString))
				{
					suggestions.push_back(tempString);
					if (suggestions.size() > max_suggestions)
						return false;
				}
			}
		}
		return false;
	}		
}

void StudentSpellCheck::spellCheckLine(const std::string& line, std::vector<SpellCheck::Position>& problems) {
	
	string wordToExamine;
	int startOfWord = 0;
	int numChars = 0;
	Position tempPosition;
	
	for (int i = 0; i < line.length(); i++)
	{
		if (!isalpha(line.at(i)) && line.at(i) != '\'') //A space or invalid char has been reached, let's examine the previous word
		{			
			wordToExamine = line.substr(startOfWord, numChars);
			if (!searchWord(root, wordToExamine)) //If we can't find it in our dictionary, push it to problems
			{
				tempPosition.start = startOfWord;
				tempPosition.end = i - 1;
				problems.push_back(tempPosition);		
			}
			startOfWord = i + 1;
			numChars = -1;
		}
		else if (i == line.length() - 1) //We've reached the end of the line, let's examine this word
		{
			wordToExamine = line.substr(startOfWord, numChars + 1);
			if (!searchWord(root, wordToExamine))
			{
				tempPosition.start = startOfWord;
				tempPosition.end = i;
				problems.push_back(tempPosition);
			}
		}
		numChars++;	
	}
}

StudentSpellCheck::trieNode::trieNode()
{
	//Initalize each trie node to have 27 children, 26 letters and 1 apostrophe
	for (int i = 0; i < 27; i++)
	{
		this->children[i] = nullptr;
		isEndOfWord = false;
	}
}


void StudentSpellCheck::insertWord(trieNode* root, std::string word)
{
	trieNode* currentNode = root;
	int index;
	for (int i = 0; i < word.length(); i++)
	{
		if (word.at(i) == '\'') //Convert characters to indexes
			index = 26;
		else
			index = word.at(i) - 'a';

		if (currentNode->children[index] == nullptr) //If the char in our word isn't there, insert it
		{
			currentNode->children[index] = new trieNode;
			
		}
		currentNode = currentNode->children[index];
	}
	currentNode->isEndOfWord = true;
}

bool StudentSpellCheck::searchWord(trieNode* root, std::string word)
{
	if (root == nullptr)
		return false;

	trieNode* currentNode = root;
	int index;
	for (int i = 0; i < word.length(); i++)
	{
		if (word.at(i) == '\'') 
			index = 26;
		else
		{
			word.at(i) = tolower(word.at(i));
			index = word.at(i) - 'a';
		}

		if (currentNode->children[index] == nullptr)
			return false;

		currentNode = currentNode->children[index];
	}
	
	if (currentNode == nullptr)
		return false;
	
	return currentNode->isEndOfWord;
	
}

void StudentSpellCheck::destructTrie(trieNode* node) 
{
	if (node == nullptr)
		return;

	for (int i = 0; i < 27; i++) //Post order destruction
		destructTrie(node->children[i]);

	delete node;
}

