import QtQuick
import "../Theme.js" as Theme

Item {
    id: root
    property var shape: null
    property bool showGrid: true
    property int shapeVersion: 0
    signal shapeEdited()

    onShapeChanged: {
        dragIndex = -1
        canvas.requestPaint()
    }
    onShapeVersionChanged: canvas.requestPaint()
    onWidthChanged: canvas.requestPaint()
    onHeightChanged: canvas.requestPaint()

    Canvas {
        id: canvas
        anchors.fill: parent
        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            ctx.fillStyle = Theme.bg
            ctx.fillRect(0, 0, width, height)

            var bounds = root.bounds()
            var scale = Math.min((width - 64) / Math.max(1, bounds.maxX - bounds.minX),
                                 (height - 64) / Math.max(1, bounds.maxY - bounds.minY))
            root.scaleValue = scale
            root.minX = bounds.minX
            root.minY = bounds.minY

            if (root.showGrid) {
                var step = 50
                var range = Math.max(bounds.maxX - bounds.minX, bounds.maxY - bounds.minY)
                if (range > 600) step = 100
                if (range > 1600) step = 200
                ctx.strokeStyle = "rgba(255,255,255,0.05)"
                ctx.lineWidth = 1
                for (var gx = Math.floor(bounds.minX / step) * step; gx <= bounds.maxX; gx += step) {
                    var px = root.sx(gx)
                    ctx.beginPath(); ctx.moveTo(px, 0); ctx.lineTo(px, height); ctx.stroke()
                }
                for (var gy = Math.floor(bounds.minY / step) * step; gy <= bounds.maxY; gy += step) {
                    var py = root.sy(gy)
                    ctx.beginPath(); ctx.moveTo(0, py); ctx.lineTo(width, py); ctx.stroke()
                }
            }

            var originX = root.sx(0)
            var originY = root.sy(0)
            ctx.strokeStyle = "rgba(91,140,255,0.35)"
            ctx.beginPath(); ctx.moveTo(originX, 0); ctx.lineTo(originX, height); ctx.stroke()
            ctx.beginPath(); ctx.moveTo(0, originY); ctx.lineTo(width, originY); ctx.stroke()

            if (!root.shape) {
                ctx.fillStyle = Theme.textDim
                ctx.font = "13px " + Theme.fontFamily
                ctx.textAlign = "center"
                ctx.fillText("从上方选择一个模板开始", width / 2, height / 2)
                return
            }

            var color = root.shape.color || "#FF8050"
            var points = root.shape.points || []
            ctx.strokeStyle = color
            ctx.fillStyle = color
            ctx.lineWidth = 2
            ctx.lineJoin = "round"

            if (root.shape.type === "point") {
                if (points.length > 0) {
                    var pp = points[0]
                    var cx = root.sx(pp.x), cy = root.sy(pp.y)
                    ctx.fillRect(cx - 5, cy - 5, 10, 10)
                }
            } else if (root.shape.type === "line" || root.shape.type === "polygon") {
                if (points.length > 1) {
                    ctx.beginPath()
                    ctx.moveTo(root.sx(points[0].x), root.sy(points[0].y))
                    for (var i = 1; i < points.length; ++i)
                        ctx.lineTo(root.sx(points[i].x), root.sy(points[i].y))
                    if (root.shape.type === "polygon" && root.shape.closed)
                        ctx.closePath()
                    ctx.stroke()
                }
                root.drawHandles(ctx, points, color)
            } else if (root.shape.type === "ellipse") {
                if (points.length > 0) {
                    var center = points[0]
                    var rx = Math.max(1, Number(root.shape.rx || 1))
                    var ry = Math.max(1, Number(root.shape.ry || 1))
                    ctx.save()
                    ctx.translate(root.sx(center.x), root.sy(center.y))
                    ctx.scale(rx * root.scaleValue, ry * root.scaleValue)
                    ctx.beginPath()
                    ctx.arc(0, 0, 1, 0, Math.PI * 2)
                    ctx.restore()
                    ctx.stroke()
                    root.drawHandles(ctx, points, color)
                }
            } else if (root.shape.type === "bezier") {
                if (points.length >= 2) {
                    ctx.strokeStyle = "rgba(192,132,252,0.45)"
                    ctx.beginPath()
                    ctx.moveTo(root.sx(points[0].x), root.sy(points[0].y))
                    for (var j = 1; j < points.length; ++j)
                        ctx.lineTo(root.sx(points[j].x), root.sy(points[j].y))
                    ctx.stroke()
                    ctx.strokeStyle = color
                    ctx.beginPath()
                    for (var s = 0; s <= 80; ++s) {
                        var t = s / 80
                        var pt = root.bezierPoint(points, t)
                        if (s === 0) ctx.moveTo(root.sx(pt.x), root.sy(pt.y))
                        else ctx.lineTo(root.sx(pt.x), root.sy(pt.y))
                    }
                    ctx.stroke()
                    root.drawHandles(ctx, points, color)
                }
            } else if (root.shape.type === "cell") {
                var cellSize = Math.min(width, height) / 10
                var offsetX = (width - cellSize * 10) / 2
                var offsetY = (height - cellSize * 10) / 2
                ctx.strokeStyle = "rgba(251,113,133,0.22)"
                for (var g = 0; g <= 10; ++g) {
                    ctx.beginPath(); ctx.moveTo(offsetX + g * cellSize, offsetY); ctx.lineTo(offsetX + g * cellSize, offsetY + 10 * cellSize); ctx.stroke()
                    ctx.beginPath(); ctx.moveTo(offsetX, offsetY + g * cellSize); ctx.lineTo(offsetX + 10 * cellSize, offsetY + g * cellSize); ctx.stroke()
                }
                ctx.fillStyle = "#FB7185"
                var cells = root.shape.cells || []
                for (var c = 0; c < cells.length; ++c) {
                    var cell = cells[c]
                    ctx.fillRect(offsetX + cell.x * cellSize + 1, offsetY + (9 - cell.y) * cellSize + 1, cellSize - 2, cellSize - 2)
                }
            } else {
                ctx.fillStyle = Theme.textMuted
                ctx.font = "13px " + Theme.fontFamily
                ctx.textAlign = "center"
                ctx.fillText("高级模式：请在右侧编辑自动机 JSON", width / 2, height / 2)
            }
        }
    }

    property real scaleValue: 1
    property real minX: 0
    property real minY: 0
    property int dragIndex: -1

    function bounds() {
        var points = root.shape && root.shape.points ? root.shape.points : []
        if (!root.shape || points.length === 0)
            return { minX: 0, minY: 0, maxX: 400, maxY: 400 }
        var minX = 1e9, minY = 1e9, maxX = -1e9, maxY = -1e9
        for (var i = 0; i < points.length; ++i) {
            minX = Math.min(minX, Number(points[i].x))
            minY = Math.min(minY, Number(points[i].y))
            maxX = Math.max(maxX, Number(points[i].x))
            maxY = Math.max(maxY, Number(points[i].y))
        }
        if (root.shape.type === "ellipse") {
            var center = points[0]
            minX = center.x - Number(root.shape.rx || 1)
            maxX = center.x + Number(root.shape.rx || 1)
            minY = center.y - Number(root.shape.ry || 1)
            maxY = center.y + Number(root.shape.ry || 1)
        }
        var padX = Math.max(30, (maxX - minX) * 0.25)
        var padY = Math.max(30, (maxY - minY) * 0.25)
        return { minX: minX - padX, minY: minY - padY, maxX: maxX + padX, maxY: maxY + padY }
    }

    function sx(value) {
        return 32 + (value - root.minX) * root.scaleValue
    }

    function sy(value) {
        return height - 32 - (value - root.minY) * root.scaleValue
    }

    function wx(screenX) {
        return root.minX + (screenX - 32) / Math.max(0.0001, root.scaleValue)
    }

    function wy(screenY) {
        return root.minY + (height - 32 - screenY) / Math.max(0.0001, root.scaleValue)
    }

    function drawHandles(ctx, points, color) {
        for (var i = 0; i < points.length; ++i) {
            var px = root.sx(points[i].x), py = root.sy(points[i].y)
            ctx.beginPath()
            ctx.arc(px, py, 5, 0, Math.PI * 2)
            ctx.fillStyle = Theme.bg
            ctx.fill()
            ctx.strokeStyle = color
            ctx.lineWidth = 2
            ctx.stroke()
        }
    }

    function bezierPoint(points, t) {
        var working = []
        for (var i = 0; i < points.length; ++i)
            working.push({ x: Number(points[i].x), y: Number(points[i].y) })
        while (working.length > 1) {
            for (var j = 0; j < working.length - 1; ++j) {
                working[j] = {
                    x: working[j].x * (1 - t) + working[j + 1].x * t,
                    y: working[j].y * (1 - t) + working[j + 1].y * t
                }
            }
            working.pop()
        }
        return working[0]
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        onPressed: function(mouse) {
            if (!root.shape) return
            if (root.shape.type === "cell") {
                var cellSize = Math.min(width, height) / 10
                var offsetX = (width - cellSize * 10) / 2
                var offsetY = (height - cellSize * 10) / 2
                var cx = Math.floor((mouse.x - offsetX) / cellSize)
                var cy = 9 - Math.floor((mouse.y - offsetY) / cellSize)
                if (cx >= 0 && cx < 10 && cy >= 0 && cy < 10) {
                    var cells = root.shape.cells || []
                    var found = -1
                    for (var i = 0; i < cells.length; ++i)
                        if (cells[i].x === cx && cells[i].y === cy) { found = i; break }
                    if (found >= 0) cells.splice(found, 1)
                    else cells.push({ x: cx, y: cy })
                    root.shapeVersion++
                    root.shapeEdited()
                }
                return
            }
            var points = root.shape.points || []
            var best = -1
            var bestDistance = 16
            for (var j = 0; j < points.length; ++j) {
                var dx = root.sx(points[j].x) - mouse.x
                var dy = root.sy(points[j].y) - mouse.y
                var distance = Math.sqrt(dx * dx + dy * dy)
                if (distance < bestDistance) { best = j; bestDistance = distance }
            }
            if (best >= 0) {
                root.dragIndex = best
            } else if (root.shape.type === "point") {
                root.shape.points[0] = { x: root.wx(mouse.x), y: root.wy(mouse.y) }
                root.shapeVersion++
                root.shapeEdited()
            }
        }
        onPositionChanged: function(mouse) {
            if (pressed && root.dragIndex >= 0) {
                root.shape.points[root.dragIndex] = { x: Math.round(root.wx(mouse.x)), y: Math.round(root.wy(mouse.y)) }
                root.shapeVersion++
                root.shapeEdited()
            }
        }
        onReleased: root.dragIndex = -1
    }
}