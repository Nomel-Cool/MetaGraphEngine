import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../Theme.js" as Theme
import "../components"

Item {
    id: root
    property string payload: ""
    property string errorText: ""
    signal payloadEdited(string value)

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        Text {
            Layout.fillWidth: true
            text: "高级模式：直接编辑发给引擎的自动机 JSON。普通用户建议使用图形模板；只有需要自定义算法参数时才使用这里。"
            color: Theme.textMuted
            font.family: Theme.fontFamily
            font.pixelSize: 12
            wrapMode: Text.WordWrap
            lineHeight: 1.45
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: Theme.bg
            radius: Theme.radiusSmall
            border.width: 1
            border.color: errorText.length > 0 ? Theme.danger : Theme.border

            ScrollView {
                anchors.fill: parent
                anchors.margins: 10
                TextArea {
                    id: editor
                    text: root.payload
                    wrapMode: TextArea.WrapAnywhere
                    color: Theme.text
                    font.family: Theme.monoFamily
                    font.pixelSize: 12
                    selectByMouse: true
                    background: null
                    onTextChanged: {
                        if (text !== root.payload)
                            root.payloadEdited(text)
                    }
                    onActiveFocusChanged: {
                        if (!activeFocus)
                            root.validate()
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            Text {
                Layout.fillWidth: true
                text: root.errorText.length > 0 ? root.errorText : "格式正确，可以保存"
                color: root.errorText.length > 0 ? Theme.danger : Theme.success
                font.family: Theme.fontFamily
                font.pixelSize: 12
                elide: Text.ElideRight
            }
            AppButton {
                text: "格式化"
                variant: "secondary"
                compact: true
                onClicked: root.format()
            }
            AppButton {
                text: "校验"
                variant: "secondary"
                compact: true
                onClicked: root.validate()
            }
        }
    }

    function validate() {
        try {
            var value = JSON.parse(editor.text)
            if (!Array.isArray(value) || value.length === 0) {
                errorText = "需要一个非空 JSON 数组"
                return false
            }
            errorText = ""
            return true
        } catch (error) {
            errorText = "JSON 错误：" + error.message
            return false
        }
    }

    function format() {
        try {
            editor.text = JSON.stringify(JSON.parse(editor.text), null, 2)
            errorText = ""
        } catch (error) {
            errorText = "JSON 错误：" + error.message
        }
    }

    function setPayload(value) {
        if (editor.text !== value)
            editor.text = value
    }
}