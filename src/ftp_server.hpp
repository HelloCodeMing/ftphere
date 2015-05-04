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

            while (true) {
                acceptor_.accept(remote_socket, ec);
                HandleClient(remote_socket);
                if (ec) {
                    // to do
                    Error(ec.what());
                }
            }
        }


    private:
        enum CMD { USER, PASS, PORT };

        void HandleClient(tcp::socket& socket) {
            LogClientAction(socket, "sign in");

            boost::system::error_code ec;
            streambuf buff;
            std::istream is(&buff);
            string line;

            while (true) {
                read_until(socket, buff, '\n', ec);
                std::getline(is, line);
                
                try {
                    auto cmd_args = ResolveCMD(line);
                    Dispatch(tcp::socket& socket, cmd_args.first, cmd_args.second);
                } catch (exception& e) {
                    // to do
                    Error(e.what());
                    break;
                }
                if (ec) {
                    // to do
                    Error(ec.what());
                }
            }
            socket.close();
        }

        std::pair<int, string>
        ResovleCMD(const string& cmd) {
            int delimit_index = cmd.find(' ');
            if (delimit_index == string::npos) {
                // to do
                throw 
            }
        }

        void LogClientAction(tcp::socket& socket, const char* msg) {
            tcp::address address = socket.remote_endpoint().address();
            string info = address.to_string() + ": " + msg;
            logger_.Log(info, Logger::INFO);
        }

        void Error(const char* msg) {
            logger_.Log(msg, Logger::ERROR);
        }

        const char* log_path_ = "./ftphere.log";
        unsigned short port_;
        tcp::endpoint local_ep_;
        io_service ios_;
        tcp::acceptor acceptor_;
        Logger logger_;
};
}
