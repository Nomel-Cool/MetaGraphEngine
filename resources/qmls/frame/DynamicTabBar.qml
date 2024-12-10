import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQml.XmlListModel
import QtQuick.Layouts

/// <summary>
/// ��Ҫ���ܣ�����xml�ļ���ʽ����ҳ���л���ť��������ʾ��Ϣ
/// </summary>
/// <property name="xmlSource">����ʹ�ö�̬���ɼ�������Ҫ�ṩXML�ļ���qrc�ļ��е�·����qrc:path/to/xml_file.xml</param>
/// <property name="xmlRoute">XML�ж�ȡ��ť��Ϣ�Ľڵ�·����һ��ʹ��/tabs/tab�̶���ʽ</param>
/// <property name="roleParams">��xmlRouteƥ��Ĳ�������tab��ǩ���ж��ٸ�attribute�����tab��ǩ�²�ֹһ���ӱ�ǩ�������ָ��elementName������ο�XmlListModel�ؼ���Ϣ</param>
/// <property name="roleParams">���û�ָ����XmlListModelRole��[name, elementName, attributeName]�����Ķ�ά���飬��Ϊ���ܲ�ֻһ��role�������tab��ǩ�²�ֻһ���ӱ�ǩ�����ʹ��</param>
/// <property name="pageIndex">�ṩ����������ӽ����л���ʾ������</param>
/// <property name="xmlListModelObject">˽�е����ڱ��涯̬���ɵ�XmlListModel���󣬴��ݸ��ڲ����������ؼ���model����ʹ��</param>
/// <property name="spacing">����ָ��Tab��ť�������</param>
/// <note>��ʱ�������ʲôԭ��Qt5��֧��Buttonʹ��background���Եģ���Qt6��û���ˣ�����Ȼ���������������źţ������޵���ν��</note>
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
