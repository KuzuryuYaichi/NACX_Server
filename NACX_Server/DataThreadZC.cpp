#include "DataThread.h"
#include "StructNetData.h"
#include "XDMA_PCIE/dllexport.h"
#include "TcpSocket.h"
#include "ThreadSafeQueue.h"
#include "StructData.h"
#include "StructDatas.h"

extern PARAMETER_SET g_Parameter;
extern threadsafe_queue<std::unique_ptr<Struct_Datas<StructDataZC>>> tsqueueZCs;

void TcpSocket::NBZCDataReplay(const StructNBWaveZCResult& ReplayParm, const std::unique_ptr<StructNetData>& res, size_t Datalen, unsigned char Channel)
{
    DataHeadToByte(0x0602, Datalen, res->data, Channel);
    *(StructNBWaveZCResult*)(res->data + sizeof(DataHead)) = ReplayParm;
    DataEndToByte(res->data + Datalen - sizeof(DataEnd));
    SendMsg(res);
}

void DataDealZC(TcpSocket& socket)
{
    auto ToWaveData = [&](const StructDataZC& recvData)
    {
        static constexpr auto MAX_CHANNEL = 16;
        static unsigned char PackIndexAll[MAX_CHANNEL] = { 0 };
        static std::unique_ptr<StructNetData> resAll[MAX_CHANNEL] = { nullptr };

        if (recvData.ChannelNo < 0 || recvData.ChannelNo >= MAX_CHANNEL)
            return;
        auto& PackIndex = PackIndexAll[recvData.ChannelNo];
        auto& res = resAll[recvData.ChannelNo];
        const auto DataLen = sizeof(DataHead) + sizeof(StructNBWaveZCResult) + 4 * sizeof(StructDataZC::DDCData) + sizeof(DataEnd);

        if (PackIndex == 0 || res == nullptr)
        {
            res = std::make_unique<StructNetData>(0, DataLen);
        }

        auto& NBWaveCXResult = g_Parameter.NBWaveZCResult[recvData.ChannelNo];
        const auto LENGTH = recvData.LENGTH;
        auto Data = recvData.DDCData;
        const auto DataBase = (NarrowDDC*)(res->data + sizeof(DataHead) + sizeof(StructNBWaveZCResult) + PackIndex * sizeof(StructDataZC::DDCData));

        for (int p = 0; p < LENGTH; ++p)
        {
            DataBase[p] = Data[p];
        }

        if (++PackIndex == 4)
        {
            socket.NBZCDataReplay(NBWaveCXResult, res, DataLen, recvData.ChannelNo);
            PackIndex = 0;
        }
    };

    while (true)
    {
        auto ptr = tsqueueZCs.wait_and_pop();
        for (int i = 0; i < ptr->PACK_NUM; ++i)
        {
            if (ptr->ptr[i].Head == 0xA1FA)
                ToWaveData(ptr->ptr[i]);
        }
    }
};