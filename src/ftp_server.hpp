/*
 * ftphere
 * Start a ftp server monitor the current directory. 
 * It could be used to transport files conveniently.
 */

#define FTPSERVER_HPP

#include <iostream>
#include <exception>
#include <unistd.h>
#include <string>

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

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
using namespace boost::filesystem;
using boost::asio::ip::tcp;
using std::string;
using std::exception;
using boost::system::error_code;

typedef unsigned short ushort;

class FTPServer {
    public:

        FTPServer(ushort ctl_port = 21, ushort data_port = 20): port_(ctl_port), data_port_(data_port) {}

        void Run() {
            logger_.Log("The server starting.", Logger::INFO);
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

            logger_.Log("The server started.", Logger::INFO);
            while (true) {
                try {
                    acceptor_.accept(remote_socket, ec);
                    HandleClient(remote_socket, data_socket);
                    remote_socket.close();
                    data_socket.close();
                } catch (exception& e) {
                    Error(e.what());
                    remote_socket.close();
                }
            }
            logger_.Log("The server ended.", Logger::INFO);
        }


    private:
        void HandleClient(tcp::socket& ctl_socket, tcp::socket& data_socket) {
            LogClientAction(ctl_socket, "sign in");

            error_code ec;
            int endable = 0;

            while (!endable && ctl_socket.is_open()) {
                streambuf buff;
                std::istream is(&buff);
                string line;

                read_until(ctl_socket, buff, "\r\n", ec);
                std::getline(is, line, '\r');

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
                    current_dir_ = cmd_args[1];
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
                    if (data_socket.is_open()) {
                        res = "150 Here comes the directory listing.\r\n";
                        ctl_socket.write_some(buffer(res));
                        std::for_each(directory_iterator(current_dir_),
                                      directory_iterator(),
                                      [&](auto& entry) {
                                        data_socket.write_some(buffer(make_file_info(entry) + "\r\n"));
                                      });
                        res = "226 Directory send OK.\r\n";
                    } else {
                        res = "425 Use PORT or PASV first.\r\n";
                    }
                    ctl_socket.write_some(buffer(res));
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
                    res += "257 ";
                    res += current_dir_.string();
                    res += "\r\n";
                    ctl_socket.write_some(buffer(res));
                    break;
                          }
                case RETR: {
                    //res = "150 Opening BINARY mode data connection for file.\r\n";
                    //ctl_socket.write_some(buffer(res));
                    char buff[1024];
                    path file_path = root_dir_;
                    file_path /= cmd_args[1];
                    
                    // exists or not?
                    if (exists(file_path)) {
                        res = "150 Opening BINARY mode data conenction for file.\r\n";
                        ctl_socket.write_some(buffer(res));
                        FILE* file = fopen(file_path.c_str(), "r");
                        size_t len;
                        size_t transfer_cnt(0);
                        
                        while ((len = fread(buff, 1024, 1, file))) {
                            transfer_cnt += len;
                            write(data_socket, buffer(buff, len));
                        }
                        fclose(file);
                        res = "226 Transfer complete";
                        res += string("(") + std::to_string(transfer_cnt) + "bytes transfered).\r\n";
                    } else {
                        res = "550 File not exists.\r\n";
                    }
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
                    boost::system::error_code ec;
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


        void Error(const string& msg) {
            logger_.Log(msg, Logger::ERROR);
        }

        const char* log_path_ = "./ftpd.log";
        Logger logger_{ log_path_ };
        const unsigned short port_;
        const unsigned short data_port_;
        const path root_dir_ = current_path();
        path current_dir_ = "/";
        tcp::endpoint local_ep_{ tcp::v4(), port_ };
        io_service ios_{ };
        tcp::acceptor acceptor_{ ios_ };
};
}
