#include "FileChunk.hpp"
#include <string>
#include <algorithm>

#include <iostream>

FileChunk::FileChunk(int numberOfWords, int wordSize)
    : _numberOfWords(numberOfWords),
      _wordSize(wordSize),
      _wordIndex(0),
      _data(_numberOfWords * _wordSize),
      _sortedWords(_numberOfWords, string(_wordSize, '\0'))
{
    for (int i = 0; i < _numberOfWords; ++i)
        _sortedIndices.push_back({i, i * _wordSize});
}

void FileChunk::sort()
{
    std::sort(_sortedIndices.begin(), _sortedIndices.end(), [&](const pair<int, int>& l, const pair<int, int>& r){
        auto wordLBegin = _data.begin() + l.second;
        auto wordRBegin = _data.begin() + r.second;
        string a(wordLBegin, wordLBegin + _wordSize);
        string b(wordRBegin, wordRBegin + _wordSize);
        return (a < b);
    });
    /*
    for (int i = 0; i < _numberOfWords; ++i)
    {
        for (int j = 0; j < _wordSize; ++j)
        {
            _sortedWords.at(i).at(j) = _data.at(i * _wordSize + j);
        }
    }
    std::sort(_sortedWords.begin(), _sortedWords.end());
    for (int i = 0; i < _numberOfWords; ++i)
    {
        for (int j = 0; j < _wordSize; ++j)
        {
            _data.at(i * _wordSize + j) = _sortedWords.at(i).at(j);
        }
    }
    */
}

string FileChunk::getWord(unsigned int index)
{
    auto wordBegin = _data.begin() + _sortedIndices.at(index).second;
    return (string(wordBegin, wordBegin + _wordSize));
    // return (_sortedWords.at(index));
}

void FileChunk::addWord(const string& word)
{
    for (int i = 0; i < word.size(); ++i)
    {
        _data.at((_wordIndex * _wordSize) + i) = word[i];
    }
    ++_wordIndex;
    //_sortedWords.at(_wordIndex++) = word;
}

size_t FileChunk::sizeWords()
{
    return (_wordIndex);
}

void FileChunk::reset()
{
    _wordIndex = 0;
}

void *FileChunk::data()
{
    return (_data.data());
}

size_t FileChunk::size()
{
    return (_data.size());
}
