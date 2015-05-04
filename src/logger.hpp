/*
 *
 * A simple logger for the ftp server
 */

#define LOGGER_HPP

#include <ctime>
#include <cstdio>
#include <string>

namespace ftp {
using std::string;

class Logger {
    public:
        enum Level { INFO, ERROR };

        explicit Logger(const char* log_path) {
            log_path_ = log_path;
            log_file_ = fopen(log_path, "a+");
        }

        ~Logger() {
            fclose(log_file_);
        }
    
        void Log(const string& msg, Level level) {
            Log(msg.data(), level);
        }

        void Log(const char* msg, Level level) {
            fprintf(log_file_, "%s %s %s\n", 
                    Date().data(), 
                    level == INFO ? "INFO" : "ERROR",
                    msg);
            fprintf(stdout, "%s %s %s\n", 
                    Date().data(), 
                    level == INFO ? "INFO" : "ERROR",
                    msg);
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
}
