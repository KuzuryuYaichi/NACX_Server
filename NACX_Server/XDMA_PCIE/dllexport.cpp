#include "dllexport.h"
#include "stream_public.hpp"
#include <exception>

xdma_device* pdev = nullptr;
std::thread CX_Thread, ZC_Thread;
P_CXDATA_CALLBACK CallbackCX = nullptr;
P_ZCDATA_CALLBACK CallbackZC = nullptr;

int OpenDevice()
{
#ifdef XDMA_PCIE_WIN_LIB
    const auto device_paths = get_device_paths(GUID_DEVINTERFACE_XDMA);
#elif defined XDMA_PCIE_UNIX_LIB
	const auto device_paths = get_device_paths();
#endif
    if (device_paths.empty()) {
		return -1;
    }
	pdev = new xdma_device(device_paths[0]);
	SetIsRunning(true);
	ReadThread(*pdev, CallbackCX, CallbackZC, CX_Thread, ZC_Thread);
	return 0;
}

int CloseDevice()
{
	SetIsRunning(false);
	if(CX_Thread.joinable())
		CX_Thread.join();
	if(ZC_Thread.joinable())
		ZC_Thread.join();
	if(pdev != nullptr)
		delete pdev;
    return 0;
}

void RegisterCallBackCX(P_CXDATA_CALLBACK pfunc)
{
    CallbackCX = pfunc;
}

void RegisterCallBackZC(P_ZCDATA_CALLBACK pfunc)
{
	CallbackZC = pfunc;
}

int StopCallbackFunc()
{
    CallbackCX = nullptr;
	CallbackZC = nullptr;
	return 0;
}

void WriteStreamCmd(char* cmd, size_t totalLen, int channel)
{
	size_t offset = 0;
	while (totalLen > 0)
	{
		auto len = pdev->write_to_engine(cmd + offset, totalLen, channel);
		totalLen -= len;
		offset += len;
	}
}

void WriteStreamSample(char* sample, size_t totalLen)
{
	size_t offset = 0;
	while (totalLen > 0)
	{
		auto len = pdev->write_to_engine(sample + offset, totalLen, 0);
		totalLen -= len;
		offset += len;
	}
}
