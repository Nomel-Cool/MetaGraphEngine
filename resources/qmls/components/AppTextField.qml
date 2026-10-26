import QtQuick
import QtQuick.Controls
import "../Theme.js" as Theme

TextField {
    id: root
    property bool invalid: false
    implicitHeight: 38
    leftPadding: 12
    rightPadding: 12
    topPadding: 0
    bottomPadding: 0
    font.family: Theme.fontFamily
    font.pixelSize: 13
    color: Theme.text
    placeholderTextColor: Theme.textDim
    selectByMouse: true
    selectionColor: Theme.accent
    selectedTextColor: "#FFFFFF"

    background: Rectangle {
        radius: Theme.radiusSmall
        color: Theme.bg
        border.width: 1
        border.color: root.invalid ? Theme.danger : (root.activeFocus ? Theme.accent : Theme.border)
        Behavior on border.color { ColorAnimation { duration: 100 } }
    }
}
