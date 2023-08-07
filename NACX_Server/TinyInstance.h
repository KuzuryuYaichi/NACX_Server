#ifndef _TINY_INSTANCE_H
#define _TINY_INSTANCE_H

#include "TcpSocket.h"
#include "TinyConfig.h"
#include "boost/asio.hpp"
#include "SerialPortSession.h"

class TinyInstance
{
public:
    TinyInstance();
    ~TinyInstance();
    void join();
    
private:
    TinyConfig tinyConfig;
    TcpSocket ServerSocket;
    SerialPortSession SerialPort;

    std::thread DataThreadCX;
    std::thread DataThreadZC;

    void InitThread();
};

#endif
