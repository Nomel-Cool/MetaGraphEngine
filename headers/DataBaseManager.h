#pragma once
#ifndef DATABASEMANAGER_H
#define DATAMBASEANAGER_H
#include <iostream>
#include <optional>
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include "DataBase.h"

class GraphModel;
class DatabaseManager
{
public:
	DatabaseManager();
	std::string QueryFilePathByModelName(const std::string& model_name);
	std::string RebuildXmlFileByModelName(const std::string& model_name);
	bool InsertModelIndex2DB(const std::string& model_name, const std::string& file_path, const GraphModel& file_manager_handler);
private:
	std::optional<std::vector<std::vector<std::string>>> QueryAutomataData(const std::string& model_name);
	bool UpdateXmlPathInDatabase(const std::string& model_name, const std::string& xml_path);

	MySQLDatabase& mysql_db = MySQLDatabase::GetInstance();
};

#endif // !DATABASEMANAGER_H
