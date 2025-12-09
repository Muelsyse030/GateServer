#pragma once
#include <iostream>
#include <mysqlx/xdevapi.h>
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <atomic>

class MySqlPool {
public:
    MySqlPool(const std::string& host, unsigned int port,
        const std::string& user, const std::string& pass,
        const std::string& schema, int poolSize)
        : host_(host), port_(port), user_(user), pass_(pass),
        schema_(schema), poolSize_(poolSize), stop_(false)
    {
        try {
            for (int i = 0; i < poolSize_; ++i) {
                auto session = std::make_unique<mysqlx::Session>(host_, port_, user_, pass_);
                session->sql("USE " + schema_).execute();

                session->sql("SELECT 1").execute();
                std::cout << "MySQL connection[" << i << "] OK" << std::endl;

                pool_.push(std::move(session));
            }
            std::cout << "MySQL X DevAPI pool initialized with " << poolSize_ << " connections.\n";
        }
        catch (const mysqlx::Error& err) {
            std::cerr << "MySQL pool init failed: " << err.what() << std::endl;
        }
        catch (std::exception& ex) {
            std::cerr << "Exception: " << ex.what() << std::endl;
        }
    }

    std::unique_ptr<mysqlx::Session> getConnection() {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this] { return stop_ || !pool_.empty(); });

        if (stop_) return nullptr;

        auto conn = std::move(pool_.front());
        pool_.pop();
        return conn;
    }

    void returnConnection(std::unique_ptr<mysqlx::Session> conn) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (stop_) return;

        pool_.push(std::move(conn));
        cond_.notify_one();
    }

    void close() {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            stop_ = true;
            while (!pool_.empty()) {
                pool_.pop();
            }
        }
        cond_.notify_all();
        std::cout << "MySQL connection pool closed.\n";
    }

    ~MySqlPool() {
        close();
    }

private:
    std::string host_;
    unsigned int port_;
    std::string user_;
    std::string pass_;
    std::string schema_;
    int poolSize_;

    std::queue<std::unique_ptr<mysqlx::Session>> pool_;
    std::mutex mutex_;
    std::condition_variable cond_;
    std::atomic<bool> stop_;
};
