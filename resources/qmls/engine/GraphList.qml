import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: root
    width: 200
    height: 200
    border.width: 1

    property var modelList: []

    ListView {
        id: listView
        anchors.fill: parent
        model: root.modelList
        orientation: Qt.Horizontal
        clip: true
        delegate: Rectangle {
            width: 100
            height: listView.height // ʹ�� listView.height ������ parent.height
            anchors.verticalCenter: parent.verticalCenter
            border.width: 1
            Text {
                text: modelData
                clip: true
                anchors.fill: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    function addModel(modelName) {
        root.modelList.push(modelName);
        listView.model = root.modelList; // ���� ListView �� model
        console.log("Model list: ", listView.model.length);
    }

    function cleanModel() {
        root.modelList = [];
        listView.model = []; // ���� ListView �� model
        console.log("Model list after cleaning: ", listView.model.length);
    }
}
