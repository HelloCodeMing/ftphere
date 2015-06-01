/*
 * ftphere
 * Start a ftp server monitor the current directory. 
 * It could be used to transport files conveniently.
 */

#ifndef FTPSERVER_HPP
#define FTPSERVER_HPP

#include <iostream>
#include <exception>
#include <unistd.h>
#include <string>

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

#include "logger.hpp"
#include "cmd.hpp"
#include "util.hpp"

namespace ftp {

using namespace boost::asio;
using namespace boost::filesystem;
using boost::asio::ip::tcp;
using std::string;
using std::exception;
using boost::system::error_code;

typedef unsigned short ushort;

class FTPServer {
    public:

        FTPServer(ushort ctl_port = 21, ushort data_port = 20): port_(ctl_port), data_port_(data_port) {}

        ~FTPServer() {
            if (acceptor_.is_open()) 
                acceptor_.close();
            if (!ios_.stopped())
                ios_.stop();
        }

        void Run() {
            logger_.Log("The server starting.", Logger::INFO);
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

            logger_.Log("The server started.", Logger::INFO);
            while (true) {
                Reset();
                try {
                    tcp::socket remote_socket(ios_);

                    acceptor_.accept(remote_socket);
                    HandleClient(remote_socket, data_socket);
                    remote_socket.close();
                    data_socket.close();
                } catch (exception& e) {
                    char buff[128];
                    sprintf(buff, "%s: line %d: %s", __FILE__, __LINE__, e.what());
                    Error(buff);
                }
            }
            logger_.Log("The server ended.", Logger::INFO);
        }


    private:
        void Reset() {
            current_dir_ = "/";
        }

        void HandleClient(tcp::socket& ctl_socket, tcp::socket& data_socket) {
            string address = ctl_socket.remote_endpoint().address().to_string();
            LogClientAction(address + " sign in.");

            error_code ec;
            bool stat = true;

            /* say hello */
            Greet(ctl_socket);

            /* read, resolve, response loop */
            while (stat && ctl_socket.is_open() && !ec) {
                string line = ReadLine(ctl_socket, ec);

                auto cmd_args = Split(line, ' ');
                LogClientAction(address + " " + line);
                stat = Dispatch(ctl_socket, data_socket, cmd_args);
            }
            LogClientAction(address + " sign out.");
        }

        void Greet(tcp::socket& socket) {
            socket.write_some(buffer("220 Hello\r\n"));
        }

        bool Dispatch(tcp::socket& ctl_socket, tcp::socket& data_socket, std::vector<std::string>& cmd_args) {
            CMD cmd_code = ResolveCMD(cmd_args[0]);
            boost::system::error_code ec;
            string res;
            switch (cmd_code) {
                /* normal cmd */
                case CWD: {
                    current_dir_ = cmd_args[1];
                    res = "250 Directory successfully changed.\r\n";
                    ctl_socket.write_some(buffer(res), ec);
                    break;
                          } 
                case HELP: {
                    res = "214 The following commands are recognized.";
                    res += "ABOR ABOR CWD HELP LIST MKD\r\n";
                    res += "MODE NLST NOOP PASS PASV PORT PWD QUIT RETR\r\n";
                    res += "SITE SIZE SMNT STAT STOR STRU SYST TYPE USER\r\n";
                    res += "214 Help okay\r\n";
                    ctl_socket.write_some(buffer(res), ec);
                    break;
                           }
                case LIST: {
                    if (data_socket.is_open()) {
                        res = "150 Here comes the directory listing.\r\n";
                        path file_path = root_dir_;
                        file_path /= cmd_args[1];
                        ctl_socket.write_some(buffer(res), ec);
                        std::for_each(directory_iterator(file_path),
                                      directory_iterator(),
                                      [&](auto& entry) {
                                        data_socket.write_some(buffer(make_file_info(entry) + "\r\n"), ec);
                                      });
                        res = "226 Directory send OK.\r\n";
                    } else {
                        res = "425 Use PORT or PASV first.\r\n";
                    }
                    ctl_socket.write_some(buffer(res), ec);
                    break;
                           }
                case NOOP: {
                    res = "200 NOOP okay.\r\n";
                    ctl_socket.write_some(buffer(res), ec);
                    break;
                           }
                case PASS: {
                    res = "230 User logged in successfully.\r\n";
                    ctl_socket.write_some(buffer(res), ec);
                    break;
                           }
                case USER: {
                    res = "331 Please specify the password.\r\n";
                    ctl_socket.write_some(buffer(res), ec);
                    break;
                           }
                case PWD: {
                    res += "257 ";
                    res += current_dir_.string();
                    res += "\r\n";
                    ctl_socket.write_some(buffer(res), ec);
                    break;
                          }
                case RETR: {
                    char buff[1024];
                    path file_path = root_dir_;
                    file_path /= cmd_args[1];
                    
                    // exists or not?
                    if (exists(file_path)) {
                        res = "150 Opening BINARY mode data conenction for file.\r\n";
                        ctl_socket.write_some(buffer(res), ec);
                        FILE* file = fopen(file_path.c_str(), "r");
                        size_t len;
                        size_t transfer_cnt(0);
                        
                        while (!feof(file)) {
                            len = fread(buff, 1, 1024, file);
                            transfer_cnt += len;
                            write(data_socket, buffer(buff, len), ec);
                        }
                        fclose(file);
                        if (data_socket.is_open())
                            data_socket.close();
                        res = "226 Transfer complete";
                        res += string("(") + std::to_string(transfer_cnt) + "bytes transfered).\r\n";
                    } else {
                        res = "550 File not exists.\r\n";
                    }
                    ctl_socket.write_some(buffer(res), ec);
                    break;
                           }
                case SYST: 
                    ctl_socket.write_some(buffer("215 UNIX Type.\r\n"), ec);
                    break;
                case TYPE: {
                    if (cmd_args[1] == "I") {
                        res = "200 Switching to binary mode.\r\n";
                    } else {
                        res = "504 Command not implemented for the parameter.\r\n";
                    }
                    ctl_socket.write_some(buffer(res), ec);
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
                    data_socket.connect(ep, ec);

                    if (ec) {
                        res = "425 Can't open data connection.\r\n";
                    } else {
                        res = "200 PORT command successful.\r\n";
                    }
                    ctl_socket.write_some(buffer(res), ec);
                    break;
                           }
                case QUIT: {
                    res = "221 Goodbye.\r\n";
                    ctl_socket.write_some(buffer(res), ec);
                    return false;
                           }
                
                /* unsupported cmd */
                case STRU:
                case MODE:
                case PASV:
                case NLST:
                case DELE:
                case MKD:
                case RMD:
                case STOR: {
                    ctl_socket.write_some(buffer("502 Command not implemented.\r\n"), ec);
                    break;
                           }
                /* illegal cmd */
                default:
                    ctl_socket.write_some(buffer("500 Syntax error, command unrecognized.\r\n"), ec);
            }
            if (ec) return false;
            return true;
        }

        void LogClientAction(const string& msg) {
            logger_.Log(msg, Logger::INFO);
        }


        void Error(const string& msg) {
            logger_.Log(msg, Logger::ERROR);
        }

        /* members */
        /* logger and path */
        const char* log_path_ = "./ftpd.log";
        Logger logger_{ log_path_ };
        const path root_dir_ = current_path();
        path current_dir_;

        /* connection parameters */
        const unsigned short port_;
        const unsigned short data_port_;
        tcp::endpoint local_ep_{ tcp::v4(), port_ };
        io_service ios_{ };
        tcp::acceptor acceptor_{ ios_ };
};
} //end of namespace ftp
#endif
