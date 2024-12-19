import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import CustomModules 1.0

Rectangle {
    signal listCleared()
    signal listCompiled()
    signal listLaunched()
    signal listPlayed()
    id: root
    width: parent.width
    height: parent.height
    border.width: 1
    ColumnLayout {
        id: buttonColumn
        width: 100
        height: parent.height
        spacing: 10
        anchors.centerIn: parent

        Button {
            id: compileButton
            width: 100
            height: 50
            text: "Compile"
            Layout.alignment: Qt.AlignHCenter
            onClicked: {
                listCompiled()
                console.log("GraphList compiled");
            }
        }

        Button {
            id: launchButton
            width: 100
            height: 50
            text: "Launch"
            Layout.alignment: Qt.AlignHCenter
            onClicked: {
                listLaunched()
                console.log("GraphList Launched");
            }
        }

        Button {
            id: playButton
            width: 100
            height: 50
            text: "Play"
            Layout.alignment: Qt.AlignHCenter
            onClicked: {
                listPlayed()
                console.log("GraphList Played");
            }
        }

        Button {
            id: clearButton
            width: 100
            height: 50
            text: "Clear"
            Layout.alignment: Qt.AlignHCenter
            onClicked: {
                listCleared();
                console.log("GraphList cleared");
            }
        }
    }
}
