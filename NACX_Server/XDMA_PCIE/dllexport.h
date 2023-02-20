#ifndef _DLL_EXPORT_H
#define _DLL_EXPORT_H

#include "XDMA_PCIE_global.h"

#define EXPORT extern "C" Q_DECL_EXPORT

EXPORT int OpenDevice();

EXPORT int CloseDevice();

EXPORT void RegisterCallBackCX(P_CXDATA_CALLBACK pfunc);

EXPORT void RegisterCallBackZC(P_ZCDATA_CALLBACK pfunc);

EXPORT int StopCallbackFunc();

EXPORT void WriteStreamCmd(char* cmd);

EXPORT void WriteStreamSample(char* sample, size_t totalLen);

#endif
