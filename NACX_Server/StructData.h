#ifndef _STRUCT_DATA_H
#define _STRUCT_DATA_H

#include <memory>

#pragma pack(1)

struct RangePhase
{
	short Range;
	short Phase;
};

struct DirectionRange
{
	short Range;
	short Direction;
};

struct StructDataCX
{
	static constexpr auto LENGTH = 6400;
	unsigned int Head; //4 Frame Head 0xCDDCABBA
	unsigned short PackNum; //2 Total Count 0~65535(When Order Update Clear To Zero)
	unsigned char l_PackNum; //1 Local Count 0~31(Frame Count per Scan Time)
	unsigned char DataType; //1 0:Direction 1~4:Channel Freqency
	unsigned char Time[8]; //8 UTC Time
	unsigned char Smooth; //1 Smooth Time 1/2/4/8/16/32
	unsigned char Resolution; //1 Log2(FFT Point Count)10 11 12 13:Resolution
	unsigned char ScanSpeed; //1 Scan Speed	0 1 2 3 4 5
	unsigned char RFMode; // RF Work Mode 0x00:Normal;0x01:Low Noise
	unsigned int DDS_CTRL; // 4 DDS Frequency Ctrl 0~2^29
	unsigned int CentreFreq; //4 Center Frequency KHz
	unsigned char RFGain; //1 Rf Desc dBm
	unsigned char IFGain; //1 If Desc dBm
	unsigned char CorrectGain; //1 Corrector Desc dBm
	unsigned char AntennaLayer; //1 0:Corrector Layer 1:Antenna Layer
	unsigned char CorrectMode; //1 Corrector Work Mode 0x00:Inner 0x01:Outer
	unsigned char WorkMode; //1 Work Mode 0x00:Correct 0x01:Antenna
	unsigned char RFInfo[5]; //5 Rf Status HW_Version SW_Version Temprature Voltage Current
	unsigned char CorrectInfo[5]; //5 Corrector Status HW_Version SW_Version Temprature Voltage Current
	unsigned int StartFreq; //4 Start Frequency kHz
	unsigned int StopFreq; //4 Stop Frequency kHz
	unsigned char Reserved[12]; //12 Reserved
	union {
		RangePhase RangePhaseData[LENGTH];
		DirectionRange DirectionRangeData[LENGTH];
	}; //6400 * 4 Frequecy/Direction Data. When Resolution is 3.125kHz including 1 frame, when 6.25kHz including two frame and so on. All Frames are ordered by time.
};

struct NarrowDDC
{
	short I;
	short Q;
};

struct StructDataZC
{
	static constexpr auto LENGTH = 512;
	unsigned short Head;//2	Frame Head 0xFAA1
	unsigned short PackNum; //2	Total Count 0~65535(When Order Update Clear To Zero)
	unsigned char Time[8];//8 UTC Time
	unsigned char ChannelNo; //1 ChannelNo Total 16 Channels
	unsigned char RFGain; //1 RF Gain Ctrl
	unsigned char MGCCtrl; //1 MGC Control
	unsigned char GainMode; //1 Gain Mode 1:AGC 2:MGC
	unsigned char GainVal; //1	Gain Value
	unsigned short Temprature; //2	Temprature
	unsigned char Window; //1 Window 1:kaiser
	unsigned char Reserved[12]; //12
	NarrowDDC DDCData[LENGTH];//2048 NDDC Data 512 Points
};

#pragma pack()

#endif
