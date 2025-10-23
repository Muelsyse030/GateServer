#include "VerifyGrpcClient.h"
#include "ConfigMgr.h"
RPCClientPoll::RPCClientPoll(size_t poolsize, std::string host, std::string port) : _poolsize(poolsize) , _host(host) , _port(port) , _b_stop(false) {
	for (size_t i = 0 ; i < _poolsize ; i++) {
		std::shared_ptr<Channel> channel = grpc::CreateChannel(host + ":" + port,
			grpc::InsecureChannelCredentials());
		_connections.push(VarifyService::NewStub(channel));
	}
}
void RPCClientPoll::Close() {
	_b_stop = true;
	_cv.notify_all(); 
}

std::unique_ptr<VarifyService::Stub> RPCClientPoll::GetConnection() {
	std::unique_lock<std::mutex> lock(_mutex);
	_cv.wait(lock, [this]() {
		if (_b_stop) {
			return true;
		}
		return !_connections.empty();
		});
	if (_b_stop) {
		return nullptr;
	}
	auto context = std::move(_connections.front());
	_connections.pop();
	return context;
}
void RPCClientPoll::returnConnection(std::unique_ptr<VarifyService::Stub> context) {
	std::lock_guard<std::mutex> lock(_mutex);
	if (_b_stop) {
		return;
	}
	_connections.push(std::move(context));
	_cv.notify_one();

}
VerifyGrpcClient::VerifyGrpcClient() {
	auto& gCfgMgr = ConfigMgr::GetInstance();
	std::string host = gCfgMgr["VarifyServer"]["host"];
	std::string port = gCfgMgr["VarifyServer"]["port"];
	_pool.reset(new RPCClientPoll(5, host, port));
}