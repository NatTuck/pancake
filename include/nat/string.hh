#ifndef NAT_STRING_HH
#define NAT_STRING_HH

/* Some common string operations for use in C++. */

#include <ctype.h>

#include <string>
#include <vector>

std::vector<std::string>
string_split(std::string sep, std::string ss)
{
    std::vector<string> ys;

    while (ss.find(sep) != std::string::npos) {
        size_t pos = ss.find(sep);
        string xx = ss.substr(0, pos);
        ys.push_back(xx);
        ss = ss.substr(pos + 1, std::string::npos);
    }

    ys.push_back(ss);

    return ys;
}

std::string
string_remove_whitespace(std::string xx)
{
    size_t ii = 0;

    while (ii < xx.size()) {
        if (isspace(xx[ii])) {
            xx.erase(ii, 1);
        }
        else {
            ii += 1;
        }
    }
    
    return xx;
}

#endif
