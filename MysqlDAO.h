#pragma once
#include <memory>
#include <string>
#include "MysqlPool.h"

class MysqlDAO
{
public:
    MysqlDAO();
    ~MysqlDAO();
    int reg_user(const std::string& name, const std::string& email, const std::string& pwd);
private:
    std::unique_ptr<MysqlPool> _pool;
};