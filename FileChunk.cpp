#include "FileChunk.hpp"
#include <string>
#include <algorithm>

FileChunk::FileChunk(int numberOfLines, int lineSizeBytes)
    : _numberOfLines(numberOfLines), _lineSizeBytes(lineSizeBytes), _sortedWords(_numberOfLines, Word(_lineSizeBytes))
{
    resize(_numberOfLines * _lineSizeBytes);
}

void FileChunk::sort()
{
    for (int i = 0; i < _numberOfLines; ++i)
    {
        for (int j = 0; j < _lineSizeBytes; ++j)
        {
            _sortedWords[i][j] = at(i * _lineSizeBytes + j);
        }
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

Word FileChunk::getWord(unsigned int index)
{
    return (_sortedWords.at(index));
}
