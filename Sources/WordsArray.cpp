#include "WordsArray.hpp"
#include <string>
#include <algorithm>
#include <set>

#include "Log.hpp"

WordsArray::WordsArray(int numberOfWords, int wordSize)
    : _numberOfWords(numberOfWords),
      _wordSize(wordSize),
      _data(_numberOfWords * _wordSize)
{
}

// TODO(david): sort _data in-place
void WordsArray::sort()
{
    vector<vector<char>> sortedWords(_numberOfWords, vector<char>(_wordSize));
    for (int wordIndex = 0; wordIndex < _numberOfWords; ++wordIndex)
    {
        for (int charIndex = 0; charIndex < _wordSize; ++charIndex)
        {
            sortedWords[wordIndex][charIndex] = _data[wordIndex * _wordSize + charIndex];
        }
    }
    std::sort(sortedWords.begin(), sortedWords.end());
    for (int wordIndex = 0; wordIndex < _numberOfWords; ++wordIndex)
    {
        for (int charIndex = 0; charIndex < _wordSize; ++charIndex)
        {
            _data[wordIndex * _wordSize + charIndex] = sortedWords[wordIndex][charIndex];
        }
    }
}

Word WordsArray::getWord(unsigned int index)
{
    auto wordBegin = _data.begin() + index * _wordSize;
    return (Word(_wordSize, wordBegin));
}

void WordsArray::addWord(Word word, unsigned int index)
{
    for (int charIndex = 0; charIndex < word.size(); ++charIndex)
        _data[(index * _wordSize) + charIndex] = word[charIndex];
}

int WordsArray::sizeWords()
{
    return (_numberOfWords);
}

void *WordsArray::data()
{
    return (_data.data());
}

int WordsArray::size()
{
    return ((int)_data.size());
}
