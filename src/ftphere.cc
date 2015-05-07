#define FTPHERE_HPP

#include "ftp_server.hpp"

using namespace boost::asio;

int main()
{
    ftp::FTPServer server;
    server.Run();
    return 0;
}
