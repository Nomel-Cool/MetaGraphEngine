import QtQuick
import "../Theme.js" as Theme

Item {
    id: root
    property var preview: ({})
    property bool showGrid: true
    property real zoom: 1.0
    property real panX: 0
    property real panY: 0
    property int pointCount: 0

    onPreviewChanged: canvas.requestPaint()
    onZoomChanged: canvas.requestPaint()
    onPanXChanged: canvas.requestPaint()
    onPanYChanged: canvas.requestPaint()
    onWidthChanged: canvas.requestPaint()
    onHeightChanged: canvas.requestPaint()

    Canvas {
        id: canvas
        anchors.fill: parent
        renderStrategy: Canvas.Cooperative
        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            ctx.fillStyle = Theme.bg
            ctx.fillRect(0, 0, width, height)

            var layers = root.preview && root.preview.layers ? root.preview.layers : []
            var bounds = root.preview && root.preview.bounds ? root.preview.bounds : { minX: 0, minY: 0, maxX: 0, maxY: 0 }
            var minX = Number(bounds.minX), maxX = Number(bounds.maxX)
            var minY = Number(bounds.minY), maxY = Number(bounds.maxY)
            var rangeX = Math.max(20, maxX - minX)
            var rangeY = Math.max(20, maxY - minY)
            if (maxX - minX < 20)
                minX -= (20 - (maxX - minX)) / 2
            if (maxY - minY < 20)
                minY -= (20 - (maxY - minY)) / 2

            if (root.showGrid) {
                var step = 50
                if (Math.max(rangeX, rangeY) > 600)
                    step = 100
                if (Math.max(rangeX, rangeY) > 1600)
                    step = 200
                ctx.strokeStyle = "rgba(255,255,255,0.045)"
                ctx.lineWidth = 1
                var startX = Math.floor(minX / step) * step
                for (var gx = startX; gx <= maxX + step; gx += step) {
                    var px = root.mapX(gx, minX, rangeX)
                    ctx.beginPath(); ctx.moveTo(px, 0); ctx.lineTo(px, height); ctx.stroke()
                }
                var startY = Math.floor(minY / step) * step
                for (var gy = startY; gy <= maxY + step; gy += step) {
                    var py = root.mapY(gy, minY, rangeY)
                    ctx.beginPath(); ctx.moveTo(0, py); ctx.lineTo(width, py); ctx.stroke()
                }
            }

            var left = root.mapX(0, minX, rangeX)
            var bottom = root.mapY(0, minY, rangeY)
            if (left >= 0 && left <= width) {
                ctx.strokeStyle = "rgba(91,140,255,0.32)"
                ctx.beginPath(); ctx.moveTo(left, 0); ctx.lineTo(left, height); ctx.stroke()
            }
            if (bottom >= 0 && bottom <= height) {
                ctx.strokeStyle = "rgba(91,140,255,0.32)"
                ctx.beginPath(); ctx.moveTo(0, bottom); ctx.lineTo(width, bottom); ctx.stroke()
            }

            var total = 0
            for (var l = 0; l < layers.length; ++l) {
                var layer = layers[l]
                var points = layer.points || []
                for (var p = 0; p < points.length; ++p) {
                    var point = points[p]
                    var x = Number(point[0])
                    var y = Number(point[1])
                    var color = point.length > 2 ? point[2] : layer.color
                    ctx.fillStyle = Theme.rgba(color)
                    var sx = root.mapX(x, minX, rangeX)
                    var sy = root.mapY(y, minY, rangeY)
                    ctx.fillRect(sx - 1, sy - 1, 2.2, 2.2)
                    total++
                }
            }
            root.pointCount = total
        }
    }

    function mapX(value, minValue, rangeValue) {
        var padding = 26
        var usable = Math.max(1, width - padding * 2)
        return padding + ((value - minValue) / Math.max(1, rangeValue)) * usable + root.panX
    }

    function mapY(value, minValue, rangeValue) {
        var padding = 26
        var usable = Math.max(1, height - padding * 2)
        return height - padding - ((value - minValue) / Math.max(1, rangeValue)) * usable + root.panY
    }
}