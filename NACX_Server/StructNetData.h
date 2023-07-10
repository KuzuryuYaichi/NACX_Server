#ifndef _STRUCT_NET_DATA_H
#define _STRUCT_NET_DATA_H

#include <memory>
#include <mutex>
#include <cmath>
#include "StructData.h"
#include "global.h"

#pragma pack(1)

struct DataHead
{
    unsigned int Head = 0xF99FEFFE;
    unsigned char Version = 0x30;
    unsigned char IsEnd = 0;
    unsigned int PackLen;
    unsigned short PackNum = 0;
    unsigned short PackType;
    unsigned char DeviceID[14];
    unsigned char TaiShiMode = 0xFF;
    unsigned short UnUsed = 0xFFFF;
    unsigned char Security = 4;

    DataHead(size_t PackLen, unsigned short PackType, unsigned char* DeviceID): PackLen(PackLen), PackType(PackType)
    {
        std::memcpy(this->DeviceID, DeviceID, sizeof(this->DeviceID));
    }

    DataHead(size_t PackLen, unsigned short PackType, unsigned char* DeviceID, unsigned short PackNum) : PackLen(PackLen), PackType(PackType), PackNum(PackNum)
    {
        std::memcpy(this->DeviceID, DeviceID, sizeof(this->DeviceID));
    }

    DataHead(size_t PackLen, unsigned short PackType, unsigned char* DeviceID, unsigned char Channel) : PackLen(PackLen), PackType(PackType), UnUsed(Channel)
    {
        std::memcpy(this->DeviceID, DeviceID, sizeof(this->DeviceID));
    }

    DataHead(const DataHead&) = default;
    DataHead& operator=(const DataHead&) = default;
};

struct DataEnd
{
    unsigned short DeviceID = 1;
    double Longitude = 104.2;
    double Latitude = 32.8;
    double Height = 504;

    DataEnd(unsigned short DeviceID): DeviceID(DeviceID) {}

    DataEnd(const DataEnd&) = default;
    DataEnd& operator=(const DataEnd&) = default;
};

struct StructNBCXResult
{
    unsigned int Task;
    long long Freq;
    float FreqResolution;
    short CXType = 6;
    short CXResolution = 1;
    short CXMode = 0;
    short CXGroupNum;
    short DataPoint;

    StructNBCXResult() = default;
    StructNBCXResult(const StructNBCXResult&) = default;
    StructNBCXResult& operator=(const StructNBCXResult&) = default;

    void SetNBCXResult(unsigned int Task, long long Freq, unsigned int Resolution, short DataPoint)
    {
        this->Task = Task;
        this->Freq = Freq;
        switch (Resolution)
        {
        case 10: FreqResolution = 25; break;
        case 11: FreqResolution = 12.5; break;
        case 12: FreqResolution = 6.25; break;
        case 13: FreqResolution = 3.125; break;
        default: break;
        }
        CXResolution = 1;
        CXMode = 0;
        CXGroupNum = (short)std::pow(2, 13 - Resolution);
        this->DataPoint = DataPoint;
    };
};

struct StructNBCXData
{
    long long StartTime;
    unsigned char Range;
    short Directivity;
    unsigned char Ranges[1];
};

struct StructFixedCXResult
{
    unsigned int Task = 0;
    long long CenterFreq = 0;
    float FreqResolution = 3.125;
    short CXType = 6;
    short CXResolution = 1;
    short CXGroupNum = 1;
    short DataPoint = BAND_WIDTH_KHZ / FreqResolution + 1;

    StructFixedCXResult() = default;
    StructFixedCXResult(const StructFixedCXResult&) = default;
    StructFixedCXResult& operator=(const StructFixedCXResult&) = default;
};

struct StructTestCXResult
{
    unsigned int Task = 0;
    short ChNum = 4;
    long long CenterFreq;
    float FreqResolution;
    short CXGroupNum;
    short DataPoint;

