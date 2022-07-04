#ifndef WORD_HPP
# define WORD_HPP

# include <vector>
# include <algorithm>

using namespace std;

class Word
{
friend bool operator<(const Word& l, const Word& r);
    int _size;
    vector<char>::iterator _begin;
public:
    Word();
    Word(int size, vector<char>::iterator begin);
    void *data();
    int size();

    char operator[](unsigned int index);
};

bool operator<(const Word& l, const Word& r);

#endif
