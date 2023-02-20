#ifndef _SERIAL_PORT_SESSION_H_
#define _SERIAL_PORT_SESSION_H_

#include "boost/asio.hpp"
#include <memory>
#include <thread>

class SerialPortSession: public std::enable_shared_from_this<SerialPortSession>
{
public:
    SerialPortSession(const std::string& name, unsigned int baudRate);
    ~SerialPortSession();
    void RunService();
    void write(std::string& buf, boost::system::error_code& ec);
    void StartAsyncRead();

private:
    void DataProcess();
    static constexpr int BUFFER_LEN = 90;
    boost::asio::io_service ioService;
    boost::asio::serial_port serialPort;
    unsigned char buffer[BUFFER_LEN];
    enum READ_STATE {
        READY,
        HEAD_0,
        HEAD_1,
        HEAD_2,
        TYPE,
        LENGTH,
        DATA,
        CHECKSUM
    };
    READ_STATE state = READY;
    int needRead = 0;
    int offset = 0;
};

#endif
