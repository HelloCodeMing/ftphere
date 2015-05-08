#define UTIL_HPP

#include <string>
#include <vector>
#include <cassert>
#include <ctime>

#include <boost/filesystem.hpp>
#include <boost/asio.hpp>

namespace ftp {
using namespace boost::filesystem;
using namespace boost::asio;

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
    char time_buff[24];
    time_t time_tp = last_write_time(path);
    strftime(time_buff, 24, "%Y-%m-%d %H:%M:%S", localtime(&time_tp));
    sprintf(buff, "%o", status(path).permissions());

    if (is_regular_file(path)) {
        int size = file_size(path);
        return std::string(buff) + " " +
               std::to_string(size) + " " +
               time_buff + " " +
               file_name;
    }
    return std::string(buff) + " " +
           time_buff + " " +
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

std::string ReadLine(ip::tcp::socket& socket, 
                     boost::system::error_code& ec) {
    streambuf buff;
    std::istream is(&buff);
    std::string line;

    read_until(socket, buff, "\r\n", ec);
    std::getline(is, line, '\r');

    return line;
}

std::string ReadLine(ip::tcp::socket& socket) {
    streambuf buff;
    std::istream is(&buff);
    std::string line;

    read_until(socket, buff, "\r\n");
    std::getline(is, line, '\r');

    return line;
}

void WriteLine(ip::tcp::socket& socket, const std::string& str) {
    socket.write_some(buffer(str + "\r\n"));
}
}
