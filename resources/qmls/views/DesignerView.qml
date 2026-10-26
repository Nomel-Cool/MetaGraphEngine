import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../Theme.js" as Theme
import "../ModelFactory.js" as ModelFactory
import "../components"
import "../editor"

Item {
    id: root
    property var app: null
    property var designerModel: null
    property var shape: ModelFactory.makeShape("point")
    property string editingName: ""
    property bool advancedMode: false
    property string advancedPayload: ""
    property int shapeVersion: 0
    property string statusText: ""
    property string nameError: ""
    property var templates: [
        { type: "point", title: "点", desc: "单像素" },
        { type: "line", title: "线段", desc: "Bresenham" },
        { type: "polygon", title: "多边形", desc: "多条线" },
        { type: "ellipse", title: "圆 / 椭圆", desc: "仅静态预览" },
        { type: "bezier", title: "贝塞尔", desc: "仅静态预览" },
        { type: "cell", title: "元胞自动机", desc: "10×10" }
    ]
    property var palette: ["#FF8050", "#5B8CFF", "#34D399", "#FBBF24", "#C084FC", "#FB7185", "#E8EDF7"]

    onDesignerModelChanged: loadBlueprint(designerModel)
    Component.onCompleted: {
        if (designerModel)
            loadBlueprint(designerModel)
        else
            newModel("point")
    }

    function loadBlueprint(blueprint) {
        if (!blueprint) {
            newModel("point")
            return
        }
        var parsed = ModelFactory.parseBlueprint(blueprint)
        if (!parsed) {
            newModel("point")
            return
        }
        shape = parsed
        editingName = parsed.name || ""
        advancedMode = parsed.type === "advanced"
        advancedPayload = parsed.rawPayload || ModelFactory.buildPayload(parsed, editingName)
        statusText = advancedMode ? "已载入高级自动机配置" : "已载入模型，可拖拽画布控制点"
        shapeVersion++
    }

    function newModel(type) {
        shape = ModelFactory.makeShape(type)
        editingName = ""
        advancedMode = false
        advancedPayload = ModelFactory.buildPayload(shape, "")
        statusText = "已创建模板，填写名称并保存"
        nameError = ""
        shapeVersion++
    }

    function currentPayload() {
        if (advancedMode)
            return advancedPayload
        return ModelFactory.buildPayload(shape, editingName)
    }

    function switchMode(toAdvanced) {
        if (toAdvanced) {
            if (advancedPayload.trim().length === 0)
                advancedPayload = ModelFactory.buildPayload(shape, editingName)
        }
        advancedMode = toAdvanced
        shapeVersion++
    }

    function save(asCopy) {
        var name = editingName.trim()
        var pattern = /^[A-Z][A-Za-z0-9_]*$/
        if (!pattern.test(name)) {
            nameError = "名称需以大写字母开头，只含字母、数字或下划线"
            app.showToast(nameError, "error")
            return
        }
        nameError = ""
        var payload = currentPayload()
        if (advancedMode) {
            try {
                var parsed = JSON.parse(payload)
                if (!Array.isArray(parsed) || parsed.length === 0)
                    throw new Error("需要一个非空数组")
            } catch (error) {
                app.showToast("高级 JSON 无效：" + error.message, "error")
                return
            }
        } else if (payload === "[]") {
            app.showToast("图形数据为空，请先在画布上创建内容", "error")
            return
        }
        var targetName = asCopy ? name + "_Copy" : name
        if (app.saveModel(targetName, payload)) {
            editingName = targetName
            shape.name = targetName
            statusText = "已保存模型 " + targetName
            app.selectModel(targetName)
            app.reloadCatalog()
        }
    }

    function setPoint(index, axis, value) {
        if (!shape || !shape.points || index < 0 || index >= shape.points.length)
            return
        var point = shape.points[index]
        var number = Number(value)
        if (isNaN(number))
            return
        if (axis === "x") point.x = Math.round(number)
        else point.y = Math.round(number)
        shapeVersion++
    }

    function addPoint() {
        if (!shape)
            return
        if (!shape.points)
            shape.points = []
        var last = shape.points.length > 0 ? shape.points[shape.points.length - 1] : { x: 100, y: 100 }
        shape.points.push({ x: Number(last.x) + 40, y: Number(last.y) + 20 })
        shapeVersion++
    }

    function removePoint(index) {
        if (!shape || !shape.points || shape.points.length <= 2)
            return
        shape.points.splice(index, 1)
        shapeVersion++
    }

    function setField(field, value) {
        if (!shape)
            return
        shape[field] = value
        shapeVersion++
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            ColumnLayout {
                spacing: 2
                Text {
                    text: editingName.length > 0 ? ("编辑 · " + editingName) : "新建模型"
                    color: Theme.text
                    font.family: Theme.fontFamily
                    font.pixelSize: 18
                    font.weight: Font.DemiBold
                }
                Text {
                    text: root.statusText
                    color: Theme.textMuted
                    font.family: Theme.fontFamily
                    font.pixelSize: 11
                }
            }
            Item { Layout.fillWidth: true }
            AppButton {
                text: root.advancedMode ? "图形模式" : "高级模式"
                variant: "secondary"
                compact: true
                onClicked: root.switchMode(!root.advancedMode)
            }
            AppButton {
                text: "另存为"
                variant: "secondary"
                compact: true
                onClicked: root.save(true)
            }
            AppButton {
                text: "保存"
                compact: true
                onClicked: root.save(false)
            }
        }

        Flow {
            Layout.fillWidth: true
            visible: !root.advancedMode
            spacing: 8
            Repeater {
                model: root.templates
                delegate: Rectangle {
                    width: 116
                    height: 44
                    radius: Theme.radiusSmall
                    color: mouse.containsMouse ? Theme.surfaceHover : Theme.surface
                    border.width: 1
                    border.color: root.shape && root.shape.type === modelData.type ? Theme.accent : Theme.border
                    Column {
                        anchors.centerIn: parent
                        spacing: 1
                        Text {
                            text: modelData.title
                            color: Theme.text
                            font.family: Theme.fontFamily
                            font.pixelSize: 12
                            font.weight: Font.Medium
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                        Text {
                            text: modelData.desc
                            color: Theme.textDim
                            font.family: Theme.fontFamily
                            font.pixelSize: 9
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }
                    MouseArea {
                        id: mouse
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: root.newModel(modelData.type)
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: Theme.radius
                color: Theme.surface
                border.width: 1
                border.color: Theme.border
                clip: true

                ShapeCanvas {
                    visible: !root.advancedMode
                    anchors.fill: parent
                    shape: root.shape
                    shapeVersion: root.shapeVersion
                    onShapeEdited: {
                        root.statusText = "图形已修改，记得保存"
                        root.shapeVersion++
                    }
                }
                AdvancedAutomataEditor {
                    visible: root.advancedMode
                    anchors.fill: parent
                    anchors.margins: 12
                    payload: root.advancedPayload
                    onPayloadEdited: function(value) { root.advancedPayload = value }
                }
                Text {
                    visible: !root.advancedMode
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.margins: 12
                    text: "拖拽控制点 · 世界坐标 y 向上"
                    color: Theme.textDim
                    font.family: Theme.fontFamily
                    font.pixelSize: 10
                }
            }

            Rectangle {
                Layout.preferredWidth: 332
                Layout.fillHeight: true
                radius: Theme.radius
                color: Theme.surface
                border.width: 1
                border.color: Theme.border

                ScrollView {
                    anchors.fill: parent
                    anchors.margins: 16
                    clip: true
                    ColumnLayout {
                        width: parent.width
                        spacing: 10

                        Text {
                            text: "模型"
                            color: Theme.text
                            font.family: Theme.fontFamily
                            font.pixelSize: 14
                            font.weight: Font.DemiBold
                        }
                        AppTextField {
                            Layout.fillWidth: true
                            text: root.editingName
                            placeholderText: "例如 MyLine"
                            invalid: root.nameError.length > 0
                            onTextChanged: {
                                root.editingName = text
                                root.nameError = ""
                            }
                        }
                        Text {
                            Layout.fillWidth: true
                            text: root.nameError.length > 0 ? root.nameError : "大写字母开头；修改名称保存会创建新模型"
                            color: root.nameError.length > 0 ? Theme.danger : Theme.textDim
                            font.family: Theme.fontFamily
                            font.pixelSize: 10
                            wrapMode: Text.WordWrap
                        }

                        Text {
                            text: "外观"
                            color: Theme.text
                            font.family: Theme.fontFamily
                            font.pixelSize: 14
                            font.weight: Font.DemiBold
                            Layout.topMargin: 4
                        }
                        Flow {
                            Layout.fillWidth: true
                            spacing: 6
                            Repeater {
                                model: root.palette
                                delegate: Rectangle {
                                    width: 26; height: 26; radius: 6
                                    color: modelData
                                    border.width: root.shape && root.shape.color === modelData ? 2 : 1
                                    border.color: root.shape && root.shape.color === modelData ? "#FFFFFF" : Theme.border
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: {
                                            root.setField("color", modelData)
                                            root.statusText = "颜色已更新"
                                        }
                                    }
                                }
                            }
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8
                            Text { text: "颜色"; color: Theme.textMuted; font.family: Theme.fontFamily; font.pixelSize: 11; width: 44 }
                            AppTextField {
                                Layout.fillWidth: true
                                text: root.shape ? root.shape.color : "#FF8050"
                                placeholderText: "#FF8050"
                                onEditingFinished: {
                                    if (/^#[0-9A-Fa-f]{6}$/.test(text)) root.setField("color", text)
                                    else text = root.shape.color
                                }
                            }
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8
                            Text { text: "透明度"; color: Theme.textMuted; font.family: Theme.fontFamily; font.pixelSize: 11; width: 44 }
                            Slider {
                                Layout.fillWidth: true
                                from: 0.05; to: 1.0; stepSize: 0.05
                                value: root.shape ? root.shape.alpha : 1.0
                                onMoved: root.setField("alpha", value)
                            }
                        }

                        Text {
                            text: "几何"
                            color: Theme.text
                            font.family: Theme.fontFamily
                            font.pixelSize: 14
                            font.weight: Font.DemiBold
                            Layout.topMargin: 4
                        }
                        Text {
                            Layout.fillWidth: true
                            text: root.shape ? ModelFactory.describeShape(root.shape) : ""
                            color: Theme.textMuted
                            font.family: Theme.fontFamily
                            font.pixelSize: 11
                            wrapMode: Text.WordWrap
                        }

                        Repeater {
                            model: root.shape && root.shape.points ? root.shape.points.length : 0
                            delegate: RowLayout {
                                Layout.fillWidth: true
                                spacing: 6
                                Text {
                                    text: "P" + (index + 1)
                                    color: Theme.textDim
                                    font.family: Theme.monoFamily
                                    font.pixelSize: 10
                                    width: 22
                                }
                                AppTextField {
                                    Layout.fillWidth: true
                                    text: root.shape.points[index].x
                                    placeholderText: "x"
                                    onEditingFinished: root.setPoint(index, "x", text)
                                }
                                AppTextField {
                                    Layout.fillWidth: true
                                    text: root.shape.points[index].y
                                    placeholderText: "y"
                                    onEditingFinished: root.setPoint(index, "y", text)
                                }
                                IconButton {
                                    text: "×"
                                    visible: root.shape && (root.shape.type === "line" || root.shape.type === "polygon" || root.shape.type === "bezier")
                                    onClicked: root.removePoint(index)
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8
                            visible: root.shape && (root.shape.type === "line" || root.shape.type === "polygon" || root.shape.type === "bezier")
                            AppButton {
                                text: "添加控制点"
                                variant: "secondary"
                                compact: true
                                onClicked: root.addPoint()
                            }
                            CheckBox {
                                visible: root.shape && root.shape.type === "polygon"
                                text: "闭合"
                                checked: root.shape ? root.shape.closed : true
                                onToggled: root.setField("closed", checked)
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8
                            visible: root.shape && root.shape.type === "ellipse"
                            Text { text: "半径 X"; color: Theme.textMuted; font.family: Theme.fontFamily; font.pixelSize: 11; width: 50 }
                            AppTextField {
                                Layout.fillWidth: true
                                text: root.shape ? root.shape.rx : 100
                                onEditingFinished: root.setField("rx", Math.max(1, Number(text)))
                            }
                            Text { text: "Y"; color: Theme.textMuted; font.family: Theme.fontFamily; font.pixelSize: 11 }
                            AppTextField {
                                Layout.fillWidth: true
                                text: root.shape ? root.shape.ry : 60
                                onEditingFinished: root.setField("ry", Math.max(1, Number(text)))
                            }
                        }

                        Text {
                            visible: root.shape && root.shape.type === "cell"
                            Layout.fillWidth: true
                            text: "元胞自动机在引擎中使用随机初始状态；这里只控制模板，不影响后端演化规则。"
                            color: Theme.warning
                            font.family: Theme.fontFamily
                            font.pixelSize: 10
                            wrapMode: Text.WordWrap
                        }

                        Text {
                            text: "能力"
                            color: Theme.text
                            font.family: Theme.fontFamily
                            font.pixelSize: 14
                            font.weight: Font.DemiBold
                            Layout.topMargin: 4
                        }
                        Rectangle {
                            Layout.fillWidth: true
                            height: 44
                            radius: Theme.radiusSmall
                            color: Theme.surfaceAlt
                            border.width: 1
                            border.color: Theme.border
                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 10
                                ModelIcon { kind: root.shape ? ModelFactory.kindOfShape(root.shape) : "unknown"; width: 24; height: 24 }
                                Text {
                                    Layout.fillWidth: true
                                    text: {
                                        if (!root.shape) return ""
                                        var dynamic = ModelFactory.isDynamicShape(root.shape)
                                        var staticOk = ModelFactory.isStaticShape(root.shape)
                                        return (staticOk ? "静态预览" : "无静态预览") + " · " + (dynamic ? "可用于动态引擎" : "仅静态/不进入引擎")
                                    }
                                    color: Theme.textMuted
                                    font.family: Theme.fontFamily
                                    font.pixelSize: 11
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }

                        Text {
                            Layout.fillWidth: true
                            text: root.advancedMode ? "高级模式保存的是原始自动机 JSON；切换回图形模式不会丢失当前图形。" : "保存后写入 MySQL、XML，并清除该模型的 Redis 缓存。"
                            color: Theme.textDim
                            font.family: Theme.fontFamily
                            font.pixelSize: 10
                            wrapMode: Text.WordWrap
                        }
                    }
                }
            }
        }
    }

    IconButton {
        text: "↺"
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 12
        onClicked: {
            if (app.selectedModelName.length > 0) app.openModel(app.selectedModelName)
            else root.newModel("point")
        }
    }
}