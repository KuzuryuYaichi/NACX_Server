#include "DataThread.h"
#include "StructNetData.h"
#include "XDMA_PCIE/dllexport.h"
#include "TcpSocket.h"
#include "ThreadSafeQueue.h"
#include "StructData.h"
#include "StructDatas.h"
#include <numbers>

extern PARAMETER_SET g_Parameter;
extern threadsafe_queue<std::shared_ptr<Struct_Datas<StructDataCX>>> tsqueueCXs;

std::chrono::time_point<std::chrono::system_clock> TimeStampToDateTime(const unsigned char* time)
{
    unsigned long fractpart = 0;
    for (int i = 0; i < 4; ++i)
    {
        fractpart = (fractpart << 8) + time[i];
    }
    fractpart &= ~(1 << 31);
    int milliseconds = fractpart / 204800;
    int seconds = ((time[3] >> 6) & 0xff) + ((time[4] << 2) & 0x3c);
    int minutes = ((time[4] >> 4) & 0xff) + ((time[5] << 4) & 0x30);
    int hours = (time[5] >> 2) & 0x1f;
    int days = ((time[5] >> 7) & 0xff) + ((time[6] & 0xffff) << 1) - 1;

    return std::chrono::system_clock::time_point() +
        std::chrono::days(days) +
        std::chrono::hours(hours) +
        std::chrono::minutes(minutes) +
        std::chrono::seconds(seconds) +
        std::chrono::milliseconds(milliseconds);
}

void TcpSocket::FixedCXDataReplay(const StructFixedCXResult& ReplayParm, std::shared_ptr<StructNetData>& res, size_t Datalen, unsigned short PackNum)
{
    DataHeadToByte(0x0515, Datalen, res->data, PackNum);
    *(StructFixedCXResult*)(res->data + sizeof(DataHead)) = ReplayParm;
    DataEndToByte(res->data + Datalen - sizeof(DataEnd));
    SendMsg(res);
}

void TcpSocket::FixedCXDataReplay(const StructFixedCXResult& ReplayParm, std::shared_ptr<StructNetData>& res, size_t Datalen)
{
    DataHeadToByte(0x0515, Datalen, res->data);
    *(StructFixedCXResult*)(res->data + sizeof(DataHead)) = ReplayParm;
    DataEndToByte(res->data + Datalen - sizeof(DataEnd));
    SendMsg(res);
}

void TcpSocket::NBCXDataReplay(const StructNBCXResult& ReplayParm, std::shared_ptr<StructNetData>& res, size_t Datalen)
{
    DataHeadToByte(0x0514, Datalen, res->data);
    *(StructNBCXResult*)(res->data + sizeof(DataHead)) = ReplayParm;
    DataEndToByte(res->data + Datalen - sizeof(DataEnd));
    SendMsg(res);
}

void TcpSocket::SweepCXDataReplay(const StructSweepCXResult& ReplayParm, std::shared_ptr<StructNetData>& res, size_t Datalen)
{
    DataHeadToByte(0x0513, Datalen, res->data);
    *(StructSweepCXResult*)(res->data + sizeof(DataHead)) = ReplayParm;
    DataEndToByte(res->data + Datalen - sizeof(DataEnd));
    SendMsg(res);
}

void TcpSocket::TestCXDataReplay(const StructTestCXResult& ReplayParm, std::shared_ptr<StructNetData>& res, size_t Datalen)
{
    DataHeadToByte(0x0516, Datalen, res->data);
    *(StructTestCXResult*)(res->data + sizeof(DataHead)) = ReplayParm;
    DataEndToByte(res->data + Datalen - sizeof(DataEnd));
    SendMsg(res);
}

float ResolveResolution(unsigned char Resolution)
{
    switch (Resolution)
    {
    case 10: return 25.0;
    case 11: return 12.5;
    case 12: return 6.25;
    case 13: return 3.125;
    default: return 0;
    }
}

