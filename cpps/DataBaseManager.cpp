#include "DataBaseManager.h"

DatabaseManager::DatabaseManager()
{
    // ���������������ޣ�û�����κ����ݿ����Ӽ��ܹ���
    mysql_db.Init("localhost", "root", "123456", "meta_graph_model");
    if (mysql_db.IsConnected())
        std::cout << "The database has connected." << std::endl;
}

std::string DatabaseManager::QueryFilePathByModelName(const std::string& model_name) 
{
    // ȷ�����ݿ�������
    if (!mysql_db.IsConnected()) 
    {
        std::cerr << "Database is not connected!" << std::endl;
        return "";
    }

    // ���� SQL ��ѯ���
    std::string query = "SELECT f.file_path FROM FilePath f JOIN ModelBluePrint m ON f.model_id = m.model_id WHERE m.name = '" + model_name + "'";

    // ִ�в�ѯ
    std::optional<MYSQL_RES*> result_opt = mysql_db.Query(query);
    if (!result_opt.has_value()) 
    {
        std::cerr << "Query failed or no result: " << query << std::endl;
        return "";
    }

    MYSQL_RES* result = result_opt.value();
    MYSQL_ROW row;
    std::string file_path;

    // ��ȡ��ѯ���
    if ((row = mysql_fetch_row(result))) 
    {
        if (row[0]) // ������Ƿ�Ϊ��
        {
            file_path = row[0];
        }
    }
    else
    {
        std::cerr << "No file path found for model: " << model_name << std::endl;
    }

    // �ͷŲ�ѯ���
    mysql_free_result(result);

    return file_path;
}

std::optional<std::vector<std::vector<std::string>>> DatabaseManager::QueryAutomataData(const std::string& model_name) 
{
    if (!mysql_db.IsConnected())
    {
        std::cerr << "Database is not connected!" << std::endl;
        return std::nullopt;
    }

    // ��ѯ Automata ��������ֶ�
    std::string query = "SELECT id, init_status, func_name, current_input, current_status, terminate_status FROM AutoMata WHERE model_name = '" + model_name + "'";

    auto result_opt = mysql_db.Query(query);
    if (!result_opt.has_value())
    {
        std::cerr << "Failed to query automata data for model: " << model_name << std::endl;
        return std::nullopt;
    }

    MYSQL_RES* result = result_opt.value();
    MYSQL_ROW row;
    std::vector<std::vector<std::string>> rows;

    // ������ѯ���
    while ((row = mysql_fetch_row(result)))
    {
        std::vector<std::string> automata_row;
        for (int i = 0; i < mysql_num_fields(result); ++i)
            automata_row.emplace_back(row[i] ? row[i] : "");
        rows.push_back(std::move(automata_row));
    }

    mysql_free_result(result);
    return rows;
}

bool DatabaseManager::UpdateXmlPathInDatabase(const std::string& model_name, const std::string& xml_path)
{
    MySQLDatabase& mysql_db = MySQLDatabase::GetInstance();
    std::string update_query = "UPDATE ModelFilePath SET file_path = '" + xml_path + "' WHERE model_name = '" + model_name + "'";
    return mysql_db.Execute(update_query);
}

std::string DatabaseManager::RebuildXmlFileByModelName(const std::string& model_name)
{
    // 1. ��ѯ���ݿ��� Automata ����
    auto automata_data_opt = QueryAutomataData(model_name);
    if (!automata_data_opt.has_value() || automata_data_opt->empty())
    {
        std::cerr << "No automata data found for model: " << model_name << std::endl;
        return "";
    }

    // 2. ���� XML �ļ�·��
    std::string file_name = model_name + "_Autogen.xml";
    std::filesystem::path file_path = std::filesystem::current_path() / file_name;

    // 3. д�� XML �ļ�
    std::ofstream ofs(file_path);
    if (!ofs.is_open()) 
    {
        std::cerr << "Failed to create XML file: " << file_path << std::endl;
        return "";
    }

    ofs << "<model name=\"" << model_name << "\">\n";

    // ������ѯ�������ݲ����� XML ����
    for (const auto& row : automata_data_opt.value())
    {
        ofs << "  <automata id=\"" << row[0] << "\">\n";
        ofs << "    <init init_status='" << row[1] << "'/>\n";
        ofs << "    <transfer func_name=\"" << row[2] << "\"/>\n";
        ofs << "    <input current_input='" << row[3] << "'/>\n";
        ofs << "    <current current_status='" << row[4] << "'/>\n";
        ofs << "    <terminate terminate_status='" << row[5] << "'/>\n";
        ofs << "  </automata>\n";
    }

    ofs << "</model>\n";
    ofs.close();

    // 4. �������ݿ��е��ļ�·��
    if (!UpdateXmlPathInDatabase(model_name, file_path.string()))
    {
        std::cerr << "Failed to update XML file path in database for model: " << model_name << std::endl;
        return "";
    }

    // 5. �������ɵ� XML �ļ�·��
    return file_path.string();
}

bool DatabaseManager::InsertModelIndex2DB(const std::string& model_name, const std::string& file_path, const GraphModel& file_manager_handler) {
    return true;
}
