import QtQuick 2.15
import QtQuick.Controls 2.15

ApplicationWindow {
    id: root
    width: 400
    height: 300
    minimumWidth: 400
    minimumHeight: 300
    maximumWidth: 400
    maximumHeight: 300
    flags: Qt.FramelessWindowHint
    visible: false
    title: qsTr("自定义图元")

    ListModel {
        id: automataModel
    }

    property var automataChildren: []
    property var currentId: 0
    property string modelName: ""

    signal closeSavePage();
    signal deleverModelData(string modelName, string data);

    ScrollView {
        anchors.fill: parent

        Column {
            width: parent.width
            spacing: 10

            Row {
                spacing: 5
                Label {
                    text: "模型名称"
                    font.bold: true
                }
                TextField {
                    id: modelNameField
                    placeholderText: "Enter Model Name"
                    validator: RegularExpressionValidator {
                        regularExpression: /^[A-Z][A-Za-z_]*$/
                    }
                    text: modelName
                    onTextChanged: modelName = text
                    width: 300
                }
            }

            Repeater {
                id: outerRepeater
                model: automataModel
                Column {
                    id: outerWraper
                    width: parent.width
                    spacing: 5
                    padding: 10
                    property var outerIndex: index
                    Row {
                        spacing: 5
                        Text {
                            text: "ID:"
                            font.bold: true
                        }
                        Label {
                            id: automataID
                            text: model.name
                            width: 200
                        }
                    }

                    Repeater {
                        id: innerRepeater
                        model: automataChildren[outerWraper.outerIndex].singleAutomata === undefined ? [] : automataChildren[outerWraper.outerIndex].singleAutomata
                        Row {
                            spacing: 10
                            Label {
                                text: modelData.type
                                width: 100
                            }
                            TextField {
                                id: textField
                                text: modelData.init_status || modelData.func_name || modelData.current_input || modelData.current_status || modelData.terminate_status
                                onTextChanged: {
                                    if (modelData.init_status !== undefined) {
                                        automataChildren[outerWraper.outerIndex].singleAutomata[index].init_status = text;
                                    }
                                    if (modelData.func_name !== undefined) {
                                        automataChildren[outerWraper.outerIndex].singleAutomata[index].func_name = text;
                                    }
                                    if (modelData.current_input !== undefined) {
                                        automataChildren[outerWraper.outerIndex].singleAutomata[index].current_input = text;
                                    }
                                    if (modelData.current_status !== undefined) {
                                        automataChildren[outerWraper.outerIndex].singleAutomata[index].current_status = text;
                                    }
                                    if (modelData.terminate_status !== undefined) {
                                        automataChildren[outerWraper.outerIndex].singleAutomata[index].terminate_status = text;
                                    }
                                }
                                width: 200
                            }
                        }
                    }
                }
            }

            Button {
                text: qsTr("添加自动机")
                onClicked: addAutomata()
            }

            Button {
                text: qsTr("提交")
                onClicked: {
                    deleverModelData(root.modelName, JSON.stringify(automataChildren));
                    resetCustomAutomata();
                    root.close();
                }
            }
            Button { 
                text: qsTr("取消") 
                onClicked: { 
                    resetCustomAutomata();
                    root.close();
                }
            }
        }
    }

    function resetCustomAutomata() {
        automataModel.clear();
        currentId = 0;
        automataChildren = []; 
        modelName = "";
    }

    function addAutomata() {
        automataChildren.push({
            singleAutomata: [
                { type: "init", init_status: "" },
                { type: "transfer", func_name: "" },
                { type: "input", current_input: "" },
                { type: "current", current_status: "" },
                { type: "terminate", terminate_status: "" }
            ]
        });
        automataModel.append({ name: currentId + 1, childIndex: automataChildren.length - 1, childCount: 5 });
        currentId++;
    }
}
