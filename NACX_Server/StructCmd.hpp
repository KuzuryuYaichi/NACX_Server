#ifndef _STRUCT_CMD
#define _STRUCT_CMD

#include <filesystem>
#include <fstream>
#include <iostream>
#include <cmath>
#include "XDMA_PCIE/dllexport.h"

#pragma pack(1)

struct StructChannel
{
    unsigned short I = 0;
    unsigned short Q = 0;
};

struct StructDirection
{
    static constexpr int CHANNEL_LENGTH = 8;
    StructChannel Channel[CHANNEL_LENGTH];
};

struct StructFreqPoint
{
    static constexpr int DIRECTION_LENGTH = 128;
    StructDirection Direction[DIRECTION_LENGTH];
};

struct StructSample
{
    static constexpr int TOTAL_FREQ_LENGTH = 321;

    unsigned int Head = 0x5A5AA5A5;
    unsigned short TotalPack = TOTAL_FREQ_LENGTH;
    unsigned short SubPack;
    unsigned int SubPackLen;
    char Reserved[4];
    StructFreqPoint FreqPoint;

    StructSample(unsigned short SubPack) : SubPack(SubPack) {}
};

struct StructCmdCX
{
    unsigned short Head = 0xAA55;
    char ScanSpeed = 4;
    char StateMachine = 0; // 2
    int StartCenterFreq = 350000;
    int StopCenterFreq = 350000;
    unsigned int AntennaFreq = 530000;
    char RFAttenuation = 10;
    char MFAttenuation = 0;
    char RfMode = 1;
    char CorrectAttenuation = 15;
    unsigned int DDS_CTRL = 0;
    char Resolution = 13;
    char CorrectMode = 1;
    char Smooth = 1;
    char FFT_SCH = 4;
    unsigned short RfProtectTime = 2; // 1/SampleRate
    unsigned short Tail = 0x55AA;

    StructCmdCX() {}

    void SendCXCmd()
    {
        WriteStreamCmd((char*)this, sizeof(StructCmdCX), 0);
    }

    void SendSample()
    {
        const std::string SAMPLE_PATH = "Sample/RealSample.dat";
        if (!std::filesystem::exists(SAMPLE_PATH))
            return;
        try
        {
            std::ifstream stream(std::filesystem::current_path().generic_string() + "/" + SAMPLE_PATH, std::ios_base::in | std::ios_base::binary);
            if (!stream.is_open())
            {
                std::cout << "Sample File Not Opened!" << std::endl;
                return;
            }
            for (auto i = 0; i < StructSample::TOTAL_FREQ_LENGTH; ++i)
            {
                static constexpr int START_FREQ = 190;
                StructSample sample(i + START_FREQ);
                stream.read((char*)&sample.FreqPoint, sizeof(StructFreqPoint));
                WriteStreamSample((char*)&sample, sizeof(StructSample));
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        }
        catch (const std::exception& e)
        {
            std::cout << "SetSample Error: " << e.what() << std::endl;
        }
    }
};

struct StructCmdZC
{
    unsigned char CmdType = 0;
    union
    {
        struct CMD_NB {
            unsigned char Channel;
            unsigned short CIC = 200;
            unsigned int DDS;
        } CmdNB;
        struct CMD_RF {
            unsigned char RfType;
            unsigned int RfData;
            unsigned char Reserved[2];
        } CmdRF;
    };
    
    StructCmdZC() {}

    void SendZCCmd()
    {
        WriteStreamCmd((char*)this, sizeof(StructCmdZC), 1);
    }
};

#pragma pack()

struct Order
{
    char* order = nullptr;
    Order() {}
    Order(const Order&) = default;
    Order& operator=(const Order&) = default;

    Order(size_t len) : order(new char[len]) {}
    ~Order()
    {
        if (order != nullptr)
            delete[] order;
    }
};

#endif