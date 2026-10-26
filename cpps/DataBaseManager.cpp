#include "DataBaseManager.h"

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <set>

namespace {

std::string EnvValue(const char* name, const char* fallback)
{
    const char* value = std::getenv(name);
    return (value && *value) ? std::string(value) : std::string(fallback);
}

std::string EscapeJsonAttribute(const std::string& value)
{
    std::string escaped;
    escaped.reserve(value.size() * 2);
    for (char c : value)
    {
        switch (c)
        {
        case '"': escaped += "\\\""; break;
        case '\\': escaped += "\\\\"; break;
        case '&': escaped += "&amp;"; break;
        case '<': escaped += "&lt;"; break;
        case '>': escaped += "&gt;"; break;
        case '\'': escaped += "&apos;"; break;
        default: escaped += c; break;
        }
    }
    return escaped;
}
std::string EscapeXmlText(const std::string& value)
{
    std::string escaped;
    escaped.reserve(value.size());
    for (char c : value)
    {
        switch (c)
        {
        case '&': escaped += "&amp;"; break;
        case '<': escaped += "&lt;"; break;
        case '>': escaped += "&gt;"; break;
        case '\'': escaped += "&apos;"; break;
        default: escaped += c; break;
        }
    }
    return escaped;
}

std::string EscapeXmlDoubleQuoted(const std::string& value)
{
    std::string escaped;
    escaped.reserve(value.size());
    for (char c : value)
    {
        switch (c)
        {
        case '&': escaped += "&amp;"; break;
        case '<': escaped += "&lt;"; break;
        case '>': escaped += "&gt;"; break;
        case '\'': escaped += "&apos;"; break;
        case '"': escaped += "&quot;"; break;
        default: escaped += c; break;
        }
    }
    return escaped;
}

} // namespace

DatabaseManager::DatabaseManager()
{
    if (mysql_db.IsConnected())
        std::cout << "The database has connected." << std::endl;
    else
    {
        const std::string host = EnvValue("QGK_DB_HOST", "localhost");
        const std::string user = EnvValue("QGK_DB_USER", "root");
        const std::string password = EnvValue("QGK_DB_PASSWORD", "123456");
        const std::string database = EnvValue("QGK_DB_NAME", "meta_graph_model");
        mysql_db.Init(host, user, password, database);
    }
}

std::string DatabaseManager::EscapeSql(const std::string& value)
{
    MYSQL* conn = mysql_db.GetConnection();
    if (!conn)
        return value;
    std::vector<char> buffer(2 * value.size() + 1);
    mysql_real_escape_string(conn, buffer.data(), value.c_str(), value.size());
    return std::string(buffer.data());
}

bool DatabaseManager::IsConnected()
{
    return mysql_db.IsConnected();
}

std::string DatabaseManager::QueryFilePathByModelName(const std::string& model_name)
{
    if (!mysql_db.IsConnected())
    {
        std::cerr << "Database is not connected!" << std::endl;
        return "";
    }

    const std::string query = "SELECT f.file_path FROM FilePath f JOIN ModelBluePrint m ON f.model_id = m.model_id WHERE m.name = '" + EscapeSql(model_name) + "'";
    std::optional<MYSQL_RES*> result_opt = mysql_db.Query(query);
    if (!result_opt.has_value())
    {
        std::cerr << "Query failed or no result: " << query << std::endl;
        return "";
    }

    MYSQL_RES* result = result_opt.value();
    MYSQL_ROW row;
    std::string file_path;
    if ((row = mysql_fetch_row(result)))
    {
        if (row[0])
            file_path = row[0];
    }
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

    const std::string query =
        "SELECT a.automata_id, a.init_status, a.transfer_func, a.current_input, a.current_status, a.terminate_status "
        "FROM Automata AS a JOIN ModelBluePrint AS m ON a.model_id = m.model_id "
        "WHERE m.name = '" + EscapeSql(model_name) + "' ORDER BY a.automata_id";

    auto result_opt = mysql_db.Query(query);
    if (!result_opt.has_value())
    {
        std::cerr << "Failed to query automata data for model: " << model_name << std::endl;
        return std::nullopt;
    }

    MYSQL_RES* result = result_opt.value();
    MYSQL_ROW row;
    std::vector<std::vector<std::string>> rows;
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
    if (!mysql_db.IsConnected())
    {
        std::cerr << "Database is not connected!" << std::endl;
        return false;
    }

    const std::string update_query =
        "UPDATE FilePath SET file_path = '" + EscapeSql(xml_path) + "' "
        "WHERE model_id = (SELECT model_id FROM ModelBluePrint WHERE name = '" + EscapeSql(model_name) + "')";
    if (!mysql_db.Execute(update_query))
    {
        std::cerr << "Failed to update file path for model: " << model_name << std::endl;
        return false;
    }
    return true;
}

