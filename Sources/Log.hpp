#ifndef LOG_HPP
# define LOG_HPP

# include <iostream>

using namespace std;

# define LOG(x) (cout << x << endl)
# define LINE() (LOG(__FILE__ << " " << __LINE__))

#endif
