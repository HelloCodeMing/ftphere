#define UTIL_HPP

#include <string>
#include <vector>
#include <cassert>

#include <boost/filesystem.hpp>

namespace ftp {
using namespace boost::filesystem;

std::vector<std::string>
Split(std::string str, char delimiter) {
    str += delimiter;

    std::vector<std::string> res;
    int start = 0;
    int iter = str.find(delimiter);

    while (iter != str.npos) {
        res.push_back(str.substr(start, iter-start));
        start = iter + 1;
        iter = str.find(delimiter, start);
    }
    str.pop_back();
    return res;
}

path relative(const path& p, const path& base) {
    assert(p.is_absolute() && base.is_absolute());
    return p.string().substr(base.string().length());
}
}
