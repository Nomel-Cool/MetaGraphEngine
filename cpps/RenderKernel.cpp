#define NOMINMAX
#include "RenderKernel.h"
#include <GraphGenerateAlgorithmLib.h>

#include <QRegularExpression>
#include <QStringList>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <functional>
#include <cstdlib>
#include <filesystem>
#include <map>
#include <set>
#include <sstream>
#include <vector>

namespace {

constexpr const char* kModelRoot = "resources/xmls/graphAutomata";

std::string JsonError(const QString& message)
{
    nlohmann::json error = nlohmann::json::object();
    error["ok"] = false;
    error["error"] = message.toStdString();
    return error.dump();
}

bool IsJsonText(const std::string& text)
{
    if (text.empty())
        return false;
    auto parsed = nlohmann::json::parse(text, nullptr, false);
    return !parsed.is_discarded();
}

void ExtractColor(const std::string& status_text, nlohmann::json& color)
{
    color = nlohmann::json::array({ 1.0, 0.5, 0.31, 1.0 });
    auto parsed = nlohmann::json::parse(status_text, nullptr, false);
    if (parsed.is_discarded())
        return;
    nlohmann::json first;
    if (parsed.is_array() && !parsed.empty())
        first = parsed[0];
    else if (parsed.is_object())
        first = parsed;
    else
        return;
    if (!first.is_object())
        return;
    const char* keys[] = { "r", "g", "b", "a" };
    double values[4] = { 1.0, 0.5, 0.31, 1.0 };
    for (int i = 0; i < 4; ++i)
    {
        if (first.contains(keys[i]) && first[keys[i]].is_number())
            values[i] = first[keys[i]].get<double>();
    }
    color = nlohmann::json::array({ values[0], values[1], values[2], values[3] });
}

std::string XmlAttribute(const std::string& value, bool single_quoted)
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
        case '"': escaped += single_quoted ? "\"" : "&quot;"; break;
        default: escaped += c; break;
        }
    }
    return escaped;
}

} // namespace

GraphFactory::GraphFactory(QObject* parent)
    : QObject(parent),
      redis_client(EnvString("QGK_REDIS_HOST", "127.0.0.1"), EnvInt("QGK_REDIS_PORT", 6379))
{
    std::unique_ptr<shabby::IXMLDocumentFactory> tinyxml_factory = std::make_unique<TinyXMLDocumentFactory>();
    sp_file_manager = std::make_shared<FileManager>(std::move(tinyxml_factory));
    sp_stream_to_modeldata = std::make_shared<XMLStreamConvertToModelDataStrategy>(sp_file_manager);
    sp_modeldata_to_stream = std::make_shared<ModelDataConvertToXMLStreamStrategy>();

    std::unique_ptr<RenderAlgorithmFactory> algo_factory = std::make_unique<RenderAlgorithmFactory>();
    algo_factory->RegisterAlgorithm("BresenhamLine", std::make_unique<BresenhamLineRenderer>(sp_file_manager));
    algo_factory->RegisterAlgorithm("JustAPoint", std::make_unique<JustAPointRenderer>(sp_file_manager));
    algo_factory->RegisterAlgorithm("BresenhamEllipse", std::make_unique<BresenhamEllipseRenderer>(sp_file_manager));
    algo_factory->RegisterAlgorithm("PartitionBezierCurve", std::make_unique<PartitionBezierCurveRenderer>(sp_file_manager));
    algo_factory->RegisterCoAlgorithm("CoBresenhamLine", std::make_unique<BresenhamLineRenderer>(sp_file_manager));
    algo_factory->RegisterCoAlgorithm("CoJustAPoint", std::make_unique<JustAPointRenderer>(sp_file_manager));
    algo_factory->RegisterCoAlgorithm("CoCellAutomataFrame_0", std::make_unique<CellAutomataFrame_0>(sp_file_manager));
    sp_render_cu = std::make_shared<RenderCU>(sp_file_manager, std::move(algo_factory));
}

std::string GraphFactory::EnvString(const char* name, const char* fallback)
{
    const char* value = std::getenv(name);
    return (value && *value) ? std::string(value) : std::string(fallback);
}

int GraphFactory::EnvInt(const char* name, int fallback)
{
    const char* value = std::getenv(name);
    if (!value || !*value)
        return fallback;
    try
    {
        return std::stoi(value);
    }
    catch (...)
    {
        return fallback;
    }
}

