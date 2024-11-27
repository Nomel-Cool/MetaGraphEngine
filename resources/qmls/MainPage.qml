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
    MetaGraphDesigner {
        id: metaGraphDesigner
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
