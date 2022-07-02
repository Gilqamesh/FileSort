#ifndef FILECHUNK_HPP
# define FILECHUNK_HPP

# include <vector>
# include <string>

using namespace std;

class FileChunk
{
    int _numberOfWords;
    int _wordSize;
    int _wordIndex;
    vector<char> _data;
    // TODO(david): store indices to the starting position in the buffer instead
    vector<pair<int, int>> _sortedIndices;
    vector<string> _sortedWords;
public:
    FileChunk(int numberOfWords, int wordSize);
    void sort();
    string getWord(unsigned int index);
    void addWord(const string& word);
    size_t sizeWords();
    void reset();
    void *data();
    size_t size();
};

#endif
