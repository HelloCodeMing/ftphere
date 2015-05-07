#define UTIL_HPP

#include <string>
#include <vector>
#include <cassert>
#include <ctime>
#include <iomanip>

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

std::string
make_file_info(const path& path) {
    char buff[5];
    
    std::string file_name = path.filename().string();
    time_t last_time = last_write_time(path);
    sprintf(buff, "%o", status(path).permissions());

    if (is_regular_file(path)) {
        int size = file_size(path);
        return std::string(buff) + " " +
               std::to_string(size) + " " +
               std::put_time(std::localtime(&last_time), "%F %T") + " " +
               file_name;
    }
    return std::string(buff) + " " +
           ctime(&last_time) + " " +
           file_name;
}

std::string
make_file_info(const directory_entry& entry) {
    return make_file_info(entry.path());
}


path relative(const path& p, const path& base) {
    assert(p.is_absolute() && base.is_absolute());
    return p.string().substr(base.string().length());
}
}
