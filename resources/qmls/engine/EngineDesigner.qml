import QtQuick 2.15
import QtQuick.Controls 2.15
Rectangle {
    id: root
    visible: true
    
    property var drawnPoints: [] // 用于保存已绘制的点
    property alias engineCore: _drawBoard
    Canvas {
        id: _drawBoard
        anchors.fill: parent

        // 每次绘制时调用
        onPaint: {
            var ctx = _drawBoard.getContext("2d");
            ctx.clearRect(0, 0, _drawBoard.width, _drawBoard.height); // 清空画布

            // 绘制所有保存的点
            ctx.fillStyle = "black"; // 设置绘图颜色
            console.log("drawnPoints.length: ", drawnPoints.length);
            for (var i = 0; i < drawnPoints.length; i++) {
                var point = drawnPoints[i];
                //ctx.fillRect(point.x, point.y, point.size, point.size); // 绘制点
                 console.log("Signal From C++" + " " + point.x + " " + point.y);
                ctx.fillRect(point.x, point.y, 5, 5); // 绘制点
            }
        }
    }
    // 用于存储最后一个绘制的点的变量（可选）
    property real lastPixelX: 0
    property real lastPixelY: 0
    property real lastBlockSize: 1
}
