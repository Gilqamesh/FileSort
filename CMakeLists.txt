cmake_minimum_required(VERSION 3.10)

# set the project name
project(SortFile)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (MSVC)
    add_compile_options(-opt:ref -Oi -MT -FC -Zi -WX -W4 -nologo -EHa -wd4100 -wd4189 -wd4702 /std:c++14)
else()
    add_compile_options(-Wall -Wextra -Werror -O3)
endif()

# add the executable
add_executable(SortFile WordsArray.cpp FileManager.cpp FileSort.cpp main.cpp Word.cpp)
