#ifndef STUDENTSPELLCHECK_H_
#define STUDENTSPELLCHECK_H_

#include "SpellCheck.h"

#include <string>
#include <vector>

class StudentSpellCheck : public SpellCheck {
public:
	StudentSpellCheck();
	virtual ~StudentSpellCheck();
	bool load(std::string dict_file);
	bool spellCheck(std::string word, int maxSuggestions, std::vector<std::string>& suggestions);
	void spellCheckLine(const std::string& line, std::vector<Position>& problems);

private:
	struct trieNode
	{
		trieNode* children[27];
		bool isEndOfWord;
		trieNode();
	};
	trieNode* root;
	void insertWord(trieNode* root, std::string word);
	bool searchWord(trieNode* root, std::string word);
	void destructTrie(trieNode* node);

};

#endif  // STUDENTSPELLCHECK_H_