std::string DatabaseManager::RebuildXmlFileByModelName(const std::string& model_name)
{
    auto automata_data_opt = QueryAutomataData(model_name);
    if (!automata_data_opt.has_value() || automata_data_opt->empty())
    {
        std::cerr << "No automata data found for model: " << model_name << std::endl;
        return "";
    }

    try
    {
        std::filesystem::path model_root = std::filesystem::current_path() / "resources" / "xmls" / "graphAutomata";
        std::filesystem::create_directories(model_root);
        std::filesystem::path file_path = model_root / (model_name + "_Autogen.xml");

        std::ofstream ofs(file_path);
        if (!ofs.is_open())
        {
            std::cerr << "Failed to create XML file: " << file_path << std::endl;
            return "";
        }

        ofs << "<model name=\"" << EscapeXmlDoubleQuoted(model_name) << "\">\n";
        for (const auto& row : automata_data_opt.value())
        {
            if (row.size() < 6)
                continue;
            ofs << "  <automata id=\"" << EscapeXmlDoubleQuoted(row[0]) << "\">\n";
            ofs << "    <init init_status='" << EscapeJsonAttribute(row[1]) << "'/>\n";
            ofs << "    <transfer func_name=\"" << EscapeXmlDoubleQuoted(row[2]) << "\"/>\n";
            ofs << "    <input current_input='" << EscapeJsonAttribute(row[3]) << "'/>\n";
            ofs << "    <current current_status='" << EscapeJsonAttribute(row[4]) << "'/>\n";
            ofs << "    <terminate terminate_status='" << EscapeJsonAttribute(row[5]) << "'/>\n";
            ofs << "  </automata>\n";
        }
        ofs << "</model>\n";
        ofs.close();

        if (!UpdateXmlPathInDatabase(model_name, file_path.string()))
        {
            std::cerr << "Failed to update XML file path in database for model: " << model_name << std::endl;
            return "";
        }
        return file_path.string();
    }
    catch (const std::exception& error)
    {
        std::cerr << "Failed to rebuild XML file: " << error.what() << std::endl;
        return "";
    }
}

bool DatabaseManager::RegistryModelIndex2DB(const std::string& model_name, const std::string& file_path)
{
    try
    {
        if (!mysql_db.IsConnected())
        {
            std::cerr << "Database is not connected!" << std::endl;
            return false;
        }
        if (!mysql_db.Execute("START TRANSACTION;"))
            return false;

        const std::string insertModelQuery =
            "INSERT INTO ModelBluePrint (name) VALUES ('" + EscapeSql(model_name) + "') "
            "ON DUPLICATE KEY UPDATE name = name;";
        if (!mysql_db.Execute(insertModelQuery))
        {
            mysql_db.Execute("ROLLBACK;");
            return false;
        }

        const std::string selectModelIdQuery = "SELECT model_id FROM ModelBluePrint WHERE name = '" + EscapeSql(model_name) + "';";
        auto result = mysql_db.Query(selectModelIdQuery);
        if (!result.has_value() || *result == nullptr)
        {
            mysql_db.Execute("ROLLBACK;");
            return false;
        }

        MYSQL_RES* res = *result;
        MYSQL_ROW row = mysql_fetch_row(res);
        if (!row || !row[0])
        {
            mysql_free_result(res);
            mysql_db.Execute("ROLLBACK;");
            return false;
        }
        const int model_id = std::stoi(row[0]);
        mysql_free_result(res);

        const std::string clearFilePathQuery =
            "DELETE FROM FilePath WHERE model_id = " + std::to_string(model_id) + ";";
        if (!mysql_db.Execute(clearFilePathQuery))
        {
            mysql_db.Execute("ROLLBACK;");
            return false;
        }
        const std::string insertFilePathQuery =
            "INSERT INTO FilePath (model_id, file_path) VALUES (" + std::to_string(model_id) + ", '" + EscapeSql(file_path) + "') "
            "ON DUPLICATE KEY UPDATE file_path = '" + EscapeSql(file_path) + "', last_updated = CURRENT_TIMESTAMP;";
        if (!mysql_db.Execute(insertFilePathQuery))
        {
            mysql_db.Execute("ROLLBACK;");
            return false;
        }

        if (!mysql_db.Execute("COMMIT;"))
            return false;
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return false;
    }
}

