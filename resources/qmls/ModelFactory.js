.pragma library

function clamp(value, low, high) {
    return Math.max(low, Math.min(high, value))
}

function parseJson(text, fallback) {
    try {
        var value = JSON.parse(text)
        return value === undefined ? fallback : value
    } catch (error) {
        return fallback
    }
}

function makeShape(type) {
    var base = {
        type: type,
        name: "",
        color: "#FF8050",
        alpha: 1.0,
        size: 1.0,
        tag: 100,
        z: 0,
        x: 160,
        y: 160,
        points: [],
        rx: 100,
        ry: 60,
        cells: [],
        closed: true,
        rawPayload: ""
    }
    switch (type) {
    case "point":
        base.points = [{ x: 160, y: 160 }]
        break
    case "line":
        base.points = [{ x: 80, y: 80 }, { x: 320, y: 240 }]
        base.closed = false
        break
    case "polygon":
        base.points = [{ x: 120, y: 80 }, { x: 300, y: 120 }, { x: 240, y: 280 }, { x: 100, y: 240 }]
        base.closed = true
        break
    case "ellipse":
        base.points = [{ x: 200, y: 180 }]
        base.rx = 110
        base.ry = 70
        break
    case "bezier":
        base.points = [{ x: 100, y: 280 }, { x: 140, y: 90 }, { x: 300, y: 90 }, { x: 340, y: 280 }]
        break
    case "cell":
        base.cells = [{ x: 4, y: 5 }, { x: 5, y: 5 }, { x: 6, y: 5 }]
        break
    case "advanced":
        base.rawPayload = ""
        break
    }
    return base
}

function colorParts(shape) {
    var rgb = []
    var hex = String(shape.color || "#FF8050").replace("#", "")
    if (hex.length === 3) hex = hex[0] + hex[0] + hex[1] + hex[1] + hex[2] + hex[2]
    if (hex.length !== 6) hex = "FF8050"
    rgb = [parseInt(hex.substring(0, 2), 16) / 255,
           parseInt(hex.substring(2, 4), 16) / 255,
           parseInt(hex.substring(4, 6), 16) / 255]
    for (var i = 0; i < 3; ++i)
        if (isNaN(rgb[i])) rgb[i] = 0.5
    return rgb
}

function pixel(x, y, z, shape, tag) {
    var color = colorParts(shape)
    return {
        x: Number(x),
        y: Number(y),
        z: Number(z || 0),
        r: color[0],
        g: color[1],
        b: color[2],
        a: Number(shape.alpha === undefined ? 1.0 : shape.alpha),
        tag: Number(tag || shape.tag || 100),
        blockSize: Number(shape.size || 1.0)
    }
}

function piece(type, key, value) {
    var item = { type: type }
    item[key] = value
    return item
}

function automata(initValue, funcName, inputValue, currentValue, terminateValue) {
    return {
        singleAutomata: [
            piece("init", "init_status", JSON.stringify(initValue)),
            piece("transfer", "func_name", funcName),
            piece("input", "current_input", JSON.stringify(inputValue)),
            piece("current", "current_status", JSON.stringify(currentValue)),
            piece("terminate", "terminate_status", JSON.stringify(terminateValue))
        ]
    }
}

function buildPayload(shape, name) {
    if (!shape)
        return "[]"
    if (shape.type === "advanced") {
        var raw = String(shape.rawPayload || "").trim()
        return raw.length > 0 ? raw : "[]"
    }
    var tag = Number(shape.tag || 100)
    var points = shape.points || []
    var result = []

    if (shape.type === "point") {
        var p = points.length > 0 ? points[0] : { x: shape.x || 160, y: shape.y || 160 }
        result.push(automata([pixel(p.x, p.y, shape.z, shape, tag)], "JustAPoint", [], [], []))
    } else if (shape.type === "line" || shape.type === "polygon") {
        var count = shape.type === "line" ? 1 : (shape.closed ? points.length : Math.max(0, points.length - 1))
        if (points.length < 2)
            return "[]"
        for (var i = 0; i < count; ++i) {
            var a = points[i % points.length]
            var b = points[(i + 1) % points.length]
            result.push(automata(
                [pixel(a.x, a.y, shape.z, shape, tag + i)],
                "BresenhamLine",
                [{ tagHead: tag + i }],
                [],
                [{ x: Number(b.x), y: Number(b.y), z: Number(shape.z || 0) }]
            ))
        }
    } else if (shape.type === "ellipse") {
        var center = points.length > 0 ? points[0] : { x: 200, y: 180 }
        result.push(automata(
            { x: Number(center.x), y: Number(center.y) },
            "BresenhamEllipse",
            {},
            {},
            { a: Number(shape.rx || 100), b: Number(shape.ry || 60) }
        ))
    } else if (shape.type === "bezier") {
        var controls = []
        for (var j = 0; j < points.length; ++j)
            controls.push({ x: Number(points[j].x), y: Number(points[j].y) })
        result.push(automata(controls, "PartitionBezierCurve", {}, {}, {}))
    } else if (shape.type === "cell") {
        result.push(automata([], "CellAutomataFrame_0", [], [], []))
    }
    return JSON.stringify(result)
}

