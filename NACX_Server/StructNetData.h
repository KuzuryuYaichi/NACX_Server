#ifndef _STRUCT_NET_DATA_H
#define _STRUCT_NET_DATA_H

#include <memory>
#include <mutex>
#include <cmath>
#include "StructData.h"

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

    void SetNBCXResult(unsigned int Task, int Freq, unsigned int Resolution, short DataPoint)
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
    short DataPoint = 20000 / FreqResolution + 1;

    StructFixedCXResult() = default;
    StructFixedCXResult(const StructFixedCXResult&) = default;
    StructFixedCXResult& operator=(const StructFixedCXResult&) = default;

    void SetFixedCXResult(unsigned int Task, int CenterFreq, unsigned int Resolution)
    {
        this->Task = Task;
        this->CenterFreq = CenterFreq;
        switch (Resolution)
        {
        case 10: FreqResolution = 25.0; break;
        case 11: FreqResolution = 12.5; break;
        case 12: FreqResolution = 6.25; break;
        case 13: FreqResolution = 3.125; break;
        default: break;
        }
        CXType = 6;
        CXResolution = 1;
        CXGroupNum = std::pow(2, 13 - Resolution);
        DataPoint = 20000 / FreqResolution + 1;
    }
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

    void SetTestCXResult(unsigned int Task, int CenterFreq, unsigned int Resolution)
    {
        this->Task = Task;
        this->CenterFreq = CenterFreq;
        switch (Resolution)
        {
        case 10: FreqResolution = 25.0; break;
        case 11: FreqResolution = 12.5; break;
        case 12: FreqResolution = 6.25; break;
        case 13: FreqResolution = 3.125; break;
        default: break;
        }
        CXGroupNum = std::pow(2, 13 - Resolution);
        DataPoint = 20000 / FreqResolution + 1;
    }
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
    short CXResolution;
    int CXResultPoint;
    int TimeNum;

    StructSweepCXResult() = default;
    StructSweepCXResult(const StructSweepCXResult&) = default;
    StructSweepCXResult& operator=(const StructSweepCXResult&) = default;

    void SetSweepResult(unsigned int Task, int StartFreq, int StopFreq, unsigned int Resolution)
    {
        this->Task = Task;
        this->StartFreq = StartFreq;
        this->StopFreq = StopFreq;
        switch (Resolution)
        {
        case 10: FreqResolution = 25; break;
        case 11: FreqResolution = 12.5; break;
        case 12: FreqResolution = 6.25; break;
        case 13: FreqResolution = 3.125; break;
        default: break;
        }
        CXType = 6;
        CXResolution = 1;
        TimeNum = (StopFreq - StartFreq) / 20000 + 1;
        CXResultPoint = 20000 / FreqResolution * TimeNum + 1;
    }
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

    char ScanSpeed = 4;
    char StateMachine = 1;
    int StartCenterFreq = 200000;
    int StopCenterFreq = 200000;
    unsigned int AntennaFreq = 530000;
    char RFAttenuation = 10;
    char MFAttenuation = 0;
    char RfMode = 0;
    char CorrectAttenuation = 15;
    unsigned int DDS_CTRL = 0;
    char Resolution = 13;
    char CorrectMode = 0;
    char Smooth = 1;
    char FFT_SCH = 0;
    unsigned short RfProtectTime = 2; // 1/SampleRate

    int NbCenterFreqRF;

    enum DATA_TRANS
    {
        TEST_CHANNEL = 0,
        CX_WB,
        CX_NB,
        CX_SWEEP
    };
    DATA_TRANS DataType = CX_WB;
    int CmdStartCenterFreq = 0;
    int CmdStopCenterFreq = 0;
    unsigned int CmdResolution = 0;
    unsigned int CXmode = 0;
    unsigned int CmdTask = 0;

    std::mutex ParameterMutex;
    void SetCmd(int CmdStartCenterFreq, int CmdStopCenterFreq, unsigned int CmdResolution, unsigned int CmdTask, unsigned int CXmode)
    {
        std::lock_guard<std::mutex> lock(ParameterMutex);
        this->CmdStartCenterFreq = this->StartCenterFreq = CmdStartCenterFreq;
        this->CmdStopCenterFreq = this->StopCenterFreq = CmdStopCenterFreq;
        this->CmdResolution = CmdResolution;
        this->CXmode = CXmode;
        this->CmdTask = CmdTask;
    }

    std::mutex FixedCXMutex;
    StructFixedCXResult FixedCXResult;
    void SetFixedCXResult(unsigned int Task, int CenterFreq, unsigned int Resolution)
    {
        std::lock_guard<std::mutex> lock(FixedCXMutex);
        FixedCXResult.SetFixedCXResult(Task, CenterFreq, Resolution);
    }

    std::mutex NarrowMutex;
    StructNBCXResult NarrowCXResult;
    void SetNBCXResult(unsigned int Task, int Freq, unsigned int Resolution, short DataPoint)
    {
        std::lock_guard<std::mutex> lock(NarrowMutex);
        NarrowCXResult.SetNBCXResult(Task, Freq, Resolution, DataPoint);
    }

    std::mutex SweepMutex;
    StructSweepCXResult SweepCXResult;
    void SetSweepCXResult(unsigned int Task, int StartCenterFreq, int StopCenterFreq, unsigned int Resolution)
    {
        std::lock_guard<std::mutex> lock(SweepMutex);
        SweepCXResult.SetSweepResult(Task, StartCenterFreq, StopCenterFreq, Resolution);
    }

    std::mutex TestMutex;
    StructTestCXResult TestCXResult;
    void SetTestCXResult(unsigned int Task, int CenterFreq, unsigned int Resolution)
    {
        std::lock_guard<std::mutex> lock(TestMutex);
        TestCXResult.SetTestCXResult(Task, CenterFreq, Resolution);
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
