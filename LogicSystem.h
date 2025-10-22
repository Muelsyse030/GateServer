#pragma once
#include "const.h"
#include <map>
#include "singleton.h"
class HttpConnection;
class LogicSystem;
typedef std::function<void(std::shared_ptr<HttpConnection>)> HttpHandler;
class LogicSystem : public Singleton<LogicSystem> {
	friend class Singleton<LogicSystem>;
public:
	~LogicSystem() {
	}
	bool HandleGet(std::string path, std::shared_ptr<HttpConnection> connection);
	bool HandlePost(std::string path, std::shared_ptr<HttpConnection> connection);
	void RegGet(std::string url, HttpHandler handler);
	void RegPost(std::string url, HttpHandler handler);
private:
	LogicSystem();
	std::map< std::string, HttpHandler> _post_handlers;
	std::map< std::string, HttpHandler> _get_handlers;
};

