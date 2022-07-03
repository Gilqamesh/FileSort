#ifndef FILEMANAGER_HPP
# define FILEMANAGER_HPP

# include <unordered_map>
# include <unordered_set>
# include <string>
# include <stdexcept>
# include "Platform.hpp"

# if defined(WINDOWS)

#  include <Windows.h>
#  define READ GENERIC_READ
#  define WRITE GENERIC_WRITE
#  define RDWR (READ | WRITE)

typedef HANDLE FileHandle;

# elif defined(LINUX)

#  include <fcntl.h>
#  define READ O_RDONLY
#  define WRITE O_WRONLY
#  define RDWR O_RDWR

typedef int FileHandle;

#endif

using namespace std;

class FileManager
{
    int _maxFileHandles;
    const string _tmpFileName;
    unsigned int _numberOfTmpFiles;
    unordered_set<FileHandle> _fileHandlesCache;
    unordered_map<string, FileHandle> _tmpFileHandles;
public:
    FileManager(int maxFileHandles, const string &tmpFileName);
    ~FileManager();
    FileHandle openTmp(int index);
    FileHandle createTmp();
    void closeTmp(int index);
    FileHandle open(const string &filePath, unsigned long fileAccess, bool exists, bool isTmp);
    void close(FileHandle fileHandle);
    void closeCached();
    void read(FileHandle fileHandle, void *buffer, int bytesToRead);
    void write(FileHandle fileHandle, void *buffer, int bytesToWrite);
    void seek(FileHandle fileHandle, unsigned long offset);

private:
    class Exception : public runtime_error
    {
    public:
        Exception(const string &msg);
        ~Exception() noexcept;
    };
};

#endif
