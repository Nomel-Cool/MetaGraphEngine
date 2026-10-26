import QtQuick
import "../Theme.js" as Theme

Canvas {
    id: root
    property string kind: "unknown"
    property color tint: Theme.kindColor(kind)
    implicitWidth: 28
    implicitHeight: 28
    onKindChanged: requestPaint()
    onTintChanged: requestPaint()
    onPaint: {
        var ctx = getContext("2d")
        ctx.clearRect(0, 0, width, height)
        ctx.strokeStyle = tint
        ctx.fillStyle = tint
        ctx.lineWidth = 2
        var cx = width / 2, cy = height / 2
        if (kind === "point") {
            ctx.beginPath()
            ctx.arc(cx, cy, 4.5, 0, Math.PI * 2)
            ctx.fill()
        } else if (kind === "line") {
            ctx.beginPath()
            ctx.moveTo(5, height - 6)
            ctx.lineTo(width - 5, 6)
            ctx.stroke()
        } else if (kind === "ellipse") {
            ctx.beginPath()
            ctx.ellipse(cx - 10, cy - 7, 10, 7, 0, 0, Math.PI * 2)
            ctx.stroke()
        } else if (kind === "bezier") {
            ctx.beginPath()
            ctx.moveTo(4, height - 6)
            ctx.bezierCurveTo(width * 0.3, 2, width * 0.7, 2, width - 4, height - 6)
            ctx.stroke()
        } else if (kind === "cell") {
            for (var i = 0; i < 3; ++i)
                for (var j = 0; j < 3; ++j)
                    if ((i + j) % 2 === 0)
                        ctx.fillRect(7 + i * 5, 7 + j * 5, 4, 4)
        } else {
            ctx.beginPath()
            ctx.arc(cx, cy, 6, 0, Math.PI * 2)
            ctx.stroke()
        }
    }
}
