#include <iostream>
#include <cstdio>
#include <cassert>

#include "logger.hpp"

using namespace ftp;

bool TestLogger() {
    const char* log_path = "./test.log";
    Logger logger(log_path);
    logger.Log("wow, amazing", Logger::INFO);
    return true;
}
int main()
{
    assert(TestLogger());
    return 0;
}
