#ifndef FILESORT_H
# define FILESORT_H

# include <string>
# include <exception>
# include "FileManager.hpp"
# include "WordsArray.hpp"
# include <memory>

# include "stopwatch.hpp"

using namespace std;

# define Kilobytes(Value) ((Value)*1024LL)
# define Megabytes(Value) (Kilobytes(Value) * 1024LL)
# define Gigabytes(Value) (Megabytes(Value) * 1024LL)
# define Terabytes(Value) (Gigabytes(Value) * 1024LL)

# define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

# define MAX_TMP_FILE_SIZE Kilobytes(1)

class FileSort
{
    const int _maxFileSizeBytes;
    const int _numberOfLinesPerSegment;
    const int _lineSizeBytes;
    const int _chunkSize;

    stopwatch sw;

public:
    FileSort(int maxFileSizeBytes, int numberOfLinesPerSegment, int lineSizeBytes);
    void Sort(const std::string &inFilePath, const std::string &outFilePath);
    void Sort(const vector<string>& inFilePathVec, const string& outFilePath);

private:
    class Exception : public runtime_error
    {
    public:
        Exception(const string &msg);
        ~Exception() noexcept;
    };

    // File sorting algorithm
    void sort(int numberOfChunks, FileHandle outFileHandle, FileManager &fileManager);
    void mergeSort(int start, int end, FileHandle outFileHandle, FileManager &fileManager, bool outToTmp, mutex& outFileMutex);
    bool mergeIsSorted(int mid, FileHandle outFileHandle, FileManager &fileManager, bool outToTmp, mutex& outFileMutex);
    void merge(int start, int mid, int end, FileHandle outFileHandle, FileManager &fileManager, bool outToTmp, mutex& outFileMutex);
};

#endif
