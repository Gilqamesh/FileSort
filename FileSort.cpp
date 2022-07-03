#include "FileSort.hpp"
#include <vector>
#include <algorithm>
#include "Log.hpp"
#include "WordsArray.hpp"

#include <thread>
#include <mutex>

FileSort::FileSort(int maxFileSizeBytes, int numberOfLinesPerSegment, int lineSizeBytes)
    : _maxFileSizeBytes(maxFileSizeBytes),
      _numberOfLinesPerSegment(numberOfLinesPerSegment),
      _lineSizeBytes(lineSizeBytes),
      _chunkSize(_numberOfLinesPerSegment * _lineSizeBytes)
{
    if (_chunkSize > MAX_TMP_FILE_SIZE)
        throw Exception("numberOfLinesPerSegment * lineSizeBytes: " + to_string(_chunkSize) + ", exceeds maximum allowed: "
        + to_string(MAX_TMP_FILE_SIZE));
    srand((unsigned int)time(NULL));
}

// TODO(david): find maximum filehandle for each platform, and have the filemanager be in a separate process
void FileSort::Sort(const std::string &inFilePath, const std::string &outFilePath)
{
    FileManager fileManager(MAX_FD - 10, to_string(rand()));
    FileHandle inFileHandle = fileManager.open(inFilePath, READ, true, false);
    FileHandle outFileHandle = fileManager.open(outFilePath, RDWR, false, false);

#if defined(WINDOWS)
    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(inFileHandle, &fileSize))
        throw Exception("Failed to check size of file: " + inFilePath);
    if (fileSize.QuadPart % _numberOfLinesPerSegment)
        throw Exception("Parameter numberOfLinesPerSegment (" + to_string(_numberOfLinesPerSegment) + ") needs to be a divisor of the input file's (" + inFilePath + ") size. File size: " + to_string(fileSize.QuadPart));
    if (fileSize.QuadPart > _maxFileSizeBytes)
        throw Exception("File: " + inFilePath + " exceeds maximum(" + to_string(_maxFileSizeBytes) + "): " + to_string(fileSize.QuadPart));
#elif defined(LINUX)
    struct stat fileInfo;
    if (stat(inFilePath.c_str(), &fileInfo) == -1)
        throw Exception("Failed to check size of file: " + inFilePath);
    if (fileInfo.st_size > _maxFileSizeBytes)
        throw Exception("File: " + inFilePath + " exceeds maximum(" + to_string(_maxFileSizeBytes) + "): " + to_string(fileInfo.st_size));
    if (fileInfo.st_size % _numberOfLinesPerSegment)
        throw Exception("Parameter numberOfLinesPerSegment (" + to_string(_numberOfLinesPerSegment) + ") needs to be a divisor of the input file's (" + inFilePath + ") size. File size: " + to_string(fileInfo.st_size));
#endif

#if defined(WINDOWS)
    int numberOfChunks = (int)(fileSize.QuadPart / _chunkSize);
#elif defined(LINUX)
    int numberOfChunks = (int)(fileInfo.st_size / _chunkSize);
#endif
    LOG("Number of chunks: " << numberOfChunks);

    sw.set();
    unique_ptr<WordsArray> chunk = make_unique<WordsArray>(_numberOfLinesPerSegment, _lineSizeBytes);
    if (numberOfChunks == 1)
    {
        fileManager.read(inFileHandle, chunk->data(), chunk->size());
        chunk->sort();

        for (int i = 0; i < chunk->sizeWords(); ++i)
        {
            Word word = chunk->getWord(i);
            fileManager.write(outFileHandle, word.data(), word.size());
        }
        return;
    }
    for (int currentIteration = 0;
         currentIteration < numberOfChunks;
         ++currentIteration)
    {
        fileManager.read(inFileHandle, chunk->data(), chunk->size());
        chunk->sort();

        FileHandle tmpFileHandle = fileManager.createTmp();
        fileManager.write(tmpFileHandle, chunk->data(), chunk->size());
        fileManager.write(outFileHandle, chunk->data(), chunk->size());
        fileManager.close(tmpFileHandle);
    }
    sw.set();
    chunk.release();
    sw.print();
    sw.clear();

    sw.set();
    sort(numberOfChunks, outFileHandle, fileManager);
    sw.set();
    sw.print();
}

// TODO(david): localize this variable
mutex outFileMutex;

void FileSort::sort(int numberOfChunks, FileHandle outFileHandle, FileManager &fileManager)
{
#if defined(WINDOWS)
    thread t1(&FileSort::mergeSort, this, 0, numberOfChunks / 2, outFileHandle, ref(fileManager), true);
    mergeSort(numberOfChunks / 2, numberOfChunks, outFileHandle, fileManager, true);
    t1.join();
    merge(0, numberOfChunks / 2, numberOfChunks, outFileHandle, fileManager, true);
#elif defined(LINUX)
    mergeSort(0, numberOfChunks, outFileHandle, fileManager, false);
#endif
}

void FileSort::mergeSort(int start, int end, FileHandle outFileHandle, FileManager &fileManager, bool outToTmp)
{
    if (end - start < 2)
        return;

    int mid = start + (end - start) / 2;
    mergeSort(start, mid, outFileHandle, fileManager, !outToTmp);
    mergeSort(mid, end, outFileHandle, fileManager, !outToTmp);
    if (mergeIsSorted(mid, outFileHandle, fileManager, outToTmp))
        return ;
    merge(start, mid, end, outFileHandle, fileManager, !outToTmp);
}

