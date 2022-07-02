#ifndef FILEMANAGER_HPP
#define FILEMANAGER_HPP

#include <unordered_map>
#include <string>
#include <stdexcept>

#if defined(_WIN64)

#include <Windows.h>
typedef HANDLE FileHandle;
#define READ GENERIC_READ
#define WRITE GENERIC_WRITE
#define RDWR (READ | WRITE)

#elif defined(__linux__)

#include <fcntl.h>
typedef int FileHandle;
#define READ O_RDONLY
#define WRITE O_WRONLY
#define RDWR O_RDWR

#endif

using namespace std;

class FileManager
{
    int _maxFileHandles;
    const string _tmpFileName;
    unsigned int _numberOfTmpFiles;
    unordered_map<string, FileHandle> _fileHandles;
    unordered_map<string, FileHandle> _fileHandlesPersist;

public:
    FileManager(int maxFileHandles, const string &tmpFileName);
    ~FileManager();
    FileHandle create(const string &filePath, unsigned long fileAccess, bool persist = false);
    FileHandle open(const string &filePath, unsigned long fileAccess, bool persist = false);
    FileHandle createTmp();
    FileHandle openTmp(unsigned long index);
    void close(FileHandle fileHandle);
    void closeAll();
    void read(FileHandle fileHandle, void *buffer, size_t bytesToRead);
    void write(FileHandle fileHandle, void *buffer, size_t bytesToWrite);
    void seek(FileHandle fileHandle, unsigned long offset);

private:
    class Exception : public runtime_error
    {
    public:
        Exception(const string &msg);
        ~Exception() throw();
    };
};

#endif
