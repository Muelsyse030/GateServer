#include "CServer.h"
#include "HttpConnection.h"
#include "AsioIOServicePool.h"

CServer::CServer(boost::asio::io_context& ioc, unsigned short& port) : 
	_ioc(ioc) , _acceptor(ioc,tcp::endpoint(tcp::v4() , port)){
	
}
void CServer::Start() {
    auto self = shared_from_this();
    auto& io_context = AsioIOServicePool::GetInstance()->GetIOService();
    std::shared_ptr<HttpConnection> new_con = std::make_shared<HttpConnection>(io_context);

    _acceptor.async_accept(new_con->GetSocket(), [self, new_con](beast::error_code ec) {
        try {
            if (!ec) {
                std::cout << "[INFO] New client connected from: "
                    << new_con->GetSocket().remote_endpoint() << std::endl;
                new_con->Start();
            }
            else {
                std::cerr << "[ERROR] Accept failed: " << ec.message() << std::endl;
            }
        }
        catch (std::exception& exp) {
            std::cerr << "[EXCEPTION] " << exp.what() << std::endl;
        }

        
        self->Start();
        });
}