namespace {

bool IsUsableModel(const GraphModel& model)
{
    if (model.automatas.empty())
        return false;
    for (const SingleAutomata& automata : model.automatas)
    {
        if (automata.func_name.empty())
            return false;
        if (!IsJsonText(automata.init_status) || !IsJsonText(automata.current_input) ||
            !IsJsonText(automata.current_status) || !IsJsonText(automata.terminate_status))
            return false;
    }
    return true;
}

} // namespace
bool GraphFactory::LoadGraphModel(const QString& model_name, GraphModel& graph_model, QString& error)
{
    auto try_load = [this](const std::string& path, GraphModel& target) -> bool
    {
        if (path.empty())
            return false;
        GraphModel loaded;
        sp_stream_to_modeldata->Reset();
        auto bound_func = std::bind(&IStreamConvertToModelDataStrategy::FillUp,
            sp_stream_to_modeldata, std::placeholders::_1, std::placeholders::_2);
        if (!sp_file_manager->TransXml2Class<GraphModel>(path, loaded, bound_func))
            return false;
        if (loaded.automatas.empty())
            return false;
        target = std::move(loaded);
        return true;
    };

    const std::string name = model_name.toStdString();
    std::string path = database_manager.QueryFilePathByModelName(name);
    if (try_load(path, graph_model) && IsUsableModel(graph_model))
        return true;

    std::string rebuilt = database_manager.RebuildXmlFileByModelName(name);
    if (!rebuilt.empty() && try_load(rebuilt, graph_model) && IsUsableModel(graph_model))
        return true;

    error = QStringLiteral("Unable to load model '%1'. Check MySQL, the XML file and Redis.").arg(model_name);
    return false;
}

int GraphFactory::KindIndex(const std::string& func_name)
{
    if (func_name == "JustAPoint") return 1;
    if (func_name == "BresenhamLine") return 2;
    if (func_name == "BresenhamEllipse") return 3;
    if (func_name == "PartitionBezierCurve") return 4;
    if (func_name == "CellAutomataFrame_0") return 5;
    return 0;
}

QString GraphFactory::KindName(int kind)
{
    switch (kind)
    {
    case 1: return QStringLiteral("point");
    case 2: return QStringLiteral("line");
    case 3: return QStringLiteral("ellipse");
    case 4: return QStringLiteral("bezier");
    case 5: return QStringLiteral("cell");
    default: return QStringLiteral("unknown");
    }
}

bool GraphFactory::IsStaticAlgorithm(const std::string& func_name)
{
    return func_name == "JustAPoint" || func_name == "BresenhamLine" ||
           func_name == "BresenhamEllipse" || func_name == "PartitionBezierCurve";
}

bool GraphFactory::IsDynamicAlgorithm(const std::string& func_name)
{
    return func_name == "JustAPoint" || func_name == "BresenhamLine" ||
           func_name == "CellAutomataFrame_0";
}

QString GraphFactory::RenderModelLayers(const GraphModel& graph_model)
{
    nlohmann::json layers = nlohmann::json::array();
    for (const SingleAutomata& automata : graph_model.automatas)
    {
        const std::string rendered = sp_render_cu->Render(automata.func_name, automata);
        if (rendered.empty())
            continue;
        auto parsed = nlohmann::json::parse(rendered, nullptr, false);
        if (parsed.is_discarded())
            continue;
        if (parsed.is_object())
        {
            nlohmann::json single = nlohmann::json::array();
            single.push_back(parsed);
            layers.push_back(single);
        }
        else if (parsed.is_array() && !parsed.empty())
        {
            layers.push_back(parsed);
        }
    }
    return QString::fromStdString(layers.dump());
}

