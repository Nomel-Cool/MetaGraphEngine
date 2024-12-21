#include "DataBaseManager.h"

DatabaseManager::DatabaseManager()
{
    // 个人能力精力所限，没加入任何数据库连接加密过程
    if (mysql_db.IsConnected())
        std::cout << "The database has connected." << std::endl;
    else
        mysql_db.Init("localhost", "root", "123456", "meta_graph_model");
}

std::string DatabaseManager::QueryFilePathByModelName(const std::string& model_name) 
{
    // 确保数据库已连接
    if (!mysql_db.IsConnected()) 
    {
        std::cerr << "Database is not connected!" << std::endl;
        return "";
    }

    // 构建 SQL 查询语句
    std::string query = "SELECT f.file_path FROM FilePath f JOIN ModelBluePrint m ON f.model_id = m.model_id WHERE m.name = '" + model_name + "'";

    // 执行查询
    std::optional<MYSQL_RES*> result_opt = mysql_db.Query(query);
    if (!result_opt.has_value()) 
    {
        std::cerr << "Query failed or no result: " << query << std::endl;
        return "";
    }

    MYSQL_RES* result = result_opt.value();
    MYSQL_ROW row;
    std::string file_path;

    // 读取查询结果
    if ((row = mysql_fetch_row(result))) 
    {
        if (row[0]) // 检查结果是否为空
        {
            file_path = row[0];
        }
    }
    else
    {
        std::cerr << "No file path found for model: " << model_name << std::endl;
    }

    // 释放查询结果
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

    MYSQL* conn = mysql_db.GetConnection();
    if (!conn)
    {
        std::cerr << "Failed to get MySQL connection!" << std::endl;
        return std::nullopt;
    }

    // 使用 std::vector 替代 VLAs
    size_t buffer_size = 2 * model_name.size() + 1;
    std::vector<char> escaped_model_name(buffer_size);

    // 转义字符串
    mysql_real_escape_string(conn, escaped_model_name.data(), model_name.c_str(), model_name.size());

    // 构建查询语句
    std::string query =
        "SELECT a.automata_id, a.init_status, a.transfer_func, a.current_input, a.current_status, a.terminate_status "
        "FROM Automata AS a "
        "JOIN ModelBluePrint AS m ON a.model_id = m.model_id "
        "WHERE m.name = '" + std::string(escaped_model_name.data()) + "'";

    // 执行查询
    auto result_opt = mysql_db.Query(query);
    if (!result_opt.has_value())
    {
        std::cerr << "Failed to query automata data for model: " << model_name << std::endl;
        return std::nullopt;
    }

    MYSQL_RES* result = result_opt.value();
    MYSQL_ROW row;
    std::vector<std::vector<std::string>> rows;

    // 遍历查询结果
    while ((row = mysql_fetch_row(result)))
    {
        std::vector<std::string> automata_row;
        for (int i = 0; i < mysql_num_fields(result); ++i)
        {
            automata_row.emplace_back(row[i] ? row[i] : "");
        }
        rows.push_back(std::move(automata_row));
    }

    // 释放查询结果
    mysql_free_result(result);
    return rows;
}


bool DatabaseManager::UpdateXmlPathInDatabase(const std::string& model_name, const std::string& xml_path)
{
    if (!mysql_db.IsConnected())
    {
        std::cerr << "Database is not connected!" << std::endl;
        return false;
    }

    MYSQL* conn = mysql_db.GetConnection();
    if (!conn)
    {
        std::cerr << "Failed to get MySQL connection!" << std::endl;
        return false;
    }

    // 动态分配缓冲区并转义 model_name 和 xml_path
    std::vector<char> escaped_model_name(2 * model_name.size() + 1);
    std::vector<char> escaped_xml_path(2 * xml_path.size() + 1);

    mysql_real_escape_string(conn, escaped_model_name.data(), model_name.c_str(), model_name.size());
    mysql_real_escape_string(conn, escaped_xml_path.data(), xml_path.c_str(), xml_path.size());

    // 使用转义后的字符串构建查询
    std::string update_query =
        "UPDATE FilePath "
        "SET file_path = '" + std::string(escaped_xml_path.data()) + "' "
        "WHERE model_id = (SELECT model_id FROM ModelBluePrint WHERE name = '" + std::string(escaped_model_name.data()) + "')";

    if (!mysql_db.Execute(update_query))
    {
        std::cerr << "Failed to update file path for model: " << model_name << std::endl;
        return false;
    }

    return true;
}




