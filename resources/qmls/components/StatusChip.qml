import QtQuick
import QtQuick.Controls
import "../Theme.js" as Theme

Rectangle {
    id: root
    property bool ok: true
    property string text: ok ? "存储已连接" : "存储未连接"
    implicitWidth: row.implicitWidth + 20
    implicitHeight: 26
    radius: 13
    color: ok ? Theme.successSoft : Theme.dangerSoft
    border.width: 1
    border.color: ok ? "#1D5C46" : "#5A2A32"

    Row {
        id: row
        anchors.centerIn: parent
        spacing: 6
        Rectangle {
            width: 7
            height: 7
            radius: 4
            color: root.ok ? Theme.success : Theme.danger
            anchors.verticalCenter: parent.verticalCenter
        }
        Text {
            text: root.text
            color: root.ok ? "#9EF0CB" : "#FFB4B4"
            font.family: Theme.fontFamily
            font.pixelSize: 11
        }
    }
}
