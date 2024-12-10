import QtQuick 2.15
import QtQuick.Controls 2.15
Rectangle {
    id: root
    visible: true
    
    property var drawnPoints: [] // ���ڱ����ѻ��Ƶĵ�
    property alias engineCore: _drawBoard
    Canvas {
        id: _drawBoard
        anchors.fill: parent

        // ÿ�λ���ʱ����
        onPaint: {
            var ctx = _drawBoard.getContext("2d");
            ctx.clearRect(0, 0, _drawBoard.width, _drawBoard.height); // ��ջ���

            // �������б���ĵ�
            ctx.fillStyle = "black"; // ���û�ͼ��ɫ
            console.log("drawnPoints.length: ", drawnPoints.length);
            for (var i = 0; i < drawnPoints.length; i++) {
                var point = drawnPoints[i];
                //ctx.fillRect(point.x, point.y, point.size, point.size); // ���Ƶ�
                 console.log("Signal From C++" + " " + point.x + " " + point.y);
                ctx.fillRect(point.x, point.y, 5, 5); // ���Ƶ�
            }
        }
    }
    // ���ڴ洢���һ�����Ƶĵ�ı�������ѡ��
    property real lastPixelX: 0
    property real lastPixelY: 0
    property real lastBlockSize: 1
}
