#pragma once
#ifndef META_GRAPH_AUTOMATA_H
#define META_GRAPH_AUTOMATA_H

#include <string>
#include <vector>


/// <summary>
/// ����GraphModel������࣬���ڱ�ʾһ���Զ���
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
/// ����ʾ����Զ�������ϣ��Լ�����FileManager��XML�����л�����
/// </summary>
class GraphModel {
public:
    std::string model_name;
    std::vector<SingleAutomata> automatas;
};

#endif // !META_GRAPH_AUTOMATA_H
