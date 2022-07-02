#ifndef FILESORT_H
#define FILESORT_H

#include <string>
#include <exception>
#include "FileManager.hpp"
#include "FileChunk.hpp"

#include "stopwatch.hpp"

#if defined(_WIN64)
#include "Windows.h"
#elif defined(__linux__)
#include <unistd.h>
#endif

using namespace std;

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value) * 1024LL)
#define Gigabytes(Value) (Megabytes(Value) * 1024LL)
#define Terabytes(Value) (Gigabytes(Value) * 1024LL)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

class FileSort
{
    const int _maxFileSizeBytes;
    const int _numberOfLinesPerSegment;
    const int _lineSizeBytes;
    FileManager _fileManager;
    FileChunk _chunkA;
    FileChunk _chunkB;
    stopwatch sw;

public:
    FileSort(int maxFileSizeBytes, int numberOfLinesPerSegment, int lineSizeBytes);
    void Sort(const std::string &inFilePath, const std::string &outFilePath);

private:
    class Exception : public runtime_error
    {
    public:
        Exception(const string &msg);
        ~Exception() throw();
    };

    // TODO(david): seperation of concerns
    Word wordRead(FileHandle fileHandle, unsigned int wordIndex);
    void wordWrite(FileHandle fileHandle, Word &word);

    // File sorting algorithm
    void mergeSort(int numberOfChunks, FileHandle outFileHandle);
    void mergeSort(int start, int end, FileHandle outFileHandle);
    void mergeComb(int start, int mid, int end, FileHandle outFileHandle);
    void mergeCopy(int start, int end, FileHandle outFileHandle);
};

#endif
