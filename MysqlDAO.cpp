#include "MysqlDAO.h"
using namespace std;

MysqlDAO::MysqlDAO(){
    auto& cfg = ConfigMgr::GetInstance();
    const auto& host = cfg["Mysql"]["Host"];
    const auto& port = cfg["Mysql"]["Port"];
    const auto& pwd = cfg["Mysql"]["Passwd"];
    const auto& schema = cfg["Mysql"]["Schema"];
    const auto& user = cfg["Mysql"]["User"];
    // MySqlPool 构造函数需要 host(string), port(unsigned int), user, pass, schema, poolSize
    _pool.reset(new MySqlPool(host,static_cast<unsigned int>(std::stoi(port)),user,pwd,schema,5));
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