#pragma once
#ifndef XML_DATA_MODEL_H
#define XML_DATA_MODEL_H

#include <list>
#include <string>
#include <QObject>

#include "IndexBarModel.h"
#include "FileManager.h"

/// <summary>
/// Todo: 可以继承同一个实现了FillUp和状态机状态集的父类
/// 用于QML控件的读取xml文件的可反序列化类工厂
/// </summary>
class XmlDataModelFactory : public QObject
{
	Q_OBJECT
public:
	explicit XmlDataModelFactory(QObject* parent = nullptr) : QObject(parent) {}
	Q_INVOKABLE QStringList Request4ModelName();
protected:
	bool FillUp(const std::string& json_string, IndexBarModel& model_index);
private:
	FileManager file_manager;
    enum class State {
        WaitingForModel,
        WaitingForModelName,
        ReadingModelName
    };
    State current_state = State::WaitingForModel;
};

#endif // !XML_DATA_MODEL_H
