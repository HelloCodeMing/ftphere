/*
 * ftphere
 * Start a ftp server monitor the current directory. 
 * It could be used to transport files conveniently.
 */

#define FTPSERVER_HPP

#include <boost/asio.hpp>
#include <iostream>
#include <stdexcept>

#ifndef LOGGER_HPP
#define LOGGER_HPP
#include "logger.hpp"
#endif

namespace ftp {

using namespace boost::asio;
using boost::asio::ip::tcp;
using std::string;
using std::exception;
using boost::system::error_code;

class FTPServer {
    public:
        FTPServer(): 
            port_(21),
            local_ep_(tcp::v4(), port_),
            ios_(),
            acceptor_(ios_),
            logger_("./ftpserver.log"){
            
        }

        ~FTPServer() {

        }

        void Run() {
            error_code ec;
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
                    Error(ec.message());
                }
            }
        }


    private:
        enum CMD { USER, PASS, PORT };

        std::pair<int, string>
        ResolveCMD(const string& cmd) {
            int delimit_index = cmd.find(' ');
            if (delimit_index == string::npos) {
                // to do
                throw std::invalid_argument("bad cmd");
            } else {
            }
        }

        void Dispatch(tcp::socket& socket, int cmd, string args) {

        }

        void HandleClient(tcp::socket& socket) {
            LogClientAction(socket, "sign in");

            error_code ec;
            streambuf buff;
            std::istream is(&buff);
            string line;

            while (true) {
                read_until(socket, buff, '\n', ec);
                std::getline(is, line);
                
                try {
                    auto cmd_args = ResolveCMD(line);
                    Dispatch(socket, cmd_args.first, cmd_args.second);
                } catch (exception& e) {
                    // to do
                    Error(e.what());
                    break;
                }
                if (ec) {
                    // to do
                    Error(ec.message());
                }
            }
            socket.close();
        }


        void LogClientAction(tcp::socket& socket, const char* msg) {
            ip::address address = socket.remote_endpoint().address();
            string info = address.to_string() + ": " + msg;
            logger_.Log(info, Logger::INFO);
        }

        void Error(const char* msg) {
            logger_.Log(msg, Logger::ERROR);
        }

        void Error(const string& msg) {
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
