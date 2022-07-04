#include "FileSort.hpp"
#include "Log.hpp"
#include "WordsArray.hpp"
#include <thread>

FileSort::FileSort(int maxFileSizeBytes, int numberOfLinesPerSegment, int lineSizeBytes)
    : _maxFileSizeBytes(maxFileSizeBytes),
      _numberOfLinesPerSegment(numberOfLinesPerSegment),
      _lineSizeBytes(lineSizeBytes),
      _chunkSize(_numberOfLinesPerSegment * _lineSizeBytes)
{
    if (maxFileSizeBytes < 0 || numberOfLinesPerSegment <= 0 || lineSizeBytes <= 0)
        throw Exception("Non-negative input");
    if (_chunkSize > MAX_TMP_FILE_SIZE)
        throw Exception("numberOfLinesPerSegment * lineSizeBytes: " + to_string(_chunkSize) + ", exceeds max tmp file size: " + to_string(MAX_TMP_FILE_SIZE));
}

int FileSort::sanitizeInFile(FileHandle inFileHandle, const string &inFilePath)
{
#if defined(WINDOWS)
    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(inFileHandle, &fileSize))
        throw Exception("Failed to check size of file: " + inFilePath);
    if (fileSize.QuadPart % _numberOfLinesPerSegment)
        throw Exception(inFilePath + ": numberOfLinesPerSegment (" + to_string(_numberOfLinesPerSegment) + ") needs to be a divisor of the file's size. File size: " + to_string(fileSize.QuadPart));
    if (fileSize.QuadPart > _maxFileSizeBytes)
        throw Exception(inFilePath + ": file size exceeds maximum(" + to_string(_maxFileSizeBytes) + "): " + to_string(fileSize.QuadPart));
#elif defined(LINUX)
    (void)inFileHandle;
    struct stat fileInfo;
    if (stat(inFilePath.c_str(), &fileInfo) == -1)
        throw Exception("Failed to check size of file: " + inFilePath);
    if (fileInfo.st_size % _numberOfLinesPerSegment)
        throw Exception(inFilePath + ": numberOfLinesPerSegment (" + to_string(_numberOfLinesPerSegment) + ") needs to be a divisor of the file's size. File size: " + to_string(fileInfo.st_size));
    if (fileInfo.st_size > _maxFileSizeBytes)
        throw Exception(inFilePath + ": file size exceeds maximum(" + to_string(_maxFileSizeBytes) + "): " + to_string(fileInfo.st_size));
#endif

#if defined(WINDOWS)
    int numberOfChunks = (int)(fileSize.QuadPart / _chunkSize);
    LOG(inFilePath << ": number of chunks " << to_string(numberOfChunks) << ", file size: " << to_string(fileSize.QuadPart));
#elif defined(LINUX)
    int numberOfChunks = (int)(fileInfo.st_size / _chunkSize);
    LOG(inFilePath << ": number of chunks " << to_string(numberOfChunks) << ", file size: " << to_string(fileInfo.st_size));
#endif

    return (numberOfChunks);
}

void FileSort::initializeChunks(FileHandle inFileHandle, FileHandle outFileHandle, FileManager &fileManager, int numberOfChunks)
{
    WordsArray chunk(_numberOfLinesPerSegment, _lineSizeBytes);
    for (int currentIteration = 0;
         currentIteration < numberOfChunks;
         ++currentIteration)
    {
        fileManager.read(inFileHandle, chunk.data(), chunk.size());
        chunk.sort();

        FileHandle tmpFileHandle = fileManager.createTmp();
        fileManager.write(tmpFileHandle, chunk.data(), chunk.size());
        fileManager.write(outFileHandle, chunk.data(), chunk.size());
    }
}

void FileSort::Sort(const std::string &inFilePath, const std::string &outFilePath)
{
    FileManager fileManager;

    FileHandle inFileHandle = fileManager.open(inFilePath, READ, true, false);
    int numberOfChunks = sanitizeInFile(inFileHandle, inFilePath);

    FileHandle outFileHandle = fileManager.open(outFilePath, RDWR, false, false);

    initializeChunks(inFileHandle, outFileHandle, fileManager, numberOfChunks);
    LOG("Number of chunks: " << to_string(numberOfChunks));
    LOG("Size of one chunk: " << to_string(_chunkSize));
    LOG("Total size of bytes: " << to_string(numberOfChunks * _chunkSize));

    sort(numberOfChunks, outFileHandle, fileManager);
}

