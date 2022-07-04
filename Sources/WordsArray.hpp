#ifndef FILECHUNK_HPP
#define FILECHUNK_HPP

#include <vector>
#include <string>
#include "Word.hpp"

using namespace std;

class WordsArray
{
    int _numberOfWords;
    int _wordSize;
    vector<char> _data;
public:
    WordsArray(int numberOfWords, int wordSize);

    void sort();
    Word getWord(unsigned int index);
    void addWord(Word word, unsigned int index);
    int sizeWords();
    void *data();
    int size();
};

#endif
