import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../Theme.js" as Theme
import "../ModelFactory.js" as ModelFactory
import "../components"

Item {
    id: root
    property var app: null
    property real outWidth: 800
    property real outHeight: 600
    property real cameraX: 0
    property real cameraY: 0
    property real cameraZ: 200
    property bool view2D: false
    property bool viewLock: false
    property int cameraMode: 0
    property string mode: "offline"

    function autoCamera() {
        var preview = app.currentPreview
        if (preview && preview.bounds) {
            var range = Math.max(Number(preview.bounds.maxX) - Number(preview.bounds.minX),
                                 Number(preview.bounds.maxY) - Number(preview.bounds.minY))
            cameraZ = Math.max(30, Math.round(range * 3))
            app.showToast("已根据当前预览自动设置相机距离 z=" + cameraZ, "success")
        } else {
            cameraZ = 20
            app.showToast("没有可用的预览范围，已使用 z=20", "info")
        }

    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            ColumnLayout {
                spacing: 2
                Text {
                    text: "动画引擎"
                    color: Theme.text
                    font.family: Theme.fontFamily
                    font.pixelSize: 20
                    font.weight: Font.DemiBold
                }
                Text {
                    text: "选择可动态演化的模型，设置输出窗口，再生成或实时预览。"
                    color: Theme.textMuted
                    font.family: Theme.fontFamily
                    font.pixelSize: 11
                }
            }
            Item { Layout.fillWidth: true }
            StatusChip { ok: app.backendReady }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12

            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 520
                clip: true
                ColumnLayout {
                    width: parent.width - 8
                    spacing: 12

                    Rectangle {
                        Layout.fillWidth: true
                        radius: Theme.radius
                        color: Theme.surface
                        border.width: 1
                        border.color: Theme.border
                        implicitHeight: modelColumn.implicitHeight + 28
                        ColumnLayout {
                            id: modelColumn
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 8
                            RowLayout {
                                Layout.fillWidth: true
                                Text { text: "1. 参演模型"; color: Theme.text; font.family: Theme.fontFamily; font.pixelSize: 14; font.weight: Font.DemiBold }
                                Item { Layout.fillWidth: true }
                                Text { text: app.engineModels.length + " 个已选"; color: Theme.accent; font.family: Theme.fontFamily; font.pixelSize: 11 }
                            }
                            Text {
                                Layout.fillWidth: true
                                text: "只有“可动态演化”的模型能进入引擎；椭圆和贝塞尔目前只支持静态预览。"
                                color: Theme.textDim
                                font.family: Theme.fontFamily
                                font.pixelSize: 10
                                wrapMode: Text.WordWrap
                            }
                            Flow {
                                Layout.fillWidth: true
                                spacing: 4
                                Repeater {
                                    model: app.catalog
                                    delegate: Rectangle {
                                        width: 190
                                        height: 38
                                        radius: Theme.radiusSmall
                                        color: checked ? Theme.accentSoft : Theme.surfaceAlt
                                        border.width: 1
                                        border.color: checked ? Theme.accent : Theme.border
                                        property bool checked: app.engineModels.indexOf(modelData.name) >= 0
                                        property bool dynamic: modelData.dynamic === true
                                        RowLayout {
                                            anchors.fill: parent
                                            anchors.leftMargin: 8
                                            anchors.rightMargin: 8
                                            spacing: 6
                                            CheckBox {
                                                checked: parent.parent.checked
                                                enabled: parent.parent.dynamic
                                                onToggled: app.toggleEngineModel(modelData.name)
                                            }
                                            ModelIcon { kind: modelData.kinds && modelData.kinds.length > 0 ? modelData.kinds[0] : "unknown"; width: 20; height: 20 }
                                            Column {
                                                Layout.fillWidth: true
                                                spacing: 0
                                                Text {
                                                    text: modelData.name
                                                    color: parent.parent.parent.dynamic ? Theme.text : Theme.textDim
                                                    font.family: Theme.fontFamily
                                                    font.pixelSize: 11
                                                    elide: Text.ElideRight
                                                    width: 120
                                                }
                                                Text {
                                                    text: parent.parent.parent.dynamic ? "可动态演化" : "仅静态 / 不支持引擎"
                                                    color: Theme.textDim
                                                    font.family: Theme.fontFamily
                                                    font.pixelSize: 8
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            Text {
                                visible: app.catalog.length === 0
                                text: "模型库为空。请到“模型设计”创建模型，或在帮助页载入示例。"
                                color: Theme.warning
                                font.family: Theme.fontFamily
                                font.pixelSize: 11
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        radius: Theme.radius
                        color: Theme.surface
                        border.width: 1
                        border.color: Theme.border
                        implicitHeight: outputColumn.implicitHeight + 28
                        ColumnLayout {
                            id: outputColumn
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 10
                            Text { text: "2. 输出窗口"; color: Theme.text; font.family: Theme.fontFamily; font.pixelSize: 14; font.weight: Font.DemiBold }
                            GridLayout {
                                Layout.fillWidth: true
                                columns: 4
                                columnSpacing: 8
                                rowSpacing: 6
                                Text { text: "宽"; color: Theme.textMuted; font.family: Theme.fontFamily; font.pixelSize: 11 }
                                AppTextField {
                                    Layout.fillWidth: true
                                    text: root.outWidth
                                    validator: IntValidator { bottom: 64; top: 4096 }
                                    onEditingFinished: root.outWidth = Number(text)
                                }
                                Text { text: "高"; color: Theme.textMuted; font.family: Theme.fontFamily; font.pixelSize: 11 }
                                AppTextField {
                                    Layout.fillWidth: true
                                    text: root.outHeight
                                    validator: IntValidator { bottom: 64; top: 4096 }
                                    onEditingFinished: root.outHeight = Number(text)
                                }
                                Text { text: "相机 X"; color: Theme.textMuted; font.family: Theme.fontFamily; font.pixelSize: 11 }
                                AppTextField {
                                    Layout.fillWidth: true
                                    text: root.cameraX
                                    validator: DoubleValidator {}
                                    onEditingFinished: root.cameraX = Number(text)
                                }
                                Text { text: "相机 Y"; color: Theme.textMuted; font.family: Theme.fontFamily; font.pixelSize: 11 }
                                AppTextField {
                                    Layout.fillWidth: true
                                    text: root.cameraY
                                    validator: DoubleValidator {}
                                    onEditingFinished: root.cameraY = Number(text)
                                }
                                Text { text: "相机 Z"; color: Theme.textMuted; font.family: Theme.fontFamily; font.pixelSize: 11 }
                                AppTextField {
                                    Layout.fillWidth: true
                                    text: root.cameraZ
                                    validator: DoubleValidator {}
                                    onEditingFinished: root.cameraZ = Number(text)
                                }
                                Item { Layout.fillWidth: true }
                                AppButton {
                                    text: "自动取景"
                                    variant: "secondary"
                                    compact: true
                                    onClicked: root.autoCamera()
                                }
                            }
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 16
                                RadioButton {
                                    text: "3D 透视"
                                    checked: !root.view2D
                                    onToggled: if (checked) root.view2D = false
                                }
                                RadioButton {
                                    text: "2D 正交"
                                    checked: root.view2D
                                    onToggled: if (checked) root.view2D = true
                                }
                                CheckBox {
                                    text: "锁定视角"
                                    checked: root.viewLock
                                    onToggled: root.viewLock = checked
                                }
                            }
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 16
                                Text {
                                    text: "鼠标视角"
                                    color: Theme.textMuted
                                    font.family: Theme.fontFamily
                                    font.pixelSize: 11
                                }
                                RadioButton {
                                    text: "自由视角（移动鼠标）"
                                    checked: root.cameraMode === 0
                                    enabled: !root.viewLock
                                    onToggled: if (checked) root.cameraMode = 0
                                    onClicked: root.cameraMode = 0
                                }
                                RadioButton {
                                    text: "点击拖拽（按住左键）"
                                    checked: root.cameraMode === 1
                                    enabled: !root.viewLock
                                    onToggled: if (checked) root.cameraMode = 1
                                    onClicked: root.cameraMode = 1
                                }
                            }
                            Text {
                                Layout.fillWidth: true
                                text: "自由视角：窗口获得焦点时移动鼠标旋转；点击拖拽：按住左键拖动旋转；锁定视角会关闭鼠标旋转。2D 为正交投影。"
                                color: Theme.textDim
                                font.family: Theme.fontFamily
                                font.pixelSize: 10
                                wrapMode: Text.WordWrap
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        radius: Theme.radius
                        color: Theme.surface
                        border.width: 1
                        border.color: Theme.border
                        implicitHeight: actionColumn.implicitHeight + 28
                        ColumnLayout {
                            id: actionColumn
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 10
                            Text { text: "3. 生成"; color: Theme.text; font.family: Theme.fontFamily; font.pixelSize: 14; font.weight: Font.DemiBold }
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 8
                                Text { text: "影片名"; color: Theme.textMuted; font.family: Theme.fontFamily; font.pixelSize: 11; width: 48 }
                                AppTextField {
                                    id: filmNameField
                                    Layout.fillWidth: true
                                    text: app.filmName
                                    placeholderText: "例如 MyFilm"
                                    onTextChanged: app.filmName = text
                                }
                            }
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 16
                                RadioButton {
                                    text: "离线生成并保存"
                                    checked: root.mode === "offline"
                                    onToggled: if (checked) root.mode = "offline"
                                    onClicked: root.mode = "offline"
                                }
                                RadioButton {
                                    text: "实时预览（仅元胞，不保存影片）"
                                    checked: root.mode === "realtime"
                                    enabled: app.realtimeAvailable
                                    onToggled: if (checked) root.mode = "realtime"
                                    onClicked: root.mode = "realtime"
                                }
                            }
                            Text {
                                Layout.fillWidth: true
                                text: root.mode === "offline" ? "离线模式在后台逐帧生成，点击“停止并保存”后影片出现在右侧列表，可播放。" : "实时模式会打开独立的 Graphs Stage 窗口，可用鼠标/WASD 交互，按 Esc 关闭；它不会生成影片。"
                                color: Theme.textDim
                                font.family: Theme.fontFamily
                                font.pixelSize: 10
                                wrapMode: Text.WordWrap
                            }
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 8
                                AppButton {
                                    text: "开始生成"
                                    enabled: !app.engineBusy && !app.engineRunning && app.engineModels.length > 0
                                    onClicked: app.startEngine(root.outWidth, root.outHeight, root.cameraX, root.cameraY, root.cameraZ, root.view2D, root.viewLock, root.mode, root.cameraMode)
                                }
                                AppButton {
                                    text: "停止并保存"
                                    variant: "danger"
                                    enabled: app.engineRunning && root.mode === "offline"
                                    onClicked: app.stopEngine()
                                }
                                AppButton {
                                    text: "清空选择"
                                    variant: "secondary"
                                    enabled: !app.engineBusy && !app.engineRunning
                                    onClicked: app.clearEngine()
                                }
                                Item { Layout.fillWidth: true }
                                BusyIndicator {
                                    running: app.engineBusy
                                    visible: running
                                    implicitWidth: 28
                                    implicitHeight: 28
                                }
                            }
                            Text {
                                Layout.fillWidth: true
                                text: app.engineStatusText
                                color: app.engineRunning ? Theme.success : Theme.textMuted
                                font.family: Theme.fontFamily
                                font.pixelSize: 11
                                wrapMode: Text.WordWrap
                            }
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 330
                spacing: 12

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: Theme.radius
                    color: Theme.surface
                    border.width: 1
                    border.color: Theme.border
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 8
                        RowLayout {
                            Layout.fillWidth: true
                            Text { text: "影片库"; color: Theme.text; font.family: Theme.fontFamily; font.pixelSize: 14; font.weight: Font.DemiBold }
                            Item { Layout.fillWidth: true }
                            Text { text: app.films.length + " 部"; color: Theme.textMuted; font.family: Theme.fontFamily; font.pixelSize: 10 }
                        }
                        Text {
                            Layout.fillWidth: true
                            text: "影片保存在本次进程内，重启后清空；生成后点播放打开 Graphs Stage。"
                            color: Theme.textDim
                            font.family: Theme.fontFamily
                            font.pixelSize: 10
                            wrapMode: Text.WordWrap
                        }
                        ListView {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            clip: true
                            model: app.films
                            spacing: 6
                            delegate: Rectangle {
                                width: ListView.view.width
                                height: 48
                                radius: Theme.radiusSmall
                                color: Theme.surfaceAlt
                                border.width: 1
                                border.color: Theme.border
                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    ModelIcon { kind: "line"; width: 20; height: 20 }
                                    Text {
                                        Layout.fillWidth: true
                                        text: modelData
                                        color: Theme.text
                                        font.family: Theme.fontFamily
                                        font.pixelSize: 12
                                        elide: Text.ElideRight
                                    }
                                    AppButton {
                                        text: "播放"
                                        compact: true
                                        enabled: !app.playbackBusy
                                        onClicked: app.playFilm(modelData)
                                    }
                                }
                            }
                        }
                        Text {
                            Layout.fillWidth: true
                            visible: app.films.length === 0
                            text: "还没有影片。完成一次离线生成后，影片会出现在这里。"
                            color: Theme.textDim
                            font.family: Theme.fontFamily
                            font.pixelSize: 11
                            wrapMode: Text.WordWrap
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 210
                    radius: Theme.radius
                    color: Theme.surface
                    border.width: 1
                    border.color: Theme.border
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 6
                        Text { text: "活动日志"; color: Theme.text; font.family: Theme.fontFamily; font.pixelSize: 13; font.weight: Font.DemiBold }
                        ListView {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            clip: true
                            model: app.activityLog
                            spacing: 3
                            delegate: Text {
                                width: ListView.view.width
                                text: modelData.text
                                color: modelData.type === "error" ? Theme.danger : (modelData.type === "success" ? Theme.success : Theme.textMuted)
                                font.family: Theme.monoFamily
                                font.pixelSize: 9
                                wrapMode: Text.WordWrap
                            }
                        }
                    }
                }
            }

        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 62
            radius: Theme.radius
            color: Theme.surfaceAlt
            border.width: 1
            border.color: Theme.border
            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10
                Text {
                    text: "快捷操作"
                    color: Theme.text
                    font.family: Theme.fontFamily
                    font.pixelSize: 12
                    font.weight: Font.DemiBold
                }
                Text {
                    Layout.fillWidth: true
                    text: app.engineModels.length + " 个模型 · " + app.filmName + " · " + app.engineStatusText
                    color: Theme.textMuted
                    font.family: Theme.fontFamily
                    font.pixelSize: 11
                    elide: Text.ElideRight
                }
                BusyIndicator {
                    running: app.engineBusy
                    visible: running
                    implicitWidth: 26
                    implicitHeight: 26
                }
                AppButton {
                    text: "开始生成"
                    enabled: !app.engineBusy && !app.engineRunning && app.engineModels.length > 0
                    onClicked: app.startEngine(root.outWidth, root.outHeight, root.cameraX, root.cameraY, root.cameraZ, root.view2D, root.viewLock, root.mode, root.cameraMode)
                }
                AppButton {
                    text: "停止并保存"
                    variant: "danger"
                    enabled: app.engineRunning && root.mode === "offline"
                    onClicked: app.stopEngine()
                }
            }
        }
    }
}