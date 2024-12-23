import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import CustomModules 1.0
import "."
import "./frame"
import "./designer"
import "./engine"

ApplicationWindow {
    id: root
    visible: true
    width: 800
    height: 600
    minimumWidth: 800
    minimumHeight: 600
    maximumWidth: 800
    maximumHeight: 600

    QGraphFactory{id: graphFactoryImp} /* This god damn thing has the singleton-object contained, so it can just be clarified once. */
    QGraphStudio{id: graphStudio}
    
    AutomataInventory {
        id: automataInventory
        onDeleverModelData: (modelName, data)=>{
            console.log("Deliverd data: ", modelName + " " + data);
            graphFactoryImp.Registry4Model(modelName, data);
        }
    }

    DynamicTabBar {
        id: dynamicTabBar
        width: root.width
        height: root.height / 5
        visible: true
        spacing: 5
        xmlSource: "qrc:/resources/xmls/componentsConfig/tab_pages.xml"
        xmlRoute: "/tabs/tab"
        roleParams: [['name','','name']]
        anchors.top: parent.top
        anchors.left: parent.left 
        anchors.right: parent.right
    }

    SearchBar {
        id: searchBar
        height: 40
        width: 80
        anchors.top: dynamicTabBar.bottom
        anchors.left: parent.horizontalCenter
        onTextCommitted: (receivedText) => {
            indexBar.keyWord = receivedText // 其他控件可以在此处理接收到的字符串
        }
    }

    IndexBar {
        id: indexBar
        width: searchBar.width
        height: parent.height - dynamicTabBar.height
        anchors.top: searchBar.bottom
        anchors.left: searchBar.left
        onPreviewItemSelected: (selectedItem) => {
           previewBoard.currentModelName = selectedItem
           var strPointsSeries = graphFactoryImp.Request4Model(selectedItem);// 查询Redis关于这个关键词的图元自动机文件
           strPointsSeries.length != 0 ? (previewBoard.strPoints = strPointsSeries) : indexBar.delItem(selectedItem);      
        }
        onEngineItemSelected: (selectedItem) => {
            if(dynamicTabBar.pageIndex === 1) {
                graph_list.addModel(selectedItem); // 把选中的模型名存放到GraphList中
            }
        }
        Connections {
            target: graphFactoryImp
            onUpdate4NameList: {
                var nameList = graphFactoryImp.Request4ModelNameList();
                indexBar.updateModel(nameList);
            }
        }
        Component.onCompleted: {
            var nameList = graphFactoryImp.Request4ModelNameList();
            indexBar.updateModel(nameList);
        }
    }

    PreviewBoard {
        id: previewBoard
        width: parent.width - indexBar.x - indexBar.width
        height: parent.height - dynamicTabBar.height
        anchors.left: indexBar.right
        anchors.right: root.right
        anchors.top: dynamicTabBar.bottom
        anchors.bottom: root.bottom
        drawSpeed: 50 // ms
    }

    DrawBoard {
        id: drawBoard
        visible: dynamicTabBar.pageIndex === 0 ? true : false
        height: parent.height - dynamicTabBar.height
        width: parent.width - indexBar.width - previewBoard.width
        anchors.top: dynamicTabBar.bottom
        anchors.left: parent.left
        onShowSavePage: {
            automataInventory.visible = true;
        }
    }

    EngineDesigner {
        id: engineDesigner
        visible: dynamicTabBar.pageIndex === 1 ? true : false
        height:  parent.height - dynamicTabBar.height
        width: parent.width - indexBar.width - previewBoard.width - buttonToolList.width
        anchors.top: graph_list.bottom
        anchors.left: buttonToolList.right
        Component.onCompleted: {
            graphStudio.InitHall(width, height);
            graphStudio.LayoutHall(1); // **Test** 1会是方格最多的了，后续交给控件来调节
        }
    }

    GraphList {
        id: graph_list
        visible: dynamicTabBar.pageIndex === 1 ? true : false
        width: engineDesigner.width
        height: 30
        anchors.top: dynamicTabBar.bottom
        anchors.left: buttonToolList.right
        anchors.right: parent.horizontalCenter
    }

    ButtonToolList {
        id: buttonToolList
        visible: dynamicTabBar.pageIndex === 1 ? true : false
        width: searchBar.width
        anchors.left: parent.left
        anchors.top: dynamicTabBar.bottom
        anchors.bottom: parent.bottom
        onListCleared: {
            graph_list.cleanModel();
        }
        onListCompiled: {
            graphStudio.RoleEmplacement(graph_list.modelList);
        }
        onListLaunched: {
            graphStudio.Launch();
        }
        onListPlayed: {
            var str_points_data = graphStudio.Display();
            console.log(str_points_data);
            engineDesigner.drawnPoints = JSON.parse(str_points_data);
            engineDesigner.engineCore.requestPaint();
        }
        onListStopped: {
            graphStudio.Stop();
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