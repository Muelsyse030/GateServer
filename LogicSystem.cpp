
#include "LogicSystem.h"
#include "HttpConnection.h"

void LogicSystem::RegGet(std::string url, HttpHandler handler) {
    _get_handlers.insert(std::make_pair(url , handler));
}

LogicSystem::LogicSystem() {
    RegGet("/get_test", [](std::shared_ptr<HttpConnection> connection) {
        beast::ostream(connection->_response.body()) << " ";
    });
}
bool LogicSystem::HandleGet(std::string path, std::shared_ptr<HttpConnection> connection) {
    if (_get_handlers.find(path) == _get_handlers.end()) {
        return false;
    }
    _get_handlers[path](connection);
   return true;
}