bool DatabaseManager::RecordModelPieces2DB(const GraphModel& graph_model)
{
    try
    {
        if (!mysql_db.IsConnected())
        {
            std::cerr << "Database is not connected!" << std::endl;
            return false;
        }
        if (!mysql_db.Execute("START TRANSACTION;"))
            return false;

        const std::string insertModelQuery =
            "INSERT INTO ModelBluePrint (name) VALUES ('" + EscapeSql(graph_model.model_name) + "') "
            "ON DUPLICATE KEY UPDATE name = name;";
        if (!mysql_db.Execute(insertModelQuery))
        {
            mysql_db.Execute("ROLLBACK;");
            return false;
        }

        const std::string selectModelIdQuery = "SELECT model_id FROM ModelBluePrint WHERE name = '" + EscapeSql(graph_model.model_name) + "';";
        auto result = mysql_db.Query(selectModelIdQuery);
        if (!result.has_value() || *result == nullptr)
        {
            mysql_db.Execute("ROLLBACK;");
            return false;
        }
        MYSQL_RES* res = *result;
        MYSQL_ROW row = mysql_fetch_row(res);
        if (!row || !row[0])
        {
            mysql_free_result(res);
            mysql_db.Execute("ROLLBACK;");
            return false;
        }
        const int model_id = std::stoi(row[0]);
        mysql_free_result(res);

        const std::string deleteAutomataQuery = "DELETE FROM Automata WHERE model_id = " + std::to_string(model_id) + ";";
        if (!mysql_db.Execute(deleteAutomataQuery))
        {
            mysql_db.Execute("ROLLBACK;");
            return false;
        }

        for (const auto& automata : graph_model.automatas)
        {
            const std::string checksum = std::to_string(std::hash<std::string>{}(automata.init_status + automata.func_name +
                automata.current_input + automata.current_status + automata.terminate_status));
            const std::string insertAutomataQuery =
                "INSERT INTO Automata (model_id, init_status, transfer_func, current_input, current_status, terminate_status, checksum) VALUES (" +
                std::to_string(model_id) + ", '" + EscapeSql(automata.init_status) + "', '" + EscapeSql(automata.func_name) + "', '" +
                EscapeSql(automata.current_input) + "', '" + EscapeSql(automata.current_status) + "', '" +
                EscapeSql(automata.terminate_status) + "', '" + EscapeSql(checksum) + "');";
            if (!mysql_db.Execute(insertAutomataQuery))
            {
                mysql_db.Execute("ROLLBACK;");
                return false;
            }
        }

        if (!mysql_db.Execute("COMMIT;"))
            return false;
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return false;
    }
}

bool DatabaseManager::DeleteModel(const std::string& model_name)
{
    if (!mysql_db.IsConnected())
    {
        std::cerr << "Database is not connected!" << std::endl;
        return false;
    }
    const std::string name = EscapeSql(model_name);
    try
    {
        if (!mysql_db.Execute("START TRANSACTION;"))
            return false;
        const std::string deleteAutomata =
            "DELETE a FROM Automata a JOIN ModelBluePrint m ON a.model_id = m.model_id WHERE m.name = '" + name + "';";
        const std::string deleteFilePath =
            "DELETE f FROM FilePath f JOIN ModelBluePrint m ON f.model_id = m.model_id WHERE m.name = '" + name + "';";
        const std::string deleteModel =
            "DELETE FROM ModelBluePrint WHERE name = '" + name + "';";
        if (!mysql_db.Execute(deleteAutomata) || !mysql_db.Execute(deleteFilePath) || !mysql_db.Execute(deleteModel))
        {
            mysql_db.Execute("ROLLBACK;");
            return false;
        }
        return mysql_db.Execute("COMMIT;");
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return false;
    }
}

std::vector<std::string> DatabaseManager::GetModelNameList()
{
    std::vector<std::string> modelNames;
    if (!mysql_db.IsConnected())
        return modelNames;
    try
    {
        auto result = mysql_db.Query("SELECT name FROM ModelBluePrint ORDER BY name;");
        if (!result.has_value() || *result == nullptr)
            return modelNames;
        MYSQL_RES* res = *result;
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res)) != nullptr)
            if (row[0])
                modelNames.emplace_back(row[0]);
        mysql_free_result(res);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
    }
    return modelNames;
}

std::vector<std::pair<std::string, std::vector<std::string>>> DatabaseManager::GetModelCatalog()
{
    std::vector<std::pair<std::string, std::vector<std::string>>> catalog;
    if (!mysql_db.IsConnected())
        return catalog;
    try
    {
        auto result = mysql_db.Query(
            "SELECT m.name, a.transfer_func FROM ModelBluePrint m "
            "LEFT JOIN Automata a ON a.model_id = m.model_id ORDER BY m.name, a.automata_id;");
        if (!result.has_value() || *result == nullptr)
            return catalog;
        MYSQL_RES* res = *result;
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res)) != nullptr)
        {
            if (!row[0])
                continue;
            const std::string name = row[0];
            if (catalog.empty() || catalog.back().first != name)
                catalog.push_back({ name, {} });
            if (row[1] && *row[1])
                catalog.back().second.emplace_back(row[1]);
        }
        mysql_free_result(res);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
    }
    return catalog;
}