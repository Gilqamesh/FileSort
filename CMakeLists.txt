cmake_minimum_required(VERSION 3.12)

set(PROJECT_NAME "FileSort")
set(SRC_DIR "Sources")
set(SRC_FILES "WordsArray.cpp" "FileManager.cpp" "FileSort.cpp" "main.cpp" "Word.cpp")
list(TRANSFORM SRC_FILES PREPEND ${SRC_DIR}/)

file(MAKE_DIRECTORY ${SRC_DIR})

# set the project name
project(${PROJECT_NAME})

# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (MSVC)
    add_compile_options(-opt:ref -Oi -MT -FC -Zi -WX -W4 -nologo -EHa -wd4100 -wd4189 -wd4702 -wd4505 /std:c++14 -DRELEASE)
else()
    add_compile_options(-Wall -Wextra -Werror -std=c++14 -O3 -DRELEASE)
endif()

# add the executable
add_executable(${PROJECT_NAME} ${SRC_FILES})

if (MSVC)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
else()
# no threading support
endif()