#include <fstream>
#include <filesystem>
#include <iostream>
#include <vector>
#include <string>
#include <optional>
#include <algorithm>

// 转义函数：将双引号（"）转义为反斜杠双引号（\"）
std::string escapeJsonString(const std::string& input) {
    std::string escaped;
    for (char c : input) {
        if (c == '\"') {
            escaped += "\\\""; // 转义双引号
        }
        else {
            escaped += c;
        }
    }
    return escaped;
}

std::string DatabaseManager::RebuildXmlFileByModelName(const std::string& model_name)
{
    // 1. 查询数据库中 Automata 数据
    auto automata_data_opt = QueryAutomataData(model_name);
    if (!automata_data_opt.has_value() || automata_data_opt->empty())
    {
        std::cerr << "No automata data found for model: " << model_name << std::endl;
        return "";
    }

    // 2. 生成 XML 文件路径
    std::string file_name = model_name + "_Autogen.xml";
    std::filesystem::path file_path = std::filesystem::current_path() / "resources\\xmls\\graphAutomata" / file_name;

    // 3. 写入 XML 文件
    std::ofstream ofs(file_path);
    if (!ofs.is_open())
    {
        std::cerr << "Failed to create XML file: " << file_path << std::endl;
        return "";
    }

    ofs << "<model name=\"" << model_name << "\">\n";

    // 遍历查询到的数据并生成 XML 内容
    for (const auto& row : automata_data_opt.value())
    {
        ofs << "  <automata id=\"" << row[0] << "\">\n";
        // 对 JSON 字符串进行转义
        ofs << "    <init init_status='" << escapeJsonString(row[1]) << "'/>\n";
        ofs << "    <transfer func_name=\"" << row[2] << "\"/>\n";
        ofs << "    <input current_input='" << escapeJsonString(row[3]) << "'/>\n";
        ofs << "    <current current_status='" << escapeJsonString(row[4]) << "'/>\n";
        ofs << "    <terminate terminate_status='" << escapeJsonString(row[5]) << "'/>\n";
        ofs << "  </automata>\n";
    }

    ofs << "</model>\n";
    ofs.close();

    // 4. 更新数据库中的文件路径
    if (!UpdateXmlPathInDatabase(model_name, file_path.string()))
    {
        std::cerr << "Failed to update XML file path in database for model: " << model_name << std::endl;
        return "";
    }

    // 5. 返回生成的 XML 文件路径
    return file_path.string();
}


bool DatabaseManager::RegistryModelIndex2DB(const std::string& model_name, const std::string& file_path) {
    try {
        // 确保数据库已连接
        if (!mysql_db.IsConnected()) {
            std::cerr << "Database is not connected!" << std::endl;
            return false;
        }

        // 开始事务
        if (!mysql_db.Execute("START TRANSACTION;")) {
            std::cerr << "Failed to start transaction." << std::endl;
            return false;
        }

        // 插入或更新 ModelBluePrint 表
        std::string insertModelQuery =
            "INSERT INTO ModelBluePrint (name) "
            "VALUES ('" + model_name + "') "
            "ON DUPLICATE KEY UPDATE name = name;";
        if (!mysql_db.Execute(insertModelQuery)) {
            std::cerr << "Failed to insert or update ModelBluePrint." << std::endl;
            mysql_db.Execute("ROLLBACK;"); // 回滚事务
            return false;
        }

        // 获取对应的 model_id
        std::string selectModelIdQuery =
            "SELECT model_id FROM ModelBluePrint WHERE name = '" + model_name + "';";
        auto result = mysql_db.Query(selectModelIdQuery);
        if (!result.has_value() || *result == nullptr) {
            std::cerr << "Failed to retrieve model_id." << std::endl;
            mysql_db.Execute("ROLLBACK;"); // 回滚事务
            return false;
        }

        MYSQL_RES* res = *result;
        MYSQL_ROW row = mysql_fetch_row(res);
        if (!row || !row[0]) {
            std::cerr << "Model ID not found." << std::endl;
            mysql_free_result(res);
            mysql_db.Execute("ROLLBACK;"); // 回滚事务
            return false;
        }
        int model_id = std::stoi(row[0]);
        mysql_free_result(res);

        // 插入或更新 FilePath 表
        std::string insertFilePathQuery =
            "INSERT INTO FilePath (model_id, file_path) "
            "VALUES (" + std::to_string(model_id) + ", '" + file_path + "') "
            "ON DUPLICATE KEY UPDATE file_path = '" + file_path + "', last_updated = CURRENT_TIMESTAMP;";
        if (!mysql_db.Execute(insertFilePathQuery)) {
            std::cerr << "Failed to insert or update FilePath." << std::endl;
            mysql_db.Execute("ROLLBACK;"); // 回滚事务
            return false;
        }

        // 提交事务
        if (!mysql_db.Execute("COMMIT;")) {
            std::cerr << "Failed to commit transaction." << std::endl;
            return false;
        }

        return true; // 操作成功
    }
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return false;
    }
}

