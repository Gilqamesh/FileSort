#include "FileManager.hpp"
#include "Log.hpp"

FileManager::FileManager(int maxFileHandles, const string &tmpFileName)
    : _maxFileHandles(maxFileHandles),
      _tmpFileName(tmpFileName),
      _numberOfTmpFiles(0)
{
}

FileManager::~FileManager()
{
    closeCached();
    for (unsigned int i = 0; i < _numberOfTmpFiles; ++i)
#if defined(WINDOWS)
        DeleteFileA((_tmpFileName + to_string(i)).c_str());
#elif defined(LINUX)
        remove((_tmpFileName + to_string(i)).c_str());
#endif
}

FileHandle FileManager::open(const string &filePath, unsigned long fileAccess, bool exists, bool isTmp)
{
    FileHandle fileHandle;

    if (filePath.size() > MAX_FILENAME_PATH)
        throw Exception("File name size (" + to_string(filePath.size()) + ") too big, max characters allowed: " + to_string(MAX_FILENAME_PATH));

    if (isTmp && _tmpFileHandles.count(filePath))
        return (_tmpFileHandles[filePath]);

    if (_tmpFileHandles.size() >= (size_t)_maxFileHandles)
    {
        auto it = _tmpFileHandles.begin();
        close(it->second);
        _tmpFileHandles.erase(it);
    }

#if defined(_WIN64)
    fileHandle = CreateFileA(filePath.c_str(), fileAccess, 0, NULL, exists ? OPEN_EXISTING : CREATE_ALWAYS, 0, NULL);
    if (fileHandle == INVALID_HANDLE_VALUE)
        throw Exception("Failed to open file: " + filePath);
#elif defined(__linux__)
    fileHandle = ::open(filePath.c_str(), fileAccess | (exists ? 0 : O_TRUNC | O_CREAT), 0644);
    if (fileHandle == -1)
        throw Exception("Failed to open file: " + filePath);
#endif

    if (isTmp)
        _tmpFileHandles[filePath] = fileHandle;
    else
        _fileHandlesCache.insert(fileHandle);

    return (fileHandle);
}

FileHandle FileManager::createTmp()
{
    return (open(_tmpFileName + to_string(_numberOfTmpFiles++), RDWR, false, true));
}

FileHandle FileManager::openTmp(int tmpFileIndex)
{
    return (open(_tmpFileName + to_string(tmpFileIndex), RDWR, true, true));
}

void FileManager::closeTmp(int tmpFileIndex)
{
    string tmpFileName = _tmpFileName + to_string(tmpFileIndex);
    if (_tmpFileHandles.count(tmpFileName))
    {
        FileHandle fileHandle = _tmpFileHandles[tmpFileName];
        close(fileHandle);
        _tmpFileHandles.erase(tmpFileName);
    }
}

void FileManager::close(FileHandle fileHandle)
{
#if defined(_WIN64)
    CloseHandle(fileHandle);
#elif defined(__linux__)
    ::close(fileHandle);
#endif
}

void FileManager::closeCached()
{
    for (auto fileHandle : _fileHandlesCache)
        close(fileHandle);
    for (auto fileHandle : _tmpFileHandles)
        close(fileHandle.second);
}

void FileManager::read(FileHandle fileHandle, void *buffer, size_t bytesToRead)
{
#if defined(WINDOWS)
    DWORD bytesRead;
    if (!ReadFile(fileHandle, buffer, (DWORD)bytesToRead, &bytesRead, NULL))
#elif defined(LINUX)
    int bytesRead;
    if ((bytesRead = ::read(fileHandle, buffer, bytesToRead)) == -1)
#endif
        throw Exception("Something went wrong while reading the file.");
    if ((int)bytesRead != bytesToRead)
        throw Exception("Unexpected bytes read from file, expected to read: " + to_string(bytesToRead) + ", but read: " + to_string(bytesRead));
}

void FileManager::write(FileHandle fileHandle, void *buffer, size_t bytesToWrite)
{
#if defined(WINDOWS)
    DWORD bytesWritten;
    if (!WriteFile(fileHandle, buffer, (DWORD)bytesToWrite, (DWORD*)&bytesWritten, NULL))
#elif defined(LINUX)
    int bytesWritten;
    if ((bytesWritten = ::write(fileHandle, buffer, bytesToWrite)) == -1)
#endif
        throw Exception("Something went wrong while writing the file.");
    if ((int)bytesWritten != bytesToWrite)
        throw Exception("Unexpected bytes written to file, expected to write: " + to_string(bytesToWrite) + ", but written " + to_string(bytesWritten));
}

void FileManager::overwrite(FileHandle fileHandle, void *buffer, size_t bytesToWrite)
{
    seek(fileHandle, 0);
    write(fileHandle, buffer, bytesToWrite);
}

void FileManager::seek(FileHandle fileHandle, unsigned long offset)
{
#if defined(WINDOWS)
    if (SetFilePointer(fileHandle, offset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
#elif defined(LINUX)
    if (lseek(fileHandle, offset, SEEK_SET) == -1)
#endif
        throw Exception("Unexpected error in seek. Offset value: " + to_string(offset));
}

FileManager::Exception::Exception(const string &msg)
    : runtime_error(msg)
{
}

FileManager::Exception::~Exception() noexcept
{
}
