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
                Layout.alignment: Qt.AlignVCenter
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
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        
        // 用于存储路径点的数组
        property var points: []
        
        onPaint: {
            var ctx = getContext("2d")
            // 清空画布
            ctx.clearRect(0, 0, width, height)
            
            // 设置绘制样式
            ctx.strokeStyle = "#000000"
            ctx.lineWidth = 3
            ctx.lineCap = "round"
            ctx.lineJoin = "round"
            
            // 开始绘制路径
            if (points.length > 0) {
                ctx.beginPath()
                ctx.moveTo(points[0].x, points[0].y)
                
                for (var i = 1; i < points.length; i++) {
                    ctx.lineTo(points[i].x, points[i].y)
                }
                ctx.stroke()
            }
        }
        MouseArea {
            anchors.fill: parent
            onPressed: {
                // 初始化路径并添加第一个点
                drawBoard.points = [{"x": mouseX, "y": mouseY}]
                drawBoard.requestPaint()
            }
            onPositionChanged: {
                if (pressed) {
                    // 添加新的路径点并请求重绘
                    drawBoard.points.push({"x": mouseX, "y": mouseY})
                    drawBoard.requestPaint()
                }
            }
            onReleased: {
                // 释放时可选的清理操作
                drawBoard.points = []
            }
        }
    }
}
