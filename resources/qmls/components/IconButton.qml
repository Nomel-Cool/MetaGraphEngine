import QtQuick
import QtQuick.Controls
import "../Theme.js" as Theme

Button {
    id: root
    property string tooltip: ""
    implicitWidth: 30
    implicitHeight: 30
    padding: 0
    hoverEnabled: true

    background: Rectangle {
        radius: 6
        color: root.hovered ? Theme.surfaceHover : "transparent"
        border.width: root.hovered ? 1 : 0
        border.color: Theme.border
    }

    contentItem: Text {
        text: root.text
        color: root.enabled ? Theme.textMuted : Theme.textDim
        font.family: Theme.fontFamily
        font.pixelSize: 15
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    ToolTip.visible: hovered && tooltip.length > 0
    ToolTip.text: tooltip
}
