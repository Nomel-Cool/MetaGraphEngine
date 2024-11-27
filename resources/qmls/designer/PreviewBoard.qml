import QtQuick 2.15
import QtQuick.Window 2.15

Rectangle {
    id: previewBoard
    border.width: 1

    property var points: []
    property var x0
    property var y0
    property var x1
    property var y1

    Component.onCompleted: { 
        points = bresenhamLine(x0, y0, x1, y1); 
        automataRender.requestPaint(); // 请求重新绘制Canvas
    }

    Canvas {
        id: automataRender
        anchors.fill: parent
    
        onPaint: {
            var ctx = automataRender.getContext("2d")
            ctx.clearRect(0, 0, width, height)
            ctx.fillStyle = "black"
            
            for (var i = 0; i < points.length; i++) {
                var p = points[i].point
                ctx.fillRect(p[0], p[1], 1, 1)
            }
        }
    }
    function bresenhamLine(x0, y0, x1, y1) { 
        var points = [];
        var dx = Math.abs(x1 - x0);
        var dy = Math.abs(y1 - y0);
        var sx = (x0 < x1) ? 1 : -1;
        var sy = (y0 < y1) ? 1 : -1;
        var err = dx - dy; 
        while(true) { 
            points.push({point: [x0, y0]}); // 将点添加到数组中 
            if ((x0 === x1) && (y0 === y1)) 
                break; 
            var e2 = 2 * err; 
            if (e2 > -dy) { 
                err -= dy;
                x0 += sx; 
            } 
            if (e2 < dx) { 
                err += dx; 
                y0 += sy;
            } 
        }
        return points;
    }
}
