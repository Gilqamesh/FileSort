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
        for (int i = 0; i < _wordSize; ++i)
        {
            sortedWords[wordIndex][i] = _data[wordIndex * _wordSize + i];
        }
    }
    std::sort(sortedWords.begin(), sortedWords.end());
    for (int wordIndex = 0; wordIndex < _numberOfWords; ++wordIndex)
    {
        for (int i = 0; i < _wordSize; ++i)
        {
            _data[wordIndex * _wordSize + i] = sortedWords[wordIndex][i];
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
    for (int i = 0; i < word.size(); ++i)
        _data[(index * _wordSize) + i] = word[i];
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

WordsArrayIterator WordsArray::begin()
{
    return (WordsArrayIterator(&_data.front(), _wordSize));
}

WordsArrayIterator WordsArray::end()
{
    return (WordsArrayIterator(&_data.back() + 1, _wordSize));
}

WordsArrayIterator::WordsArrayIterator(char *data, unsigned int pitch)
    : _data(data),
      _pitch(pitch)
{
}

WordsArrayIterator &WordsArrayIterator::operator++()
{
    _data += _pitch;
    return (*this);
}

WordsArrayIterator WordsArrayIterator::operator++(int)
{
    WordsArrayIterator tmp(*this);
    ++(*this);
    return (tmp);
}

char *WordsArrayIterator::operator->() const
{
    return (_data);
}

char WordsArrayIterator::operator*() const
{
    return (*_data);
}

char WordsArrayIterator::operator[](unsigned int index) const
{
    return (_data[index]);
}
