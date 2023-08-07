#ifndef _TINY_CONFIG_H
#define _TINY_CONFIG_H

#include "boost/property_tree/ini_parser.hpp"
#include "boost/property_tree/ptree.hpp"
#include <filesystem>
#include "boost/asio.hpp"
#include "global.h"

class TinyConfig
{
public:
	TinyConfig(const std::string&);
	const std::string& Get_LocalIP();
	const unsigned short& Get_DataPort();
	const short& Get_Direction();

private:
	static constexpr char DEFAULT_LOCAL_DATA_IP[] = "127.0.0.1";
	static constexpr unsigned short DEFAULT_LOCAL_DATA_PORT = 5021;
	static constexpr short DEFAULT_BASE_DIRECTION = BASE_DIRECTION;
	static constexpr char Local_Area[] = "Local", Data_Area[] = "Data", Direction_Area[] = "Direction";

	const std::filesystem::path FilePath;
	std::string LocalIP = DEFAULT_LOCAL_DATA_IP;
	unsigned short DataPort = DEFAULT_LOCAL_DATA_PORT;
	short BaseDirection = DEFAULT_BASE_DIRECTION;
};

#endif
