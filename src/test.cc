#include <iostream>
#include <cstdio>

#include "logger.hpp"

using namespace ftp;

int main()
{
    const char* log_path = "./test.log";
    Logger logger(log_path);
    logger.Log("wow, amazing", Logger::INFO);
    return 0;
}
