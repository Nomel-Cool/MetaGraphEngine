import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root

    signal setting(int w, int h);

    border.width: 1
    width: 300
    height: 200

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Label {
                text: qsTr("宽:")
                Layout.alignment: Qt.AlignLeft
            }
            TextField {
                id: widthField
                Layout.fillWidth: true
                placeholderText: qsTr("请输入宽度")
                inputMethodHints: Qt.ImhDigitsOnly
                validator: IntValidator { bottom: 0 }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Label {
                text: qsTr("高:")
                Layout.alignment: Qt.AlignLeft
            }
            TextField {
                id: heightField
                Layout.fillWidth: true
                placeholderText: qsTr("请输入高度")
                inputMethodHints: Qt.ImhDigitsOnly
                validator: IntValidator { bottom: 0 }
            }
        }
    }

    Button {
        id: confirmSettingBtn
        width: 100
        height: 40
        text: qsTr("完成")
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 10

        onClicked: {
            let widthValue = parseInt(widthField.text);
            let heightValue = parseInt(heightField.text);

            if (isNaN(widthValue) || isNaN(heightValue)) {
                console.error("输入无效，请填写非负整数！");
                return;
            }

            // 发送 setting 信号
            root.setting(widthValue, heightValue);
            console.log("设置宽:", widthValue, "高:", heightValue);
        }
    }
}
