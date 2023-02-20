#include "SerialPortSession.h"
#include <iostream>

SerialPortSession::SerialPortSession(const std::string& name, unsigned int baudRate): serialPort(ioService)
{
    boost::system::error_code ec;
    serialPort.open(name, ec);
    if (ec.failed())
    {
        std::cout << "Serial Port Open Failed" << std::endl;
        return;
    }
    serialPort.set_option(boost::asio::serial_port::baud_rate(baudRate));
    serialPort.set_option(boost::asio::serial_port::flow_control());
    serialPort.set_option(boost::asio::serial_port::parity());
    serialPort.set_option(boost::asio::serial_port::stop_bits());
    serialPort.set_option(boost::asio::serial_port::character_size(8));
}

SerialPortSession::~SerialPortSession()
{
    if (serialPort.is_open())
        serialPort.close();
}

void SerialPortSession::RunService()
{
    if (!serialPort.is_open())
    {
        std::cout << "Serial Port Open Failed" << std::endl;
        return;
    }
    StartAsyncRead();
    ioService.run();
}

void SerialPortSession::StartAsyncRead()
{
    if (needRead == 0)
    {
        switch (state)
        {
        case READY: 
        {
            offset = 0;
            needRead = 1;
            state = HEAD_0;
            break;
        }
        case HEAD_0:
        {
            if (buffer[offset - 1] != 0x0D)
            {
                needRead = 1;
                offset = 0;
                state = HEAD_0;
            }
            else
            {
                needRead = 1;
                state = HEAD_1;
            }
            break;
        }
        case HEAD_1:
        {
            if (buffer[offset - 1] != 0x0A)
            {
                needRead = 1;
                offset = 0;
                state = HEAD_0;
            }
            else
            {
                needRead = 1;
                state = HEAD_2;
            }
            break;
        }
        case HEAD_2:
        {
            if (buffer[offset - 1] != 0x7E)
            {
                needRead = 1;
                offset = 0;
                state = HEAD_0;
            }
            else
            {
                needRead = 1;
                state = TYPE;
            }
            break;
        }
        case TYPE:
        {
            needRead = 1;
            state = LENGTH;
            break;
        }
        case LENGTH:
        {
            if (buffer[offset - 1] < 0 || buffer[offset - 1] > 80)
            {
                needRead = 1;
                offset = 0;
                state = HEAD_0;
            }
            else
            {
                needRead = buffer[offset - 1];
                state = DATA;
            }
            break;
        }
        case DATA:
        {
            needRead = 1;
            state = CHECKSUM;
            break;
        }
        case CHECKSUM:
        {
            unsigned char checksum = 0;
            std::stringstream ss;
            offset -= 1;
            for (int i = 0; i < offset; ++i)
            {
                checksum += buffer[i];
                //ss << std::setfill('0') << std::setw(2) << std::hex << (unsigned int)buffer[i] << " ";
            }
            //ss << std::setfill('0') << std::setw(2) << std::hex << (unsigned int)buffer[offset] << " ";
            //std::cout << ss.str() << " | ";
            if (checksum != buffer[offset])
            {
                std::cout << "Serial Port: CheckSum Error" << std::endl;
            }
            else
            {
                DataProcess();
            }
            state = READY;
            break;
        }
        }
    }
    serialPort.async_read_some(boost::asio::buffer(buffer + offset, needRead), [this](const boost::system::error_code& ec, size_t bytes_transferred) {
        if (ec.failed())
        {
            std::cout << "Serial Port Error: " << ec.what() << std::endl;
        }
        needRead -= bytes_transferred;
        offset += bytes_transferred;
        StartAsyncRead();
    });
}

void SerialPortSession::DataProcess()
{
    switch (buffer[4])
    {
    case 0x12:
    {
        static constexpr char DORIENT_TEMPLATE[] = "Phi: %01f, Omega: %01f, Kappa: %01f | "
                                                   "accelerated_x: %01f, accelerated_y: %01f, accelerated_z: %01f | "
                                                   "magnetic_x: %01f, magnetic_y: %01f, magnetic_z: %01f";
        char result[200];
        float kappa = 360.0 * *(unsigned short*)(buffer + 5) / 65536;
        float omega = 360.0 * *(unsigned short*)(buffer + 7) / 65536;
        float phi = 360.0 * *(unsigned short*)(buffer + 9) / 65536;
        float accelerated_x = 360.0 * *(unsigned short*)(buffer + 11) / 65536;
        float accelerated_y = 360.0 * *(unsigned short*)(buffer + 13) / 65536;
        float accelerated_z = 360.0 * *(unsigned short*)(buffer + 15) / 65536;
        float magnetic_x = 360.0 * *(unsigned short*)(buffer + 17) / 65536;
        float magnetic_y = 360.0 * *(unsigned short*)(buffer + 19) / 65536;
        float magnetic_z = 360.0 * *(unsigned short*)(buffer + 21) / 65536;
        sprintf_s(result, DORIENT_TEMPLATE, phi, omega, kappa, accelerated_x, accelerated_y, accelerated_z, magnetic_x, magnetic_y, magnetic_z);
        std::cout << result << std::endl;
        break;
    }
    }
}

void SerialPortSession::write(std::string& buf, boost::system::error_code& ec)
{
    serialPort.write_some(boost::asio::buffer(buf), ec);
}