void FileSort::Sort(const vector<string>& inFilePathVec, const string& outFilePath)
{
    FileManager fileManager;

    vector<int> numberOfChunks(inFilePathVec.size());
    vector<FileHandle> inFileHandles(inFilePathVec.size());
    for (int inFileHandleIndex = 0; inFileHandleIndex < (int)inFileHandles.size(); ++inFileHandleIndex)
    {
        inFileHandles[inFileHandleIndex] = fileManager.open(inFilePathVec[inFileHandleIndex], READ, true, false);
        numberOfChunks[inFileHandleIndex] = sanitizeInFile(inFileHandles[inFileHandleIndex], inFilePathVec[inFileHandleIndex]);
    }

    FileHandle outFileHandle = fileManager.open(outFilePath, RDWR, false, false);

    for (int inFileHandleIndex = 0; inFileHandleIndex < (int)inFileHandles.size(); ++inFileHandleIndex)
    {
        initializeChunks(inFileHandles[inFileHandleIndex], outFileHandle, fileManager, numberOfChunks[inFileHandleIndex]);
    }

    int totalNumberOfChunks = 0;
    for (int i = 0; i < (int)numberOfChunks.size(); ++i)
        totalNumberOfChunks += numberOfChunks[i];
    LOG("Total number of chunks: " << to_string(totalNumberOfChunks));
    LOG("Size of one chunk: " << to_string(_chunkSize));
    LOG("Total size of bytes: " << to_string(totalNumberOfChunks * _chunkSize));

    sort(totalNumberOfChunks, outFileHandle, fileManager);
}

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
        return;
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
        fileManager.lockOutFileMutex();
        fileManager.seek(outFileHandle, mid * _chunkSize);
        fileManager.read(outFileHandle, secondHalfFirst.data(), secondHalfFirst.size());
        fileManager.unlockOutFileMutex();
        if (firstHalfLast < secondHalfFirst)
            return (true);
    }
    else
    {
        FileHandle tmpFileHandle = fileManager.openTmp(mid);
        fileManager.seek(tmpFileHandle, 0);
        fileManager.read(tmpFileHandle, secondHalfFirst.data(), secondHalfFirst.size());
        fileManager.lockOutFileMutex();
        fileManager.seek(outFileHandle, mid * _chunkSize - _lineSizeBytes);
        fileManager.read(outFileHandle, firstHalfLast.data(), firstHalfLast.size());
        fileManager.unlockOutFileMutex();
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
                fileManager.seek(tmpFileHandle, 0);
                fileManager.read(tmpFileHandle, leftChunk.data(), leftChunk.size());
            }
            else
            {
                fileManager.lockOutFileMutex();
                fileManager.seek(outFileHandle, _chunkSize * leftChunkIndex);
                fileManager.read(outFileHandle, leftChunk.data(), leftChunk.size());
                fileManager.unlockOutFileMutex();
            }
            leftWord = leftChunk.getWord(leftTmpWordIndex++);
        }
        if (rightChunkIndex < end && rightTmpWordIndex == 0)
        {
            if (outToTmp)
            {
                FileHandle tmpFileHandle = fileManager.openTmp(rightChunkIndex);
                fileManager.seek(tmpFileHandle, 0);
                fileManager.read(tmpFileHandle, rightChunk.data(), rightChunk.size());
            }
            else
            {
                fileManager.lockOutFileMutex();
                fileManager.seek(outFileHandle, _chunkSize * rightChunkIndex);
                fileManager.read(outFileHandle, rightChunk.data(), rightChunk.size());
                fileManager.unlockOutFileMutex();
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
                fileManager.lockOutFileMutex();
                fileManager.seek(outFileHandle, chunkIndex * _chunkSize);
                fileManager.write(outFileHandle, outChunk.data(), outChunk.size());
                fileManager.unlockOutFileMutex();
            }
            else
            {
                FileHandle tmpFileHandle = fileManager.openTmp(chunkIndex);
                fileManager.seek(tmpFileHandle, 0);
                fileManager.write(tmpFileHandle, outChunk.data(), outChunk.size());
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
