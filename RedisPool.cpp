#include "RedisPool.h"

RedisPool::RedisPool(int pool_size, const char* host, int port, const char* password)
	: _pool_size(pool_size), _host(host), _port(port), _password(password), stop_(false) {
	try {
		for (int i = 0; i < _pool_size; i++) {
			auto* context = redisConnect(_host, _port);
			if (context == nullptr || context->err != 0) {
				if (context != nullptr) {
					redisFree(context);
				}
				continue;
			}
			auto reply = (redisReply*)redisCommand(context, "AUTH %s", _password);
			if (reply->type == REDIS_REPLY_ERROR) {
				std::cout << "认证失败" << std::endl;
				//执行成功 释放redisCommand执行后返回的redisReply所占用的内存
				freeReplyObject(reply);
				continue;
			}
			//执行成功 释放redisCommand执行后返回的redisReply所占用的内存
			freeReplyObject(reply);
			std::cout << "认证成功" << std::endl;
			_connections.push(context);
		}
	}
	catch (std::exception& e) {
		std::cout << "" << std::endl;
	}
}
RedisPool::~RedisPool() {
	Close();
}
redisContext* RedisPool::getConnection() {
	std::unique_lock<std::mutex> lock(mutex_);
	cond_.wait(lock, [this] {
		if (stop_) {
			return true;
		}
		return !_connections.empty();
		});

	if (stop_) return nullptr;
	auto* context = _connections.front();
	_connections.pop();
	return context;
}

void RedisPool::returnConnection(redisContext* con) {
	std::lock_guard<std::mutex> lock(mutex_);
	if (stop_) {
		return;
	}
	_connections.push(con);
	cond_.notify_one();
}

void RedisPool::Close() {
	stop_ = true;
	cond_.notify_all();

	std::lock_guard<std::mutex> lock(mutex_);
	while (!_connections.empty()) {
		auto* context = _connections.front();
		_connections.pop();
		if (context) {
			redisFree(context); // 关键：释放 hiredis 内存
		}
	}
}




