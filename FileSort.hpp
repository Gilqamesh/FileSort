#ifndef FILESORT_H
#define FILESORT_H

#include <string>
#include <exception>
#include "FileManager.hpp"
#include "WordsArray.hpp"
#include <memory>

#include "stopwatch.hpp"

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

    stopwatch sw;

public:
    FileSort(int maxFileSizeBytes, int numberOfLinesPerSegment, int lineSizeBytes);
    void Sort(const std::string &inFilePath, const std::string &outFilePath);
    // TODO(david): support sort of multiple files into a single file
    // this function signature doesnt work as it creates ambiguity with the above one
    // void Sort(const string& outFilePath, const string& inFilePathArgs ...);

private:
    class Exception : public runtime_error
    {
    public:
        Exception(const string &msg);
        ~Exception() noexcept;
    };

    // File sorting algorithm
    void mergeSort(int numberOfChunks, FileHandle outFileHandle, shared_ptr<FileManager> fileManager);
    void mergeSortH(int start, int end, FileHandle outFileHandle, shared_ptr<FileManager> fileManager);
    void mergeComb(int start, int mid, int end, FileHandle outFileHandle, shared_ptr<FileManager> fileManager);
    void mergeCopy(int start, int end, FileHandle outFileHandle, shared_ptr<FileManager> fileManager);
};

#endif
