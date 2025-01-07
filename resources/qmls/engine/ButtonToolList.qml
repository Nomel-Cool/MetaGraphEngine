import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import CustomModules 1.0

Rectangle {
    signal filmNameDelivered(string filmName)
    signal listCleared()
    signal listCompiled()
    signal listLaunched()
    signal listPlayInGL()
    signal listStopped()

    id: root
    border.width: 1

    ColumnLayout {
        id: buttonColumn
        width: parent.width
        height: parent.height
        spacing: 10
        anchors.centerIn: parent

        Row {
            width: parent.width
            height: 50
            spacing: 0
            TextField {
                id: inputField
                width: 60
                height: 40
                placeholderText: qsTr("输入电影名")
                validator: RegularExpressionValidator { regularExpression: /^[A-Za-z_][A-Za-z0-9_]*$/ }
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignLeft
            }

            Button {
                id: confirmButton
                width: 40
                height: 40
                text: qsTr("确认")
                onClicked: {
                    confirmButton.enabled = false;
                    compileButton.enabled = true;
                    inputField.readOnly = true;
                    filmNameDelivered(inputField.text);
                    console.log("Film name confirmed:", inputField.text)
                }
            }
        }

        Button {
            id: compileButton
            width: 100
            height: 50
            enabled: false
            text: "Compile"
            Layout.alignment: Qt.AlignHCenter
            onClicked: {
                listCompiled()
                clearButton.enabled = false;
                compileButton.enabled = false;
                launchButton.enabled = true;
                console.log("GraphList compiled")
            }
        }

        Button {
            id: launchButton
            width: 100
            height: 50
            enabled: false
            text: "Launch"
            Layout.alignment: Qt.AlignHCenter
            onClicked: {
                listLaunched()
                launchButton.enabled = false;
                glPlayBtn.enabled = true;
                console.log("GraphList Launched")
            }
        }

        Button {
            id: glPlayBtn
            width: 100
            height: 50
            enabled: false
            text: "GLPlay"
            Layout.alignment: Qt.AlignHCenter
            onClicked: {
                listPlayInGL()
                glPlayBtn.enabled = false;
                ceizeButton.enabled = true;
                console.log("OpenGL: GraphList Played")
            }
        }
         
        Button {
            id: ceizeButton
            width: 100
            height: 50
            enabled: false
            text: "Ceize"
            Layout.alignment: Qt.AlignHCenter
            onClicked: {
                listStopped()
                ceizeButton.enabled = false;
                clearButton.enabled = true;
                inputField.readOnly = false;
                confirmButton.enabled = true;
                console.log("GraphList stopped")
            }
        }

        Button {
            id: clearButton
            width: 100
            height: 50
            text: "Clear"
            Layout.alignment: Qt.AlignHCenter
            onClicked: {
                listCleared()
                console.log("GraphList cleared")
            }
        }
    }

    function reset() {
        ceizeButton.enabled = false;
        clearButton.enabled = true;
        inputField.readOnly = false;
        confirmButton.enabled = true;
    }
}
