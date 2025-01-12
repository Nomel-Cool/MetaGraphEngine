import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root

    signal setting(int w, int h, var cameraX, var cameraY, var cameraZ, var viewType, var islock);

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
                placeholderText: "800"
                inputMethodHints: Qt.ImhDigitsOnly
                validator: IntValidator { bottom: 0 }
                Component.onCompleted: {
                    if (text === "")
                        text = placeholderText
                }
            }
            Label {
                text: qsTr("高:")
                Layout.alignment: Qt.AlignLeft
            }
            TextField {
                id: heightField
                Layout.fillWidth: true
                placeholderText: "600"
                inputMethodHints: Qt.ImhDigitsOnly
                validator: IntValidator { bottom: 0 }
                Component.onCompleted: {
                    if (text === "")
                        text = placeholderText
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Label {
                text: qsTr("相机位置:")
                Layout.alignment: Qt.AlignLeft
            }
            TextField {
                id: xCord
                Layout.fillWidth: true
                placeholderText: "0.0"
                inputMethodHints: Qt.ImhDigitsOnly
                validator: IntValidator { bottom: 0 }
                Component.onCompleted: {
                    if (text === "")
                        text = placeholderText
                }
            }
            TextField {
                id: yCord
                Layout.fillWidth: true
                placeholderText: "0.0"
                inputMethodHints: Qt.ImhDigitsOnly
                validator: IntValidator { bottom: 0 }
                Component.onCompleted: {
                    if (text === "")
                        text = placeholderText
                }
            }
            TextField {
                id: zCord
                Layout.fillWidth: true
                placeholderText: "100.0"
                inputMethodHints: Qt.ImhDigitsOnly
                validator: IntValidator { bottom: 0 }
                Component.onCompleted: {
                    if (text === "")
                        text = placeholderText
                }
            }
        }
        
        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            ButtonGroup {
                id: perspectiveWay
            }
            RadioButton {
                id: twoD
                text: qsTr("2D")
                ButtonGroup.group: perspectiveWay
            }
            RadioButton {
                id: threeD
                text: qsTr("3D")
                checked: true
                ButtonGroup.group: perspectiveWay
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            ButtonGroup {
                id: lockType
            }
            RadioButton {
                id: lockView
                text: qsTr("lock")
                ButtonGroup.group: lockType
            }
            RadioButton {
                id: unlockView
                text: qsTr("unlock")
                checked: true
                ButtonGroup.group: lockType
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
            confirmSettingBtn.enabled = false;
            let widthValue = parseInt(widthField.text);
            let heightValue = parseInt(heightField.text);
            let cameraX = parseFloat(xCord.text);
            let cameraY = parseFloat(yCord.text);
            let cameraZ = parseFloat(zCord.text);
            let perspectiveType = twoD.checked ? true : false;
            let isLock = lockView.checked ? true : false


            if (isNaN(widthValue) || isNaN(heightValue)) {
                console.error("输入无效，请填写非负整数！");
                return;
            }

            // 发送 setting 信号
            root.setting(widthValue, heightValue, cameraX, cameraY, cameraZ, perspectiveType, isLock);
            console.log("设置宽:", widthValue, "高:", heightValue);

            // 确认后禁止修改，直到 ceize 信号发出
            widthField.readOnly = true;
            heightField.readOnly = true;
            xCord.readOnly = true;
            yCord.readOnly = true;
            zCord.readOnly = true;
            twoD.enabled = false;
            threeD.enabled = false;
            lockView.enabled = false;
            unlockView.enabled = false;
        }
    }

    function reset() {
        confirmSettingBtn.enabled = true;
        widthField.readOnly = false;
        heightField.readOnly = false;
        xCord.readOnly = false;
        yCord.readOnly = false;
        zCord.readOnly = false;
        twoD.enabled = true;
        threeD.enabled = true;
        lockView.enabled = true;
        unlockView.enabled = true;
    }
}
