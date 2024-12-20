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

bool DatabaseManager::RegistryModelIndex2DB(const std::string& model_name, const std::string& file_path) {
    try {
        // ȷ�����ݿ�������
        if (!mysql_db.IsConnected()) {
            std::cerr << "Database is not connected!" << std::endl;
            return false;
        }

        // ��ʼ����
        if (!mysql_db.Execute("START TRANSACTION;")) {
            std::cerr << "Failed to start transaction." << std::endl;
            return false;
        }

        // �������� ModelBluePrint ��
        std::string insertModelQuery =
            "INSERT INTO ModelBluePrint (name) "
            "VALUES ('" + model_name + "') "
            "ON DUPLICATE KEY UPDATE name = name;";
        if (!mysql_db.Execute(insertModelQuery)) {
            std::cerr << "Failed to insert or update ModelBluePrint." << std::endl;
            mysql_db.Execute("ROLLBACK;"); // �ع�����
            return false;
        }

        // ��ȡ��Ӧ�� model_id
        std::string selectModelIdQuery =
            "SELECT model_id FROM ModelBluePrint WHERE name = '" + model_name + "';";
        auto result = mysql_db.Query(selectModelIdQuery);
        if (!result.has_value() || *result == nullptr) {
            std::cerr << "Failed to retrieve model_id." << std::endl;
            mysql_db.Execute("ROLLBACK;"); // �ع�����
            return false;
        }

        MYSQL_RES* res = *result;
        MYSQL_ROW row = mysql_fetch_row(res);
        if (!row || !row[0]) {
            std::cerr << "Model ID not found." << std::endl;
            mysql_free_result(res);
            mysql_db.Execute("ROLLBACK;"); // �ع�����
            return false;
        }
        int model_id = std::stoi(row[0]);
        mysql_free_result(res);

        // �������� FilePath ��
        std::string insertFilePathQuery =
            "INSERT INTO FilePath (model_id, file_path) "
            "VALUES (" + std::to_string(model_id) + ", '" + file_path + "') "
            "ON DUPLICATE KEY UPDATE file_path = '" + file_path + "', last_updated = CURRENT_TIMESTAMP;";
        if (!mysql_db.Execute(insertFilePathQuery)) {
            std::cerr << "Failed to insert or update FilePath." << std::endl;
            mysql_db.Execute("ROLLBACK;"); // �ع�����
            return false;
        }

        // �ύ����
        if (!mysql_db.Execute("COMMIT;")) {
            std::cerr << "Failed to commit transaction." << std::endl;
            return false;
        }

        return true; // �����ɹ�
    }
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return false;
    }
}

bool DatabaseManager::RecordModelPieces2DB(const GraphModel& graph_model) {
    try {
        // ȷ�����ݿ�������
        if (!mysql_db.IsConnected()) {
            std::cerr << "Database is not connected!" << std::endl;
            return false;
        }

        // ��ʼ����
        if (!mysql_db.Execute("START TRANSACTION;")) {
            std::cerr << "Failed to start transaction." << std::endl;
            return false;
        }

        // ������ȡ ModelBluePrint �е� model_id
        std::string insertModelQuery =
            "INSERT INTO ModelBluePrint (name) "
            "VALUES ('" + graph_model.model_name + "') "
            "ON DUPLICATE KEY UPDATE name = name;";
        if (!mysql_db.Execute(insertModelQuery)) {
            std::cerr << "Failed to insert or update ModelBluePrint." << std::endl;
            mysql_db.Execute("ROLLBACK;"); // �ع�����
            return false;
        }

        // ��ȡ��Ӧ�� model_id
        std::string selectModelIdQuery =
            "SELECT model_id FROM ModelBluePrint WHERE name = '" + graph_model.model_name + "';";
        auto result = mysql_db.Query(selectModelIdQuery);
        if (!result.has_value() || *result == nullptr) {
            std::cerr << "Failed to retrieve model_id." << std::endl;
            mysql_db.Execute("ROLLBACK;"); // �ع�����
            return false;
        }

        MYSQL_RES* res = *result;
        MYSQL_ROW row = mysql_fetch_row(res);
        if (!row || !row[0]) {
            std::cerr << "Model ID not found." << std::endl;
            mysql_free_result(res);
            mysql_db.Execute("ROLLBACK;"); // �ع�����
            return false;
        }
        int model_id = std::stoi(row[0]);
        mysql_free_result(res);

        // ɾ�� Automata ���е�ǰ model_id �ľ�����
        std::string deleteAutomataQuery = "DELETE FROM Automata WHERE model_id = " + std::to_string(model_id) + ";";
        if (!mysql_db.Execute(deleteAutomataQuery)) {
            std::cerr << "Failed to delete old Automata records." << std::endl;
            mysql_db.Execute("ROLLBACK;"); // �ع�����
            return false;
        }

        // �����µ� Automata ����
        for (const auto& automata : graph_model.automatas) {
            // ���� checksum (����򵥼��㣬ʵ��ʵ�ֿ����ø����ӵĹ�ϣ����)
            std::string checksum = std::to_string(std::hash<std::string>{}(automata.init_status + automata.func_name +
                automata.current_input + automata.current_status +
                automata.terminate_status));

            std::string insertAutomataQuery =
                "INSERT INTO Automata (model_id, init_status, transfer_func, current_input, current_status, terminate_status, checksum) "
                "VALUES (" +
                std::to_string(model_id) + ", '" +
                automata.init_status + "', '" +
                automata.func_name + "', '" +
                automata.current_input + "', '" +
                automata.current_status + "', '" +
                automata.terminate_status + "', '" +
                checksum + "');";

            if (!mysql_db.Execute(insertAutomataQuery)) {
                std::cerr << "Failed to insert Automata record." << std::endl;
                mysql_db.Execute("ROLLBACK;"); // �ع�����
                return false;
            }
        }

        // �ύ����
        if (!mysql_db.Execute("COMMIT;")) {
            std::cerr << "Failed to commit transaction." << std::endl;
            return false;
        }

        return true; // �����ɹ�
    }
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return false;
    }
}

std::vector<std::string> DatabaseManager::GetModelNameList()
{
    std::vector<std::string> modelNames;

    try {
        // ȷ�����ݿ�������
        if (!mysql_db.IsConnected()) {
            std::cerr << "Database is not connected!" << std::endl;
            return modelNames; // ���ؿ��б�
        }

        // ��ѯ ModelBluePrint ���е�����ģ������
        std::string queryStr = "SELECT name FROM ModelBluePrint;";
        auto result = mysql_db.Query(queryStr);

        if (!result.has_value() || *result == nullptr) {
            std::cerr << "Failed to retrieve model names from ModelBluePrint." << std::endl;
            return modelNames; // ���ؿ��б�
        }

        MYSQL_RES* res = *result;
        MYSQL_ROW row;

        // ������ѯ�������ÿ��ģ��������ӵ� QStringList ��
        while ((row = mysql_fetch_row(res)) != nullptr)
            if (row[0])
                modelNames.emplace_back(row[0]);

        // �ͷŲ�ѯ���
        mysql_free_result(res);

    }
    catch (const std::exception& e) {
        std::cerr << "Exception occurred:" << e.what() << std::endl;
    }

    return modelNames;
}
