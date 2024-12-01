#pragma once
#ifndef META_GRAPH_AUTOMATA_H
#define META_GRAPH_AUTOMATA_H

#include <string>
#include <vector>


/// <summary>
/// 它是GraphModel的组合类，用于表示一个自动机
/// </summary>
class SingleAutomata {
public:
    std::string id;
    std::string init_status;
    std::string func_name;
    std::string current_input;
    std::string current_status;
    std::string terminate_status;
};

/// <summary>
/// 它表示多个自动机的组合，以及依赖FileManager的XML反序列化功能
/// </summary>
class GraphModel {
public:
    std::string model_name;
    std::vector<SingleAutomata> automatas;
};

#endif // !META_GRAPH_AUTOMATA_H