    StructTestCXResult() = default;
    StructTestCXResult(const StructTestCXResult&) = default;
    StructTestCXResult& operator=(const StructTestCXResult&) = default;
};

struct StructNBWaveZCResult
{
    long long StartTime;
    int NanoSeconds;
    long long Frequency; // Hz
    int BandWidth; // Hz
    int Sps;
    char Accuracy = 1; // 0: Byte; 1: Int16; 2: Int32; 3: Float
    char ChannelNum = 2; // 1:Real; 2: Real-Imagine
    short DataPoint = 2048; // Default 2048

    StructNBWaveZCResult() = default;
    StructNBWaveZCResult(const StructNBWaveZCResult&) = default;
    StructNBWaveZCResult& operator=(const StructNBWaveZCResult&) = default;

    void SetNBWaveResultFrequency(unsigned long long Frequency)
    {
        this->Frequency = Frequency;
    }

    void SetNBWaveResultBandWidth(unsigned int BandWidth)
    {
        this->BandWidth = BandWidth;
        Sps = BandWidth / 2048;
    }
};

struct StructCheckCXResult
{
    unsigned int Task = 400;
    int SignalNum;
    short CXType = 6;
    short CXResolution = 1;
    short CXMode = 0;

    StructCheckCXResult() = default;
    StructCheckCXResult(const StructCheckCXResult&) = default;
    StructCheckCXResult& operator=(const StructCheckCXResult&) = default;
};

struct StructCheckData
{
    long long Freq;
    int BandWidth;
    short Range;
    short SNR;
    short Directivity;
    short GetCXNum;
    short Confidence;
    int KeepTime;
    long long FindTime;
};

struct StructSweepRangeDirectionData
{
    char Range;
    short Direction;
};

struct StructSweepTimeData
{
    long long StartFreq;
    long long StopFreq;
    long long Time;
};

struct StructSweepCXResult
{
    unsigned int Task = 0;
    long long StartTime;
    float FreqResolution;
    long long StartFreq;
    long long StopFreq;
    short CXType = 6;
    short CXResolution = 1;
    int CXResultPoint;
    int TimeNum;

    StructSweepCXResult() = default;
    StructSweepCXResult(const StructSweepCXResult&) = default;
    StructSweepCXResult& operator=(const StructSweepCXResult&) = default;
};

struct StructSweepCXParam
{
    long long SFreq;

    long long EFreq;
    long long DataTime;

    StructSweepCXParam(long long SFreq, long long EFreq, long long DataTime): SFreq(SFreq), EFreq(EFreq), DataTime(DataTime) {}
};

struct StructSweepCXData
{
    char Range;
    short Directivity;
};

struct StructControlRev
{
    unsigned int Task = 400;
    short ControlFlag;
    short ErrorMsg;

    StructControlRev(unsigned int Task, short ControlFlag, short ErrorMsg): Task(Task), ControlFlag(ControlFlag), ErrorMsg(ErrorMsg) {}
};

struct StructWorkCommandRev
{
    unsigned int Task;
    short Data = 0;
    short Detect = 0;
    float FreqRes;
    int SimBW;
    short GMode = 0;
    short MGC;
    short AGC;
    short SmNum;
    short SmMode = 0;
    short LmMode;
    short LmVal;
    short RcvMode;
};

struct StructDeviceScheckRev
{
    unsigned int Task = 400;
    unsigned char DeviveChNum;
    unsigned int ScheckResult;
    unsigned char AGroupNum;
    unsigned int AScheckResult;
};

#pragma pack()

struct StructNetData
{
    int type;
    size_t length = 0;
    char* data = nullptr;

    StructNetData(int type, size_t length) : type(type), length(length), data(new char[length]) {}
    ~StructNetData()
    {
        if (data != nullptr)
            delete[] data;
    }
};

