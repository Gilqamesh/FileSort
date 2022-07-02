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
    closeAll();
    for (unsigned int i = 0; i < _numberOfTmpFiles; ++i)
        DeleteFileA((_tmpFileName + to_string(i)).c_str());
}

FileHandle FileManager::create(const string &filePath, unsigned long fileAccess, bool persist)
{
    FileHandle fileHandle;

#if defined(_WIN64)
    fileHandle = CreateFileA(filePath.c_str(), fileAccess, 0, NULL, CREATE_ALWAYS, 0, NULL);
    if (fileHandle == INVALID_HANDLE_VALUE)
        throw Exception("Failed to open file: " + filePath);
#elif defined(__linux__)
    fileHandle = open(filePath.c_str(), fileAccess | O_CREAT);
#endif
    if (persist)
        _fileHandlesPersist[filePath] = fileHandle;
    else
        _fileHandles[filePath] = fileHandle;

    return (fileHandle);
}

FileHandle FileManager::open(const string &filePath, unsigned long fileAccess, bool persist)
{
    if (_fileHandlesPersist.count(filePath))
        return (_fileHandlesPersist[filePath]);
    if (_fileHandles.count(filePath))
        return (_fileHandles[filePath]);

    FileHandle fileHandle;

#if defined(_WIN64)
    fileHandle = CreateFileA(filePath.c_str(), fileAccess, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (fileHandle == INVALID_HANDLE_VALUE)
        throw Exception("Failed to open file: " + filePath);
#elif defined(__linux__)
    fileHandle = open(filePath.c_str(), fileAccess);
#endif
    if (persist)
        _fileHandlesPersist[filePath] = fileHandle;
    else
        _fileHandles[filePath] = fileHandle;

    return (fileHandle);
}

FileHandle FileManager::createTmp()
{
    return (create(_tmpFileName + to_string(_numberOfTmpFiles++), RDWR));
}

FileHandle FileManager::openTmp(unsigned long index)
{
    return (open(_tmpFileName + to_string(index), RDWR));
}

void FileManager::close(FileHandle fileHandle)
{
#if defined(_WIN64)
    CloseHandle(fileHandle);
#elif defined(__linux__)
    close(fileHandle);
#endif
}

void FileManager::closeAll()
{
    for (auto &p : _fileHandles)
        close(p.second);
    for (auto &p : _fileHandlesPersist)
        close(p.second);
}

void FileManager::read(FileHandle fileHandle, void *buffer, size_t bytesToRead)
{
    DWORD bytesRead;

    if (!ReadFile(fileHandle, buffer, (DWORD)bytesToRead, &bytesRead, NULL))
        throw Exception("Something went wrong while reading the file.");
    if (bytesRead != bytesToRead)
        throw Exception("Unexpected bytes read from file, expected to read: " + to_string(bytesToRead) + ", but read: " + to_string(bytesRead));
}

void FileManager::write(FileHandle fileHandle, void *buffer, size_t bytesToWrite)
{
    DWORD bytesWritten;

    if (!WriteFile(fileHandle, buffer, (DWORD)bytesToWrite, &bytesWritten, NULL))
        throw Exception("Something went wrong while writing the file.");
    if (bytesWritten != bytesToWrite)
        throw Exception("Unexpected bytes written to file, expected to write: " + to_string(bytesToWrite) + ", but written " + to_string(bytesWritten));
}

void FileManager::seek(FileHandle fileHandle, unsigned long offset)
{
    if (SetFilePointer(fileHandle, offset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
        throw Exception("Unexpected error in SetFilePointer");
}

FileManager::Exception::Exception(const string &msg)
    : runtime_error(msg)
{
}

FileManager::Exception::~Exception() throw()
{
}
