/*
 *
 * A simple logger for the ftp server
 */

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <ctime>
#include <cstdio>
#include <string>

#include <boost/filesystem.hpp>

namespace ftp {
using std::string;
using namespace boost::filesystem;

class Logger {
    public:
        enum Level { INFO, ERROR };

        explicit Logger(const char* log_path) {
            log_path_ = log_path;
            log_file_ = fopen(log_path, "a+");
            
            /* chmod 644 */
            path file_path(log_path);
            permissions(file_path, owner_read |
                                   owner_write|
                                   group_read|
                                   others_read);
        }

        ~Logger() {
            fclose(log_file_);
        }
    

        void Log(const string& msg, Level level = Level::INFO) {
            fprintf(log_file_, "%s %s %s\n", 
                    Date().data(), 
                    level == INFO ? "INFO" : "ERROR",
                    msg.data());
#ifndef NDEBUG
            fprintf(stdout, "%s %s %s\n", 
                    Date().data(), 
                    level == INFO ? "INFO" : "ERROR",
                    msg.data());
#endif
        }
        

    private:

        string Date() const {
            time_t now_time = time(NULL);
            tm* local = localtime(&now_time);
            char buff[80];
            strftime(buff, 80, "%Y-%m-%d %H:%M:%S", local);
            return buff;
        }
        
        const char* log_path_;
        FILE* log_file_;
};
} // end of namespace ftp
#endif 
