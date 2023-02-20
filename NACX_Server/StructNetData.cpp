#include "StructNetData.h"

PARAMETER_SET g_Parameter;

void DataHeadToByte(unsigned short PackType, size_t PackLen, char* result, unsigned short PackNum)
{
    new (result) DataHead(PackLen, PackType, g_Parameter.DeviceID, PackNum);
}

void DataHeadToByte(unsigned short PackType, size_t PackLen, char* result)
{
    new (result) DataHead(PackLen, PackType, g_Parameter.DeviceID);
}

void DataHeadToByte(unsigned short PackType, size_t PackLen, char* result, unsigned char Channel)
{
    new (result) DataHead(PackLen, PackType, g_Parameter.DeviceID, Channel);
}

void DataEndToByte(char* result)
{
    new (result) DataEnd(1);
}