bool FileSort::mergeIsSorted(int mid, FileHandle outFileHandle, FileManager &fileManager, bool outToTmp)
{
    // NOTE(david): check if first half chunk is less than the second
    vector<char> firstHalfLast(_lineSizeBytes);
    vector<char> secondHalfFirst(_lineSizeBytes);
    if (outToTmp)
    {
        FileHandle tmpFileHandle = fileManager.openTmp(mid - 1);
        fileManager.seek(tmpFileHandle, _chunkSize - _lineSizeBytes);
        fileManager.read(tmpFileHandle, firstHalfLast.data(), firstHalfLast.size());
        fileManager.close(tmpFileHandle);
        outFileMutex.lock();
        fileManager.seek(outFileHandle, mid * _chunkSize);
        fileManager.read(outFileHandle, secondHalfFirst.data(), secondHalfFirst.size());
        outFileMutex.unlock();
        if (firstHalfLast < secondHalfFirst)
            return (true);
    }
    else
    {
        FileHandle tmpFileHandle = fileManager.openTmp(mid);
        fileManager.read(tmpFileHandle, secondHalfFirst.data(), secondHalfFirst.size());
        fileManager.close(tmpFileHandle);
        outFileMutex.lock();
        fileManager.seek(outFileHandle, mid * _chunkSize - _lineSizeBytes);
        fileManager.read(outFileHandle, firstHalfLast.data(), firstHalfLast.size());
        outFileMutex.unlock();
        if (firstHalfLast < secondHalfFirst)
            return (true);
    }
    return (false);
}

void FileSort::merge(int start, int mid, int end, FileHandle outFileHandle, FileManager &fileManager, bool outToTmp)
{
    int leftChunkIndex = start;
    int rightChunkIndex = mid;
    int leftTmpWordIndex = 0;
    int rightTmpWordIndex = 0;
    WordsArray leftChunk(_numberOfLinesPerSegment, _lineSizeBytes);
    WordsArray rightChunk(_numberOfLinesPerSegment, _lineSizeBytes);
    WordsArray outChunk(_numberOfLinesPerSegment, _lineSizeBytes);
    Word leftWord;
    Word rightWord;

    int chunkIndex = start;

    uint64 numberOfChunks = end - start;
    uint64 numberOfWords = numberOfChunks * _numberOfLinesPerSegment;
    int outChunkWordIndex = 0;

    for (uint64 curWordIndex = 0;
         curWordIndex < numberOfWords;
         ++curWordIndex)
    {
        if (leftChunkIndex < mid && leftTmpWordIndex == 0)
        {
            if (outToTmp)
            {
                FileHandle tmpFileHandle = fileManager.openTmp(leftChunkIndex);
                fileManager.read(tmpFileHandle, leftChunk.data(), leftChunk.size());
                fileManager.close(tmpFileHandle);
            }
            else
            {
                outFileMutex.lock();
                fileManager.seek(outFileHandle, _chunkSize * leftChunkIndex);
                fileManager.read(outFileHandle, leftChunk.data(), leftChunk.size());
                outFileMutex.unlock();
            }
            leftWord = leftChunk.getWord(leftTmpWordIndex++);
        }
        if (rightChunkIndex < end && rightTmpWordIndex == 0)
        {
            if (outToTmp)
            {
                FileHandle tmpFileHandle = fileManager.openTmp(rightChunkIndex);
                fileManager.read(tmpFileHandle, rightChunk.data(), rightChunk.size());
                fileManager.close(tmpFileHandle);
            }
            else
            {
                outFileMutex.lock();
                fileManager.seek(outFileHandle, _chunkSize * rightChunkIndex);
                fileManager.read(outFileHandle, rightChunk.data(), rightChunk.size());
                outFileMutex.unlock();
            }
            rightWord = rightChunk.getWord(rightTmpWordIndex++);
        }

        if (leftChunkIndex < mid && (rightChunkIndex >= end || leftWord < rightWord))
        {
            outChunk.addWord(leftWord, outChunkWordIndex++);
            if (leftTmpWordIndex == _numberOfLinesPerSegment)
            {
                ++leftChunkIndex;
                leftTmpWordIndex = 0;
            }
            else
            {
                leftWord = leftChunk.getWord(leftTmpWordIndex++);
            }
        }
        else
        {
            outChunk.addWord(rightWord, outChunkWordIndex++);
            if (rightTmpWordIndex == _numberOfLinesPerSegment)
            {
                ++rightChunkIndex;
                rightTmpWordIndex = 0;
            }
            else
            {
                rightWord = rightChunk.getWord(rightTmpWordIndex++);
            }
        }

        if (outChunk.sizeWords() == outChunkWordIndex)
        {
            outChunkWordIndex = 0;

            if (outToTmp)
            {
                outFileMutex.lock();
                fileManager.seek(outFileHandle, chunkIndex * _chunkSize);
                fileManager.write(outFileHandle, outChunk.data(), outChunk.size());
                outFileMutex.unlock();
            }
            else
            {
                FileHandle tmpFileHandle = fileManager.openTmp(chunkIndex);
                fileManager.write(tmpFileHandle, outChunk.data(), outChunk.size());
                fileManager.close(tmpFileHandle);
            }
            ++chunkIndex;
        }
    }
}

FileSort::Exception::Exception(const string &msg)
    : runtime_error(msg)
{
}

FileSort::Exception::~Exception() noexcept
{
}
