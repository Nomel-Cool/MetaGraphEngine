import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ModuleGraphFactory 1.0
import "."

Rectangle {
    id: root
    visible: true
    width: parent.width

    SearchBar {
        id: searchBar
        height: 40
        width: 80
        anchors.top: parent.top
        anchors.left: parent.horizontalCenter
        onTextCommitted: (receivedText) => {
            indexBar.keyWord = receivedText // 其他控件可以在此处理接收到的字符串
        }
    }
    QGraphFactory{
        id: graphFactoryImp
    }
    IndexBar {
        id: indexBar
        width: searchBar.width
        height: parent.height - searchBar.height
        anchors.top: searchBar.bottom
        anchors.left: searchBar.left
        onItemSelected: (selectedItem) => {
           console.log("Selected:", selectedItem); 
           previewBoard.strPoints = graphFactoryImp.Request4Model(selectedItem);// 查询Redis关于这个关键词的图元自动机文件
        }
    }
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
    PreviewBoard {
        id: previewBoard
        width: parent.width - indexBar.x - indexBar.width
        height: parent.height
        anchors.left: indexBar.right
        anchors.right: root.right
        anchors.top: root.top
        anchors.bottom: root.bottom
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
