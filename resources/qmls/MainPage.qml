import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "."
import "./designer"
import "./engine"

ApplicationWindow {
    id: root
    visible: true
    width: 640
    height: 400

    QGraphFactory{id: graphFactoryImp}

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
        anchors.top: parent.top
        anchors.left: parent.horizontalCenter
        onTextCommitted: (receivedText) => {
            indexBar.keyWord = receivedText // �����ؼ������ڴ˴�����յ����ַ���
        }
    }

    IndexBar {
        id: indexBar
        width: searchBar.width
        height: parent.height - searchBar.height
        anchors.top: searchBar.bottom
        anchors.left: searchBar.left
        onPreviewItemSelected: (selectedItem) => {
           previewBoard.currentModelName = selectedItem
           var strPointsSeries = graphFactoryImp.Request4Model(selectedItem);// ��ѯRedis��������ؼ��ʵ�ͼԪ�Զ����ļ�
           strPointsSeries.length != 0 ? (previewBoard.strPoints = strPointsSeries) : indexBar.delItem(selectedItem);      
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
        drawSpeed: 50 // ms
    }

    DrawBoard {
        id: drawBoard
        visible: dynamicTabBar.pageIndex === 0 ? true : false
        width: parent.width
        height: parent.height - dynamicTabBar.height
        anchors.top: dynamicTabBar.bottom
    }
    EngineDesigner {
        id: engineDesigner
        visible: dynamicTabBar.pageIndex === 1 ? true : false
        width: parent.width
        height: parent.height - dynamicTabBar.height
        anchors.top: dynamicTabBar.bottom
    }
}