struct PARAMETER_SET
{
    static constexpr int CX_CH_NUM = 4;
    static constexpr int ZC_CH_NUM = 16;
    static constexpr int CALC_MASK()
    {
        int res = 0;
        for (int i = 0; i < CX_CH_NUM; ++i)
        {
            res |= 1 << i;
        }
        return res;
    }

    unsigned char DeviceID[14];
    long long StartCenterFreq = CENTER_FREQ_HZ;
    long long StopCenterFreq = CENTER_FREQ_HZ;
    char RFAttenuation = 10;
    char MFAttenuation = 0;
    char RfMode = 0;
    char CorrectAttenuation = 15;
    char Resolution = 13;
    char Smooth = 1;

    int NbCenterFreqRF;

    enum DATA_TRANS
    {
        TEST_CHANNEL = 0,
        CX_WB,
        CX_NB,
        CX_SWEEP
    };
    DATA_TRANS DataType = CX_WB;

    std::mutex ParameterMutex;
    void SetCmd(long long CmdStartCenterFreq, long long CmdStopCenterFreq)
    {
        std::lock_guard<std::mutex> lock(ParameterMutex);
        this->StartCenterFreq = CmdStartCenterFreq;
        this->StopCenterFreq = CmdStopCenterFreq;
    }

    std::mutex FixedCXMutex;
    StructFixedCXResult FixedCXResult;
    void SetFixedCXResult(unsigned int Task, long long CenterFreq)
    {
        std::lock_guard<std::mutex> lock(FixedCXMutex);
        FixedCXResult.Task = Task;
        FixedCXResult.CenterFreq = CenterFreq;
        
    }
    void SetFixedCXResult(unsigned int Task, unsigned int Resolution)
    {
        std::lock_guard<std::mutex> lock(FixedCXMutex);
        FixedCXResult.Task = Task;
        switch (Resolution)
        {
        case 10: FixedCXResult.FreqResolution = 25.0; break;
        case 11: FixedCXResult.FreqResolution = 12.5; break;
        case 12: FixedCXResult.FreqResolution = 6.25; break;
        case 13: FixedCXResult.FreqResolution = 3.125; break;
        default: break;
        }
        FixedCXResult.CXType = 6;
        FixedCXResult.CXResolution = 1;
        FixedCXResult.CXGroupNum = std::pow(2, 13 - Resolution);
        FixedCXResult.DataPoint = BAND_WIDTH_KHZ / FixedCXResult.FreqResolution + 1;
    }

    std::mutex NarrowMutex;
    StructNBCXResult NarrowCXResult;
    void SetNBCXResult(unsigned int Task, long long CenterFreq, unsigned int Resolution, short DataPoint)
    {
        std::lock_guard<std::mutex> lock(NarrowMutex);
        NarrowCXResult.SetNBCXResult(Task, CenterFreq, Resolution, DataPoint);
    }

    std::mutex SweepMutex;
    StructSweepCXResult SweepCXResult;
    void SetSweepCXResult(unsigned int Task, long long StartCenterFreq, long long StopCenterFreq)
    {
        std::lock_guard<std::mutex> lock(SweepMutex);
        SweepCXResult.Task = Task;
        SweepCXResult.StartFreq = StartCenterFreq;
        SweepCXResult.StopFreq = StopCenterFreq;
        switch (Resolution)
        {
        case 10: SweepCXResult.FreqResolution = 25; break;
        case 11: SweepCXResult.FreqResolution = 12.5; break;
        case 12: SweepCXResult.FreqResolution = 6.25; break;
        case 13: SweepCXResult.FreqResolution = 3.125; break;
        default: break;
        }
        SweepCXResult.TimeNum = (SweepCXResult.StopFreq - SweepCXResult.StartFreq) / BAND_WIDTH_HZ + 1;
        SweepCXResult.CXResultPoint = BAND_WIDTH_KHZ / SweepCXResult.FreqResolution * SweepCXResult.TimeNum + 1;
    }
    void SetSweepCXResult(unsigned int Task, unsigned int Resolution)
    {
        std::lock_guard<std::mutex> lock(SweepMutex);
        SweepCXResult.Task = Task;
        switch (Resolution)
        {
        case 10: SweepCXResult.FreqResolution = 25; break;
        case 11: SweepCXResult.FreqResolution = 12.5; break;
        case 12: SweepCXResult.FreqResolution = 6.25; break;
        case 13: SweepCXResult.FreqResolution = 3.125; break;
        default: break;
        }
        SweepCXResult.CXResultPoint = BAND_WIDTH_KHZ / SweepCXResult.FreqResolution * SweepCXResult.TimeNum + 1;
    }

