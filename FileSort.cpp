#include "FileSort.hpp"
#include <vector>
#include <algorithm>
#include "Log.hpp"
#include "WordsArray.hpp"

FileSort::FileSort(int maxFileSizeBytes, int numberOfLinesPerSegment, int lineSizeBytes)
    : _maxFileSizeBytes(maxFileSizeBytes),
      _numberOfLinesPerSegment(numberOfLinesPerSegment),
      _lineSizeBytes(lineSizeBytes)
{
    srand((unsigned int)time(NULL));
}

void FileSort::Sort(const std::string &inFilePath, const std::string &outFilePath)
{
    shared_ptr<FileManager> fileManager = make_shared<FileManager>(MAX_FD * 3, to_string(rand()));
    FileHandle inFileHandle = fileManager->open(inFilePath, READ, true, false);
    FileHandle outFileHandle = fileManager->open(outFilePath, RDWR, false, false);

#if defined(WINDOWS)
    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(inFileHandle, &fileSize))
        throw Exception("Failed to check size of file: " + inFilePath);
    if (fileSize.QuadPart > _maxFileSizeBytes)
        throw Exception("File: " + inFilePath + " exceeds maximum(" + to_string(_maxFileSizeBytes) + "): " + to_string(fileSize.QuadPart));
#elif defined(LINUX)
    struct stat fileInfo;
    if (stat(inFilePath.c_str(), &fileInfo) == -1)
        throw Exception("Failed to check size of file: " + inFilePath);
    if (fileInfo.st_size > _maxFileSizeBytes)
        throw Exception("File: " + inFilePath + " exceeds maximum(" + to_string(_maxFileSizeBytes) + "): " + to_string(fileInfo.st_size));
#endif

    uint32 chunkSize = _numberOfLinesPerSegment * _lineSizeBytes;
#if defined(WINDOWS)
    int numberOfChunks = (int)(fileSize.QuadPart / chunkSize);
#elif defined(LINUX)
    int numberOfChunks = (int)(fileInfo.st_size / chunkSize);
#endif
    LOG("Number of chunks: " << numberOfChunks);

    unique_ptr<WordsArray> chunk = make_unique<WordsArray>(_numberOfLinesPerSegment, _lineSizeBytes);
    if (numberOfChunks == 1)
    {
        fileManager->read(inFileHandle, chunk->data(), chunk->size());
        chunk->sort();

        for (int i = 0; i < chunk->sizeWords(); ++i)
        {
            Word word = chunk->getWord(i);
            fileManager->write(outFileHandle, word.data(), word.size());
        }
        return;
    }
    for (int currentIteration = 0;
         currentIteration < numberOfChunks;
         ++currentIteration)
    {
        fileManager->read(inFileHandle, chunk->data(), chunk->size());
        chunk->sort();

        FileHandle tmpFileHandle = fileManager->createTmp();
        fileManager->write(tmpFileHandle, chunk->data(), chunk->size());
        fileManager->write(outFileHandle, chunk->data(), chunk->size());
        fileManager->seek(tmpFileHandle, 0);
    }
    chunk.release();

    sw.set();
    mergeSort(numberOfChunks, outFileHandle, fileManager);
    sw.set();
    sw.print();
}

void FileSort::mergeSort(int numberOfChunks, FileHandle outFileHandle, shared_ptr<FileManager> fileManager)
{
    mergeSortH(0, numberOfChunks, outFileHandle, fileManager);
}

// TODO(david): remove mergeCopy and do copy back and forth between outFile and tmp buffer (tmp files)
void FileSort::mergeSortH(int start, int end, FileHandle outFileHandle, shared_ptr<FileManager> fileManager)
{
    if (end - start < 2)
        return;

    int mid = (start + end) / 2;
    mergeSortH(start, mid, outFileHandle, fileManager);
    mergeSortH(mid, end, outFileHandle, fileManager);
    mergeComb(start, mid, end, outFileHandle, fileManager);
    mergeCopy(start, end, outFileHandle, fileManager);
}

void FileSort::mergeComb(int start, int mid, int end, FileHandle outFileHandle, shared_ptr<FileManager> fileManager)
{
    int leftChunkIndex = start;
    int rightChunkIndex = mid;
    int leftWordIndex = 0;
    int rightWordIndex = 0;
    FileHandle leftFileHandle = 0;
    FileHandle rightFileHandle = 0;
    WordsArray leftChunk(_numberOfLinesPerSegment, _lineSizeBytes);
    WordsArray rightChunk(_numberOfLinesPerSegment, _lineSizeBytes);
    WordsArray outChunk(_numberOfLinesPerSegment, _lineSizeBytes);
    Word leftWord;
    Word rightWord;
    fileManager->seek(outFileHandle, start * _lineSizeBytes);

    int numberOfChunks = end - start;
    int numberOfWords = numberOfChunks * _numberOfLinesPerSegment;
    int outChunkWordIndex = 0;
    for (int curWordIndex = 0;
         curWordIndex < numberOfWords;
         ++curWordIndex)
    {
        if (leftChunkIndex < mid && leftWordIndex == 0)
        {
            leftFileHandle = fileManager->openTmp(leftChunkIndex);
            fileManager->read(leftFileHandle, leftChunk.data(), leftChunk.size());
            fileManager->seek(leftFileHandle, 0);
            leftWord = leftChunk.getWord(leftWordIndex++);
        }
        if (rightChunkIndex < end && rightWordIndex == 0)
        {
            rightFileHandle = fileManager->openTmp(rightChunkIndex);
            fileManager->read(rightFileHandle, rightChunk.data(), rightChunk.size());
            fileManager->seek(rightFileHandle, 0);
            rightWord = rightChunk.getWord(rightWordIndex++);
        }

        if (leftChunkIndex < mid && (rightChunkIndex >= end || leftWord < rightWord))
        {
            outChunk.addWord(leftWord, outChunkWordIndex++);
            if (leftWordIndex == _numberOfLinesPerSegment)
            {
                ++leftChunkIndex;
                leftWordIndex = 0;
            }
            else
            {
                leftWord = leftChunk.getWord(leftWordIndex++);
            }
        }
        else
        {
            outChunk.addWord(rightWord, outChunkWordIndex++);
            if (rightWordIndex == _numberOfLinesPerSegment)
            {
                ++rightChunkIndex;
                rightWordIndex = 0;
            }
            else
            {
                rightWord = rightChunk.getWord(rightWordIndex++);
            }
        }

        if (outChunk.sizeWords() == outChunkWordIndex)
        {
            outChunkWordIndex = 0;
            fileManager->write(outFileHandle, outChunk.data(), outChunk.size());
        }
    }
}

void FileSort::mergeCopy(int start, int end, FileHandle outFileHandle, shared_ptr<FileManager> fileManager)
{
    WordsArray chunk(_numberOfLinesPerSegment, _lineSizeBytes);
    fileManager->seek(outFileHandle, start * _lineSizeBytes);
    for (int i = start; i < end; ++i)
    {
        fileManager->read(outFileHandle, chunk.data(), chunk.size());
        FileHandle chunkHandle = fileManager->openTmp(i);
        fileManager->write(chunkHandle, chunk.data(), chunk.size());
        fileManager->seek(chunkHandle, 0);
    }
}

FileSort::Exception::Exception(const string &msg)
    : runtime_error(msg)
{
}

FileSort::Exception::~Exception() noexcept
{
}
