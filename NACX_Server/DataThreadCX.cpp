#include "StructNetData.h"
#include "TcpSocket.h"
#include "ThreadSafeQueue.h"
#include "StructData.h"
#include "StructDatas.h"
#include <numbers>

extern PARAMETER_SET g_Parameter;
extern threadsafe_queue<std::unique_ptr<Struct_Datas<StructDataCX>>> tsqueueCXs;

void TcpSocket::FixedCXDataReplay(const StructFixedCXResult& ReplayParm, const std::unique_ptr<StructNetData>& res, size_t Datalen, unsigned short PackNum)
{
    DataHeadToByte(0x0515, Datalen, res->data, PackNum);
    *(StructFixedCXResult*)(res->data + sizeof(DataHead)) = ReplayParm;
    DataEndToByte(res->data + Datalen - sizeof(DataEnd));
    SendMsg(res);
}

void TcpSocket::FixedCXDataReplay(const StructFixedCXResult& ReplayParm, const std::unique_ptr<StructNetData>& res, size_t Datalen)
{
    DataHeadToByte(0x0515, Datalen, res->data);
    *(StructFixedCXResult*)(res->data + sizeof(DataHead)) = ReplayParm;
    DataEndToByte(res->data + Datalen - sizeof(DataEnd));
    SendMsg(res);
}

void TcpSocket::NBCXDataReplay(const StructNBCXResult& ReplayParm, const std::unique_ptr<StructNetData>& res, size_t Datalen)
{
    DataHeadToByte(0x0514, Datalen, res->data);
    *(StructNBCXResult*)(res->data + sizeof(DataHead)) = ReplayParm;
    DataEndToByte(res->data + Datalen - sizeof(DataEnd));
    SendMsg(res);
}

void TcpSocket::SweepCXDataReplay(const StructSweepCXResult& ReplayParm, const std::unique_ptr<StructNetData>& res, size_t Datalen)
{
    DataHeadToByte(0x0513, Datalen, res->data);
    *(StructSweepCXResult*)(res->data + sizeof(DataHead)) = ReplayParm;
    DataEndToByte(res->data + Datalen - sizeof(DataEnd));
    SendMsg(res);
}

void TcpSocket::TestCXDataReplay(const StructTestCXResult& ReplayParm, const std::unique_ptr<StructNetData>& res, size_t Datalen)
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

long long timeConvert(unsigned long long t)
{
    auto UnixTimeToFileTime = [](time_t tmUnixTime) -> long long
    {
        static constexpr long long EPOCH_DIFF = 116444736000000000; //FILETIME starts from 1601-01-01 UTC, epoch from 1970- 01-01
        static constexpr long long RATE_DIFF = 10000000;
        long long ll = tmUnixTime * RATE_DIFF + EPOCH_DIFF;

        FILETIME FileTime;
        FileTime.dwLowDateTime = (DWORD)ll;
        FileTime.dwHighDateTime = ll >> 32;
        return *(long long*)&FileTime;
    };

    auto year = (t >> 58) & 0xFF;
    if (year < 23)
        return UnixTimeToFileTime(time(nullptr));
    year += 100;
    unsigned long long month = 0;
    int dayOffset = (t >> 49) & 0x1FF;
    auto hour = (((t >> 48) & 0x1) ? 12 : 0) + ((t >> 44) & 0xF);
    auto minute = (t >> 38) & 0x3F;
    auto second = (t >> 32) & 0x3F;
    int millisecond = (t & 0xFFFFFFFF) / 102.4;

    static short MONTH_DAYS[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    MONTH_DAYS[1] = ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) ? 29 : 28;
    for (month = 0; dayOffset > 0; ++month)
    {
        if (dayOffset - MONTH_DAYS[month] > 0)
            dayOffset -= MONTH_DAYS[month];
        else
        {
            ++month;
            break;
        }
    }

    std::tm tTime;
    tTime.tm_year = 70 + year;
    tTime.tm_mon = month - 1;
    tTime.tm_mday = dayOffset;
    //tTime.tm_yday = days;
    //tTime.tm_wday = 0;
    tTime.tm_hour = hour;
    tTime.tm_min = minute;
    tTime.tm_sec = second;

    return UnixTimeToFileTime(std::mktime(&tTime));
}

