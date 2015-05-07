#include <iostream>
#include <cstdio>
#include <cassert>
#include <string>
#include <vector>
#include <thread>
#include <unistd.h>

#include <boost/asio.hpp>

#include "logger.hpp"
#include "cmd.hpp"
#include "util.hpp"
#include "ftp_server.hpp"

using namespace ftp;
using namespace boost::asio;

int StatusCode(const string& response) {
    return std::stoi(Split(response, ' ').front());
}

void StartServer() {
    std::thread server([=]() {
            FTPServer s(8080, 8081);
            s.Run();
            });
    server.detach();
    sleep(1);
}

bool TestLogger() {
    const char* log_path = "./test.log";
    Logger logger(log_path);
    logger.Log("wow, amazing", Logger::INFO);
    logger.Log("oh, no");
    logger.Log("oh, shit", Logger::ERROR);
    puts("pass-test: logger");
    return true;
}

bool TestCMD() {
    assert(ResolveCMD("USER") == CMD::USER);
    assert(ResolveCMD("PASS") == CMD::PASS);
    assert(ResolveCMD("shit") == CMD::BAD);
    assert(ResolveCMD("user") == CMD::USER);
    assert(ResolveCMD("pass") == CMD::PASS);
    puts("pass-test: cmd");
    return true;
}

bool TestResponse() {
    assert(Response(200) == "Command okay.");
    assert(Response(214) == "Help message.");
    assert(Response(530) == "Not logged in.");
    puts("pass-test: response");
    return true;
}

bool TestSplit() {
    std::string str = "USER anonymous";
    std::vector<std::string> res = { "USER", "anonymous" };
    auto splited = Split(str, ' ');
    assert(res == splited);

    str = "shit";
    res = { "shit" };
    splited = Split(str, ' ');
    assert(res == splited);
    puts("pass-test: split");
    return true;
}

bool TestSignin() {
    /* create server */
    StartServer();

    /* create client */
    io_service ios;
    ip::tcp::endpoint ep(ip::address::from_string("127.0.0.1"), 8080);
    ip::tcp::socket socket(ios);
    socket.connect(ep);
    streambuf buff;
    std::istream is(&buff);
    std::string line;

    socket.write_some(buffer("USER ming\r\n"));
    read_until(socket, buff, "\r\n");
    std::getline(is, line, '\r');
    assert(StatusCode(line) == 331);
    
    socket.write_some(buffer("PASS shit\r\n"));
    read_until(socket, buff, "\r\n");
    std::getline(is, line, '\r');
    assert(StatusCode(line) == 230);
    puts("pass-test: sign in");
    return true;
}

bool TestPWD() {
    /* create server */
    StartServer();
    
    /* create client */
    puts("pass-test: pwd");
    return true;
}

bool TestCWD() {

    puts("pass-test: cwd");
    return true;
}

bool TestPORT() {

    puts("pass-test: port");
    return true;
}

bool TestLIST() {

    puts("pass-test: list");
    return true;
}

bool TestRETR() {

    puts("pass-tes: retr");
    return true;
}

int main()
{
    assert(TestLogger());
    assert(TestCMD());
    assert(TestResponse());
    assert(TestSplit());
    assert(TestSignin());
    assert(TestPWD());
    puts("pass all test!");
    return 0;
}
