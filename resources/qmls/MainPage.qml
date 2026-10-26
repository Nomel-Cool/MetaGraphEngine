import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Window
import CustomModules 1.0
import "Theme.js" as Theme
import "ModelFactory.js" as ModelFactory
import "components"
import "views"
import "preview"

ApplicationWindow {
    id: root
    visible: true
    width: Math.min(1440, Screen.desktopAvailableWidth - 80)
    height: Math.min(900, Screen.desktopAvailableHeight - 80)
    minimumWidth: 1100
    minimumHeight: 700
    title: "Graph Engine Kit"
    color: Theme.bg

    Material.theme: Material.Dark
    Material.primary: Theme.accent
    Material.accent: Theme.accent
    Material.background: Theme.bg
    Material.foreground: Theme.text

    QGraphFactory { id: factory }
    QGraphStudio { id: studio }

    property string currentPage: "library"
    property var catalog: []
    property var filteredCatalog: []
    property string selectedModelName: ""
    property var selectedPreview: ({})
    property var designerModel: null
    property var engineModels: []
    property var films: []
    property var activityLog: [{ "text": "应用已启动", "type": "info" }]
    property string filmName: "MyFilm"
    property bool backendReady: false
    property bool engineBusy: false
    property bool engineRunning: false
    property bool playbackBusy: false
    property string engineStatusText: "等待开始"
    property string pendingMode: "offline"
    property string blockingAction: ""
    property string blockingFilm: ""
    property string deleteTarget: ""
    property string searchText: ""
    property int filterType: 0
    property int pageIndex: 0
    readonly property var pageNames: ["library", "designer", "engine", "help"]
    readonly property bool realtimeAvailable: realtimeModelsOnly()

    onCurrentPageChanged: {
        var index = pageNames.indexOf(currentPage)
        pageIndex = index < 0 ? 0 : index
        if (currentPage === "library")
            reloadCatalog()
    }

    Component.onCompleted: {
        reloadCatalog()
        if (catalog.length > 0)
            selectModel(catalog[0].name)
        addLog("模型目录已加载，共 " + catalog.length + " 个模型", "info")
    }

    function addLog(text, type) {
        var entry = {
            "text": new Date().toTimeString().slice(0, 8) + "  " + text,
            "type": type === undefined ? "info" : type
        }
        activityLog = [entry].concat(activityLog).slice(0, 40)
    }

    function showToast(text, type) {
        toast.show(text, type)
        addLog(text, type)
    }

    function parseJson(text, fallback) {
        try {
            return JSON.parse(text)
        } catch (error) {
            return fallback
        }
    }

    function reloadCatalog() {
        var raw = factory.Request4ModelCatalog()
        var result = parseJson(raw, null)
        if (!result || !result.ok) {
            backendReady = false
            showToast("无法读取模型目录：" + (result && result.error ? result.error : "未知错误"), "error")
            return
        }
        backendReady = result.database === true
        catalog = result.models || []
        refreshFiltered()
        if (selectedModelName.length > 0) {
            var found = false
            for (var i = 0; i < catalog.length; ++i)
                if (catalog[i].name === selectedModelName) { found = true; break }
            if (!found) {
                selectedModelName = ""
                selectedPreview = ({})
            }
        }
        if (selectedModelName.length === 0 && catalog.length > 0)
            selectModel(catalog[0].name)
    }

    function refreshFiltered() {
        var query = searchText.toLowerCase()
        var result = []
        for (var i = 0; i < catalog.length; ++i) {
            var item = catalog[i]
            if (query.length > 0 && item.name.toLowerCase().indexOf(query) < 0)
                continue
            var kinds = item.kinds || []
            var matches = filterType === 0
            for (var k = 0; k < kinds.length; ++k) {
                if (filterType === 1 && kinds[k] === "point") matches = true
                if (filterType === 2 && kinds[k] === "line") matches = true
                if (filterType === 3 && (kinds[k] === "ellipse" || kinds[k] === "bezier")) matches = true
                if (filterType === 4 && kinds[k] === "cell") matches = true
                if (filterType === 5 && kinds[k] === "unknown") matches = true
            }
            if (matches)
                result.push(item)
        }
        filteredCatalog = result
    }

    function selectModel(name) {
        selectedModelName = name
        var raw = factory.Request4ModelPreview(name)
        var result = parseJson(raw, null)
        if (result && result.ok) {
            selectedPreview = result
        } else {
            selectedPreview = ({ "ok": false, "error": result && result.error ? result.error : "预览失败" })
            if (result && result.error)
                showToast("预览失败：" + result.error, "error")
        }
    }

    function openModel(name) {
        var raw = factory.Request4ModelBlueprint(name)
        var result = parseJson(raw, null)
        if (!result || !result.ok) {
            showToast("读取蓝图失败：" + (result && result.error ? result.error : "未知错误"), "error")
            return
        }
        designerModel = result
        selectModel(name)
        goPage("designer")
    }

    function newModel() {
        designerModel = null
        goPage("designer")
    }

    function saveModel(name, payload) {
        var ok = factory.Registry4Model(name, payload)
        if (ok) {
            reloadCatalog()
            selectModel(name)
        }
        return ok
    }

    function requestDelete(name) {
        deleteTarget = name
        confirmDialog.titleText = "删除模型"
        confirmDialog.message = "确定删除模型 “" + name + "” 吗？"
        confirmDialog.detail = "会同时清理 MySQL 记录、模型 XML 和 Redis 缓存。此操作不可撤销。"
        confirmDialog.confirmText = "删除"
        confirmDialog.open()
    }

    function deleteSelectedModel() {
        if (deleteTarget.length === 0)
            return
        var name = deleteTarget
        var ok = factory.DeleteModel(name)
        if (ok) {
            if (selectedModelName === name) {
                selectedModelName = ""
                selectedPreview = ({})
            }
            var newModels = []
            for (var i = 0; i < engineModels.length; ++i)
                if (engineModels[i] !== name) newModels.push(engineModels[i])
            engineModels = newModels
            reloadCatalog()
            showToast("已删除模型 " + name, "success")
        }
        deleteTarget = ""
    }

    function realtimeModelsOnly() {
        if (engineModels.length === 0)
            return false
        for (var i = 0; i < engineModels.length; ++i) {
            var model = null
            for (var j = 0; j < catalog.length; ++j)
                if (catalog[j].name === engineModels[i]) { model = catalog[j]; break }
            if (!model || !model.kinds || model.kinds.indexOf("cell") < 0)
                return false
        }
        return true
    }

    function toggleEngineModel(name) {
        var index = engineModels.indexOf(name)
        if (index >= 0) {
            engineModels = engineModels.slice(0, index).concat(engineModels.slice(index + 1))
            engineStatusText = "已移除 " + name
        } else {
            var model = null
            for (var i = 0; i < catalog.length; ++i)
                if (catalog[i].name === name) { model = catalog[i]; break }
            if (!model || !model.dynamic) {
                showToast("该模型只支持静态预览，不能加入动态引擎", "error")
                return
            }
            engineModels = engineModels.concat([name])
            engineStatusText = "已加入 " + name
        }
    }

    function clearEngine() {
        engineModels = []
        studio.RoleDismiss()
        engineStatusText = "已清空参演模型"
    }

    function autoEngineCameraZ() {
        var maxCoordinate = 0
        for (var i = 0; i < engineModels.length; ++i) {
            var raw = factory.Request4ModelPreview(engineModels[i])
            var preview = parseJson(raw, null)
            if (!preview || !preview.ok || !preview.bounds)
                continue
            var b = preview.bounds
            maxCoordinate = Math.max(maxCoordinate,
                Math.abs(Number(b.minX)), Math.abs(Number(b.maxX)),
                Math.abs(Number(b.minY)), Math.abs(Number(b.maxY)))
        }
        if (maxCoordinate <= 0)
            return 30
        return Math.max(28, Math.min(600, Math.round(maxCoordinate * 2.5)))
    }

    function startEngine(w, h, cx, cy, cz, is2D, lock, mode, cameraMode) {
        if (engineModels.length === 0) {
            showToast("请先选择至少一个可动态演化的模型", "error")
            return
        }
        if (!/^[A-Za-z_][A-Za-z0-9_]*$/.test(filmName)) {
            showToast("影片名需以字母或下划线开头，只含字母、数字、下划线", "error")
            return
        }
        if (!(w > 0) || !(h > 0)) {
            showToast("输出宽高必须大于 0", "error")
            return
        }
        if (mode === "realtime" && !realtimeAvailable) {
            showToast("实时预览目前只支持元胞自动机；点/线模型请使用离线生成并播放", "error")
            return
        }
        pendingMode = mode
        engineBusy = true
        engineRunning = false
        var effectiveZ = cz
        if (Number(cx) === 0 && Number(cy) === 0 && Number(cz) === 200)
            effectiveZ = autoEngineCameraZ()
        engineStatusText = effectiveZ === cz ? "正在加载模型..." : ("正在加载模型（自动取景 z=" + effectiveZ + "）...")
        studio.SetCameraControlMode(cameraMode === 1 ? 1 : 0)
        studio.InitWindow(w, h, cx, cy, effectiveZ, is2D, lock)
        studio.SetFilmName(filmName)
        studio.RoleEmplacement(engineModels)
        engineLoadTimer.restart()
        addLog("开始加载 " + engineModels.length + " 个模型，模式=" + mode, "info")
    }

    function stopEngine() {
        studio.Ceize()
    }

    function playFilm(name) {
        if (playbackBusy)
            return
        if (films.indexOf(name) < 0)
            return
        playbackBusy = true
        blockingAction = "play"
        blockingFilm = name
        engineStatusText = "正在打开播放窗口..."
        addLog("播放影片 " + name, "info")
        blockingTimer.start()
    }

    function goPage(page) {
        currentPage = page
    }

    Timer {
        id: blockingTimer
        interval: 80
        onTriggered: {
            if (blockingAction === "realtime") {
                blockingAction = ""
                engineStatusText = "实时窗口已打开，按 Esc 关闭"
                studio.RealTimeRender()
            } else if (blockingAction === "play") {
                blockingAction = ""
                var name = blockingFilm
                blockingFilm = ""
                studio.Display([name])
            }
        }
    }

    Timer {
        id: engineLoadTimer
        interval: 8000
        onTriggered: {
            engineBusy = false
            engineRunning = false
            engineStatusText = "模型加载超时"
            studio.RoleDismiss()
            showToast("模型加载超过 8 秒，已取消。请检查模型 XML/数据库。", "error")
        }
    }

    Connections {
        target: factory
        function onUpdate4NameList() {
            root.reloadCatalog()
        }
        function onOperationSucceeded(message) {
            root.showToast(message, "success")
        }
        function onOperationFailed(message) {
            root.showToast(message, "error")
            if (root.engineBusy) {
                root.engineBusy = false
                root.engineStatusText = "启动失败"
            }
        }
    }

    Connections {
        target: studio
        function onModelsLoaded(modelNames) {
            engineLoadTimer.stop()
            engineBusy = false
            addLog("模型加载完成：" + modelNames.join(", "), "success")
            if (pendingMode === "offline") {
                engineStatusText = "正在生成..."
                studio.Launch()
            } else {
                engineStatusText = "正在打开实时窗口..."
                blockingAction = "realtime"
                blockingTimer.start()
            }
        }
        function onModelsLoadFailed(message) {
            engineLoadTimer.stop()
            engineBusy = false
            engineStatusText = "模型加载失败"
            showToast("模型加载失败：" + message, "error")
        }
        function onEngineStarted(mode) {
            engineBusy = false
            engineRunning = true
            if (mode === "realtime") {
                engineStatusText = "实时窗口已打开，按 Esc 关闭"
            } else {
                engineStatusText = "生成中：点击“停止并保存”完成影片"
            }
            addLog("引擎已启动：" + mode, "success")
        }
        function onEngineStopped(mode) {
            engineRunning = false
            engineBusy = false
            if (mode === "realtime")
                engineStatusText = "实时窗口已关闭"
            else
                engineStatusText = "已停止"
            addLog("引擎已停止：" + mode, "info")
        }
        function onFilmSaved(filmName) {
            if (films.indexOf(filmName) < 0)
                films = films.concat([filmName])
            engineStatusText = "已保存影片 " + filmName
            addLog("影片已保存：" + filmName, "success")
        }
        function onPlaybackStarted(filmName) {
            playbackBusy = true
            engineStatusText = "正在播放 " + filmName + "，按 Esc 关闭窗口"
            addLog("开始播放 " + filmName, "info")
        }
        function onPlaybackStopped() {
            playbackBusy = false
            engineStatusText = "播放窗口已关闭"
            addLog("播放结束", "info")
        }
        function onOperationFailed(message) {
            showToast(message, "error")
            if (engineBusy) {
                engineBusy = false
                engineRunning = false
            }
        }
    }

    Rectangle {
        id: topBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 62
        color: Theme.surfaceAlt
        border.width: 0
        border.color: Theme.border

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 18
            anchors.rightMargin: 18
            spacing: 12

            Rectangle {
                width: 34; height: 34; radius: 9
                color: Theme.accent
                Text {
                    anchors.centerIn: parent
                    text: "G"
                    color: "#FFFFFF"
                    font.family: Theme.fontFamily
                    font.pixelSize: 18
                    font.weight: Font.Bold
                }
            }
            ColumnLayout {
                spacing: 0
                Text { text: "Graph Engine Kit"; color: Theme.text; font.family: Theme.fontFamily; font.pixelSize: 15; font.weight: Font.DemiBold }
                Text { text: "像素自动机可视化引擎"; color: Theme.textMuted; font.family: Theme.fontFamily; font.pixelSize: 9 }
            }
            Item { width: 8; height: 1 }
            Repeater {
                model: [
                    { "id": "library", "label": "模型库" },
                    { "id": "designer", "label": "模型设计" },
                    { "id": "engine", "label": "动画引擎" },
                    { "id": "help", "label": "帮助" }
                ]
                delegate: AppButton {
                    text: modelData.label
                    variant: root.currentPage === modelData.id ? "primary" : "ghost"
                    compact: true
                    onClicked: root.goPage(modelData.id)
                }
            }
            Item { Layout.fillWidth: true }
            StatusChip { ok: root.backendReady; text: root.backendReady ? "存储已连接" : "存储未连接" }
            IconButton {
                text: "⟳"
                tooltip: "刷新模型目录"
                onClicked: root.reloadCatalog()
            }
        }
    }

    RowLayout {
        anchors.top: topBar.bottom
        anchors.bottom: statusBar.top
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 0

        Rectangle {
            Layout.preferredWidth: 292
            Layout.fillHeight: true
            color: Theme.surface
            border.width: 1
            border.color: Theme.border

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 14
                spacing: 10

                RowLayout {
                    Layout.fillWidth: true
                    Text { text: "模型库"; color: Theme.text; font.family: Theme.fontFamily; font.pixelSize: 16; font.weight: Font.DemiBold }
                    Item { Layout.fillWidth: true }
                    Text { text: root.catalog.length; color: Theme.textMuted; font.family: Theme.fontFamily; font.pixelSize: 11 }
                }

                AppTextField {
                    Layout.fillWidth: true
                    placeholderText: "搜索模型名称…"
                    onTextChanged: {
                        root.searchText = text
                        root.refreshFiltered()
                    }
                }

                ComboBox {
                    Layout.fillWidth: true
                    model: ["全部类型", "点", "线 / 多边形", "曲线", "元胞自动机", "其他"]
                    currentIndex: root.filterType
                    onActivated: {
                        root.filterType = currentIndex
                        root.refreshFiltered()
                    }
                }

                ListView {
                    id: modelList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    spacing: 6
                    model: root.filteredCatalog
                    delegate: Rectangle {
                        id: modelRow
                        width: modelList.width
                        height: 58
                        radius: Theme.radiusSmall
                        color: root.selectedModelName === modelData.name ? Theme.accentSoft : (rowMouse.containsMouse ? Theme.surfaceHover : "transparent")
                        border.width: root.selectedModelName === modelData.name ? 1 : 0
                        border.color: Theme.accent
                        property bool rowHovered: rowMouse.containsMouse

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 8
                            anchors.rightMargin: 8
                            spacing: 8
                            ModelIcon {
                                kind: modelData.kinds && modelData.kinds.length > 0 ? modelData.kinds[0] : "unknown"
                                width: 26; height: 26
                            }
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 1
                                Text {
                                    Layout.fillWidth: true
                                    text: modelData.name
                                    color: Theme.text
                                    font.family: Theme.fontFamily
                                    font.pixelSize: 12
                                    font.weight: Font.Medium
                                    elide: Text.ElideRight
                                }
                                RowLayout {
                                    spacing: 4
                                    Text {
                                        text: Theme.kindLabel(modelData.kinds && modelData.kinds.length > 0 ? modelData.kinds[0] : "unknown")
                                        color: Theme.textDim
                                        font.family: Theme.fontFamily
                                        font.pixelSize: 9
                                    }
                                    Text {
                                        visible: modelData.dynamic === true
                                        text: "动态"
                                        color: Theme.success
                                        font.family: Theme.fontFamily
                                        font.pixelSize: 9
                                    }
                                    Text {
                                        visible: modelData.static === true
                                        text: "预览"
                                        color: Theme.warning
                                        font.family: Theme.fontFamily
                                        font.pixelSize: 9
                                    }
                                }
                            }
                            RowLayout {
                                spacing: 1
                                visible: modelRow.rowHovered || root.selectedModelName === modelData.name
                                IconButton { text: "编"; tooltip: "编辑模型"; onClicked: root.openModel(modelData.name) }
                                IconButton { text: "＋"; tooltip: "加入动画引擎"; onClicked: root.toggleEngineModel(modelData.name) }
                                IconButton { text: "删"; tooltip: "删除模型"; onClicked: root.requestDelete(modelData.name) }
                            }
                        }

                        MouseArea {
                            id: rowMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            acceptedButtons: Qt.LeftButton
                            onClicked: root.selectModel(modelData.name)
                            onDoubleClicked: root.openModel(modelData.name)
                        }
                    }
                }

                Text {
                    Layout.fillWidth: true
                    visible: root.filteredCatalog.length === 0
                    text: root.catalog.length === 0 ? "还没有模型。点击下方“新建模型”开始。" : "没有匹配的模型，试试清空搜索。"
                    color: Theme.textDim
                    font.family: Theme.fontFamily
                    font.pixelSize: 11
                    wrapMode: Text.WordWrap
                }

                AppButton {
                    Layout.fillWidth: true
                    text: "＋ 新建模型"
                    onClicked: root.newModel()
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            Rectangle {
                Layout.fillWidth: true
                visible: !root.backendReady
                height: 38
                color: Theme.dangerSoft
                border.width: 0
                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 14
                    anchors.rightMargin: 14
                    spacing: 8
                    Text {
                        Layout.fillWidth: true
                        text: "数据库未连接：请确认 MySQL80 服务运行，并已创建 meta_graph_model。模型目录和保存操作可能不可用。"
                        color: "#FFB4B4"
                        font.family: Theme.fontFamily
                        font.pixelSize: 10
                        elide: Text.ElideRight
                    }
                    AppButton {
                        text: "重试"
                        variant: "danger"
                        compact: true
                        onClicked: root.reloadCatalog()
                    }
                }
            }

            StackLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: root.pageIndex

                Item {
                    id: libraryPage
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 12

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 10
                            ColumnLayout {
                                spacing: 2
                                Text {
                                    text: root.selectedModelName.length > 0 ? root.selectedModelName : "模型库"
                                    color: Theme.text
                                    font.family: Theme.fontFamily
                                    font.pixelSize: 20
                                    font.weight: Font.DemiBold
                                }
                                Text {
                                    text: root.selectedModelName.length > 0 ? (Theme.kindLabel(root.selectedPreview.kinds && root.selectedPreview.kinds.length > 0 ? root.selectedPreview.kinds[0] : "unknown") + " · " + root.selectedPreview.pointCount + " 个点") : "选择左侧模型查看预览"
                                    color: Theme.textMuted
                                    font.family: Theme.fontFamily
                                    font.pixelSize: 11
                                }
                            }
                            Item { Layout.fillWidth: true }
                            AppButton {
                                text: "编辑"
                                variant: "secondary"
                                compact: true
                                enabled: root.selectedModelName.length > 0
                                onClicked: root.openModel(root.selectedModelName)
                            }
                            AppButton {
                                text: "加入动画"
                                compact: true
                                enabled: root.selectedModelName.length > 0 && root.selectedPreview.ok === true
                                onClicked: root.toggleEngineModel(root.selectedModelName)
                            }
                            AppButton {
                                text: "删除"
                                variant: "danger"
                                compact: true
                                enabled: root.selectedModelName.length > 0
                                onClicked: root.requestDelete(root.selectedModelName)
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            radius: Theme.radius
                            color: Theme.surface
                            border.width: 1
                            border.color: Theme.border
                            clip: true

                            PreviewCanvas {
                                anchors.fill: parent
                                anchors.margins: 12
                                preview: root.selectedPreview
                            }

                            Column {
                                anchors.centerIn: parent
                                spacing: 8
                                visible: root.selectedModelName.length === 0 || root.selectedPreview.ok !== true || root.selectedPreview.pointCount === 0
                                width: parent.width - 60
                                Text {
                                    text: root.selectedModelName.length === 0 ? "从左侧选择一个模型" : (root.selectedPreview.runtimeOnly === true ? "该模型没有静态预览" : "预览不可用")
                                    color: Theme.text
                                    font.family: Theme.fontFamily
                                    font.pixelSize: 15
                                    font.weight: Font.Medium
                                    horizontalAlignment: Text.AlignHCenter
                                    width: parent.width
                                }
                                Text {
                                    text: root.selectedPreview.runtimeOnly === true ? "元胞自动机在引擎运行时演化，可在“动画引擎”中加入并实时预览。" : (root.selectedPreview.error ? root.selectedPreview.error : "该模型可能在数据库中缺少有效 XML，请尝试刷新或编辑后保存。")
                                    color: Theme.textMuted
                                    font.family: Theme.fontFamily
                                    font.pixelSize: 11
                                    wrapMode: Text.WordWrap
                                    horizontalAlignment: Text.AlignHCenter
                                    width: parent.width
                                    lineHeight: 1.4
                                }
                                AppButton {
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    text: "刷新"
                                    variant: "secondary"
                                    compact: true
                                    visible: root.selectedModelName.length > 0
                                    onClicked: root.selectModel(root.selectedModelName)
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 54
                            radius: Theme.radiusSmall
                            color: Theme.surfaceAlt
                            border.width: 1
                            border.color: Theme.border
                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 16
                                Text { text: "能力"; color: Theme.textDim; font.family: Theme.fontFamily; font.pixelSize: 10 }
                                Text {
                                    text: root.selectedPreview.ok === true ? ((root.selectedPreview.pointCount > 0 ? "静态预览" : "无静态预览") + " · " + ((root.catalog.filter(function(x) { return x.name === root.selectedModelName })[0] || {}).dynamic === true ? "可动态演化" : "仅静态/不支持引擎")) : "—"
                                    color: Theme.textMuted
                                    font.family: Theme.fontFamily
                                    font.pixelSize: 10
                                }
                                Item { Layout.fillWidth: true }
                                Text {
                                    text: root.selectedPreview.bounds ? ("bounds x[" + root.selectedPreview.bounds.minX + ", " + root.selectedPreview.bounds.maxX + "] y[" + root.selectedPreview.bounds.minY + ", " + root.selectedPreview.bounds.maxY + "]") : ""
                                    color: Theme.textDim
                                    font.family: Theme.monoFamily
                                    font.pixelSize: 9
                                }
                            }
                        }
                    }
                }

                DesignerView {
                    app: root
                    designerModel: root.designerModel
                }

                EngineView {
                    app: root
                }

                HelpView {
                    app: root
                }
            }
        }
    }

    Rectangle {
        id: statusBar
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 28
        color: Theme.surfaceAlt
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 14
            anchors.rightMargin: 14
            spacing: 12
            Text {
                text: "●"
                color: root.backendReady ? Theme.success : Theme.danger
                font.pixelSize: 9
            }
            Text {
                text: root.backendReady ? "MySQL 已连接" : "MySQL 未连接"
                color: Theme.textMuted
                font.family: Theme.fontFamily
                font.pixelSize: 9
            }
            Text { text: "·"; color: Theme.textDim; font.pixelSize: 9 }
            Text { text: root.catalog.length + " 个模型"; color: Theme.textMuted; font.family: Theme.fontFamily; font.pixelSize: 9 }
            Text { text: "·"; color: Theme.textDim; font.pixelSize: 9 }
            Text { text: root.engineStatusText; color: Theme.textMuted; font.family: Theme.fontFamily; font.pixelSize: 9 }
            Item { Layout.fillWidth: true }
            Text {
                text: root.selectedModelName.length > 0 ? ("当前：" + root.selectedModelName) : "未选择模型"
                color: Theme.textDim
                font.family: Theme.fontFamily
                font.pixelSize: 9
            }
        }
    }

    Toast {
        id: toast
        anchors.horizontalCenter: parent.horizontalCenter
        parent: root.contentItem
    }

    ConfirmDialog {
        id: confirmDialog
        onConfirmed: root.deleteSelectedModel()
    }
}