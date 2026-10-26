import QtQuick
import QtQuick.Controls
import "../Theme.js" as Theme

Button {
    id: root
    property string variant: "primary"
    property string tooltip: ""
    property bool compact: false

    implicitHeight: compact ? 32 : 38
    implicitWidth: Math.max(compact ? 96 : 110, text.length * 16 + 32)
    padding: 0
    leftPadding: compact ? 10 : 16
    rightPadding: compact ? 10 : 16
    hoverEnabled: true
    font.family: Theme.fontFamily
    font.pixelSize: compact ? 12 : 13
    font.weight: Font.Medium

    palette.buttonText: {
        if (!root.enabled) return Theme.textDim
        if (root.variant === "primary" || root.variant === "danger") return "#FFFFFF"
        if (root.variant === "ghost") return Theme.textMuted
        return Theme.text
    }

    background: Rectangle {
        radius: Theme.radiusSmall
        color: {
            if (!root.enabled) return Theme.surfaceAlt
            if (root.variant === "primary") return root.hovered ? Theme.accentHover : Theme.accent
            if (root.variant === "danger") return root.hovered ? "#FF8C8C" : Theme.danger
            if (root.variant === "ghost") return root.hovered ? Theme.surfaceHover : "transparent"
            return root.hovered ? Theme.surfaceHover : Theme.surfaceAlt
        }
        border.width: root.variant === "primary" || root.variant === "danger" ? 0 : 1
        border.color: root.variant === "ghost" ? "transparent" : Theme.border
        Behavior on color { ColorAnimation { duration: 120 } }
    }

    ToolTip.visible: hovered && tooltip.length > 0
    ToolTip.text: tooltip
    ToolTip.delay: 500
}