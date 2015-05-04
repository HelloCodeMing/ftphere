#include <boost/asio.hpp>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <algorithm>

using namespace boost::asio;

void DispatchReq(ip::tcp::socket& socket, int code) {
    switch (code) {
        case 100:
        case 200:
        case 300:
    }
}

void HandleClient(ip::tcp::socket& socket) {
    streambuf buff;
    std::istream is(&buff);
    std::string line;
    boost::system::error_code ec;
    
    do {
        read_until(socket, buff, '\n', ec);
        if (ec) break;
        std::getline(is, line);
        int code = ResolveCMD(line);
        DispatchReq(socket, code);
    } while (!ec);
    socket.close();
}

void Log(const char* message) {
    const char* log_file_path = "./ftphere.log";
    FILE* log_file = fopen(log_file_path, "a+");
    std::time_t now_ = std::time(NULL);
    char* now = std::ctime(&now_);

    fprintf(log_file, "%s\t%s\n", now, message);
    fprintf(stdout, "%s\t%s\n", now, message);
    fclose(log_file);
}

void Log(const std::string& message) {
    Log(message.data());
}

int main()
{
    FTPServer server;
    server.Run();
    io_service ios;
    int port = 8080;
    ip::tcp::endpoint ep(ip::tcp::v4(), port);
    ip::tcp::acceptor acceptor(ios, ep);
    ip::tcp::socket socket(ios);
    
    while (true) {
        acceptor.accept(socket);
        
        ip::tcp::endpoint remote_ep = socket.remote_endpoint();
        ip::tcp::address = remote_ep.address();
        unsigned short port = remote_ep.port();
        std::string visitor_info = address.to_string() + ":" + port;
        Log("new visitor:" + visitor_info);
        HandleClient(socket);
    }
    return 0;
}
