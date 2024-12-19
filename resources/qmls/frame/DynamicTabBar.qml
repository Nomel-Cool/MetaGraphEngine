import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQml.XmlListModel
import QtQuick.Layouts

/// <summary>
/// 主要功能：利用xml文件格式控制页面切换按钮数量与显示信息
/// </summary>
/// <property name="xmlSource">由于使用动态生成技术，需要提供XML文件在qrc文件中的路径：qrc:path/to/xml_file.xml</param>
/// <property name="xmlRoute">XML中读取按钮信息的节点路径，一般使用/tabs/tab固定格式</param>
/// <property name="roleParams">与xmlRoute匹配的参数，即tab标签下有多少个attribute，如果tab标签下不止一类子标签，则可以指定elementName，具体参考XmlListModel控件信息</param>
/// <property name="roleParams">由用户指定的XmlListModelRole的[name, elementName, attributeName]参数的多维数组，因为可能不只一个role，即配合tab标签下不只一类子标签的情况使用</param>
/// <property name="pageIndex">提供给主界面绑定子界面切换显示的索引</param>
/// <property name="xmlListModelObject">私有的用于保存动态生成的XmlListModel对象，传递给内部其他派生控件的model属性使用</param>
/// <property name="spacing">用于指定Tab按钮间隔像素</param>
/// <note>暂时不清楚是什么原因，Qt5是支持Button使用background属性的，但Qt6就没有了，但依然能用且正常触发信号，所以无吊所谓了</note>
Rectangle {
    id: dynamicTabBar
    required property string xmlSource
    required property string xmlRoute
    required property var roleParams
    property int pageIndex: 0
    property alias spacing: tabBar.spacing

    QtObject {
        id: __private
        property var xmlListModelObject
    }

    Component.onCompleted: {
        var xmlListModelString = "import QtQml.XmlListModel;XmlListModel{id:tabModel; source:\"" + xmlSource + "\"; query:\"" + xmlRoute + "\"";
        for (var i = 0; i < roleParams.length; i++) {
            var role = roleParams[i];
            xmlListModelString += "; XmlListModelRole { name:\"" + roleParams[i][0] + "\"; elementName:\"" + roleParams[i][1] + "\"; attributeName:\"" + roleParams[i][2] + "\"}";
        }
        xmlListModelString += "}";
        
        var xmlListModelComponent = Qt.createComponent("data:text/plain," + encodeURIComponent(xmlListModelString)); 
        if (xmlListModelComponent.status === Component.Ready) { 
            __private.xmlListModelObject = xmlListModelComponent.createObject(dynamicTabBar); 
            if (__private.xmlListModelObject === null) { 
                console.error("Error creating XmlListModel object"); 
            }
        } 
        else { 
            console.error("Error loading XmlListModel component");
        }
    }

    Row {
        id: tabBar
        spacing: 1
        width: parent.width
        height: parent.height
        Repeater {
            model: __private.xmlListModelObject
            delegate: Button {
                width: (dynamicTabBar.width - (__private.xmlListModelObject.count === 0 ? 0 : (__private.xmlListModelObject.count - 1)) * tabBar.spacing) / __private.xmlListModelObject.count
                height: dynamicTabBar.height
                text: model.name
                onClicked: {
                    dynamicTabBar.pageIndex = index
                }
            }
        }
    }
}
