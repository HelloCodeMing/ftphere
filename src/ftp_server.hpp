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
#include <string>

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
            data_port_(20),
            local_ep_(tcp::v4(), port_),
            ios_(),
            acceptor_(ios_),
            logger_("./ftpserver.log") {
            getcwd(cwd_, 1024);
        }

        ~FTPServer() {

        }

        void Run() {
            error_code ec;
            tcp::socket remote_socket(ios_);
            tcp::endpoint data_ep(tcp::v4(), data_port_);
            tcp::socket data_socket(ios_, data_ep);

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
                    HandleClient(remote_socket, data_socket);
                } catch (exception& e) {
                    Error(e.what());
                    remote_socket.close();
                }
            }
        }


    private:
        void HandleClient(tcp::socket& ctl_socket, tcp::socket& data_socket) {
            LogClientAction(ctl_socket, "sign in");

            error_code ec;
            streambuf buff;
            std::istream is(&buff);
            string line;
            int endable = 0;

            while (!endable) {
                read_until(socket, buff, '\n', ec);
                std::getline(is, line);

                auto cmd_args = Split(line, ' ');
                endable = Dispatch(ctl_socket, data_socket, cmd_args);
            }
        }

        int Dispatch(tcp::socket& ctl_socket, tcp::socket& data_socket, std::vector<std::string>& cmd_args) {
            CMD cmd_code = ResolveCMD(cmd_args[0]);
            string res;
            switch (cmd_code) {
                /* normal cmd */
                case CWD: {
                    // todo 
                    strcat(cwd_, cmd_args[1].data());
                    res = "250 Directory successfully changed.\r\n";
                    ctl_socket.write_some(buffer(res));
                    break;
                          } 
                case HELP: {
                    res = "214 The following commands are recognized.";
                    res += "ABOR ABOR CWD HELP LIST MKD\r\n";
                    res += "MODE NLST NOOP PASS PASV PORT PWD QUIT RETR\r\n";
                    res += "SITE SIZE SMNT STAT STOR STRU SYST TYPE USER\r\n";
                    res += "214 Help okay\r\n";
                    ctl_socket.write_some(buffer(res));
                    break;
                           }
                case LIST: {
                    // todo
                           }
                case MODE:
                    // todo
                case NLST:
                    // todo
                case NOOP: {
                    res = "200 NOOP okay.\r\n";
                    ctl_socket.write_some(buffer(res));
                    break;
                           }
                case PASV: {
                    res = "227 Entering passive mode(";
                    int remote_port = rand() % 65536;
                    string address = ctl_socket.remote_endpoint().address().to_string();
                    std::replace(address.begin(), address.end(), '.', ',');
                    res += address + "," +
                        std::to_string(remote_port / 256) + "," + 
                        std::to_string(remote_port % 256) + ").\r\n";
                    ctl_socket.write_some(buffer(res));
                    break;
                           }
                case PASS: {
                    res = "230 User logged in successfully.\r\n";
                    ctl_socket.write_some(buffer(res));
                    break;
                           }
                case USER: {
                    res = "331 Please specify the password.\r\n";
                    ctl_socket.write_some(buffer(res));
                    break;
                           }
                case PWD: {
                    res = "257 ";
                    res += cwd_;
                    res += "\r\n";
                    ctl_socket.write_some(buffer(res));
                    break;
                          }
                case RETR: {
                    res = "150 Opening BINARY mode data connection for file.\r\n";
                    ctl_socket.write_some(buffer(res));
                    /* todo */
                    char buff[1024];
                    FILE* file = fopen(cmd_args[1].data(), "r");
                    size_t len;

                    while ((len = fread(buff, 1024, 1, file))) {
                        write(data_socket, buffer(buff, len));
                    }
                    fclose(file);
                    res = "226 Transfer complete.\r\n";
                    ctl_socket.write_some(buffer(res));
                    break;
                           }
                case STRU:
                    // todo
                case SYST: 
                    ctl_socket.write_some(buffer("215 UNIX Type.\r\n"));
                    break;
                case TYPE: {
                    if (cmd_args[1] == "I") {
                        res = "200 Switching to binary mode.\r\n";
                    } else {
                        res = "504 Command not implemented for the parameter.\r\n";
                    }
                    ctl_socket.write_some(buffer(res));
                    break;
                           }
                case PORT: {
                    auto args = Split(cmd_args[1], ',');
                    string address;
                    for (int i = 0; i < 4; i++)
                        address += args[i] + ".";
                    address.pop_back();
                    int port = stoi(args[4]) * 256 + stoi(args[5]);
                    tcp::endpoint ep(ip::address::from_string(address), port);
                    data_socket.connect(ep);

                    res = "200 PORT command successful.\r\n";
                    ctl_socket.write_some(buffer(res));
                    break;
                           }
                case QUIT: {
                    res = "221 Goodbye.\r\n";
                    ctl_socket.write_some(buffer(res));
                    return 1;
                           }
                
                /* unsupported cmd */
                case DELE:
                case MKD:
                case RMD:
                case STOR: {
                    ctl_socket.write_some(buffer("502 Command not implemented.\r\n"));
                    break;
                           }
                /* illegal cmd */
                default:
                    ctl_socket.write_some(buffer("500 Syntax error, command unrecognized.\r\n"));
            }
            return 0;
        }

        void OpenDataConnection(int port) {
            tcp::endpoint data_ep(ip::tcp::v4(), port);
            
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
        unsigned short data_port_;
        char cwd_[1024];
        tcp::endpoint local_ep_;
        io_service ios_;
        tcp::acceptor acceptor_;
        Logger logger_;
};
}
