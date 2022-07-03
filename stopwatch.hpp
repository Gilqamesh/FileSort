#ifndef STOPWATCH_HPP
# define STOPWATCH_HPP

#include <ctime>
#include <iostream>
#include <map>

using namespace std;

class stopwatch
{
map<unsigned int, time_t> stops;
map<unsigned int, time_t> elapsed_time;
int i;
public:
    stopwatch() : i(0) { }
    void set()
    {
        stops[i] = clock();
        if (i)
        {
            elapsed_time[i - 1] += stops[i] - stops[i - 1];
        }
        ++i;
    }

    void clear()
    {
        reset();
        stops.clear();
        elapsed_time.clear();
    }

    void reset()
    {
        i = 0;
    }

    void print()
    {
        for (const auto &p : elapsed_time)
        {
            std::cout << p.first << " -> " << p.second / (double)CLOCKS_PER_SEC << "s" << std::endl;
        }
    }
};

#endif
