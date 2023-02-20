#ifndef DATATHREAD_H
#define DATATHREAD_H

#include <chrono>
#include "TcpSocket.h"

void DataDealCX(TcpSocket&);
void DataDealZC(TcpSocket&);

std::chrono::time_point<std::chrono::system_clock> TimeStampToDateTime(const unsigned char* time);

#endif // DATATHREAD_H
