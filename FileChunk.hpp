#ifndef FILECHUNK_HPP
# define FILECHUNK_HPP

# include <vector>
# include <string>

using namespace std;

class FileChunk : public vector<char>
{
    int _numberOfLines;
    int _lineSizeBytes;
    // TODO(david): store indices to the starting position in the buffer instead
    vector<string> _sortedWords;
public:
    FileChunk(int numberOfLines, int lineSizeBytes);
    void sort();
    string getWord(unsigned int index);
};

#endif