QString GraphFactory::BuildPreview(const GraphModel& graph_model)
{
    nlohmann::json result = nlohmann::json::object();
    result["ok"] = true;
    result["name"] = graph_model.model_name;
    nlohmann::json layers = nlohmann::json::array();
    nlohmann::json kinds = nlohmann::json::array();
    std::set<std::string> kind_set;
    double min_x = 0.0, min_y = 0.0, max_x = 0.0, max_y = 0.0;
    bool has_bounds = false;
    std::size_t point_count = 0;

    for (const SingleAutomata& automata : graph_model.automatas)
    {
        const int kind = KindIndex(automata.func_name);
        const QString kind_name = KindName(kind);
        if (kind_set.insert(kind_name.toStdString()).second)
            kinds.push_back(kind_name.toStdString());

        const std::string rendered = sp_render_cu->Render(automata.func_name, automata);
        if (rendered.empty())
            continue;
        auto parsed = nlohmann::json::parse(rendered, nullptr, false);
        if (parsed.is_discarded() || !parsed.is_array())
            continue;

        nlohmann::json color;
        ExtractColor(automata.init_status, color);
        nlohmann::json points = nlohmann::json::array();

        for (const auto& item : parsed)
        {
            if (!item.is_object())
                continue;
            double x = 0.0;
            double y = 0.0;
            bool valid = false;
            if (item.contains("point") && item["point"].is_array() && item["point"].size() >= 2 &&
                item["point"][0].is_number() && item["point"][1].is_number())
            {
                x = item["point"][0].get<double>();
                y = item["point"][1].get<double>();
                valid = true;
            }
            else if (item.contains("x") && item.contains("y") &&
                     item["x"].is_number() && item["y"].is_number())
            {
                x = item["x"].get<double>();
                y = item["y"].get<double>();
                valid = true;
            }
            if (!valid || !std::isfinite(x) || !std::isfinite(y))
                continue;

            nlohmann::json point = nlohmann::json::array({ x, y });
            nlohmann::json point_color = color;
            if (item.contains("r") && item.contains("g") && item.contains("b"))
            {
                point_color = nlohmann::json::array({
                    item.value("r", 1.0), item.value("g", 0.5), item.value("b", 0.31), item.value("a", 1.0) });
            }
            point.push_back(point_color);
            points.push_back(point);

            if (!has_bounds)
            {
                min_x = max_x = x;
                min_y = max_y = y;
                has_bounds = true;
            }
            else
            {
                min_x = std::min(min_x, x);
                max_x = std::max(max_x, x);
                min_y = std::min(min_y, y);
                max_y = std::max(max_y, y);
            }
            ++point_count;
        }

        if (points.empty())
            continue;
        nlohmann::json layer = nlohmann::json::object();
        layer["func"] = automata.func_name;
        layer["kind"] = kind_name.toStdString();
        layer["color"] = color;
        layer["points"] = points;
        layers.push_back(layer);
    }

    if (kinds.empty())
        kinds.push_back("unknown");
    result["kinds"] = kinds;
    result["layers"] = layers;
    result["pointCount"] = point_count;
    result["bounds"] = { { "minX", min_x }, { "minY", min_y }, { "maxX", max_x }, { "maxY", max_y } };
    result["runtimeOnly"] = (point_count == 0);
    return QString::fromStdString(result.dump());
}

QString GraphFactory::Request4Model(const QString& model_name)
{
    std::unique_lock<std::mutex> lock(storage_mutex);
    GraphModel graph_model;
    QString error;
    if (!LoadGraphModel(model_name, graph_model, error))
    {
        lock.unlock();
        emit operationFailed(error);
        return QStringLiteral("[]");
    }
    const QString rendered = RenderModelLayers(graph_model);
    redis_client.Set(model_name.toStdString(), rendered.toStdString());
    return rendered;
}

QString GraphFactory::Request4ModelPreview(const QString& model_name)
{
    std::unique_lock<std::mutex> lock(storage_mutex);
    GraphModel graph_model;
    QString error;
    if (!LoadGraphModel(model_name, graph_model, error))
        return QString::fromStdString(JsonError(error));
    return BuildPreview(graph_model);
}

QString GraphFactory::Request4ModelBlueprint(const QString& model_name)
{
    std::unique_lock<std::mutex> lock(storage_mutex);
    GraphModel graph_model;
    QString error;
    if (!LoadGraphModel(model_name, graph_model, error))
        return QString::fromStdString(JsonError(error));

    nlohmann::json result = nlohmann::json::object();
    result["ok"] = true;
    result["name"] = graph_model.model_name;
    nlohmann::json automatas = nlohmann::json::array();
    for (const SingleAutomata& automata : graph_model.automatas)
    {
        nlohmann::json item = nlohmann::json::object();
        item["id"] = automata.id;
        item["kind"] = KindName(KindIndex(automata.func_name)).toStdString();
        item["init_status"] = automata.init_status;
        item["func_name"] = automata.func_name;
        item["current_input"] = automata.current_input;
        item["current_status"] = automata.current_status;
        item["terminate_status"] = automata.terminate_status;
        automatas.push_back(item);
    }
    result["automata"] = automatas;
    return QString::fromStdString(result.dump());
}

