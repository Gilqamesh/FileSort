#include "FileSort.hpp"
#include <vector>
#include <memory>
#include <algorithm>
#include "Log.hpp"
#include "FileChunk.hpp"

FileSort::FileSort(int maxFileSizeBytes, int numberOfLinesPerSegment, int lineSizeBytes)
    : _maxFileSizeBytes(maxFileSizeBytes),
      _numberOfLinesPerSegment(numberOfLinesPerSegment),
      _lineSizeBytes(lineSizeBytes),
      _fileManager(1000, "HtANm0ECUCjVFVSPRWT7_"),
      _chunkA(_numberOfLinesPerSegment, _lineSizeBytes),
      _chunkB(_numberOfLinesPerSegment, _lineSizeBytes),
      _chunkC(_numberOfLinesPerSegment, _lineSizeBytes)
{
}

void FileSort::Sort(const std::string &inFilePath, const std::string &outFilePath)
{
    _fileManager.closeAll();
    // NOTE(david): file open happens here
    if (inFilePath.size() > MAX_PATH)
        throw Exception("Input file name size (" + to_string(inFilePath.size()) + ") too big, max characters allowed: " + to_string(MAX_PATH));
    if (outFilePath.size() > MAX_PATH)
        throw Exception("Output file name size (" + to_string(outFilePath.size()) + ") too big, max characters allowed: " + to_string(MAX_PATH));
    HANDLE inFileHandle = _fileManager.open(inFilePath, READ, true);
    HANDLE outFileHandle = _fileManager.create(outFilePath, RDWR, true);

    // NOTE(david): file size check happens here
    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(inFileHandle, &fileSize))
        throw Exception("Failed to check size of the file: " + inFilePath);
    if (fileSize.QuadPart > _maxFileSizeBytes)
        throw Exception("File: " + inFilePath + " exceeds the maximum(" + to_string(_maxFileSizeBytes) + "): " + to_string(fileSize.QuadPart));

    // NOTE(david): file chunks creation happens here
    DWORD chunkSize = _numberOfLinesPerSegment * _lineSizeBytes;
    int totalBytesRead = 0;
    int numberOfChunks = (int)fileSize.QuadPart / chunkSize;
    for (int currentIteration = 0;
         currentIteration < numberOfChunks;
         ++currentIteration)
    {
        _fileManager.read(inFileHandle, _chunkA.data(), _chunkA.size());

        _chunkA.sort();
        FileHandle tmpFileHandle = _fileManager.createTmp();

        _fileManager.write(tmpFileHandle, _chunkA.data(), _chunkA.size());
        _fileManager.write(outFileHandle, _chunkA.data(), _chunkA.size());
        _fileManager.seek(tmpFileHandle, 0);
    }

    // NOTE(david): sorting into outfile happens here
    // NOTE(david): Merge sort
    mergeSort(numberOfChunks, outFileHandle);
    sw.print();
}

FileSort::Exception::Exception(const string &msg)
    : runtime_error(msg)
{
}

FileSort::Exception::~Exception() throw()
{
}

void FileSort::mergeSort(int numberOfChunks, FileHandle outFileHandle)
{
    mergeSort(0, numberOfChunks, outFileHandle);
}

void FileSort::mergeSort(int start, int end, FileHandle outFileHandle)
{
    if (end - start < 2)
        return;

    int mid = (start + end) / 2;
    mergeSort(start, mid, outFileHandle);
    mergeSort(mid, end, outFileHandle);
    mergeComb(start, mid, end, outFileHandle);
    mergeCopy(start, end, outFileHandle);
}

// TODO(david): replace seek with close perhaps?
void FileSort::mergeComb(int start, int mid, int end, FileHandle outFileHandle)
{
    int leftChunkIndex = start;
    int rightChunkIndex = mid;
    int leftWordIndex = 0;
    int rightWordIndex = 0;
    FileHandle leftFileHandle = NULL;
    FileHandle rightFileHandle = NULL;
    string leftWord;
    string rightWord;
    _fileManager.seek(outFileHandle, start * _lineSizeBytes);

    sw.set();
    int numberOfChunks = end - start;
    size_t numberOfWords = numberOfChunks * _numberOfLinesPerSegment;
    for (size_t curWordIndex = 0; curWordIndex < numberOfWords; ++curWordIndex)
    {
        if (leftChunkIndex < mid && leftWordIndex == 0)
        {
            leftFileHandle = _fileManager.openTmp(leftChunkIndex);
            _fileManager.read(leftFileHandle, _chunkA.data(), _chunkA.size());
            _chunkA.sort();
            _fileManager.seek(leftFileHandle, 0);
            leftWord = _chunkA.getWord(leftWordIndex++);
        }
        if (rightChunkIndex < end && rightWordIndex == 0)
        {
            rightFileHandle = _fileManager.openTmp(rightChunkIndex);
            _fileManager.read(rightFileHandle, _chunkB.data(), _chunkB.size());
            _chunkB.sort();
            _fileManager.seek(rightFileHandle, 0);
            rightWord = _chunkB.getWord(rightWordIndex++);
        }
        
        if (leftChunkIndex < mid && (rightChunkIndex >= end || leftWord < rightWord))
        {
            _chunkC.addWord(leftWord);
            //_fileManager.write(outFileHandle, (void *)leftWord.data(), leftWord.size());
            if (leftWordIndex == _numberOfLinesPerSegment)
            {
                ++leftChunkIndex;
                leftWordIndex = 0;
            }
            else
            {
                leftWord = _chunkA.getWord(leftWordIndex++);
            }
        }
        else
        {
            _chunkC.addWord(rightWord);
            //_fileManager.write(outFileHandle, (void *)rightWord.data(), rightWord.size());
            if (rightWordIndex == _numberOfLinesPerSegment)
            {
                ++rightChunkIndex;
                rightWordIndex = 0;
            }
            else
            {
                rightWord = _chunkB.getWord(rightWordIndex++);
            }
        }
        if (_chunkC.sizeWords() == _numberOfLinesPerSegment)
        {
            _fileManager.write(outFileHandle, _chunkC.data(), _chunkC.size());
            _chunkC.reset();
        }
    }
    sw.set();
    sw.reset();
}

void FileSort::mergeCopy(int start, int end, FileHandle outFileHandle)
{
    _fileManager.seek(outFileHandle, start * _lineSizeBytes);
    for (int i = start; i < end; ++i)
    {
        FileHandle chunkHandle = _fileManager.openTmp(i);
        _fileManager.read(outFileHandle, _chunkA.data(), _chunkA.size());
        _fileManager.write(chunkHandle, _chunkA.data(), _chunkA.size());
        _fileManager.seek(chunkHandle, 0);
    }
}
