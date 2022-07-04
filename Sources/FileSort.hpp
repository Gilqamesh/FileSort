#ifndef FILESORT_H
# define FILESORT_H

# include <string>
# include <exception>
# include <vector>
# include "FileManager.hpp"

using namespace std;

# define Kilobytes(Value) ((Value) * 1024LL)
# define Megabytes(Value) (Kilobytes(Value) * 1024LL)
# define Gigabytes(Value) (Megabytes(Value) * 1024LL)
# define Terabytes(Value) (Gigabytes(Value) * 1024LL)

# define MAX_TMP_FILE_SIZE Megabytes(20)

class FileSort
{
    const int _maxFileSizeBytes;
    const int _numberOfLinesPerSegment;
    const int _lineSizeBytes;
    const int _chunkSize;

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

    int sanitizeInFile(FileHandle inFileHandle, const string &inFilePath);
    void initializeChunks(FileHandle inFileHandle, FileHandle outFileHandle, FileManager &fileManager, int numberOfChunks);

    // sorting algorithm
    void sort(int numberOfChunks, FileHandle outFileHandle, FileManager &fileManager);
    void mergeSort(int start, int end, FileHandle outFileHandle, FileManager &fileManager, bool outToTmp);
    bool mergeIsSorted(int mid, FileHandle outFileHandle, FileManager &fileManager, bool outToTmp);
    void merge(int start, int mid, int end, FileHandle outFileHandle, FileManager &fileManager, bool outToTmp);
};

#endif
