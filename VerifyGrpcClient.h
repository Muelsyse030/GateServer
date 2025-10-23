#pragma once
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "const.h"
#include "Singleton.h"
#include <atomic>
#include <queue>

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;


using message::GetVarifyReq;
using message::GetVarifyRsp;
using message::VarifyService;

class RPCClientPoll {
public:
	RPCClientPoll(size_t poolsize , std::string host , std::string port);
	~RPCClientPoll() {
		std::lock_guard<std::mutex> lock(_mutex);
		while (!_connections.empty()) {
			_connections.pop();
		}
	}
	void Close();
	std::unique_ptr<VarifyService::Stub> GetConnection();
	void returnConnection(std::unique_ptr<VarifyService::Stub> context);
private:
	std::atomic<bool> _b_stop;
	size_t _poolsize;
	std::string _host;
	std::string _port;
	std::queue<std::unique_ptr<VarifyService::Stub>> _connections;
	std::mutex _mutex;
	std::condition_variable _cv;
};

class VerifyGrpcClient : public Singleton<VerifyGrpcClient>
{
	friend class Singleton<VerifyGrpcClient>;
public:
	GetVarifyRsp GetVarifyCode(std::string email) {
		ClientContext context;
		GetVarifyRsp reply;
		GetVarifyReq request;
		request.set_email(email);
		auto stub = _pool->GetConnection();
		Status status = stub->GetVarifyCode(&context, request, &reply);
		if (status.ok()) {
			_pool->returnConnection(std::move(stub));
			return reply;
		}
		else {
			reply.set_error(ErrorCode::RPCFailed);
			_pool->returnConnection(std::move(stub));
			return reply;
		}
	}
private:
	VerifyGrpcClient();
	std::unique_ptr<RPCClientPoll> _pool;
};