QString GraphFactory::Request4ModelCatalog()
{
    std::unique_lock<std::mutex> lock(storage_mutex);
    nlohmann::json result = nlohmann::json::object();
    result["ok"] = true;
    result["database"] = database_manager.IsConnected();
    nlohmann::json models = nlohmann::json::array();

    const auto rows = database_manager.GetModelCatalog();
    std::vector<std::string> order;
    std::map<std::string, std::set<std::string>> functions;
    for (const auto& row : rows)
    {
        if (functions.find(row.first) == functions.end())
            order.push_back(row.first);
        for (const auto& func : row.second)
            functions[row.first].insert(func);
    }
    for (const auto& name : order)
    {
        nlohmann::json model = nlohmann::json::object();
        model["name"] = name;
        nlohmann::json kinds = nlohmann::json::array();
        std::set<std::string> kind_set;
        bool is_static = false;
        bool is_dynamic = false;
        for (const auto& func : functions[name])
        {
            const QString kind = KindName(KindIndex(func));
            if (kind_set.insert(kind.toStdString()).second)
                kinds.push_back(kind.toStdString());
            is_static = is_static || IsStaticAlgorithm(func);
            is_dynamic = is_dynamic || IsDynamicAlgorithm(func);
        }
        if (kinds.empty())
            kinds.push_back("unknown");
        model["kinds"] = kinds;
        model["static"] = is_static;
        model["dynamic"] = is_dynamic;
        models.push_back(model);
    }
    result["models"] = models;
    return QString::fromStdString(result.dump());
}

QString GraphFactory::GetBackendStatus()
{
    std::unique_lock<std::mutex> lock(storage_mutex);
    nlohmann::json result = nlohmann::json::object();
    result["ok"] = true;
    result["database"] = database_manager.IsConnected();
    try
    {
        result["modelRoot"] = (std::filesystem::current_path() / kModelRoot).string();
    }
    catch (...)
    {
        result["modelRoot"] = kModelRoot;
    }
    return QString::fromStdString(result.dump());
}

QStringList GraphFactory::Request4ModelNameList()
{
    std::unique_lock<std::mutex> lock(storage_mutex);
    QStringList names;
    try
    {
        for (const auto& name : database_manager.GetModelNameList())
            names.emplaceBack(QString::fromStdString(name));
    }
    catch (const std::exception& error)
    {
        lock.unlock();
        emit operationFailed(QString::fromUtf8(error.what()));
    }
    return names;
}