    std::mutex TestMutex;
    StructTestCXResult TestCXResult;
    void SetTestCXResult(unsigned int Task, long long CenterFreq)
    {
        std::lock_guard<std::mutex> lock(TestMutex);
        TestCXResult.Task = Task;
        TestCXResult.CenterFreq = CenterFreq;
    }
    void SetTestCXResult(unsigned int Task, unsigned int Resolution)
    {
        std::lock_guard<std::mutex> lock(TestMutex);
        TestCXResult.Task = Task;
        switch (Resolution)
        {
        case 10: TestCXResult.FreqResolution = 25.0; break;
        case 11: TestCXResult.FreqResolution = 12.5; break;
        case 12: TestCXResult.FreqResolution = 6.25; break;
        case 13: TestCXResult.FreqResolution = 3.125; break;
        default: break;
        }
        TestCXResult.CXGroupNum = std::pow(2, 13 - Resolution);
        TestCXResult.DataPoint = BAND_WIDTH_KHZ / TestCXResult.FreqResolution + 1;
    }
    void SetTestCXResult(unsigned int Task, long long CenterFreq, unsigned int Resolution)
    {
        std::lock_guard<std::mutex> lock(TestMutex);
        TestCXResult.Task = Task;
        TestCXResult.CenterFreq = CenterFreq;
        switch (Resolution)
        {
        case 10: TestCXResult.FreqResolution = 25.0; break;
        case 11: TestCXResult.FreqResolution = 12.5; break;
        case 12: TestCXResult.FreqResolution = 6.25; break;
        case 13: TestCXResult.FreqResolution = 3.125; break;
        default: break;
        }
        TestCXResult.CXGroupNum = std::pow(2, 13 - Resolution);
        TestCXResult.DataPoint = BAND_WIDTH_KHZ / TestCXResult.FreqResolution + 1;
    }

    std::mutex NBWaveMutex;
    StructNBWaveZCResult NBWaveZCResult[ZC_CH_NUM];
    void SetNBWaveResultFrequency(int ChNum, unsigned long long Frequency)
    {
        if (ChNum < 0 || ChNum >= ZC_CH_NUM)
            return;
        std::lock_guard<std::mutex> lock(NBWaveMutex);
        NBWaveZCResult[ChNum].SetNBWaveResultFrequency(Frequency);
    }
    void SetNBWaveResultBandWidth(int ChNum, unsigned int BandWidth)
    {
        if (ChNum < 0 || ChNum >= ZC_CH_NUM)
            return;
        std::lock_guard<std::mutex> lock(NBWaveMutex);
        NBWaveZCResult[ChNum].SetNBWaveResultBandWidth(BandWidth);
    }

    char isTestingInner = CALC_MASK();
    char isTestingOuter = CALC_MASK();
    bool SelfTestInner[CX_CH_NUM] = { false };
    bool SelfTestOuter[CX_CH_NUM] = { false };
};

void DataHeadToByte(unsigned short, size_t, char*, unsigned short);
void DataHeadToByte(unsigned short, size_t, char*);
void DataHeadToByte(unsigned short, size_t, char*, unsigned char);
void DataEndToByte(char*);

#endif
