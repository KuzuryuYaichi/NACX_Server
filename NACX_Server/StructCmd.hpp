#ifndef _STRUCT_CMD
#define _STRUCT_CMD

#include <filesystem>
#include <fstream>
#include <iostream>
#include "XDMA_PCIE/dllexport.h"

#pragma pack(1)

struct StructCmdCX
{
    unsigned short Head = 0xAA55;
    char ScanSpeed = 4;
    char StateMachine = 0; // 2
    int StartCenterFreq = 350000;
    int StopCenterFreq = 350000;
    unsigned int AntennaFreq = 530000;
    char RFAttenuation = 0;
    char MFAttenuation = 0;
    char RfMode = 1;
    char CorrectAttenuation = 3;
    unsigned int DDS_CTRL = 0;
    char Resolution = 13;
    char CorrectMode = 1;
    char Smooth = 1;
    char FFT_SCH = 4;
    unsigned short RfProtectTime = 2; // 1/SampleRate
    unsigned short Tail = 0x55AA;

    StructCmdCX() {}

    void ResetCmd()
    {
        new (this) StructCmdCX();
    }
};

struct StructCmdZC
{
    unsigned char CmdType = 0;
    union {
        struct CMD_NB
        {
            unsigned char Channel;
            unsigned short CIC;
            unsigned int DDS;
        } CmdNB;
        struct CMD_RF
        {
            unsigned RfType;
            unsigned int RfData;
            unsigned char Reserved[2];
        } CmdRF;
    };
    
    StructCmdZC() {}

    void ResetCmd()
    {
        new (this) StructCmdZC();
    }
};

struct StructChannel
{
    static constexpr int IQ_LENGTH = 2 * sizeof(unsigned short);
    unsigned short I;
    unsigned short Q;
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
    static constexpr int FREQ_LENGTH = 1;

    unsigned int Head = 0x5A5AA5A5;
    unsigned short TotalPack = TOTAL_FREQ_LENGTH;
    unsigned short SubPack;
    unsigned int SubPackLen;
    char Reserved[4];
    StructFreqPoint FreqPoint[FREQ_LENGTH];

    StructSample(unsigned short SubPack): SubPack(SubPack) {}
};

#pragma pack()

class CmdProcess
{
public:
    StructCmdCX CmdCX;

    void setStructCmd(const StructCmdCX& commamd)
    {
        CmdCX = commamd;
    }

    void SetFFT_SCH(int FFT_SCH)
    {
        CmdCX.FFT_SCH = FFT_SCH;
    }

    void SetScanSpeed(int ScanSpeed)
    {
        CmdCX.ScanSpeed = ScanSpeed;
    }

    void SetStateMachine(int State)
    {
        CmdCX.StateMachine = State;
    }

    void SetStartCenterFreq(int StartCenterFreq)
    {
        CmdCX.StartCenterFreq = StartCenterFreq;
    }

    void SetStopCenterFreq(int StopCenterFreq)
    {
        CmdCX.StopCenterFreq = StopCenterFreq;
    }

    void SetAntennaFreq(int AntennaFreq)
    {
        CmdCX.AntennaFreq = AntennaFreq;
    }

    void SetRFAttenuation(int RFGain)
    {
        CmdCX.RFAttenuation = RFGain;
    }

    void SetMFAttenuation(int IFGain)
    {
        CmdCX.MFAttenuation = IFGain;
    }

    void SetRfMode(int RfMode)
    {
        CmdCX.RfMode = RfMode;
    }

    void SetCorrectAttenuation(int CorrectGain)
    {
        CmdCX.CorrectAttenuation = CorrectGain;
    }

    void SetDDS_CTRL(int DDS_CTRL)
    {
        CmdCX.DDS_CTRL = DDS_CTRL;
    }

    void SetResolution(int Resolution)
    {
        CmdCX.Resolution = Resolution;
    }

    void SetCorrectMode(int CorrectMode)
    {
        CmdCX.CorrectMode = CorrectMode;
    }

    void SetSmooth(int Smooth)
    {
        CmdCX.Smooth = Smooth;
    }

    void SendCmd()
    {
        WriteStreamCmd((char*)&CmdCX);
    }

    void SetSample()
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
                stream.read((char*)sample.FreqPoint, sizeof(StructFreqPoint));
                WriteStreamSample((char*)&sample, sizeof(StructSample));
            }
        }
        catch (const std::exception& e)
        {
            std::cout << "SetSample Error: " << e.what() << std::endl;
        }
    }
};

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