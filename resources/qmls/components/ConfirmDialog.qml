import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../Theme.js" as Theme

Dialog {
    id: root
    property string titleText: "确认操作"
    property string message: ""
    property string detail: ""
    property string confirmText: "确认"
    signal confirmed()

    modal: true
    anchors.centerIn: parent
    width: 440
    padding: 22
    closePolicy: Popup.CloseOnEscape
    background: Rectangle {
        color: Theme.surface
        radius: Theme.radiusLarge
        border.width: 1
        border.color: Theme.borderStrong
    }

    contentItem: ColumnLayout {
        spacing: 12
        Text {
            text: root.titleText
            color: Theme.text
            font.family: Theme.fontFamily
            font.pixelSize: 17
            font.weight: Font.DemiBold
        }
        Text {
            Layout.fillWidth: true
            text: root.message
            color: Theme.textMuted
            font.family: Theme.fontFamily
            font.pixelSize: 13
            wrapMode: Text.WordWrap
            lineHeight: 1.45
        }
        Text {
            Layout.fillWidth: true
            visible: root.detail.length > 0
            text: root.detail
            color: Theme.textDim
            font.family: Theme.monoFamily
            font.pixelSize: 11
            wrapMode: Text.WordWrap
        }
    }

    footer: RowLayout {
        spacing: 8
        Item { Layout.fillWidth: true }
        AppButton {
            text: "取消"
            variant: "secondary"
            onClicked: root.reject()
        }
        AppButton {
            text: root.confirmText
            variant: "danger"
            onClicked: {
                root.accept()
                root.confirmed()
            }
        }
    }
}
