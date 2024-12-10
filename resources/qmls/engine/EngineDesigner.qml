import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import CustomModules 1.0
import "."

Rectangle {
    id: root
    visible: true
    border.width: 1
    Canvas {
        id: drawBoard
        anchors.fill: parent
        Text {
            anchors.fill: parent
            text: "Engine Desinger"
            verticalAlignment: Text.AlignHCenter
            horizontalAlignment: Text.AlignVCenter
        }
    }
}