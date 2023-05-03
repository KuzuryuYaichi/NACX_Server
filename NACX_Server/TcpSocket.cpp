#include "TcpSocket.h"

boost::asio::ip::tcp::endpoint TcpSocket::GetHostAddress(const boost::asio::ip::tcp::endpoint& config)
{
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::resolver resolver(io_service);
    boost::asio::ip::tcp::resolver::query query(boost::asio::ip::host_name(), "");
    boost::asio::ip::tcp::resolver::iterator end;
    for (auto iter = resolver.resolve(query); iter != end; iter++)
    {
        if (config == *iter)
            return *iter;
    }
    return boost::asio::ip::tcp::endpoint();
}

void TcpSocket::async_accept()
{
    try
    {
        m_acceptor.async_accept(m_socket, [this](const boost::system::error_code& err)
        {
            if (err.failed())
            {
                std::cout << "Accept Failed: " << err.what() << std::endl;
            }
            auto session = std::make_shared<TcpSession>(std::move(m_socket), SessionSet, SessionSetMutex);
            session->AddToSet();
            async_accept();
        });
    }
    catch (const std::exception& e)
    {
        std::cout << "Accept Exception:" << e.what() << std::endl;
    }
}

TcpSocket::TcpSocket(const std::string& ip, const unsigned short& port) : m_acceptor(ioService, { boost::asio::ip::tcp::v4(), 5021 }), m_socket(ioService)
{
    m_acceptor.listen(MAX_CONNECTIONS);
    async_accept();
}

void TcpSocket::Run()
{
    ioService.run();
}

void TcpSocket::SendMsg(const std::unique_ptr<StructNetData>& data)
{
    std::lock_guard<std::mutex> lk(SessionSetMutex);
    for (auto& session : SessionSet)
    {
        session->write(data);
    }
}
