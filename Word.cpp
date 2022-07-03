#include "Word.hpp"

Word::Word()
    : _size(0)
{
}

Word::Word(int size, vector<char>::iterator begin)
    : _size(size), _begin(begin)
{
}

void *Word::data()
{
#if defined (_WIN64)
    return (_begin._Unwrapped());
#elif defined(__linux__)
    return (_begin.base());
#endif
}

int Word::size()
{
    return (_size);
}

char Word::operator[](unsigned int index)
{
    return (*(_begin + index));
}

bool operator<(const Word& l, const Word& r)
{
    return (lexicographical_compare(l._begin, l._begin + l._size,
                                    r._begin, r._begin + r._size));
}
