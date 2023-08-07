#include "TinyConfig.h"

#include <iostream>

TinyConfig::TinyConfig(const std::string& fileName):
	FilePath(std::filesystem::current_path() / "config" / fileName)
{
	if (!std::filesystem::exists(FilePath))
	{
		//SetConfigData(Local_Area, "LocalIP", DEFAULT_LOCAL_DATA_IP);
		//SetConfigData(Data_Area, "DataPort", DEFAULT_LOCAL_DATA_PORT);
		//SetConfigData(Order_Area, "OrderPort", DEFAULT_LOCAL_ORDER_PORT);

  //      boost::property_tree::ptree root_node;
		//boost::property_tree::ptree localIP("LocalIP");
		//localIP.put()
		//root_node.add_child(Local_Area, );
  //      boost::property_tree::ini_parser::write_ini(configFileName, root_node);
		std::filesystem::create_directory(std::filesystem::current_path() / "config");
		std::ofstream ofstream(FilePath, std::ios_base::out);
		ofstream << "[" << Local_Area << "]" << std::endl;
		ofstream << "LocalIP = " << DEFAULT_LOCAL_DATA_IP << std::endl;
		ofstream << "[" << Data_Area << "]" << std::endl;
		ofstream << "DataPort = " << DEFAULT_LOCAL_DATA_PORT << std::endl;
		ofstream << "[" << Direction_Area << "]" << std::endl;
		ofstream << "BaseDirection = " << DEFAULT_BASE_DIRECTION / 10.0 << std::endl;
		ofstream.close();
	}
}

const std::string& TinyConfig::Get_LocalIP()
{
	boost::property_tree::ptree root_node;
	boost::property_tree::ini_parser::read_ini(FilePath.string(), root_node);
	auto tag = root_node.get_child(Local_Area);
	LocalIP = tag.get<std::string>("LocalIP");
	std::cout << "LocalIP: " << LocalIP << std::endl;
	return LocalIP;
}

const unsigned short& TinyConfig::Get_DataPort()
{
	boost::property_tree::ptree root_node;
	boost::property_tree::ini_parser::read_ini(FilePath.string(), root_node);
	auto tag = root_node.get_child(Data_Area);
	DataPort = tag.get<unsigned short>("DataPort");
	std::cout << "DataPort: " << DataPort << std::endl;
	return DataPort;
}

const short& TinyConfig::Get_Direction()
{
	boost::property_tree::ptree root_node;
	boost::property_tree::ini_parser::read_ini(FilePath.string(), root_node);
	auto tag = root_node.get_child(Direction_Area);
	BaseDirection = tag.get<double>("BaseDirection") * 10;
	std::cout << "BaseDirection: " << BaseDirection << std::endl;
	return BaseDirection;
}
