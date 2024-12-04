#pragma once
#ifndef XML_DATA_MODEL_H
#define XML_DATA_MODEL_H

#include <list>
#include <string>
#include <QObject>

#include "IndexBarModel.h"
#include "FileManager.h"

/// <summary>
/// Todo: ���Լ̳�ͬһ��ʵ����FillUp��״̬��״̬���ĸ���
/// ����QML�ؼ��Ķ�ȡxml�ļ��Ŀɷ����л��๤��
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