bool GraphFactory::Registry4Model(const QString& model_name, const QString& str_model_data)
{
    std::unique_lock<std::mutex> lock(storage_mutex);
    const QString name = model_name.trimmed();
    const QRegularExpression pattern(QStringLiteral("^[A-Z][A-Za-z0-9_]*$"));
    if (!pattern.match(name).hasMatch())
    {
        lock.unlock();
        emit operationFailed(QStringLiteral("Model name must start with an uppercase letter and contain only letters, digits or underscore."));
        return false;
    }

    auto raw = nlohmann::json::parse(str_model_data.toStdString(), nullptr, false);
    if (raw.is_discarded() || !raw.is_array() || raw.empty())
    {
        lock.unlock();
        emit operationFailed(QStringLiteral("Model data is not a non-empty JSON array."));
        return false;
    }

    GraphModel graph_model;
    graph_model.model_name = name.toStdString();
    const std::set<std::string> allowed = {
        "JustAPoint", "BresenhamLine", "BresenhamEllipse",
        "PartitionBezierCurve", "CellAutomataFrame_0" };

    for (std::size_t i = 0; i < raw.size(); ++i)
    {
        if (!raw[i].is_object() || !raw[i].contains("singleAutomata") || !raw[i]["singleAutomata"].is_array())
        {
            lock.unlock();
            emit operationFailed(QStringLiteral("Each model item must contain a singleAutomata array."));
            return false;
        }
        SingleAutomata automata;
        automata.id = std::to_string(i + 1);
        automata.init_status = "[]";
        automata.current_input = "[]";
        automata.current_status = "[]";
        automata.terminate_status = "[]";
        for (const auto& piece : raw[i]["singleAutomata"])
        {
            if (!piece.is_object() || !piece.contains("type"))
                continue;
            const std::string type = piece.value("type", "");
            if (type == "init") automata.init_status = piece.value("init_status", "[]");
            else if (type == "transfer") automata.func_name = piece.value("func_name", "");
            else if (type == "input") automata.current_input = piece.value("current_input", "[]");
            else if (type == "current") automata.current_status = piece.value("current_status", "[]");
            else if (type == "terminate") automata.terminate_status = piece.value("terminate_status", "[]");
        }
        if (allowed.find(automata.func_name) == allowed.end())
        {
            lock.unlock();
            emit operationFailed(QStringLiteral("Unknown transfer function: %1").arg(QString::fromStdString(automata.func_name)));
            return false;
        }
        if (!IsJsonText(automata.init_status) || !IsJsonText(automata.current_input) ||
            !IsJsonText(automata.current_status) || !IsJsonText(automata.terminate_status))
        {
            lock.unlock();
            emit operationFailed(QStringLiteral("Automata fields must contain valid JSON text."));
            return false;
        }
        graph_model.automatas.push_back(std::move(automata));
    }

    if (graph_model.automatas.empty())
    {
        lock.unlock();
        emit operationFailed(QStringLiteral("The model has no automata."));
        return false;
    }

    try
    {
        std::filesystem::path model_root = std::filesystem::current_path() / kModelRoot;
        std::filesystem::create_directories(model_root);
        const std::string path = (model_root / (name.toStdString() + ".xml")).string();
        auto bound_func = [this](std::shared_ptr<shabby::IXMLDocument> doc, GraphModel& model) {
            return sp_modeldata_to_stream->Depart(doc, model);
        };
        if (!sp_file_manager->TransClass2Xml<GraphModel>(graph_model, path, bound_func))
        {
            lock.unlock();
            emit operationFailed(QStringLiteral("Failed to write the model XML file."));
            return false;
        }
        if (!database_manager.RegistryModelIndex2DB(name.toStdString(), path) ||
            !database_manager.RecordModelPieces2DB(graph_model))
        {
            lock.unlock();
            emit operationFailed(QStringLiteral("Failed to write the model into MySQL."));
            return false;
        }
        redis_client.Del(name.toStdString());
    }
    catch (const std::exception& error)
    {
        lock.unlock();
        emit operationFailed(QString::fromUtf8(error.what()));
        return false;
    }

    lock.unlock();

    emit update4NameList();
    emit operationSucceeded(QStringLiteral("Model '%1' saved.").arg(name));
    return true;
}

bool GraphFactory::DeleteModel(const QString& model_name)
{
    std::unique_lock<std::mutex> lock(storage_mutex);
    const std::string name = model_name.toStdString();
    std::string path;
    try
    {
        path = database_manager.QueryFilePathByModelName(name);
    }
    catch (...)
    {
    }

    if (!database_manager.DeleteModel(name))
    {
        lock.unlock();
        emit operationFailed(QStringLiteral("Failed to delete model '%1' from MySQL.").arg(model_name));
        return false;
    }

    try
    {
        if (!path.empty())
        {
            std::filesystem::path file(path);
            std::string lower = file.lexically_normal().string();
            std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            if (lower.find("graphautomata") != std::string::npos && std::filesystem::exists(file))
                std::filesystem::remove(file);
        }
    }
    catch (...)
    {
    }

    redis_client.Del(name);
    lock.unlock();
    emit update4NameList();
    emit operationSucceeded(QStringLiteral("Model '%1' deleted.").arg(model_name));
    return true;
}

ModelGenerator<SingleAutomata> GraphFactory::OfferDynamicModel(const QString& model_name)
{
    GraphModel graph_model;
    QString error;
    {
        std::unique_lock<std::mutex> lock(storage_mutex);
        if (!LoadGraphModel(model_name, graph_model, error))
        {
            lock.unlock();
            emit operationFailed(error);
            co_return;
        }
    }

    for (SingleAutomata& automata : graph_model.automatas)
    {
        if (!IsDynamicAlgorithm(automata.func_name))
            continue;
        auto generator = sp_render_cu->CoRender("Co" + automata.func_name, automata);
        do
        {
            co_yield generator.GetValue();
        } while (generator.Resume());
    }
    co_return;
}