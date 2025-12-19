#include "MysqlDAO.h"
#include "ConfigMgr.h"
#include <iostream>

MysqlDAO::MysqlDAO() {
    auto& cfg = ConfigMgr::GetInstance();
    const std::string host = cfg["Mysql"]["host"];
    const std::string port_str = cfg["Mysql"]["port"];
    const std::string pwd = cfg["Mysql"]["passwd"];
    const std::string schema = cfg["Mysql"]["schema"];
    const std::string user = cfg["Mysql"]["user"];

    unsigned int port = 3306; // 默认值
    if (!port_str.empty()) {
        port = std::stoi(port_str);
    }

    _pool.reset(new MysqlPool(host, port, user, pwd, schema, 5));
}

MysqlDAO::~MysqlDAO() {
    // 智能指针会自动析构 Pool
}

int MysqlDAO::reg_user(const std::string& name, const std::string& email, const std::string& pwd) {
    MYSQL* conn = _pool->getConnection();
    if (!conn) {
        return -1;
    }

    // 1. 构造 SQL 语句
    // 注意：实际生产中应使用 mysql_real_escape_string 防止 SQL 注入
    std::string sql = "CALL reg_user('" + name + "','" + email + "','" + pwd + "', @result)";

    // 2. 执行存储过程
    if (mysql_query(conn, sql.c_str())) {
        std::cerr << "CALL reg_user failed: " << mysql_error(conn) << std::endl;
        _pool->returnConnection(conn);
        return -1;
    }

    // 3. 重要：消耗所有结果集
    // 存储过程调用后，必须把所有产生的结果集（包括受影响行数等）都读完
    // 否则下一次执行查询时会报 "Commands out of sync"
    do {
        MYSQL_RES* res = mysql_store_result(conn);
        if (res) {
            mysql_free_result(res); // 我们这里不关心 CALL 本身的返回表，只关心 @result
        }
    } while (mysql_next_result(conn) == 0);

    // 4. 查询输出变量 @result
    if (mysql_query(conn, "SELECT @result")) {
        std::cerr << "SELECT @result failed: " << mysql_error(conn) << std::endl;
        _pool->returnConnection(conn);
        return -1;
    }

    // 5. 获取结果
    MYSQL_RES* res = mysql_store_result(conn);
    int result_id = -1;
    if (res) {
        MYSQL_ROW row = mysql_fetch_row(res);
        if (row && row[0]) {
            result_id = std::atoi(row[0]); // 将字符串转为 int
        }
        mysql_free_result(res);
    }

    // 6. 归还连接
    _pool->returnConnection(conn);
    return result_id;
}