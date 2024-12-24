import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: root
    visible: true
    width: 800
    height: 600

    // 属性定义
    property var drawnPoints: [] // 用于保存已绘制的点，初始为空
    property alias engineCore: _drawBoard
    property int currentFrame: 0 // 当前绘制的帧索引
    property int frameInterval: 8 // 每帧时间间隔（毫秒）
    property bool clearCanvas: false // 是否每帧清空画布

    // 画布定义
    Canvas {
        id: _drawBoard
        anchors.fill: parent

        // 每次绘制时调用
        onPaint: {
            var ctx = _drawBoard.getContext("2d");

            // 清空画布（如果需要清空）
            if (clearCanvas) {
                ctx.clearRect(0, 0, _drawBoard.width, _drawBoard.height);
            }

            // 确保当前帧存在
            if (currentFrame < drawnPoints.length) {
                var frame = drawnPoints[currentFrame];
                console.log("Drawing frame:", currentFrame, "Points in frame:", frame.length);

                // 绘制当前帧的所有点
                for (var i = 0; i < frame.length; ++i) {
                    var point = frame[i];
                    //ctx.fillRect(point.x, point.y, point.blockSize, point.blockSize);
                    ctx.fillStyle = Qt.rgba(point.r, point.g, point.b, point.a); // 设置绘图样式
                    ctx.fillRect(point.x, point.y, 5, 5);
                }
            }
        }
    }

    // 定时器控制绘制节奏
    Timer {
        id: drawTimer
        interval: frameInterval
        running: true
        repeat: true

        onTriggered: {
            // 如果还有帧未绘制，触发绘制
            if (currentFrame < drawnPoints.length) {
                _drawBoard.requestPaint();
                currentFrame++;
            } else {
                // 停止计时器，绘制完成
                console.log("All frames drawn.");
                drawTimer.stop();
            }
        }
    }

    // 按钮控制
    Column {
        spacing: 10
        anchors {
            right: parent.right
            top: parent.top
            margins: 10
        }

        Button {
            text: "Start Drawing"
            onClicked: {
                currentFrame = 0; // 重置帧计数器
                drawTimer.start(); // 开始绘制
            }
        }

        Button {
            text: "Clear Canvas"
            onClicked: {
                clearCanvas = true; // 启用画布清空
                _drawBoard.requestPaint(); // 手动触发清空
                currentFrame = 0; // 重置帧
                drawnPoints = []; // 清空点数据
                console.log("Canvas cleared.");
            }
        }

        Button {
            text: "Toggle Clear Mode"
            onClicked: {
                clearCanvas = !clearCanvas;
                console.log("Clear canvas mode:", clearCanvas);
            }
        }
    }
}
