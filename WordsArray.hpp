#ifndef FILECHUNK_HPP
#define FILECHUNK_HPP

#include <vector>
#include <string>
#include "Word.hpp"

using namespace std;

// NOTE(david): UNUSED CURRENTLY
// TODO(david): _data in wordsarray need to have an object which is comparable when using sort algo
class WordsArrayIterator
{
    char *_data;
    unsigned int _pitch;
public:
    WordsArrayIterator(char *data, unsigned int pitch);

    WordsArrayIterator &operator++();
    WordsArrayIterator operator++(int);
    char *operator->() const;
    char operator*() const;
    char operator[](unsigned int index) const;
};

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

    // iterator
    WordsArrayIterator begin();
    WordsArrayIterator end();
};

#endif
