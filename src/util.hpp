#define UTIL_HPP

#include <string>
#include <vector>

namespace ftp {
    std::vector<std::string>
        Split(std::string& str, char delimiter) {
            std::vector<std::string> res;
            int start = 0;
            int iter = str.find(delimiter);

            str += delimiter;
            while (iter != str.npos) {
                res.push_back(str.substr(start, iter-start));
                start = iter + 1;
                iter = str.find(delimiter, start);
            }
            str.pop_back();
            return res;
        }
}
