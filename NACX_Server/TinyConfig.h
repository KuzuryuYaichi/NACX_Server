#ifndef _TINY_CONFIG_H
#define _TINY_CONFIG_H

#include "boost/property_tree/ini_parser.hpp"
#include "boost/property_tree/ptree.hpp"
#include <filesystem>
#include "boost/asio.hpp"

class TinyConfig
{
public:
	TinyConfig(const std::string&);
	const std::string& Get_LocalIP();
	const unsigned short& Get_DataPort();

private:
	static constexpr char DEFAULT_LOCAL_DATA_IP[] = "127.0.0.1";
	static constexpr int DEFAULT_LOCAL_DATA_PORT = 5021;
	static constexpr char Local_Area[] = "Local", Data_Area[] = "Data";

	const std::string fileName;
	const std::string ConfigFilePath;
	std::string LocalIP;
	unsigned short DataPort = 0;
	unsigned short OrderPort = 0;
};

#endif
