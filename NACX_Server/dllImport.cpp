#include "dllImport.h"
#include "ThreadSafeQueue.h"
#include "StructData.h"
#include "StructDatas.h"

threadsafe_queue<std::unique_ptr<Struct_Datas<StructDataCX>>> tsqueueCXs;
threadsafe_queue<std::unique_ptr<Struct_Datas<StructDataZC>>> tsqueueZCs;

void DataCX(std::unique_ptr<Struct_Datas<StructDataCX>>& pBuf_CX)
{
    tsqueueCXs.push(std::move(pBuf_CX));
}

void DataZC(std::unique_ptr<Struct_Datas<StructDataZC>>& pBuf_ZC)
{
    tsqueueZCs.push(std::move(pBuf_ZC));
}
