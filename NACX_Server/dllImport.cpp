#include "dllImport.h"
#include "ThreadSafeQueue.h"
#include "StructData.h"
#include "StructDatas.h"

threadsafe_queue<std::shared_ptr<Struct_Datas<StructDataCX>>> tsqueueCXs;
threadsafe_queue<std::shared_ptr<Struct_Datas<StructDataZC>>> tsqueueZCs;

void DataCX(std::shared_ptr<Struct_Datas<StructDataCX>>& pBuf_CX)
{
    tsqueueCXs.push(pBuf_CX);
}

void DataZC(std::shared_ptr<Struct_Datas<StructDataZC>>& pBuf_ZC)
{
    tsqueueZCs.push(pBuf_ZC);
}
