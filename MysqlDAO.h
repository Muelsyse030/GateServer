#pragma once
#include <mysqlx/xdevapi.h>
#include <memory>
#include "MysqlPool.h"
#include "ConfigMgr.h"

class MysqlDAO
{
public:
	MysqlDAO();
	~MysqlDAO();
	int reg_user(const std::string& name , const std::string& email , const std::string& pwd);
private:
	std::unique_ptr<MySqlPool> _pool;
};

