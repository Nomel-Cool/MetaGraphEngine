import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: root
    visible: true
    width: 800
    height: 600

    // ���Զ���
    property var drawnPoints: [] // ���ڱ����ѻ��Ƶĵ㣬��ʼΪ��
    property alias engineCore: _drawBoard
    property int currentFrame: 0 // ��ǰ���Ƶ�֡����
    property int frameInterval: 33 // ÿ֡ʱ���������룩
    property bool clearCanvas: false // �Ƿ�ÿ֡��ջ���

    // ��������
    Canvas {
        id: _drawBoard
        anchors.fill: parent

        // ÿ�λ���ʱ����
        onPaint: {
            var ctx = _drawBoard.getContext("2d");

            // ��ջ����������Ҫ��գ�
            if (clearCanvas) {
                ctx.clearRect(0, 0, _drawBoard.width, _drawBoard.height);
            }

            // ȷ����ǰ֡����
            if (currentFrame < drawnPoints.length) {
                var frame = drawnPoints[currentFrame];
                console.log("Drawing frame:", currentFrame, "Points in frame:", frame.length);

                // ���û�ͼ��ʽ
                ctx.fillStyle = "black";

                // ���Ƶ�ǰ֡�����е�
                for (var i = 0; i < frame.length; ++i) {
                    var point = frame[i];
                    //ctx.fillRect(point.x, point.y, point.blockSize, point.blockSize);
                    ctx.fillRect(point.x, point.y, 5, 5);
                }
            }
        }
    }

    // ��ʱ�����ƻ��ƽ���
    Timer {
        id: drawTimer
        interval: frameInterval
        running: true
        repeat: true

        onTriggered: {
            // �������֡δ���ƣ���������
            if (currentFrame < drawnPoints.length) {
                _drawBoard.requestPaint();
                currentFrame++;
            } else {
                // ֹͣ��ʱ�����������
                console.log("All frames drawn.");
                drawTimer.stop();
            }
        }
    }

    // ��ť����
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
                currentFrame = 0; // ����֡������
                drawTimer.start(); // ��ʼ����
            }
        }

        Button {
            text: "Clear Canvas"
            onClicked: {
                clearCanvas = true; // ���û������
                _drawBoard.requestPaint(); // �ֶ��������
                currentFrame = 0; // ����֡
                drawnPoints = []; // ��յ�����
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
