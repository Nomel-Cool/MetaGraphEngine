import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import CustomModules 1.0
import "."

Rectangle {
    id: root
    visible: true
    border.width: 1

    signal showSavePage();

    ButtonGroup {
        buttons: toolRow.children
    }

    Rectangle {
        id: toolRowContainer
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: toolRow.height + 10
        border.width: 1

        RowLayout {
            id: rowLayout
            width: parent.width
            spacing: 10

            Row {
                id: toolRow
                spacing: 10
                leftPadding: 10
                topPadding: 10
                anchors.verticalCenter: parent.verticalCenter
                RadioButton {
                    checked: true
                    text: qsTr("点")
                }

                RadioButton {
                    text: qsTr("线段")
                }

                RadioButton {
                    text: qsTr("多边形")
                }

                RadioButton {
                    text: qsTr("椭圆")
                }

                RadioButton {
                    text: qsTr("曲线")
                }

                RadioButton {
                    text: qsTr("自定义")
                    onCheckedChanged: {
                        showSavePage();
                    }
                }           
                Button {
                    id: saveGraph
                    text: qsTr("保存")
                    Layout.alignment: Qt.AlignRight | Qt.AlignHCenter // 使按钮在水平和垂直方向上居中
                    anchors.rightMargin: 10 // 使按钮距离右边界10个单位
                    onClicked: {
                        showSavePage();
                    }
                }
            }
        }
    }

    Canvas {
        id: drawBoard
        anchors.top: toolRowContainer.bottom
        anchors.left: parent.left
    }
}
