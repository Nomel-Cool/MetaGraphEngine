import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15

Rectangle {
    id: previewBoard
    border.width: 1

    property var strPoints
    property alias currentModelName: modelName.text
    property int drawSpeed: 50 // 绘制速度，单位：毫秒

    QtObject {
        id: __private
        property var points: []
        property int currentPointIndex: 0 // 当前绘制的点索引
    }

    Timer {
        id: drawTimer
        interval: drawSpeed
        repeat: true
        running: false
        onTriggered: {
            if (__private.currentPointIndex < __private.points.length) {
                automataRender.requestPaint();
                __private.currentPointIndex++;
            } else {
                drawTimer.stop();
            }
        }
    }

    onStrPointsChanged: {
        __private.points = JSON.parse(strPoints);
        __private.currentPointIndex = 0;
        drawTimer.start(); // 开始绘制动画
    }

    Label {
        id: modelName
        background: Rectangle {
            anchors.fill: parent
            border.width: 1
        }
        anchors.top: parent.top
        anchors.left: parent.left
        font.pixelSize: 15
        font.italic: true
        width: parent.width
        height: font.pixelSize + 4
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        text: ""
    }

    Canvas {
        id: automataRender
        anchors.top: modelName.bottom
        anchors.left: parent.left
        width: parent.width
        height: parent.height - modelName.height
        onPaint: {
            var ctx = automataRender.getContext("2d")
            ctx.clearRect(0, 0, width, height)
            ctx.fillStyle = "black"
            for (var i = 0; i < __private.currentPointIndex; i++) {
                var p = __private.points[i]["point"]
                ctx.fillRect(p[0], p[1], 1, 1)
            }
        }
    }
}
