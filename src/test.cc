#include <iostream>
#include <cstdio>
#include <cassert>

#include "logger.hpp"
#include "cmd.hpp"

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
int main()
{
    assert(TestLogger());
    assert(TestCMD());
    assert(TestResponse());
    return 0;
}