void DataDealCX(TcpSocket& socket)
{
    auto ToSelfCheck = [&](const StructDataCX& recvData)
    {
        const auto LENGTH = g_Parameter.TestCXResult.DataPoint - 1;
        auto ch = recvData.DataType - 1;
        if (ch < 0 || ch >= 4)
            return;
        auto Data = recvData.RangePhaseData;
        
        auto Range = Data[LENGTH / 2].Range / 100 + 12;
        if (g_Parameter.isTestingInner != 0xF && recvData.WorkMode == 0 && recvData.CorrectMode == 0 && recvData.CentreFreq == 350000)
        {
            g_Parameter.SelfTestInner[ch] = Range > 70;
            g_Parameter.isTestingInner |= 1 << ch;
            std::cout << "Inner Channel: " << ch << ", Range: " << Range << std::endl;
        }
        else if (g_Parameter.isTestingOuter != 0xF && recvData.WorkMode == 0 && recvData.CorrectMode == 1 && recvData.CentreFreq == 350000)
        {
            g_Parameter.SelfTestOuter[ch] = Range > 70;
            g_Parameter.isTestingOuter |= 1 << ch;
            std::cout << "Outer Channel: " << ch << ", Range: " << Range << std::endl;
        }
    };

    auto ToNBCXdata = [&](const StructDataCX& recvData)
    {
        auto& NarrowCXResult = g_Parameter.NarrowCXResult;
        auto Data = recvData.DirectionRangeData;
        auto time = std::chrono::duration_cast<std::chrono::milliseconds>(TimeStampToDateTime(recvData.Time).time_since_epoch()).count();
        const auto CXPerDataLen = sizeof(long long) + sizeof(char) + sizeof(short) + sizeof(char) * NarrowCXResult.DataPoint,
            CXDataLen = NarrowCXResult.CXGroupNum * CXPerDataLen,
            DataLen = sizeof(DataHead) + sizeof(StructNBCXResult) + CXDataLen + sizeof(DataEnd);
        auto res = std::make_shared<StructNetData>(0, DataLen);

        for (auto CXData = (char*)res->data + sizeof(DataHead) + sizeof(StructNBCXResult), end = CXData + CXDataLen; CXData < end; CXData += CXPerDataLen)
        {
            auto& StartTime = *(long long*)CXData;
            auto& Range = *(CXData + sizeof(long long));
            auto& Directivity = *(short*)(Range + sizeof(char));
            auto Ranges = ((char*)Directivity) + sizeof(short);
            StartTime = time;

            auto mid = NarrowCXResult.DataPoint / 2;
            Range = 20 * std::log10(Data[mid].Range) + 40;
            float direction = Data[mid].Direction / 64;
            if (direction < 0)
                direction += 360;
            Directivity = direction * 10;
            for (int j = 0; j < NarrowCXResult.DataPoint - 1; ++j)
            {
                Ranges[j] = 20 * std::log10(Data[j].Range) + 40;
            }
            Data += NarrowCXResult.DataPoint;
        }
        socket.NBCXDataReplay(NarrowCXResult, res, DataLen);
    };

    auto ToFixedCXdata = [&](const StructDataCX& recvData)
    {
        auto& FixedCXResult = g_Parameter.FixedCXResult;
        auto time = *(long long*)recvData.Time;
        const auto CXPerDataLen = sizeof(long long) + (sizeof(char) + sizeof(short)) * FixedCXResult.DataPoint,
            CXDataLen = FixedCXResult.CXGroupNum * CXPerDataLen,
            Datalen = sizeof(DataHead) + sizeof(StructFixedCXResult) + CXDataLen + sizeof(DataEnd);
        auto res = std::make_shared<StructNetData>(0, Datalen);

        const auto LENGTH = FixedCXResult.DataPoint - 1;
        auto Data = recvData.DirectionRangeData;
        auto start = res->data + sizeof(DataHead) + sizeof(StructFixedCXResult);
        for (auto g = 0; g < FixedCXResult.CXGroupNum; ++g)
        {
            auto& StartTime = *(long long*)start;
            auto Range = start + sizeof(long long);
            auto Direction = (short*)(Range + sizeof(char) * FixedCXResult.DataPoint);
            StartTime = time;
            for (int p = 0; p < LENGTH; ++p)
            {
                Range[p] = Data[p].Range / 100 - 125;
                Direction[p] = Data[p].Direction - 750;
            }
            Range[LENGTH] = Range[LENGTH - 1];
            Direction[LENGTH] = Direction[LENGTH - 1];
            start += CXPerDataLen;
        }
        socket.FixedCXDataReplay(FixedCXResult, res, Datalen, recvData.PackNum);
    };

    auto ToSweepCXdata = [&](const StructDataCX& recvData)
    {
        auto& SweepCXResult = g_Parameter.SweepCXResult;
        auto DataPoint = (int)(20000 / SweepCXResult.FreqResolution);
        auto CXDataLen = sizeof(StructSweepTimeData) * SweepCXResult.TimeNum + sizeof(StructSweepRangeDirectionData) * SweepCXResult.CXResultPoint;
        auto DataLen = sizeof(DataHead) + sizeof(StructSweepCXResult) + CXDataLen + sizeof(DataEnd);
        
        static std::shared_ptr<StructNetData> res = nullptr;
        auto packIndex = (int)(recvData.CentreFreq - SweepCXResult.StartFreq) / 20000;
        if (packIndex < 0 || packIndex >= SweepCXResult.TimeNum)
            return;

        if (packIndex == 0)
        {
            SweepCXResult.StartTime = *(long long*)recvData.Time;
            res = std::make_shared<StructNetData>(0, DataLen);
        }

        if (res == nullptr || res->length != DataLen)
            return;
        
        const auto TIME_START = (StructSweepTimeData*)(res->data + sizeof(DataHead) + sizeof(StructSweepCXResult));
        auto TimeStruct = TIME_START + packIndex;
        TimeStruct->Time = *(long long*)recvData.Time;
        TimeStruct->StartFreq = recvData.CentreFreq - 10000;
        TimeStruct->StopFreq = recvData.CentreFreq + 10000;
        auto DataNet = ((StructSweepRangeDirectionData*)(TIME_START + SweepCXResult.TimeNum)) + packIndex * DataPoint;
        auto Data = recvData.DirectionRangeData;
        for (int p = 0; p < DataPoint; ++p)
        {
            DataNet[p].Range = Data[p].Range / 100 - 125;
            DataNet[p].Direction = Data[p].Direction - 750;
        }
        if (packIndex == SweepCXResult.TimeNum - 1)
        {
            DataNet[DataPoint].Range = DataNet[DataPoint - 1].Range;
            DataNet[DataPoint].Direction = DataNet[DataPoint - 1].Direction;
            socket.SweepCXDataReplay(SweepCXResult, res, DataLen);
        }
    };

    auto ToChdata = [&](const StructDataCX& recvData)
    {
        static unsigned char last_PackNum = 0;
        static std::shared_ptr<StructNetData> res = nullptr;

        auto& TestCXResult = g_Parameter.TestCXResult;
        const auto CXPerDataLen = sizeof(char) * TestCXResult.ChNum * TestCXResult.DataPoint + sizeof(short) * (TestCXResult.ChNum - 1) * TestCXResult.DataPoint,
            CXDataLen = TestCXResult.CXGroupNum * CXPerDataLen;
        auto DataLen = sizeof(DataHead) + sizeof(StructTestCXResult) + CXDataLen + sizeof(DataEnd);

        auto ch = recvData.DataType - 1;
        if (ch == 0)
        {
            res = std::make_shared<StructNetData>(0, DataLen);
            last_PackNum = recvData.l_PackNum;
        }
        else if (res == nullptr || res->length != DataLen || ch != 0 && recvData.l_PackNum != last_PackNum)
        {
            return;
        }

        const auto LENGTH = TestCXResult.DataPoint - 1;
        auto Data = recvData.RangePhaseData;
        const auto DATA_BASE = res->data + sizeof(DataHead) + sizeof(StructTestCXResult);
        const auto RANGE_POINTS = TestCXResult.ChNum * TestCXResult.DataPoint;
        const auto PHASE_POINTS = (TestCXResult.ChNum - 1) * TestCXResult.DataPoint;
        const auto DATA_LENGTH = RANGE_POINTS * sizeof(char) + PHASE_POINTS * sizeof(short);

        for (auto g = 0; g < TestCXResult.CXGroupNum; ++g)
        {
            auto RangeBase = DATA_BASE + DATA_LENGTH * g;
            auto Range = RangeBase + ch * TestCXResult.DataPoint;
            if (ch > 0)
            {
                auto PhaseBase = (short*)(RangeBase + RANGE_POINTS);
                auto Phase = PhaseBase + (ch - 1) * TestCXResult.DataPoint;
                for (int p = 0; p < LENGTH; ++p)
                {
                    Range[p] = (unsigned char)(Data[p].Range / 100) + 12;
                    Phase[p] = ((double)Data[p].Phase) * 1800 / std::numbers::pi / 8192;
                }
                Range[LENGTH] = Range[LENGTH - 1];
                Range += TestCXResult.DataPoint;
                Phase[LENGTH] = Phase[LENGTH - 1];
                Phase += TestCXResult.DataPoint;
                Data += LENGTH;
            }
            else
            {
                for (int p = 0; p < LENGTH; ++p)
                {
                    Range[p] = (unsigned char)(Data[p].Range / 100) + 12;
                }
                Range[LENGTH] = Range[LENGTH - 1];
                Range += TestCXResult.DataPoint;
                Data += LENGTH;
            }
        }
        if (ch == TestCXResult.ChNum - 1)
        {
            socket.TestCXDataReplay(TestCXResult, res, DataLen);
        }
    };

    auto GetQueueDataFun = [&](const StructDataCX& recvData)
    {
        switch (g_Parameter.DataType)
        {
        case 0: //CH Test
        {
            auto& TestCXResult = g_Parameter.TestCXResult;
            std::lock_guard<std::mutex> lk(g_Parameter.TestMutex);
            if (recvData.DataType == 0 || TestCXResult.FreqResolution != ResolveResolution(recvData.Resolution))
                return;
            if (g_Parameter.isTestingInner != 0xF || g_Parameter.isTestingOuter != 0xF)
            {
                ToSelfCheck(recvData);
            }
            ToChdata(recvData);
            break;
        }
        case 1: //WB
        {
            auto& FixedCXResult = g_Parameter.FixedCXResult;
            std::lock_guard<std::mutex> lk(g_Parameter.FixedCXMutex);
            if (recvData.DataType != 0 || FixedCXResult.FreqResolution != ResolveResolution(recvData.Resolution))
                return;
            ToFixedCXdata(recvData);
            break;
        }
        case 2: //NB
        {
            auto& NarrowCXResult = g_Parameter.NarrowCXResult;
            std::lock_guard<std::mutex> lk(g_Parameter.NarrowMutex);
            if (recvData.DataType != 0 || NarrowCXResult.FreqResolution != ResolveResolution(recvData.Resolution))
                return;
            ToNBCXdata(recvData);
            break;
        }
        case 3: //SW
        {
            auto& SweepCXResult = g_Parameter.SweepCXResult;
            std::lock_guard<std::mutex> lk(g_Parameter.SweepMutex);
            if (recvData.DataType != 0 || SweepCXResult.FreqResolution != ResolveResolution(recvData.Resolution))
                return;
            ToSweepCXdata(recvData);
            break;
        }
        default:
            return;
        }
    };

    while (true)
    {
        auto ptr = tsqueueCXs.wait_and_pop();
        for (int i = 0; i < ptr->PACK_NUM; ++i)
        {
            GetQueueDataFun(ptr->ptr[i]);
        }
    }
};