function blueprintToPayload(automataList) {
    var result = []
    for (var i = 0; i < (automataList || []).length; ++i) {
        var item = automataList[i]
        result.push({
            singleAutomata: [
                { type: "init", init_status: item.init_status || "[]" },
                { type: "transfer", func_name: item.func_name || "" },
                { type: "input", current_input: item.current_input || "[]" },
                { type: "current", current_status: item.current_status || "[]" },
                { type: "terminate", terminate_status: item.terminate_status || "[]" }
            ]
        })
    }
    return JSON.stringify(result)
}

function parseBlueprint(blueprint) {
    if (!blueprint || !blueprint.automata || blueprint.automata.length === 0)
        return null
    var list = blueprint.automata
    var funcs = []
    for (var i = 0; i < list.length; ++i)
        funcs.push(list[i].func_name || "")
    var first = list[0]
    var init = parseJson(first.init_status || "[]", [])
    var terminate = parseJson(first.terminate_status || "[]", [])
    var shape = null

    if (funcs[0] === "JustAPoint") {
        shape = makeShape("point")
        var point = Array.isArray(init) && init.length > 0 ? init[0] : {}
        shape.points = [{ x: Number(point.x || 0), y: Number(point.y || 0) }]
        shape.z = Number(point.z || 0)
        shape.color = rgbHex(point.r, point.g, point.b)
        shape.alpha = point.a === undefined ? 1.0 : Number(point.a)
        shape.size = Number(point.blockSize || 1.0)
        shape.tag = Number(point.tag || 100)
    } else if (funcs[0] === "BresenhamLine") {
        var isPolygon = funcs.length > 1
        shape = makeShape(isPolygon ? "polygon" : "line")
        shape.points = []
        for (var j = 0; j < list.length; ++j) {
            var starts = parseJson(list[j].init_status || "[]", [])
            var ends = parseJson(list[j].terminate_status || "[]", [])
            var start = Array.isArray(starts) && starts.length > 0 ? starts[0] : {}
            if (j === 0) {
                shape.points.push({ x: Number(start.x || 0), y: Number(start.y || 0) })
                shape.color = rgbHex(start.r, start.g, start.b)
                shape.alpha = start.a === undefined ? 1.0 : Number(start.a)
                shape.size = Number(start.blockSize || 1.0)
                shape.tag = Number(start.tag || 100)
            }
            var end = Array.isArray(ends) && ends.length > 0 ? ends[0] : {}
            shape.points.push({ x: Number(end.x || 0), y: Number(end.y || 0) })
        }
        shape.closed = isPolygon
    } else if (funcs[0] === "BresenhamEllipse") {
        shape = makeShape("ellipse")
        var center = Array.isArray(init) ? (init[0] || {}) : (init || {})
        var radii = Array.isArray(terminate) ? (terminate[0] || {}) : (terminate || {})
        shape.points = [{ x: Number(center.x || 0), y: Number(center.y || 0) }]
        shape.rx = Number(radii.a || 100)
        shape.ry = Number(radii.b || 60)
    } else if (funcs[0] === "PartitionBezierCurve") {
        shape = makeShape("bezier")
        shape.points = []
        for (var k = 0; k < (Array.isArray(init) ? init.length : 0); ++k)
            shape.points.push({ x: Number(init[k].x || 0), y: Number(init[k].y || 0) })
    } else if (funcs[0] === "CellAutomataFrame_0") {
        shape = makeShape("cell")
        shape.points = []
    } else {
        shape = makeShape("advanced")
        shape.rawPayload = blueprintToPayload(list)
    }

    if (!shape)
        return null
    shape.name = blueprint.name || ""
    if (shape.type === "advanced")
        shape.rawPayload = blueprintToPayload(list)
    return shape
}

function rgbHex(r, g, b) {
    function part(value) {
        var n = Math.max(0, Math.min(255, Math.round(Number(value) * 255)))
        var s = n.toString(16).toUpperCase()
        return s.length === 1 ? "0" + s : s
    }
    return "#" + part(r === undefined ? 1.0 : r) + part(g === undefined ? 0.5 : g) + part(b === undefined ? 0.31 : b)
}

function kindOfShape(shape) {
    return shape ? (shape.type === "polygon" ? "line" : shape.type) : "unknown"
}

function isDynamicShape(shape) {
    if (!shape) return false
    return shape.type === "point" || shape.type === "line" || shape.type === "polygon" || shape.type === "cell"
}

function isStaticShape(shape) {
    if (!shape) return false
    return shape.type !== "cell" && shape.type !== "advanced"
}

function describeShape(shape) {
    if (!shape) return ""
    if (shape.type === "point") return "点 (" + shape.points[0].x + ", " + shape.points[0].y + ")"
    if (shape.type === "line") return "线段 · " + shape.points.length + " 个端点"
    if (shape.type === "polygon") return "多边形 · " + shape.points.length + " 个顶点"
    if (shape.type === "ellipse") return "椭圆 · 半径 " + shape.rx + " × " + shape.ry
    if (shape.type === "bezier") return "贝塞尔 · " + shape.points.length + " 个控制点"
    if (shape.type === "cell") return "10 × 10 元胞自动机"
    return "高级自动机配置"
}
