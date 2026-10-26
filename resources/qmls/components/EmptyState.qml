import QtQuick
import QtQuick.Controls
import "../Theme.js" as Theme

Column {
    id: root
    property string title: "这里还没有内容"
    property string description: ""
    property string actionText: ""
    signal action()

    spacing: 10
    anchors.centerIn: parent
    width: Math.min(360, parent ? parent.width - 48 : 320)

    Text {
        text: root.title
        color: Theme.text
        font.family: Theme.fontFamily
        font.pixelSize: 15
        font.weight: Font.Medium
        horizontalAlignment: Text.AlignHCenter
        width: parent.width
    }
    Text {
        text: root.description
        visible: text.length > 0
        color: Theme.textMuted
        font.family: Theme.fontFamily
        font.pixelSize: 12
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter
        width: parent.width
        lineHeight: 1.4
    }
    AppButton {
        anchors.horizontalCenter: parent.horizontalCenter
        text: root.actionText
        visible: root.actionText.length > 0
        variant: "secondary"
        onClicked: root.action()
    }
}