bool DatabaseManager::RecordModelPieces2DB(const GraphModel& graph_model) {
    try {
        // 确保数据库已连接
        if (!mysql_db.IsConnected()) {
            std::cerr << "Database is not connected!" << std::endl;
            return false;
        }

        // 开始事务
        if (!mysql_db.Execute("START TRANSACTION;")) {
            std::cerr << "Failed to start transaction." << std::endl;
            return false;
        }

        // 插入或获取 ModelBluePrint 中的 model_id
        std::string insertModelQuery =
            "INSERT INTO ModelBluePrint (name) "
            "VALUES ('" + graph_model.model_name + "') "
            "ON DUPLICATE KEY UPDATE name = name;";
        if (!mysql_db.Execute(insertModelQuery)) {
            std::cerr << "Failed to insert or update ModelBluePrint." << std::endl;
            mysql_db.Execute("ROLLBACK;"); // 回滚事务
            return false;
        }

        // 获取对应的 model_id
        std::string selectModelIdQuery =
            "SELECT model_id FROM ModelBluePrint WHERE name = '" + graph_model.model_name + "';";
        auto result = mysql_db.Query(selectModelIdQuery);
        if (!result.has_value() || *result == nullptr) {
            std::cerr << "Failed to retrieve model_id." << std::endl;
            mysql_db.Execute("ROLLBACK;"); // 回滚事务
            return false;
        }

        MYSQL_RES* res = *result;
        MYSQL_ROW row = mysql_fetch_row(res);
        if (!row || !row[0]) {
            std::cerr << "Model ID not found." << std::endl;
            mysql_free_result(res);
            mysql_db.Execute("ROLLBACK;"); // 回滚事务
            return false;
        }
        int model_id = std::stoi(row[0]);
        mysql_free_result(res);

        // 删除 Automata 表中当前 model_id 的旧数据
        std::string deleteAutomataQuery = "DELETE FROM Automata WHERE model_id = " + std::to_string(model_id) + ";";
        if (!mysql_db.Execute(deleteAutomataQuery)) {
            std::cerr << "Failed to delete old Automata records." << std::endl;
            mysql_db.Execute("ROLLBACK;"); // 回滚事务
            return false;
        }

        // 插入新的 Automata 数据
        for (const auto& automata : graph_model.automatas) {
            // 计算 checksum (假设简单计算，实际实现可以用更复杂的哈希函数)
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
                mysql_db.Execute("ROLLBACK;"); // 回滚事务
                return false;
            }
        }

        // 提交事务
        if (!mysql_db.Execute("COMMIT;")) {
            std::cerr << "Failed to commit transaction." << std::endl;
            return false;
        }

        return true; // 操作成功
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
        // 确保数据库已连接
        if (!mysql_db.IsConnected()) {
            std::cerr << "Database is not connected!" << std::endl;
            return modelNames; // 返回空列表
        }

        // 查询 ModelBluePrint 表中的所有模型名称
        std::string queryStr = "SELECT name FROM ModelBluePrint;";
        auto result = mysql_db.Query(queryStr);

        if (!result.has_value() || *result == nullptr) {
            std::cerr << "Failed to retrieve model names from ModelBluePrint." << std::endl;
            return modelNames; // 返回空列表
        }

        MYSQL_RES* res = *result;
        MYSQL_ROW row;

        // 遍历查询结果，将每个模型名称添加到 QStringList 中
        while ((row = mysql_fetch_row(res)) != nullptr)
            if (row[0])
                modelNames.emplace_back(row[0]);

        // 释放查询结果
        mysql_free_result(res);

    }
    catch (const std::exception& e) {
        std::cerr << "Exception occurred:" << e.what() << std::endl;
    }

    return modelNames;
}
