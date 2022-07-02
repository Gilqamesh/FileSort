#ifndef FILECHUNK_HPP
# define FILECHUNK_HPP

# include <vector>

using namespace std;

typedef vector<char> Word;

class FileChunk : public vector<char>
{
    int _numberOfLines;
    int _lineSizeBytes;
    // TODO(david): store indices to the starting position in the buffer instead
    vector<Word> _sortedWords;
public:
    FileChunk(int numberOfLines, int lineSizeBytes);
    void sort();
    Word getWord(unsigned int index);
};

#endif
