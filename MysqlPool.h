#pragma once
#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <string>

// Windows 下必须先包含 winsock2
#ifdef _WIN32
#include <winsock2.h>
#endif
#include <mysql/mysql.h>

class MysqlPool {
public:
    MysqlPool(const std::string& host, unsigned int port,
        const std::string& user, const std::string& pass,
        const std::string& schema, int poolSize);

    ~MysqlPool();

    // 获取一个原生 MySQL 连接
    MYSQL* getConnection();

    // 归还连接
    void returnConnection(MYSQL* conn);

    // 关闭连接池
    void close();

private:
    std::string host_;
    unsigned int port_;
    std::string user_;
    std::string pass_;
    std::string schema_;
    int poolSize_;

    std::queue<MYSQL*> pool_; // 存储原生指针
    std::mutex mutex_;
    std::condition_variable cond_;
    std::atomic<bool> stop_;
};