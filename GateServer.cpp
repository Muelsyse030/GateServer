#include "const.h"
#include "CServer.h"

int main(int argc, char* argv[]) {
    try {
        unsigned short port = static_cast<unsigned short>(8080);
        net::io_context ioc{1};
        boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc](const boost::system::error_code& ec, int signal_number) {
            if (ec) {
                return;
            }

            ioc.stop();

         });

        std::make_shared<CServer>(ioc, port)->Start();
        std::cout << "Gate Server listen on port:" << port << std::endl;
        ioc.run();
    }
    catch (std::exception const& ec) {
        std::cout << "ERROR" << ec.what() << std::endl;
        return EXIT_FAILURE;
    }
    return 0;
}