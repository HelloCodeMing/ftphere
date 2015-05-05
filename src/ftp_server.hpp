/*
 * ftphere
 * Start a ftp server monitor the current directory. 
 * It could be used to transport files conveniently.
 */

#define FTPSERVER_HPP

#include <boost/asio.hpp>
#include <iostream>
#include <exception>
#include <unistd.h>

#ifndef LOGGER_HPP
#define LOGGER_HPP
#include "logger.hpp"
#endif

#ifndef CMD_HPP
#define CMD_HPP
#include "cmd.hpp"
#endif

#ifndef UTIL_HPP
#define UTIL_HPP
#include "util.hpp"
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
            logger_("./ftpserver.log"),
            getcwd(cwd_, 1024) {
            
        }

        ~FTPServer() {

        }

        void Run() {
            error_code ec;
            tcp::socket remote_socket(ios_);

            /* init the acceptor */
            try {
                acceptor_.open(tcp::v4());
                acceptor_.bind(local_ep_);
                acceptor_.listen();
            } catch (exception& e) {
                Error(e.what());
                exit(1);
            }

            while (true) {
                try {
                    acceptor_.accept(remote_socket, ec);
                    HandleClient(remote_socket);
                } catch (exception& e) {
                    Error(e.what());
                }
            }
        }


    private:
        void HandleClient(tcp::socket& socket) {
            LogClientAction(socket, "sign in");

            error_code ec;
            streambuf buff;
            std::istream is(&buff);
            string line;
            int endable = 0;

            while (!endable) {
                read_until(socket, buff, '\n', ec);
                std::getline(is, line);

                auto cmd_args = Split(line, ' ');
                endable = Dispatch(socket, cmd_args);
            }
            socket.close();
        }

        int Dispatch(tcp::socket& socket, std::vector<std::string>& cmd_args) {
            CMD cmd_code = ResolveCMD(cmd_args[0]);
            switch (cmd_code) {
                /* normal cmd */
                case CWD:
                    
                case HELP:
                case LIST:
                case MODE:
                case NLST:
                case NOOP:
                case PORT:
                case PASV:
                case PASS:
                case USER:
                case PWD:
                case RETR:
                case STRU:
                case TYPE:
                
                /* unsupported cmd */
                case DELE:
                case MKD:
                case PORT:
                case RMD:
                case STOR:
                    
                /* illegal cmd */
                default:
            }
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
        char cwd_[1024];
        tcp::endpoint local_ep_;
        io_service ios_;
        tcp::acceptor acceptor_;
        Logger logger_;
};
}
