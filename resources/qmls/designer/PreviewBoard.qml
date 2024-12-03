import QtQuick 2.15
import QtQuick.Window 2.15

Rectangle {
    id: previewBoard
    border.width: 1

    property var strPoints

    QtObject {
        id: __private
        property var points: []
    }

    onStrPointsChanged: { 
        __private.points = JSON.parse(strPoints);
        automataRender.requestPaint(); // 请求重新绘制Canvas
    }

    Canvas {
        id: automataRender
        anchors.fill: parent
        onPaint: {
            var ctx = automataRender.getContext("2d")
            ctx.clearRect(0, 0, width, height)
            ctx.fillStyle = "black"

            for (var i = 0; i < __private.points.length; i++) {
                var p = __private.points[i].point
                ctx.fillRect(p[0], p[1], 1, 1)
            }
        }
    }
}
