import QtQuick
import QtQuick.Controls
import "../Theme.js" as Theme

Rectangle {
    id: root
    property string message: ""
    property string level: "info"
    property bool shown: false
    width: Math.min(parent ? parent.width - 80 : 520, Math.max(280, label.implicitWidth + 48))
    height: 42
    radius: Theme.radiusSmall
    color: level === "error" ? Theme.dangerSoft : (level === "success" ? Theme.successSoft : Theme.surfaceAlt)
    border.width: 1
    border.color: level === "error" ? Theme.danger : (level === "success" ? Theme.success : Theme.borderStrong)
    opacity: shown ? 1 : 0
    visible: opacity > 0
    y: shown ? 24 : 8
    z: 1000

    Behavior on opacity { NumberAnimation { duration: 160 } }
    Behavior on y { NumberAnimation { duration: 160; easing.type: Easing.OutCubic } }

    Text {
        id: label
        anchors.centerIn: parent
        width: parent.width - 32
        text: root.message
        color: Theme.text
        font.family: Theme.fontFamily
        font.pixelSize: 13
        horizontalAlignment: Text.AlignHCenter
        elide: Text.ElideRight
    }

    Timer {
        id: hideTimer
        interval: 3200
        onTriggered: root.shown = false
    }

    function show(text, type) {
        message = text
        level = type === undefined ? "info" : type
        shown = true
        hideTimer.restart()
    }
}
