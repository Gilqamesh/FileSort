#ifndef PLATFORM_HPP
# define PLATFORM_HPP

# include <cstdint>

# if defined(_WIN64)
#  define WINDOWS
#  define MAX_FILENAME_PATH MAX_PATH
#  define MAX_FD 16777216

#  include "Windows.h"

# elif defined(__linux__)
#  define LINUX
#  define MAX_FILENAME_PATH PATH_MAX
#  define MAX_FD 1024

#  include <fcntl.h>
#  include <unistd.h>
#  include <linux/limits.h>
#  include <sys/stat.h>

# endif

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

#endif
