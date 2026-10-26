import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../Theme.js" as Theme
import "../components"

Item {
    id: root
    property var app: null

    function card(title, body) {
        return { title: title, body: body }
    }

    ScrollView {
        anchors.fill: parent
        clip: true

        ColumnLayout {
            width: parent.width - 16
            spacing: 14

            Text {
                text: "使用帮助"
                color: Theme.text
                font.family: Theme.fontFamily
                font.pixelSize: 20
                font.weight: Font.DemiBold
            }
            Text {
                Layout.fillWidth: true
                text: "这个工具把“像素自动机”包装成可视化的模型和动画。你不需要先会写 JSON，也不需要证明自动机；先用模板做出图形，再到动画引擎里生成即可。"
                color: Theme.textMuted
                font.family: Theme.fontFamily
                font.pixelSize: 12
                wrapMode: Text.WordWrap
                lineHeight: 1.5
            }

            Rectangle {
                Layout.fillWidth: true
                radius: Theme.radius
                color: Theme.surface
                border.width: 1
                border.color: Theme.border
                implicitHeight: quickColumn.implicitHeight + 28
                ColumnLayout {
                    id: quickColumn
                    anchors.fill: parent
                    anchors.margins: 14
                    spacing: 10
                    Text { text: "四步上手"; color: Theme.text; font.family: Theme.fontFamily; font.pixelSize: 15; font.weight: Font.DemiBold }
                    Repeater {
                        model: [
                            { n: "1", t: "选模板", d: "在“模型设计”顶部点“点 / 线 / 多边形 / 椭圆 / 贝塞尔 / 元胞”，画布会出现一个可编辑图形。" },
                            { n: "2", t: "改图形", d: "拖拽画布上的白色控制点，或在右侧输入精确坐标；颜色、透明度、尺寸也在右侧修改。" },
                            { n: "3", t: "保存模型", d: "名称以大写字母开头，点保存。模型会写入 MySQL 与 XML，并清除 Redis 缓存。" },
                            { n: "4", t: "生成动画", d: "到“动画引擎”，勾选可动态演化的模型，设置窗口和影片名，开始生成；停止并保存后可在右侧播放。" }
                        ]
                        delegate: RowLayout {
                            Layout.fillWidth: true
                            spacing: 10
                            Rectangle {
                                width: 26; height: 26; radius: 13
                                color: Theme.accentSoft
                                border.width: 1; border.color: Theme.accent
                                Text {
                                    anchors.centerIn: parent
                                    text: modelData.n
                                    color: Theme.accent
                                    font.family: Theme.fontFamily
                                    font.pixelSize: 12
                                    font.weight: Font.Bold
                                }
                            }
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 1
                                Text { text: modelData.t; color: Theme.text; font.family: Theme.fontFamily; font.pixelSize: 12; font.weight: Font.Medium }
                                Text { Layout.fillWidth: true; text: modelData.d; color: Theme.textMuted; font.family: Theme.fontFamily; font.pixelSize: 10; wrapMode: Text.WordWrap; lineHeight: 1.4 }
                            }
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                radius: Theme.radius
                color: Theme.surface
                border.width: 1
                border.color: Theme.border
                implicitHeight: conceptColumn.implicitHeight + 28
                ColumnLayout {
                    id: conceptColumn
                    anchors.fill: parent
                    anchors.margins: 14
                    spacing: 8
                    Text { text: "自动机五字段是什么意思？"; color: Theme.text; font.family: Theme.fontFamily; font.pixelSize: 15; font.weight: Font.DemiBold }
                    Repeater {
                        model: [
                            { k: "init", v: "初始状态。点类模型这里放一个像素数组（x/y/z/颜色/tag/blockSize）；线类模型放起点。" },
                            { k: "transfer", v: "转移函数/算法名。例如 JustAPoint、BresenhamLine、CellAutomataFrame_0。UI 会根据模板自动填写。" },
                            { k: "input", v: "当前输入。线类模型用它给 tagHead，让引擎知道新像素的编号从哪里开始。" },
                            { k: "current", v: "当前状态。通常保存空数组，运行时由引擎写入。" },
                            { k: "terminate", v: "终止状态。线类模型放终点；椭圆放半径 a/b；元胞通常为空。" }
                        ]
                        delegate: RowLayout {
                            Layout.fillWidth: true
                            spacing: 10
                            Text {
                                width: 74
                                text: modelData.k
                                color: Theme.accent
                                font.family: Theme.monoFamily
                                font.pixelSize: 11
                            }
                            Text {
                                Layout.fillWidth: true
                                text: modelData.v
                                color: Theme.textMuted
                                font.family: Theme.fontFamily
                                font.pixelSize: 10
                                wrapMode: Text.WordWrap
                                lineHeight: 1.4
                            }
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                radius: Theme.radius
                color: Theme.surface
                border.width: 1
                border.color: Theme.border
                implicitHeight: matrixColumn.implicitHeight + 28
                ColumnLayout {
                    id: matrixColumn
                    anchors.fill: parent
                    anchors.margins: 14
                    spacing: 6
                    Text { text: "能力矩阵"; color: Theme.text; font.family: Theme.fontFamily; font.pixelSize: 15; font.weight: Font.DemiBold }
                    GridLayout {
                        Layout.fillWidth: true
                        columns: 4
                        columnSpacing: 14
                        rowSpacing: 6
                        Text { text: "模板"; color: Theme.textMuted; font.pixelSize: 10; font.family: Theme.fontFamily }
                        Text { text: "算法"; color: Theme.textMuted; font.pixelSize: 10; font.family: Theme.fontFamily }
                        Text { text: "静态预览"; color: Theme.textMuted; font.pixelSize: 10; font.family: Theme.fontFamily }
                        Text { text: "动态引擎"; color: Theme.textMuted; font.pixelSize: 10; font.family: Theme.fontFamily }
                        Text { text: "点"; color: Theme.text; font.pixelSize: 10; font.family: Theme.fontFamily }
                        Text { text: "JustAPoint"; color: Theme.textDim; font.pixelSize: 10; font.family: Theme.monoFamily }
                        Text { text: "✓"; color: Theme.success; font.pixelSize: 11 }
                        Text { text: "✓"; color: Theme.success; font.pixelSize: 11 }
                        Text { text: "线/多边形"; color: Theme.text; font.pixelSize: 10; font.family: Theme.fontFamily }
                        Text { text: "BresenhamLine"; color: Theme.textDim; font.pixelSize: 10; font.family: Theme.monoFamily }
                        Text { text: "✓"; color: Theme.success; font.pixelSize: 11 }
                        Text { text: "✓"; color: Theme.success; font.pixelSize: 11 }
                        Text { text: "圆/椭圆"; color: Theme.text; font.pixelSize: 10; font.family: Theme.fontFamily }
                        Text { text: "BresenhamEllipse"; color: Theme.textDim; font.pixelSize: 10; font.family: Theme.monoFamily }
                        Text { text: "✓"; color: Theme.success; font.pixelSize: 11 }
                        Text { text: "—"; color: Theme.textDim; font.pixelSize: 11 }
                        Text { text: "贝塞尔"; color: Theme.text; font.pixelSize: 10; font.family: Theme.fontFamily }
                        Text { text: "PartitionBezierCurve"; color: Theme.textDim; font.pixelSize: 10; font.family: Theme.monoFamily }
                        Text { text: "✓"; color: Theme.success; font.pixelSize: 11 }
                        Text { text: "—"; color: Theme.textDim; font.pixelSize: 11 }
                        Text { text: "元胞"; color: Theme.text; font.pixelSize: 10; font.family: Theme.fontFamily }
                        Text { text: "CellAutomataFrame_0"; color: Theme.textDim; font.pixelSize: 10; font.family: Theme.monoFamily }
                        Text { text: "—"; color: Theme.textDim; font.pixelSize: 11 }
                        Text { text: "✓"; color: Theme.success; font.pixelSize: 11 }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                radius: Theme.radius
                color: Theme.surface
                border.width: 1
                border.color: Theme.border
                implicitHeight: glColumn.implicitHeight + 28
                ColumnLayout {
                    id: glColumn
                    anchors.fill: parent
                    anchors.margins: 14
                    spacing: 6
                    Text { text: "3D / Graphs Stage 操作"; color: Theme.text; font.family: Theme.fontFamily; font.pixelSize: 15; font.weight: Font.DemiBold }
                    Text {
                        Layout.fillWidth: true
                        text: "W/A/S/D 前后左右平移，Q/E 上下，鼠标旋转视角，滚轮缩放 FOV，I/K 绕 X 轴旋转，J/L 绕 Z 轴旋转，U/O 绕 Y 轴旋转。按 Esc 或关闭窗口返回主界面。实时预览窗口关闭后，主界面会自动恢复按钮状态。"
                        color: Theme.textMuted
                        font.family: Theme.fontFamily
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                        lineHeight: 1.5
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                radius: Theme.radius
                color: Theme.surface
                border.width: 1
                border.color: Theme.border
                implicitHeight: troubleColumn.implicitHeight + 28
                ColumnLayout {
                    id: troubleColumn
                    anchors.fill: parent
                    anchors.margins: 14
                    spacing: 6
                    Text { text: "排障入口"; color: Theme.text; font.family: Theme.fontFamily; font.pixelSize: 15; font.weight: Font.DemiBold }
                    Text {
                        Layout.fillWidth: true
                        text: "启动时会检查 Redis(6379) 与 MySQL(3306) 端口。日志在程序目录的 logs/app.log。若模型预览空白，先确认数据库 FilePath 与 XML 是否存在；程序会自动按数据库重建缺失 XML。"
                        color: Theme.textMuted
                        font.family: Theme.fontFamily
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                        lineHeight: 1.5
                    }
                    RowLayout {
                        spacing: 8
                        AppButton {
                            text: "回到模型库"
                            variant: "secondary"
                            compact: true
                            onClicked: app.goPage("library")
                        }
                        AppButton {
                            text: "去模型设计"
                            compact: true
                            onClicked: app.goPage("designer")
                        }
                    }
                }
            }
        }
    }
}
