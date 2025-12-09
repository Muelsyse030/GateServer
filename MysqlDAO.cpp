#include "MysqlDAO.h"
using namespace std;

MysqlDAO::MysqlDAO(){
    auto& cfg = ConfigMgr::GetInstance();
    const std::string host = cfg["Mysql"]["host"];
    const std::string port_str = cfg["Mysql"]["port"];
    const std::string pwd = cfg["Mysql"]["passwd"];
    const std::string schema = cfg["Mysql"]["schema"];
    const std::string user = cfg["Mysql"]["user"];

    if (port_str.empty()) {
        throw std::runtime_error("Config error: missing Mysql.Port");
    }

    unsigned long port_ul = 0;
    try {
        size_t idx = 0;
        port_ul = std::stoul(port_str, &idx, 10);
        if (idx != port_str.length()) {
            throw std::invalid_argument("Mysql.Port contains invalid characters");
        }
        if (port_ul > 65535) {
            throw std::out_of_range("Mysql.Port out of range");
        }
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Invalid Mysql.Port in config: ") + e.what());
    }

    _pool.reset(new MySqlPool(host, static_cast<unsigned int>(port_ul), user, pwd, schema, 5));
}
MysqlDAO::~MysqlDAO(){
    _pool->close();
}
int MysqlDAO::reg_user(const std::string& name, const std::string& email, const std::string& pwd){
    auto con = _pool->getConnection();
    try {
        if (!con) return -1;

        std::string sqlcall = 
            "CALL reg_user('" + name + "','" + email + "','" + pwd + "', @result)";
           
        con->sql(sqlcall).execute();
        auto res = con->sql("SELECT @Result").execute();
        auto row = res.fetchOne();
        int result = row[0].get<int>();
        _pool->returnConnection(std::move(con));
        return result;
    }
    catch (const mysqlx::Error err) {
        std::cerr << "ERROR: " << err.what() << std::endl;
        _pool->returnConnection(std::move(con));
        return -1;
    }
    catch (std::exception& ex) {
        std::cerr << "STD EXCEPTION: " << ex.what() << std::endl;
        _pool->returnConnection(std::move(con));
        return -1;
    }
}