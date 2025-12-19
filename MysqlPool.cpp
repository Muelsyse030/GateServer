#include "MysqlPool.h"
#include <iostream>

MysqlPool::MysqlPool(const std::string& host, unsigned int port,
    const std::string& user, const std::string& pass,
    const std::string& schema, int poolSize)
    : host_(host), port_(port), user_(user), pass_(pass),
    schema_(schema), poolSize_(poolSize), stop_(false)
{
    for (int i = 0; i < poolSize_; ++i) {
        MYSQL* conn = mysql_init(nullptr);
        if (!conn) {
            std::cerr << "Mysql init failed!" << std::endl;
            continue;
        }

        // 开启自动重连 (非常重要)
        char value = 1;
        mysql_options(conn, MYSQL_OPT_RECONNECT, &value);
        // 设置字符集 (可选)
        mysql_options(conn, MYSQL_SET_CHARSET_NAME, "utf8mb4");

        // 建立连接
        if (!mysql_real_connect(conn, host_.c_str(), user_.c_str(), pass_.c_str(),
            schema_.c_str(), port_, nullptr, 0)) {
            std::cerr << "MySQL connection failed: " << mysql_error(conn) << std::endl;
            mysql_close(conn); // 连接失败要释放
            continue;
        }

        std::cout << "MySQL connection[" << i << "] OK" << std::endl;
        pool_.push(conn);
    }
}

MysqlPool::~MysqlPool() {
    close();
}

MYSQL* MysqlPool::getConnection() {
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [this] {
        if (stop_) return true;
        return !pool_.empty();
        });

    if (stop_ || pool_.empty()) return nullptr;

    MYSQL* conn = pool_.front();
    pool_.pop();

    // 简单的保活检查
    if (mysql_ping(conn) != 0) {
        std::cerr << "MySQL connection lost, trying to reconnect..." << std::endl;
        // 由于开启了 RECONNECT 选项，ping 会尝试重连
        // 如果彻底断开，这里可能需要更复杂的重连逻辑，但通常足够
    }

    return conn;
}

void MysqlPool::returnConnection(MYSQL* conn) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (stop_) {
        mysql_close(conn);
        return;
    }
    pool_.push(conn);
    cond_.notify_one();
}

void MysqlPool::close() {
    std::unique_lock<std::mutex> lock(mutex_);
    stop_ = true;
    while (!pool_.empty()) {
        MYSQL* conn = pool_.front();
        pool_.pop();
        mysql_close(conn); // 显式关闭 C API 连接
    }
    cond_.notify_all();
    std::cout << "MySQL connection pool closed.\n";
}