.pragma library

var bg = "#0B0E14"
var surface = "#121722"
var surfaceAlt = "#182030"
var surfaceHover = "#1E283A"
var border = "#263247"
var borderStrong = "#34445E"
var text = "#E8EDF7"
var textMuted = "#8A96AC"
var textDim = "#5F6B80"
var accent = "#5B8CFF"
var accentHover = "#7AA2FF"
var accentSoft = "#1B2A4A"
var success = "#34D399"
var successSoft = "#123326"
var warning = "#FBBF24"
var warningSoft = "#3A2E10"
var danger = "#F87171"
var dangerSoft = "#3B1D24"

var fontFamily = "Microsoft YaHei UI"
var monoFamily = "Cascadia Mono"
var radius = 10
var radiusSmall = 8
var radiusLarge = 16

function rgba(colorArray, fallbackAlpha) {
    if (!colorArray || colorArray.length < 3)
        return "#5B8CFF"
    var a = (colorArray.length > 3 && colorArray[3] !== undefined) ? colorArray[3] : (fallbackAlpha === undefined ? 1.0 : fallbackAlpha)
    return "rgba(" + Math.round(colorArray[0] * 255) + "," + Math.round(colorArray[1] * 255) + "," + Math.round(colorArray[2] * 255) + "," + a + ")"
}

function hexToRgb(hex) {
    var value = String(hex || "").replace("#", "")
    if (value.length === 3) {
        value = value[0] + value[0] + value[1] + value[1] + value[2] + value[2]
    }
    if (value.length !== 6)
        return [1.0, 0.5, 0.31]
    var r = parseInt(value.substring(0, 2), 16) / 255
    var g = parseInt(value.substring(2, 4), 16) / 255
    var b = parseInt(value.substring(4, 6), 16) / 255
    if (isNaN(r) || isNaN(g) || isNaN(b))
        return [1.0, 0.5, 0.31]
    return [r, g, b]
}

function rgbToHex(r, g, b) {
    function part(v) {
        var n = Math.max(0, Math.min(255, Math.round(v * 255)))
        var s = n.toString(16).toUpperCase()
        return s.length === 1 ? "0" + s : s
    }
    return "#" + part(r) + part(g) + part(b)
}

function kindColor(kind) {
    switch (kind) {
    case "point": return "#5B8CFF"
    case "line": return "#34D399"
    case "ellipse": return "#FBBF24"
    case "bezier": return "#C084FC"
    case "cell": return "#FB7185"
    default: return "#8A96AC"
    }
}

function kindLabel(kind) {
    switch (kind) {
    case "point": return "点"
    case "line": return "线 / 多边形"
    case "ellipse": return "圆 / 椭圆"
    case "bezier": return "贝塞尔曲线"
    case "cell": return "元胞自动机"
    default: return "自定义"
    }
}
