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
            height: listView.height // 使用 listView.height 而不是 parent.height
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
        listView.model = root.modelList; // 更新 ListView 的 model
        console.log("Model list: ", listView.model.length);
    }

    function cleanModel() {
        root.modelList = [];
        listView.model = []; // 更新 ListView 的 model
        console.log("Model list after cleaning: ", listView.model.length);
    }
}
