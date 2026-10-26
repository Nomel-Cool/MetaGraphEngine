import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Window
import "Theme.js" as Theme

ApplicationWindow {
    id: root
    visible: true
    width: 640
    height: 480
    title: "Graph Engine Kit · 启动检查"
    color: Theme.bg
    Material.theme: Material.Dark
    Material.primary: Theme.accent
    Material.accent: Theme.accent
    Material.background: Theme.bg
    Material.foreground: Theme.text

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 32
        spacing: 16

        RowLayout {
            spacing: 12
            Rectangle {
                width: 40; height: 40; radius: 10
                color: Theme.accent
                Text { anchors.centerIn: parent; text: "G"; color: "white"; font.family: Theme.fontFamily; font.pixelSize: 20; font.weight: Font.Bold }
            }
            ColumnLayout {
                spacing: 2
                Text { text: "启动检查未通过"; color: Theme.text; font.family: Theme.fontFamily; font.pixelSize: 20; font.weight: Font.DemiBold }
                Text { text: "Graph Engine Kit 需要先启动 Redis 与 MySQL"; color: Theme.textMuted; font.family: Theme.fontFamily; font.pixelSize: 12 }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: Theme.radius
            color: Theme.surface
            border.width: 1
            border.color: Theme.border
            ScrollView {
                anchors.fill: parent
                anchors.margins: 16
                TextArea {
                    text: startupErrorText
                    readOnly: true
                    wrapMode: TextArea.WrapAnywhere
                    color: Theme.text
                    font.family: Theme.monoFamily
                    font.pixelSize: 12
                    background: null
                    selectByMouse: true
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            Text {
                Layout.fillWidth: true
                text: "修复服务后重新启动程序。日志：程序目录/logs/app.log"
                color: Theme.textDim
                font.family: Theme.fontFamily
                font.pixelSize: 10
                wrapMode: Text.WordWrap
            }
            Button {
                text: "退出"
                onClicked: Qt.quit()
            }
        }
    }
}