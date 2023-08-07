#ifndef _STREAM_PUBLIC_H
#define _STREAM_PUBLIC_H

#include <iostream>
#include <thread>

#include "dllexport.h"

#ifdef XDMA_PCIE_WIN_LIB
#include "struct_device_win.h"
#include "stream_dma_win.hpp"
#elif defined XDMA_PCIE_UNIX_LIB
#include "struct_device_unix.h"
#include "stream_dma_unix.hpp"
#endif

bool isRunning = true;

void SetIsRunning(bool state)
{
    isRunning = state;
}

size_t TransferByte_CX = 0, TransferByte_ZC = 0, CX_Count = 0, ZC_Count = 0;

void ThreadProcDataCX(xdma_device& dev, P_CXDATA_CALLBACK& CallBack, int channel, size_t& TransferByte, const size_t PACK_LEN, const int PACK_NUM)
{
    const size_t BLOCK_LEN = PACK_LEN * PACK_NUM;
    while (isRunning)
    {
        size_t bytes_remaining = BLOCK_LEN;
        auto ptr = std::make_unique<Struct_Datas<StructDataCX>>(PACK_NUM);
        auto buffer = (char*)ptr->ptr;
        try
        {
            while (bytes_remaining > 0)
            {
                size_t offset = BLOCK_LEN - bytes_remaining;
                size_t read_len = dev.read_from_engine(buffer + offset, bytes_remaining, channel);
                if (read_len > 0)
                {
                    bytes_remaining -= read_len;
                    TransferByte += read_len;
                }
            }
            if (CallBack != nullptr)
            {
                ++CX_Count;
                CallBack(ptr);
            }
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }
    }
}

void ThreadProcDataZC(xdma_device& dev, P_ZCDATA_CALLBACK& CallBack, int channel, size_t& TransferByte, const size_t PACK_LEN, const int PACK_NUM)
{
    const size_t BLOCK_LEN = PACK_LEN * PACK_NUM;
    while (isRunning)
    {
        size_t bytes_remaining = BLOCK_LEN;
        auto ptr = std::make_unique<Struct_Datas<StructDataZC>>(PACK_NUM);
        auto buffer = (char*)ptr->ptr;
        try
        {
            while (bytes_remaining > 0)
            {
                size_t offset = BLOCK_LEN - bytes_remaining;
                size_t read_len = dev.read_from_engine(buffer + offset, bytes_remaining, channel);
                if (read_len > 0)
                {
                    bytes_remaining -= read_len;
                    TransferByte += read_len;
                }
            }
            if (CallBack != nullptr)
            {
                ++ZC_Count;
                CallBack(ptr);
            }
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }
    }
}

void ReadThread(xdma_device& dev, P_CXDATA_CALLBACK& CallBackCX, P_ZCDATA_CALLBACK& CallBackZC, std::thread& CX_Thread, std::thread& ZC_Thread)
{
    try
    {
        CX_Thread = std::thread(ThreadProcDataCX, std::ref(dev), std::ref(CallBackCX), 0, std::ref(TransferByte_CX), sizeof(StructDataCX), 1);
        ZC_Thread = std::thread(ThreadProcDataZC, std::ref(dev), std::ref(CallBackZC), 1, std::ref(TransferByte_ZC), sizeof(StructDataZC), 1);
        std::thread([]()
        {
            while (isRunning)
            {
                std::printf("\rCX PCIE: %lluB/s CallBack: %llu/s | ZC PCIE: %lluB/s CallBack: %llu/s", TransferByte_CX, CX_Count, TransferByte_ZC, ZC_Count);
                CX_Count = 0;
                ZC_Count = 0;
                TransferByte_CX = 0;
                TransferByte_ZC = 0;
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }).detach();
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}

#endif
