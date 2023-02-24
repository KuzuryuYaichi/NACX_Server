#include "TcpSession.h"
#include <regex>
#include "boost/property_tree/ini_parser.hpp"
#include "boost/property_tree/ptree.hpp"

extern PARAMETER_SET g_Parameter;
extern threadsafe_queue<std::shared_ptr<Struct_Datas<StructDataCX>>> tsqueueCXs;

TcpSession::TcpSession(boost::asio::ip::tcp::socket&& socket, std::unordered_set<TcpSession*>& SessionSet, std::mutex& SessionSetMutex) :
    socket(std::move(socket)), SessionSet(SessionSet), SessionSetMutex(SessionSetMutex)
{
    boost::system::error_code err_code;
    //this->socket.set_option(boost::asio::ip::tcp::socket::reuse_address(true), err_code);
    //this->socket.set_option(boost::asio::ip::tcp::no_delay(true), err_code);
    //this->socket.set_option(boost::asio::socket_base::linger(true, 0), err_code);
    //this->socket.set_option(boost::asio::socket_base::keep_alive(true), err_code);
    this->socket.set_option(boost::asio::socket_base::send_buffer_size(4 * 1024 * 1024), err_code);
}

TcpSession::~TcpSession()
{
    RemoveFromSet();
}

void TcpSession::StartRevDataWork()
{
    SendState = true;
}

void TcpSession::StopRevDataWork()
{
    SendState = false;
}

void TcpSession::SetAppConfig()
{
    //boost::property_tree::ptree root_node;
    //boost::property_tree::read_ini("Params.ini", root_node);
    //auto tag = root_node.get_child("Params");
    //tag.put("StateMachine", g_Parameter.StateMachine);
    //tag.put("StartFreq", g_Parameter.StartCenterFreq);
    //tag.put("StopFreq", g_Parameter.StopCenterFreq);
    //tag.put("AntennaFreq", g_Parameter.AntennaFreq);
    //tag.put("RFAttenuation", g_Parameter.RFAttenuation);
    //tag.put("MFAttenuation", g_Parameter.MFAttenuation);
    //tag.put("RfMode", g_Parameter.RfMode);
    //tag.put("CorrectAttenuation", g_Parameter.CorrectAttenuation);
    //tag.put("DDS_CTRL", g_Parameter.DDS_CTRL);
    //tag.put("RFAttenuation", g_Parameter.CmdResolution);
    //tag.put("CorrectMode", g_Parameter.CorrectMode);
    //tag.put("Smooth", g_Parameter.Smooth);
    //boost::property_tree::write_ini("Params.ini", root_node);
}

