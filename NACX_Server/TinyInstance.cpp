#include "TinyInstance.h"
#include "../XDMA_PCIE/dllexport.h"
#include "dllImport.h"
#include "DataThread.h"

constexpr char CONFIG_FILE[] = "config.ini";

TinyInstance::TinyInstance(): tinyConfig(CONFIG_FILE),
    ServerSocket(tinyConfig.Get_LocalIP(), tinyConfig.Get_DataPort()), SerialPort("COM1", 9600),
    DataThreadCX(DataDealCX, std::ref(ServerSocket), tinyConfig.Get_Direction()), DataThreadZC(DataDealZC, std::ref(ServerSocket))
{
    InitThread();
    StructCmdCX CmdCX;
    CmdCX.SendSample();
    ServerSocket.Run();
}

void TinyInstance::join()
{
    DataThreadCX.join();
    DataThreadZC.join();
}

void TinyInstance::InitThread()
{
    RegisterCallBackCX(DataCX);
    RegisterCallBackZC(DataZC);
    OpenDevice();
}

TinyInstance::~TinyInstance()
{
    CloseDevice();
}
