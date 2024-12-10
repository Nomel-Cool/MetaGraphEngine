import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import CustomModules 1.0
import "."

Rectangle {
    id: root
    visible: true
    width: parent.width
    Rectangle {
        id: drawBoard
        border.width: 2
        anchors.left: parent.left
        anchors.top: parent.top
        width: indexBar.x
        height: parent.height
        Text {
            anchors.fill: parent
            text: "Draw Board"
            verticalAlignment: Text.AlignHCenter
            horizontalAlignment: Text.AlignVCenter
        }
    }
}

	//ApplicationWindow {
    //visible: true
    //width: 400
    //height: 600
    //title: "Custom ListView with ComboBox"
    //
    //ColumnLayout {
    //    anchors.fill: parent
    //    spacing: 10
    //
    //    // Input Row for Adding Words
    //    RowLayout {
    //        spacing: 5
    //        Layout.fillWidth: true // Make RowLayout fill the entire width
    //
    //        TextField {
    //            id: wordInput
    //            placeholderText: "Enter a word"
    //            Layout.minimumHeight: 40
    //            Layout.preferredHeight: 40
    //            Layout.fillWidth: true
    //        }
    //
    //        Button {
    //            text: "Add"
    //            Layout.preferredWidth: 80 // Set appropriate width for the button
    //            onClicked: indexBar.addWord(wordInput.text)
    //        }
    //    }
    //
    //    // Input Row for Modifying keyWord
    //    RowLayout {
    //        spacing: 5
    //        Layout.fillWidth: true
    //
    //        TextField {
    //            id: keyWordInput
    //            placeholderText: "Enter keyword"
    //            Layout.minimumHeight: 40
    //            Layout.preferredHeight: 40
    //            Layout.fillWidth: true
    //        }
    //
    //        Button {
    //            text: "Set"
    //            Layout.preferredWidth: 80
    //            onClicked: {
    //                indexBar.keyWord = keyWordInput.text; // Update the keyWord property
    //            }
    //        }
    //    }
    //
    //    // IndexBar for ComboBox ListView
    //    IndexBar {
    //        id: indexBar
    //        Layout.fillWidth: true
    //        Layout.fillHeight: true
    //    }
    //}
//}
