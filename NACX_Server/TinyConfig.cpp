#include "TinyConfig.h"

TinyConfig::TinyConfig(const std::string& fileName): fileName(fileName),
	ConfigFilePath(std::filesystem::current_path().string() + "/config/")
{
	if (!std::filesystem::exists(ConfigFilePath + fileName))
	{
		//SetConfigData(Local_Area, "LocalIP", DEFAULT_LOCAL_DATA_IP);
		//SetConfigData(Data_Area, "DataPort", DEFAULT_LOCAL_DATA_PORT);
		//SetConfigData(Order_Area, "OrderPort", DEFAULT_LOCAL_ORDER_PORT);

  //      boost::property_tree::ptree root_node;
		//boost::property_tree::ptree localIP("LocalIP");
		//localIP.put()
		//root_node.add_child(Local_Area, );
  //      boost::property_tree::ini_parser::write_ini(configFileName, root_node);
		std::filesystem::create_directory(ConfigFilePath);
		std::ofstream ofstream(ConfigFilePath + fileName, std::ios_base::out);
		ofstream << "[" << Local_Area << "]" << std::endl;
		ofstream << "LocalIP = " << DEFAULT_LOCAL_DATA_IP << std::endl;
		ofstream << "[" << Data_Area << "]" << std::endl;
		ofstream << "DataPort = " << DEFAULT_LOCAL_DATA_PORT << std::endl;
		ofstream << "[" << Order_Area << "]" << std::endl;
		ofstream << "OrderPort = " << DEFAULT_LOCAL_ORDER_PORT << std::endl;
		ofstream.close();
	}
}

const std::string& TinyConfig::Get_LocalIP()
{
	boost::property_tree::ptree root_node;
	boost::property_tree::ini_parser::read_ini(ConfigFilePath + fileName, root_node);
	auto tag = root_node.get_child(Local_Area);
	return LocalIP = tag.get<std::string>("LocalIP");
}

const unsigned short& TinyConfig::Get_DataPort()
{
	boost::property_tree::ptree root_node;
	boost::property_tree::ini_parser::read_ini(ConfigFilePath + fileName, root_node);
	auto tag = root_node.get_child(Data_Area);
	return DataPort = tag.get<unsigned short>("DataPort");
}

const unsigned short& TinyConfig::Get_OrderPort()
{
	boost::property_tree::ptree root_node;
	boost::property_tree::ini_parser::read_ini(ConfigFilePath + fileName, root_node);
	auto tag = root_node.get_child(Order_Area);
	return OrderPort = tag.get<unsigned short>("OrderPort");
}
