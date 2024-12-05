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
        property int currentLayerIndex: 0 // 当前绘制的层索引
        property int currentPointIndex: 0 // 当前绘制的点索引
    }

    Timer {
        id: drawTimer
        interval: drawSpeed
        repeat: true
        running: false
        onTriggered: {
            if (__private.currentLayerIndex < __private.points.length) {
                if (__private.currentPointIndex < __private.points[__private.currentLayerIndex].length) {
                    automataRender.requestPaint();
                    __private.currentPointIndex++;
                } else {
                    __private.currentLayerIndex++;
                    __private.currentPointIndex = 0;
                }
            } else {
                drawTimer.stop();
            }
        }
    }

    onStrPointsChanged: {
        __private.points = JSON.parse(strPoints);
        __private.currentLayerIndex = 0;
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
            
            // 按当前索引逐层逐点绘制
            for (var i = 0; i <= __private.currentLayerIndex; ++i) { // automata layers
                var maxPointIndex = (i < __private.currentLayerIndex) ? __private.points[i].length : __private.currentPointIndex;
                for (var j = 0; j < maxPointIndex; ++j) { // single automata
                    var p = __private.points[i][j]["point"];
                    ctx.fillRect(p[0], p[1], 1, 1);
                }
            }
        }
    }
}
