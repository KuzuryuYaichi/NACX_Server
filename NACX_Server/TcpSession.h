#ifndef TCP_SESSION_H
#define TCP_SESSION_H

#include <memory>
#include <thread>
#include <unordered_set>
#include "boost/asio.hpp"
#include "StructCmd.hpp"
#include "ThreadSafeQueue.h"
#include "StructNetData.h"

class TcpSession : public std::enable_shared_from_this<TcpSession>
{
public:
	TcpSession(boost::asio::ip::tcp::socket&&, std::unordered_set<TcpSession*>&, std::mutex&);
	TcpSession(const TcpSession&) = delete;
	TcpSession& operator=(const TcpSession&) = delete;
	TcpSession(TcpSession&&) = default;
	TcpSession& operator=(TcpSession&&) = default;
	~TcpSession();
	void read();
	void write(std::shared_ptr<StructNetData>);
	void RecvCommandFun(std::shared_ptr<Order>);
	void AddToSet();
	void RemoveFromSet();
	bool SendState = true;
	boost::asio::ip::tcp::socket socket;

protected:
	void StartWork();

private:
	bool isRunning = true;
	std::unordered_set<TcpSession*>& SessionSet;
	std::mutex& SessionSetMutex;
	std::mutex WriteLock;
	unsigned int TaskValue = 0;
	CmdProcess CmdControl;
	StructCmdCX& CmdCX;
	StructWorkCommandRev ReplayCommand;
	StructDeviceScheckRev ReplayScheck;
	size_t TransferByte = 0;

	void SelfCheck();
	bool SetCXParmCommand(const std::vector<std::string>& CommandName);
	void SetNBDataCommmand(const std::vector<std::string>& CommandName);
	void SetWBDataCommand(const std::vector<std::string>& CommandName);
	void SetSweepDataCommand(const std::vector<std::string>& CommandName);
	void SetTestDataCommand(const std::vector<std::string>& CommandName);

	void StartRevDataWork();
	void StopRevDataWork();
	void SetAppConfig();

	void ControlReplay(unsigned int, short, short);
	void WorkParmReplay(const StructWorkCommandRev&);
	void ScheckReplay(const StructDeviceScheckRev&);
};

#endif
