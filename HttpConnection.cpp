#include "HttpConnection.h"
#include "LogicSystem.h"

HttpConnection::HttpConnection(tcp::socket socket) : _socket(std::move(socket)){

}
void HttpConnection::Start() {
	auto self = shared_from_this();
	http::async_read(_socket, _buffer, _request, [self](beast::error_code ec , std::size_t bytes_tansferred) {
		try {
			if (ec) {
				std::cout << "http read is error" << ec.what() << std::endl;
				return;
			}
			boost::ignore_unused(bytes_tansferred);
			self->HandleRequest();
			self->CheekDeadLine();
		}
		catch (std::exception e) {
			std::cout << "http read exception:" << e.what() << std::endl;
		}
		});
}

void HttpConnection::CheekDeadLine() {
	auto self = shared_from_this();
	_deadline.async_wait([self](beast::error_code ec) {
		if (!ec) {
			self->_socket.close(ec);
		}
		});
}
void HttpConnection::WriteResponse() {
	auto self = shared_from_this();
	_response.content_length(_response.body().size());
	http::async_write(_socket, _response , [self](beast::error_code ec, std::size_t bytes_tansferred) {
		self->_socket.shutdown(tcp::socket::shutdown_send , ec);
		self->_deadline.cancel();
		});
}
void HttpConnection::HandleRequest() {
	//ÉèÖÃ°æ±¾
	_response.version(_request.version());
	_response.keep_alive(false);
		
	if (_request.method() == http::verb::get) {
		bool sucsess = LogicSystem::GetInstance()->HandleGet(_request.target(), shared_from_this());
		if (!sucsess) {
			_response.result(http::status::not_found);
			_response.set(http::field::content_type, "text/pain");
			beast::ostream(_response.body()) << "url not found\r\n";
			WriteResponse();
		}
		_response.result(http::status::ok);
		_response.set(http::field::server, "GateServer");
		WriteResponse();
		return;
	}
}