void TcpSession::StartWork()
{
    auto self(shared_from_this());
    std::thread([self, this]
    {
        while (isRunning)
        {
            read();
        }
        std::cout << "Read Thread Exit" << std::endl;
    }).detach();

    std::thread([self, this]()
    {
        while (isRunning)
        {
            std::printf("\r%llu Packets Transferred\n", TransferByte);
            TransferByte = 0;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        std::cout << "Print Thread Exit" << std::endl;
    }).detach();
}

void TcpSession::AddToSet()
{
    StartWork();
    std::lock_guard<std::mutex> lk(SessionSetMutex);
    SessionSet.emplace(this);
}

void TcpSession::read()
{
    static constexpr int BUFFER_LEN = 1024;
    try
    {
        auto data = std::make_shared<Order>(BUFFER_LEN);
        boost::system::error_code err;
        size_t left = BUFFER_LEN, offset = 0;
        //while (left > 0)
        //{
            auto bytes_transferred = socket.read_some(boost::asio::buffer(data->order + offset, left), err);
            if (err.failed())
            {
                std::cout << "Read Failed: " << err.what() << std::endl;
                isRunning = false;
                return;
            }
            //left -= bytes_transferred;
            //offset += bytes_transferred;
        //}
        RecvCommandFun(data);
    }
    catch (const std::exception& e)
    {
        std::cout << "Read Exception:" << e.what() << std::endl;
        isRunning = false;
    }
}

void TcpSession::write(std::shared_ptr<StructNetData> data)
{
    std::lock_guard<std::mutex> lk(WriteLock);
    try
    {
        boost::system::error_code err;
        size_t left = data->length, offset = 0;
        while (left > 0)
        {
            auto bytes_transferred = socket.write_some(boost::asio::buffer(data->data + offset, left), err);
            if (err.failed())
            {
                std::cout << "Write Failed: " << err.what() << std::endl;
                isRunning = false;
                return;
            }
            left -= bytes_transferred;
            offset += bytes_transferred;
        }
        ++TransferByte;
    }
    catch (const std::exception& e)
    {
        std::cout << "Write Exception: " << e.what() << std::endl;
        isRunning = false;
    }
}

void TcpSession::RemoveFromSet()
{
    std::lock_guard<std::mutex> lk(SessionSetMutex);
    if (SessionSet.count(this))
        SessionSet.erase(this);
}

void TcpSession::ControlReplay(unsigned int Task, short ControlFlag, short ErrorMsg)
{
    static constexpr auto DataLen = sizeof(DataHead) + sizeof(StructControlRev);
    auto res = std::make_shared<StructNetData>(1, DataLen);
    DataHeadToByte(0x0500, DataLen, res->data);
    new (res->data + sizeof(DataHead)) StructControlRev(Task, ControlFlag, ErrorMsg);
    write(res);
}

void TcpSession::WorkParmReplay(const StructWorkCommandRev& ReplayParm)
{
    static constexpr auto DataLen = sizeof(DataHead) + sizeof(StructWorkCommandRev);
    auto res = std::make_shared<StructNetData>(1, DataLen);
    DataHeadToByte(0x0511, DataLen, res->data);
    *(StructWorkCommandRev*)(res->data + sizeof(DataHead)) = ReplayParm;
    write(res);
}

void TcpSession::ScheckReplay(const StructDeviceScheckRev& ReplayParm)
{
    static constexpr auto DataLen = sizeof(DataHead) + sizeof(StructDeviceScheckRev);
    auto res = std::make_shared<StructNetData>(1, DataLen);
    DataHeadToByte(0x0512, DataLen, res->data);
    *(StructDeviceScheckRev*)(res->data + sizeof(DataHead)) = ReplayParm;
    write(res);
}

void TcpSession::RecvCommandFun(std::shared_ptr<Order> buffer)
{
    auto head = (DataHead*)buffer->order;
    if (head->Head != 0xF99FEFFE || head->PackType != 0x08FE)
    {
        return;
    }
    auto CommandPack = std::string(buffer->order + sizeof(DataHead), head->PackLen - sizeof(DataHead));

    std::regex re(";|\r\n");
    std::sregex_token_iterator first(CommandPack.begin(), CommandPack.end(), re, -1), last;
    std::vector<std::string> Cmd(first, last);

    if (Cmd.size() == 0)
        return;
    unsigned int TaskValue = 0;
    if (Cmd[0].find_first_of("Task:") != 0)
        return;
    TaskValue = std::stoul(Cmd[0].substr(sizeof("Task")));
    if (Cmd.size() <= 2)
    {
        ControlReplay(TaskValue, 0, 1);
        return;
    }
    if (Cmd[2].find_first_of("Type:") != 0)
        return;
    short TypeValue = std::stoul(Cmd[2].substr(sizeof("Type")), 0, 16);
    switch (TypeValue)
    {
    case 0x0201:
    {
        if (Cmd[3].find_first_of("Scheck:") == 0)
        {
            ControlReplay(TaskValue, 1, 0);
            std::cout << "Type: SelfCheck, Val: Dev State SelfCheck, State: SelfChecking" << std::endl;
            SelfCheck();
        }
        break;
    }
    case 0x0202:
    {
        if (Cmd[3].find_first_of("WorkCtrl:") == 0)
        {
            short WorkCtrlValue = std::stoul(Cmd[3].substr(sizeof("WorkCtrl")));
            switch (WorkCtrlValue)
            {
            case 0:
            {
                ControlReplay(TaskValue, 1, 0);
                std::cout << "Type: WorkCtrl, Val: Work Param Inquire, State: Inquiring" << std::endl;
                ReplayCommand.Task = TaskValue;
                WorkParmReplay(ReplayCommand);
                std::cout << "State: Inquired" << std::endl;
                break;
            }
            case 1:
            {
                std::cout << "Type: WorkCtrl, Val: Work Param Stop, State: Stopping" << std::endl;
                StopRevDataWork();
                ControlReplay(TaskValue, 1, 0);
                std::cout << "State: Stopped" << std::endl;
                break;
            }
            case 2:
            {
                std::cout << "Type: WorkCtrl, Val: Reset, State: Resetting" << std::endl;
                //initWorkCommandRev();
                ControlReplay(TaskValue, 1, 0);
                std::cout << "State: Resetted" << std::endl;
                break;
            }
            default:
            {
                ControlReplay(TaskValue, 0, 0);
                break;
            }
            }
        }
        break;
    }
    case 0x0203:
    {
        bool ControlState = SetCmdCXParm(Cmd);
        if (ControlState)
        {
            std::cout << "Type: WorkParam Set, Val: CX Param Set, State: Setting" << std::endl;
            CmdCX.SendCXCmd();
            g_Parameter.SetFixedCXResult(TaskValue, CmdCX.StartCenterFreq, CmdCX.Resolution);
            g_Parameter.SetSweepCXResult(TaskValue, CmdCX.StartCenterFreq, CmdCX.StopCenterFreq, CmdCX.Resolution);
            g_Parameter.SetTestCXResult(TaskValue, CmdCX.StartCenterFreq, CmdCX.Resolution);
            ControlReplay(TaskValue, 1, 0);
            ReplayCommand.Task = TaskValue;
            WorkParmReplay(ReplayCommand);
            SetAppConfig();
            std::cout << "State: Setted" << std::endl;
        }
        else
        {
            ControlReplay(TaskValue, 0, 0);
        }
        break;
    }
    case 0x0204: // Sweep
    {
        if (Cmd[4].find_first_of("SFreq:") == 0)
        {
            SetCmdSweepData(Cmd);
            g_Parameter.DataType = PARAMETER_SET::CX_SWEEP;
        }
        break;
    }
    case 0x0205: // NarrowBand
    {
        if (Cmd[4].find_first_of("DFreq:") == 0)
        {
            SetCmdNBData(Cmd);
            g_Parameter.DataType = PARAMETER_SET::CX_NB;
        }
        break;
    }
    case 0x0206: // WideBand
    {
        if (Cmd[4].find_first_of("CFreq:") == 0)
        {
            SetCmdWBData(Cmd);
            g_Parameter.DataType = PARAMETER_SET::CX_WB;
        }
        break;
    }
    case 0x0207: // Test Data
    {
        if (Cmd[4].find_first_of("CFreq:") == 0)
        {
            SetCmdTestData(Cmd);
            g_Parameter.DataType = PARAMETER_SET::TEST_CHANNEL;
        }
        break;
    }
    case 0x0403:
    {
        SetCmdNBReceiver(Cmd);
        break;
    }
    case 0x0411:
    {
        SetCmdNBChannel(Cmd);
        break;
    }
    default:
    {
        ControlReplay(TaskValue, 0, 0);
        return;
    }
    }
    tsqueueCXs.clear();
}

void TcpSession::SelfCheck()
{
    std::thread([this]()
    {
        CmdCX.StateMachine = 3;
        CmdCX.Resolution = 13;
        CmdCX.CorrectMode = 0;
        //CmdCX.RFAttenuation = 0;
        //CmdCX.MFAttenuation = 0;
        CmdCX.CorrectAttenuation = 0;
        //CmdCX.StartCenterFreq = 350000;
        //CmdCX.StopCenterFreq = 350000;
        //g_Parameter.SetTestCXResult(TaskValue, CmdCX.StartCenterFreq, CmdCX.Resolution);
        CmdCX.SendCXCmd();
        std::this_thread::sleep_for(std::chrono::seconds(200));
        //g_Parameter.isTestingInner = true;
        //while (g_Parameter.isTestingInner);
        //CmdCX.CorrectMode = 1;
        //CmdControl.SendCmd();
        //std::this_thread::sleep_for(std::chrono::seconds(200));
        //g_Parameter.isTestingOuter = true;
        //while (g_Parameter.isTestingOuter);

        //bool CorrectRes = true, RFRes = true, AntennaRes = true;
        //std::string TestResStr;

        //ReplayScheck.Task = TaskValue;
        //ReplayScheck.DeviveChNum = 4;
        //ReplayScheck.ScheckResult = 0x0000FFE0;

        //TestResStr += "Correct: \r\n";
        //for (int i = 0; i < PARAMETER_SET::CX_CH_NUM; ++i)
        //{
        //    CorrectRes &= g_Parameter.SelfTestInner[i];
        //    if (g_Parameter.SelfTestInner[i])
        //    {
        //        TestResStr += "Check Success";
        //        CorrectRes = true;
        //        break;
        //    }
        //    else
        //    {
        //        ReplayScheck.ScheckResult |= 0x80000000;
        //    }
        //}
        //TestResStr += "\r\n\r\nPDU Check: ";
        //TestResStr += (ReplayScheck.ScheckResult & ((long long)1 << 63)) ? "Success " : "Failed "; //Correct Status
        //TestResStr += "\r\n\r\nRF: \r\n";
        //for (int i = 0; i < PARAMETER_SET::CX_CH_NUM; ++i)
        //{
        //    if (!g_Parameter.SelfTestInner[i])
        //        ReplayScheck.ScheckResult |= 1 << i;
        //    RFRes &= g_Parameter.SelfTestInner[i];
        //    TestResStr += std::to_string(i + 1) + "Channel Check: " + (g_Parameter.SelfTestInner[i] ? "Success " : "Failed ");
        //}

        //ReplayScheck.AGroupNum = 2;
        //ReplayScheck.AScheckResult = 0xFFFFFFE0;
        //TestResStr += "\r\n\r\nAntenna: \r\n";
        //for (int i = 0; i < PARAMETER_SET::CX_CH_NUM; ++i)
        //{
        //    if (!g_Parameter.SelfTestOuter[i])
        //    {
        //        ReplayScheck.AScheckResult |= 1 << i;
        //        ReplayScheck.AScheckResult |= 0x100 << i;
        //    }
        //    AntennaRes &= g_Parameter.SelfTestOuter[i];
        //    TestResStr += std::to_string(i + 1) + "Channel Check: " + (g_Parameter.SelfTestOuter[i] ? "Success " : "Failed ");
        //}

        //if (g_Parameter.StartScheck)
        //{
        //    ScheckReplay(ReplayScheck);
        //    std::cout << "State: SelfCheck Finished" << std::endl;
        //    g_Parameter.StartScheck = false;
        //}

        //CmdCX.Resolution = g_Parameter.CmdResolution;
        //CmdCX.StartCenterFreq = g_Parameter.CmdStartCenterFreq;
        //CmdCX.StopCenterFreq = g_Parameter.CmdStopCenterFreq;

        //auto MGCvalue = ReplayCommand.MGC;
        //if (MGCvalue < 30)
        //{
        //    CmdCX.RFAttenuation = MGCvalue;
        //}
        //else
        //{
        //    CmdCX.RFAttenuation = 30;
        //    CmdCX.MFAttenuation = (MGCvalue - 30);
        //}

        //switch (ReplayCommand.SmNum)
        //{
        //case 4: CmdCX.Smooth = 4; break;
        //case 8: CmdCX.Smooth = 8; break;
        //case 16: CmdCX.Smooth = 16; break;
        //case 32: CmdCX.Smooth = 32; break;
        //default: break;
        //}

        //switch (ReplayCommand.RcvMode)
        //{
        //case 0: CmdCX.RfMode = 0; break;
        //case 2: CmdCX.RfMode = 1; break;
        //default: break;
        //}
        //CmdCX.StateMachine = 1;
        //CmdControl.SendCmd();
    }).detach();
}

bool TcpSession::SetCmdCXParm(const std::vector<std::string>& Cmd)
{
    bool res = true;
    for (size_t n = Cmd.size(), i = 3; i < n; ++i)
    {
        auto index = Cmd[i].find_first_of(':');
        if (index >= 0)
        {
            auto ParmName = Cmd[i].substr(0, index);
            if (ParmName == "Data")
            {
                switch (std::stoi(Cmd[i].substr(sizeof("Data"))))
                {
                case 0: ReplayCommand.Data = 0; break;
                case 1: ReplayCommand.Data = 1; break;
                default: res = false;
                }
                std::cout << "DataType: Ampl Freq Data" << std::endl;
            }
            else if (ParmName == "Detect")
            {
                switch (std::stoi(Cmd[i].substr(sizeof("Detect"))))
                {
                case 0: break;
                case 1: break;
                default: res = false;
                }
                ReplayCommand.Detect = 0;
                std::cout << "SignalSort: FrontBand Not Deal" << std::endl;
            }
            else if (ParmName == "FreqRes")
            {
                float FreqResValue = std::stof(Cmd[i].substr(sizeof("FreqRes")));
                if (FreqResValue == 25.0)
                    CmdCX.Resolution = 10;
                else if (FreqResValue == 12.5)
                    CmdCX.Resolution = 11;
                else if (FreqResValue == 6.25)
                    CmdCX.Resolution = 12;
                else if (FreqResValue == 3.125)
                    CmdCX.Resolution = 13;
                else
                    res = false;
                std::cout << "FreqResolution: " << FreqResValue << std::endl;
                ReplayCommand.FreqRes = FreqResValue;
            }
            else if (ParmName == "SimBW")
            {
                auto simBW = std::stoi(Cmd[i].substr(sizeof("SimBW"))); //SimulatonBandWidth  "KHz"
                if (simBW != 20000)
                    res = false;
                std::cout << "SimulateBandwidth: " << simBW << std::endl;
                ReplayCommand.SimBW = simBW;
            }
            else if (ParmName == "GMode")
            {
                auto GModeValue = std::stoi(Cmd[i].substr(sizeof("GMode")));
                if (GModeValue >= 0 && GModeValue <= 3)
                {
                    ReplayCommand.GMode = GModeValue;
                    if (i + 1 < n && Cmd[i + 1].find_first_of("MGC:") == 0)
                    {
                        ReplayCommand.MGC = std::stoi(Cmd[++i].substr(sizeof("MGC")));//MGC
                    }
                    switch (GModeValue)
                    {
                    case 0:
                    {
                        auto MGCvalue = ReplayCommand.MGC;
                        if (MGCvalue >= 0 && MGCvalue <= 60)
                        {
                            std::cout << "GainType: MGC, GainValue: " << MGCvalue << std::endl;
                            if (MGCvalue < 30)
                                CmdCX.RFAttenuation = MGCvalue;
                            else
                            {
                                CmdCX.RFAttenuation = 30;
                                CmdCX.MFAttenuation = MGCvalue - 30;
                            }
                        }
                        else
                            res = false;
                        break;
                    }
                    case 1:
                    {
                        auto AGCvalue = ReplayCommand.AGC; //AGC
                        switch (AGCvalue)
                        {
                        case 0: break;
                        case 1: break;
                        case 2: break;
                        case 3: break;
                        case 4: break;
                        default: res = false;
                        }
                        std::cout << "GainType: AGC, GainValue: NULL" << std::endl;
                        break;
                    }
                    default: res = false;
                    }
                }
                else
                    res = false;
            }
            else if (ParmName == "SmNum")
            {
                auto SmNumValue = std::stoi(Cmd[i].substr(sizeof("SmNum")));
                switch (SmNumValue)
                {
                case 1:  CmdCX.Smooth = 1; break;
                case 2:  CmdCX.Smooth = 2; break;
                case 4:  CmdCX.Smooth = 4; break;
                case 8:  CmdCX.Smooth = 8; break;
                case 16: CmdCX.Smooth = 16; break;
                case 32: CmdCX.Smooth = 32; break;
                default: res = false;
                }
                std::cout << "SmoothTime: " << CmdCX.Smooth;

                ReplayCommand.SmNum = CmdCX.Smooth;
                auto SmModeValue = std::stoi(Cmd[++i].substr(sizeof("SmMode")));
                switch (SmModeValue)
                {
                case 0: std::cout << ", SmoothMode: Average" << std::endl; break;
                case 1: res = false; break;
                case 2: res = false; break;
                case 3: res = false; break;
                case 4: res = false; break;
                default: res = false; break;
                }
                ReplayCommand.SmMode = SmModeValue;
            }
            else if (ParmName == "LmMode")
            {
                auto LmModeValue = std::stoi(Cmd[i].substr(sizeof("LmMode"))),
                    LmValValue = std::stoi(Cmd[++i].substr(sizeof("LmVal")));
                switch (LmModeValue)
                {
                case 0:
                {
                    if (LmValValue >= 0 && LmValValue <= 50)
                    {
                        std::cout << "LimitedMode: Auto, LimitedValue: " << LmValValue << std::endl;
                    }
                    else
                    {
                        res = false;
                    }
                    break;
                }
                case 1:
                {
                    if (LmValValue <= 0 && LmValValue >= -160)
                    {
                        std::cout << "LimitedMode: Manual, LimitedValue: " << LmValValue << std::endl;
                    }
                    else
                    {
                        res = false;
                    }
                    break;
                }
                default: res = false; break;
                }
                ReplayCommand.LmMode = LmModeValue;
                ReplayCommand.LmVal = LmValValue;
            }
            else if (ParmName == "RcvMode")
            {
                auto RcvModeValue = std::stoi(Cmd[i].substr(sizeof("RcvMode")));
                switch (RcvModeValue)
                {
                case 0: CmdCX.RfMode = 1; std::cout << "WorkMode: Normal Mode" << std::endl; break;
                case 1: res = false; break;
                case 2: CmdCX.RfMode = 2; std::cout << "WorkMode: Low Noise Mode" << std::endl; break;
                default: res = false; break;
                }
                ReplayCommand.RcvMode = RcvModeValue;
            }
        }
    }
    return res;
}

void TcpSession::SetCmdNBData(const std::vector<std::string>& Cmd)
{
    unsigned int BWvalue = 80000;
    int ActValue = 3, CenterFreq = 0;
    short DFMethod = 6, FNumber = 20;

    for (size_t n = Cmd.size(), i = 3; i < n; ++i)
    {
        auto index = Cmd[i].find_first_of(':');
        if (index >= 0)
        {
            auto ParmName = Cmd[i].substr(0, index);
            if (ParmName == "Act")
                ActValue = std::stol(Cmd[i].substr(sizeof("Act")));
            else if (ParmName == "DFreq")
                CenterFreq = std::stol(Cmd[i].substr(sizeof("DFreq"))) / 1000;
            else if (ParmName == "BW")
                BWvalue = std::stoul(Cmd[i].substr(sizeof("BW"))) / 1000;
            else if (ParmName == "DFMethod")
                DFMethod = std::stoul(Cmd[i].substr(sizeof("DFMethod")));
            else if (ParmName == "FNumber")
                FNumber = std::stoul(Cmd[i].substr(sizeof("FNumber")));
        }
    }

    if (ActValue == 1 && CenterFreq >= 200000 && CenterFreq <= 500000)
    {
        ControlReplay(TaskValue, 1, 0);
        CmdCX.StartCenterFreq = CenterFreq;
        CmdCX.StopCenterFreq = CenterFreq;
        std::cout << "CenterFreq: " << CenterFreq << std::endl;
        //Cmd.CorrectMode = 0;
        CmdCX.StateMachine = 0;
        CmdCX.SendCXCmd();
        //NarrowCXResult.CXType = DFMethod;
        //NarrowCXResult.DataPoint = FNumber;
        g_Parameter.SetCmd(CmdCX.StartCenterFreq, CmdCX.StopCenterFreq, CmdCX.Resolution, TaskValue, 0);
        StartRevDataWork();
        std::cout << "Type: Fixed NB CX, Val: Start Gather, State: Gathering" << std::endl;
        SetAppConfig();
    }
    else if (ActValue == 2)
    {
        ControlReplay(TaskValue, 1, 0);
        StopRevDataWork();
        std::cout << "Type: Fixed NB CX, Val: Stop Gather, State: Stopped" << std::endl;
    }
    else
    {
        ControlReplay(TaskValue, 0, 0);
    }
}

void TcpSession::SetCmdWBData(const std::vector<std::string>& Cmd)
{
    int ActValue = 3, CenterFreq = 0;
    for (size_t n = Cmd.size(), i = 3; i < n; i++)
    {
        auto index = Cmd[i].find_first_of(':');
        if (index >= 0)
        {
            auto ParmName = Cmd[i].substr(0, index);
            if (ParmName == "Act")
                ActValue = std::stol(Cmd[i].substr(sizeof("Act")));
            else if (ParmName == "CFreq")
                CenterFreq = std::stol(Cmd[i].substr(sizeof("CFreq"))) / 1000;
        }
    }

    if (ActValue == 1 && CenterFreq >= 200000 && CenterFreq <= 500000)
    {
        ControlReplay(TaskValue, 1, 0);
        CmdCX.StartCenterFreq = CenterFreq;
        CmdCX.StopCenterFreq = CenterFreq;
        std::cout << "CenterFreq: " << CenterFreq << std::endl;
        CmdCX.CorrectMode = 1;
        CmdCX.StateMachine = 0; //1
        CmdCX.SendCXCmd();
        //g_Parameter.SetCmd(Cmd.StartCenterFreq, Cmd.StopCenterFreq, Cmd.Resolution, TaskValue, 1);
        g_Parameter.SetFixedCXResult(TaskValue, CmdCX.StartCenterFreq, CmdCX.Resolution);
        StartRevDataWork();
        std::cout << "Type: Fixed WB CX, Val: Start Gather, State: Gathering" << std::endl;
        SetAppConfig();
    }
    else if (ActValue == 2)
    {
        ControlReplay(TaskValue, 1, 0);
        StopRevDataWork();
        std::cout << "Type: Fixed WB CX, Val: Stop Gather, State: Stopped" << std::endl;
    }
    else
    {
        ControlReplay(TaskValue, 0, 0);
    }
}

void TcpSession::SetCmdSweepData(const std::vector<std::string>& Cmd)
{
    int ActValue = 3, StartFreq = 0, StopFreq = 0;
    for (size_t n = Cmd.size(), i = 3; i < n; ++i)
    {
        auto index = Cmd[i].find_first_of(':');
        if (index >= 0)
        {
            auto ParmName = Cmd[i].substr(0, index);
            if (ParmName == "Act")
                ActValue = std::stol(Cmd[i].substr(sizeof("Act")));
            else if (ParmName == "SFreq")
                StartFreq = std::stol(Cmd[i].substr(sizeof("SFreq"))) / 1000;
            else if (ParmName == "EFreq")
                StopFreq = std::stol(Cmd[i].substr(sizeof("EFreq"))) / 1000;
        }
    }

    auto SetFreqCmd = [&]()
    {
        g_Parameter.SetCmd(CmdCX.StartCenterFreq, CmdCX.StopCenterFreq, CmdCX.Resolution, TaskValue, 1);
        int startCenterFreq = 0, stopCenterFreq = 0;
        static constexpr int MIN_START_CENTER_FREQ = 200000, MAX_STOP_CENTER_FREQ = 500000, BAND_WIDTH = 20000, HALF_BAND_WIDTH = BAND_WIDTH / 2;
        if (StartFreq == StopFreq)
        {
            startCenterFreq = StartFreq;
            stopCenterFreq = StopFreq;
        }
        else
        {
            startCenterFreq = MIN_START_CENTER_FREQ;
            stopCenterFreq = MAX_STOP_CENTER_FREQ;
            for (startCenterFreq = MIN_START_CENTER_FREQ; startCenterFreq + HALF_BAND_WIDTH < StartFreq && startCenterFreq < MAX_STOP_CENTER_FREQ; startCenterFreq += BAND_WIDTH);
            for (stopCenterFreq = MAX_STOP_CENTER_FREQ; stopCenterFreq - HALF_BAND_WIDTH > StopFreq && stopCenterFreq > MIN_START_CENTER_FREQ; stopCenterFreq -= BAND_WIDTH);
        }

        if (startCenterFreq < MIN_START_CENTER_FREQ)
            startCenterFreq = MIN_START_CENTER_FREQ;
        if (stopCenterFreq > MAX_STOP_CENTER_FREQ)
            stopCenterFreq = MAX_STOP_CENTER_FREQ;

        CmdCX.StartCenterFreq = startCenterFreq;
        CmdCX.StopCenterFreq = stopCenterFreq;
        std::cout << "StartFreq: " << startCenterFreq << " StopFreq: " << stopCenterFreq << std::endl;
        CmdCX.SendCXCmd();
        g_Parameter.SetSweepCXResult(TaskValue, CmdCX.StartCenterFreq, CmdCX.StopCenterFreq, CmdCX.Resolution);
    };

    if (ActValue == 1 && StartFreq <= StopFreq)
    {
        ControlReplay(TaskValue, 1, 0);
        CmdCX.StateMachine = 0;
        SetFreqCmd();
        StartRevDataWork();
        std::cout << "Type: BandScan, Val: Start Scan, State: Scanning" << std::endl;
        SetAppConfig();
    }
    else if (ActValue == 2)
    {
        ControlReplay(TaskValue, 1, 0);
        StopRevDataWork();
        std::cout << "Type: BandScan, Val: Stop Scan, State: Stopped" << std::endl;
    }
    else
    {
        ControlReplay(TaskValue, 0, 0);
    }
}

void TcpSession::SetCmdTestData(const std::vector<std::string>& Cmd)
{
    int ActValue = 3, CenterFreq = 0, mode = 0, scope = 0;
    for (size_t n = Cmd.size(), i = 3; i < n; ++i)
    {
        auto index = Cmd[i].find_first_of(':');
        if (index >= 0)
        {
            auto ParmName = Cmd[i].substr(0, index);
            if (ParmName == "Act")
                ActValue = std::stol(Cmd[i].substr(sizeof("Act")));
            else if (ParmName == "CFreq")
                CenterFreq = std::stol(Cmd[i].substr(sizeof("CFreq"))) / 1000;
            else if (ParmName == "Mode")
                mode = std::stol(Cmd[i].substr(sizeof("Mode")));
            else if (ParmName == "Scope")
                scope = std::stol(Cmd[i].substr(sizeof("Scope")));
        }
    }
    switch (ActValue)
    {
    case 1:
    {
        ControlReplay(TaskValue, 1, 0);
        CmdCX.StartCenterFreq = CenterFreq;
        CmdCX.StopCenterFreq = CenterFreq;
        std::cout << "StartFreq: " << CenterFreq << " StopFreq: " << CenterFreq << std::endl;
        CmdCX.StateMachine = 2;
        CmdCX.CorrectAttenuation = scope;
        //Cmd.CorrectMode = 0;
        CmdCX.SendCXCmd();
        g_Parameter.SetTestCXResult(TaskValue, CmdCX.StartCenterFreq, CmdCX.Resolution);
        StartRevDataWork();
        std::cout << "Type: Test Data, Val: Start Transfer State: Transfering" << std::endl;
        SetAppConfig();
        break;
    }
    case 2:
    {
        ControlReplay(TaskValue, 1, 0);
        StopRevDataWork();
        std::cout << "Type: Test Data, Val: Stop Transfer State: Stopped" << std::endl;
        break;
    }
    default:
    {
        ControlReplay(TaskValue, 0, 0);
        break;
    }
    }
}

void TcpSession::SetCmdNBReceiver(const std::vector<std::string>& Cmd)
{
    CmdZC.CmdType = 0;
    for (size_t n = Cmd.size(), i = 3; i < n; ++i)
    {
        auto index = Cmd[i].find_first_of(':');
        if (index >= 0)
        {
            auto ParmName = Cmd[i].substr(0, index);
            if (ParmName == "Freq")
            {
                auto Freq = std::stol(Cmd[i].substr(sizeof("Freq"))) / 1000;
                CmdZC.CmdRF.RfType = 1;
                CmdZC.CmdRF.RfData = Freq;
                g_Parameter.NbCenterFreqRF = Freq;
            } 
        }
    }
    CmdZC.SendZCCmd();
}

void TcpSession::SetCmdNBChannel(const std::vector<std::string>& Cmd)
{
    CmdZC.CmdType = 1;
    for (size_t n = Cmd.size(), i = 3; i < n; ++i)
    {
        auto index = Cmd[i].find_first_of(':');
        if (index >= 0)
        {
            auto ParmName = Cmd[i].substr(0, index);
            if (ParmName == "BankNum")
            {
                auto BankNum = std::stol(Cmd[i].substr(sizeof("BankNum")));
                if (BankNum < 0 || BankNum > 15)
                    return;
                CmdZC.CmdNB.Channel = BankNum;
            }
            else if (ParmName == "Freq")
            {
                auto Freq = std::stol(Cmd[i].substr(sizeof("Freq"))) / 1000;
                CmdZC.CmdNB.DDS = std::round(std::pow(2ll, 32) * (Freq - g_Parameter.NbCenterFreqRF) / 250000);
            }
            else if (ParmName == "DDCBW")
            {
                auto DDCBW = std::stol(Cmd[i].substr(sizeof("DDCBW"))) / 1000;
                switch (DDCBW)
                {
                case 2400: CmdZC.CmdNB.CIC = 8000; break;
                case 4800: CmdZC.CmdNB.CIC = 4000; break;
                case 9600: CmdZC.CmdNB.CIC = 2000; break;
                case 19200: CmdZC.CmdNB.CIC = 1000; break;
                case 38400: CmdZC.CmdNB.CIC = 500; break;
                case 76800: CmdZC.CmdNB.CIC = 250; break;
                case 96000: CmdZC.CmdNB.CIC = 200; break;
                default: break;
                }
            }
        }
    }
    CmdZC.SendZCCmd();
}
