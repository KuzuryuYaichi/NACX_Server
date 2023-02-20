#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include <iostream>
#include <algorithm>
#include <thread>
#include <regex>
#include <unordered_set>
#include "boost/asio.hpp"
#include "TcpSession.h"

class TcpSocket
{
public:
	explicit TcpSocket(const std::string&, const unsigned short&);
	void SendMsg(std::shared_ptr<StructNetData> data);
	void FixedCXDataReplay(const StructFixedCXResult&, std::shared_ptr<StructNetData>&, size_t, unsigned short);
	void FixedCXDataReplay(const StructFixedCXResult&, std::shared_ptr<StructNetData>&, size_t);
	void NBZCDataReplay(const StructNBWaveZCResult&, std::shared_ptr<StructNetData>&, size_t, unsigned char);
	void NBCXDataReplay(const StructNBCXResult&, std::shared_ptr<StructNetData>&, size_t);
	void SweepCXDataReplay(const StructSweepCXResult&, std::shared_ptr<StructNetData>&, size_t);
	void TestCXDataReplay(const StructTestCXResult&, std::shared_ptr<StructNetData>&, size_t);
	void Run();

private:
	static constexpr int MAX_CONNECTIONS = 1;
	boost::asio::io_service ioService;
	boost::asio::ip::tcp::acceptor m_acceptor;
	boost::asio::ip::tcp::socket m_socket;
	std::unordered_set<TcpSession*> SessionSet;
	std::mutex SessionSetMutex;
	boost::asio::ip::tcp::endpoint GetHostAddress(const boost::asio::ip::tcp::endpoint&);
	void async_accept();
};

#endif
