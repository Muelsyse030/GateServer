#pragma once
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <atomic>
#include "Singleton.h"
#include <hiredis/hiredis.h>

class RedisPool
{
public:
	RedisPool(int pool_size, const char* host, int port, const char* password);
	~RedisPool();
	redisContext* getConnection();
	void returnConnection(redisContext* con);
	void Close();
private:
	int _pool_size;
	const char* _host;
	int _port;
	const char* _password; 
	std::queue<redisContext*> _connections;

	std::mutex mutex_;
	std::condition_variable cond_;
	std::atomic<bool> stop_;
};

