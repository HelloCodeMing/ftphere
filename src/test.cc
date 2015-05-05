#include <iostream>
#include <cstdio>
#include <cassert>
#include <string>
#include <vector>

#include "logger.hpp"
#include "cmd.hpp"
#include "util.hpp"

using namespace ftp;

bool TestLogger() {
    const char* log_path = "./test.log";
    Logger logger(log_path);
    logger.Log("wow, amazing", Logger::INFO);
    logger.Log("oh, shit", Logger::ERROR);
    puts("pass-test: logger");
    return true;
}

bool TestCMD() {
    assert(ResolveCMD("USER") == USER);
    assert(ResolveCMD("PASS") == PASS);
    assert(ResolveCMD("CWD") == CWD);
    assert(ResolveCMD("shit") == BAD);
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
    puts("pass-test: split");
    return true;
}

int main()
{
    assert(TestLogger());
    assert(TestCMD());
    assert(TestResponse());
    assert(TestSplit());
    return 0;
}