void DataDealCX(TcpSocket& socket, short BaseDirection)
{
    auto ToSelfCheck = [&](const StructDataCX& recvData)
    {
        const auto LENGTH = g_Parameter.TestCXResult.DataPoint - 1;
        auto ch = recvData.DataType - 1;
        if (ch < 0 || ch >= 4)
            return;
        auto Data = recvData.RangePhaseData;
        
        auto Range = Data[LENGTH / 2].Range / 100 + 12;
        if (g_Parameter.isTestingInner != g_Parameter.CALC_MASK() && recvData.WorkMode == 0 && recvData.CorrectMode == 0 && recvData.CentreFreq == 350000)
        {
            g_Parameter.SelfTestInner[ch] = Range > 68;
            g_Parameter.isTestingInner |= 1 << ch;
            std::cout << "Inner Channel: " << ch << ", Range: " << Range << std::endl;
        }
        else if (g_Parameter.isTestingOuter != g_Parameter.CALC_MASK() && recvData.WorkMode == 0 && recvData.CorrectMode == 1 && recvData.CentreFreq == 350000)
        {
            g_Parameter.SelfTestOuter[ch] = Range > 68;
            g_Parameter.isTestingOuter |= 1 << ch;
            std::cout << "Outer Channel: " << ch << ", Range: " << Range << std::endl;
        }
    };

    auto ToNBCXdata = [&](const StructDataCX& recvData)
    {
        auto& NarrowCXResult = g_Parameter.NarrowCXResult;
        auto Data = recvData.DirectionRangeData;
        const auto CXPerDataLen = sizeof(long long) + sizeof(char) + sizeof(short) + sizeof(char) * NarrowCXResult.DataPoint,
            CXDataLen = NarrowCXResult.CXGroupNum * CXPerDataLen,
            DataLen = sizeof(DataHead) + sizeof(StructNBCXResult) + CXDataLen + sizeof(DataEnd);
        auto res = std::make_unique<StructNetData>(0, DataLen);

        for (auto CXData = (char*)res->data + sizeof(DataHead) + sizeof(StructNBCXResult), end = CXData + CXDataLen; CXData < end; CXData += CXPerDataLen)
        {
            *(long long*)CXData = timeConvert(*(long long*)recvData.Time);
            auto& Range = *(CXData + sizeof(long long));
            auto& Directivity = *(short*)(Range + sizeof(char));
            auto Ranges = ((char*)Directivity) + sizeof(short);

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
        const auto CXPerDataLen = sizeof(long long) + (sizeof(char) + sizeof(short)) * FixedCXResult.DataPoint,
            CXDataLen = FixedCXResult.CXGroupNum * CXPerDataLen,
            Datalen = sizeof(DataHead) + sizeof(StructFixedCXResult) + CXDataLen + sizeof(DataEnd);
        auto res = std::make_unique<StructNetData>(0, Datalen);

        const auto LENGTH = FixedCXResult.DataPoint - 1;
        auto Data = recvData.DirectionRangeData;
        auto start = res->data + sizeof(DataHead) + sizeof(StructFixedCXResult);
        for (auto g = 0; g < FixedCXResult.CXGroupNum; ++g)
        {
            auto& StartTime = *(long long*)start;
            auto Range = start + sizeof(long long);
            auto Direction = (short*)(Range + sizeof(char) * FixedCXResult.DataPoint);
            StartTime = timeConvert(*(long long*)recvData.Time);
            for (int p = 0; p < LENGTH; ++p)
            {
                Range[p] = std::max(Data[p].Range / 100 + 12, 0);
                Direction[p] = BaseDirection - (Data[p].Direction - 750);
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
        auto DataPoint = (int)(BAND_WIDTH_KHZ / SweepCXResult.FreqResolution);
        auto CXDataLen = sizeof(StructSweepTimeData) * SweepCXResult.TimeNum + sizeof(StructSweepRangeDirectionData) * SweepCXResult.CXResultPoint;
        auto DataLen = sizeof(DataHead) + sizeof(StructSweepCXResult) + CXDataLen + sizeof(DataEnd);
        
        static std::unique_ptr<StructNetData> res = nullptr;
        auto packIndex = (int)(recvData.CentreFreq * 1e3 - SweepCXResult.StartFreq) / BAND_WIDTH_HZ;
        if (packIndex < 0 || packIndex >= SweepCXResult.TimeNum)
            return;

        if (packIndex == 0)
        {
            SweepCXResult.StartTime = timeConvert (*(long long*)recvData.Time);
            res = std::make_unique<StructNetData>(0, DataLen);
        }

        if (res == nullptr || res->length != DataLen)
            return;
        
        const auto TIME_START = (StructSweepTimeData*)(res->data + sizeof(DataHead) + sizeof(StructSweepCXResult));
        auto TimeStruct = TIME_START + packIndex;
        TimeStruct->Time = timeConvert(*(long long*)recvData.Time);
        TimeStruct->StartFreq = recvData.CentreFreq - HALF_BAND_WIDTH_KHZ;
        TimeStruct->StopFreq = recvData.CentreFreq + HALF_BAND_WIDTH_KHZ;
        auto DataNet = ((StructSweepRangeDirectionData*)(TIME_START + SweepCXResult.TimeNum)) + packIndex * DataPoint;
        auto Data = recvData.DirectionRangeData;
        for (int p = 0; p < DataPoint; ++p)
        {
            DataNet[p].Range = std::max(Data[p].Range / 100 + 12, 0);
            DataNet[p].Direction = BaseDirection - (Data[p].Direction - 750);
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
        static std::unique_ptr<StructNetData> res = nullptr;

        auto& TestCXResult = g_Parameter.TestCXResult;
        const auto CXPerDataLen = sizeof(char) * TestCXResult.ChNum * TestCXResult.DataPoint + sizeof(short) * (TestCXResult.ChNum - 1) * TestCXResult.DataPoint,
            CXDataLen = TestCXResult.CXGroupNum * CXPerDataLen;
        auto DataLen = sizeof(DataHead) + sizeof(StructTestCXResult) + CXDataLen + sizeof(DataEnd);

        auto ch = recvData.DataType - 1;
        if (ch == 0)
        {
            res = std::make_unique<StructNetData>(0, DataLen);
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
                    Range[p] = std::max(Data[p].Range / 100 + 12, 0);
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
                    Range[p] = std::max(Data[p].Range / 100 + 12, 0);
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
            if (ptr->ptr[i].Head == 0xBAABDCCD)
                GetQueueDataFun(ptr->ptr[i]);
        }
    }
};
