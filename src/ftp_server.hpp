/*
 * ftphere
 * Start a ftp server monitor the current directory. 
 * It could be used to transport files conveniently.
 */


#include <boost/asio.hpp>
#include <iostream>

namespace ftp {

using namespace boost::asio;
using boost::asio::ip::tcp;
using std::string;

class FTPServer {
    public:
        FTPServer(): 
            port_(8080),
            local_ep_(tcp::v4(), port_),
            ios_(),
            acceptor_(ios_) {
            
        }

        ~FTPServer() {}

        void Run() {
            boost::system::error_code ec;
            tcp::socket remote_socket(ios_);

            /* init the acceptor */
            acceptor_.open(tcp::v4());
            acceptor_.bind(local_ep_);
            acceptor_.listen();

            while (!ec) {
                acceptor_.accept(remote_socket, ec);
                HandleClient(remote_socket);
            }
        }


    private:
        void HandleClient(tcp::socket& socket) {
            puts("a new comer");
        }

        const char* log_path_ = "./ftphere.log";
        unsigned short port_;
        tcp::endpoint local_ep_;
        io_service ios_;
        tcp::acceptor acceptor_;
        Logger logger_;
};

class Logger {
    public:
        Logger(const char* log_path):
            log_path_(log_path) {
            log_file_ = fopen(log_path_, "a+");
        }

        void Log(const string& message) {
            Log(message.data());
        }

        void Log(const char* message) {
            fprintf(log_file_, "%s:\t%s\n", Date(), message);
        }

    private:

        const char* Date() {
            return "shit";
        }

        char* log_path_;
        FILE* log_file_;
};
}
