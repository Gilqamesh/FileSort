#include "FileChunk.hpp"
#include <string>
#include <algorithm>

#include <iostream>

FileChunk::FileChunk(int numberOfLines, int lineSizeBytes)
    : _numberOfLines(numberOfLines),
    _lineSizeBytes(lineSizeBytes),
    _sortedWords(_numberOfLines, string(_lineSizeBytes, '\0'))
{
    resize(_numberOfLines * _lineSizeBytes);
}

void FileChunk::sort()
{
    for (int i = 0; i < _numberOfLines; ++i)
    {
        //Word word(_lineSizeBytes);
        for (int j = 0; j < _lineSizeBytes; ++j)
        {
            //word[j] = at(i * _lineSizeBytes + j);
            _sortedWords[i][j] = at(i * _lineSizeBytes + j);
        }
        //_sortedWords[i] = word;
    }
    std::sort(_sortedWords.begin(), _sortedWords.end());
    for (int i = 0; i < _numberOfLines; ++i)
    {
        for (int j = 0; j < _lineSizeBytes; ++j)
        {
            at(i * _lineSizeBytes + j) = _sortedWords[i][j];
        }
    }
}

string FileChunk::getWord(unsigned int index)
{
    return (_sortedWords.at(index));
}
