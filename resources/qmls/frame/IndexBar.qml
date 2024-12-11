import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import CustomModules 1.0
/// <summary>
/// ��26��ComboBox�洢A~Z��ͷ�ĵ��ʣ���֧�ֹؼ��������������ʾ���ö�
/// </summary>
/// <property name="keyWord">�û���ָ���������ؼ���</param>
///
/// <signal name="itemSelected">
/// <param name="selectedItem">ѡ�еĹؼ���</param>
/// </signal>
///
/// <function name="pin2TopInListView">
/// <summary>���ؼ��ʹ�����ComboBox�ö�</summary>
/// <param name="list_view">ComboBox���ڵ�ListView��id</param>
/// <param name="indice">ListView��ComboBox������</param>
/// </function>
///
/// <function name="openComboBoxInListView">
/// <summary>��ListView�б�ѡ�е�ComboBoxչ��</summary>
/// <param name="list_view">ComboBox���ڵ�ListView��id</param>
/// <param name="indice">ListView��ComboBox������</param>
/// </function>
///
/// <function name="focusItemInComboBoxByKeyword">
/// <summary>���ؼ������У������չ����ComboBox�����</summary>
/// <param name="list_view">ComboBox���ڵ�ListView��id</param>
/// <param name="indice">ListView��ComboBox������</param>
/// </function>
///
/// <function name="addWord">
/// <summary>ǰ������ģ�����Ƶ�ͼԪ���Ľӿ�</summary>
/// <param name="word">�����ӵ�ģ������</param>
/// </function>

Item {
    property string keyWord: ""
    signal previewItemSelected(string selectedItem)
    signal engineItemSelected(string selectedItem)

    QIndexBarFactory {
        id: indexBarFactoryImpl
    }

    ListModel {
        id: wordModel
    }

    Component.onCompleted: {
        for (var i = 0; i < 26; i++)
            wordModel.append({ letter: String.fromCharCode(65 + i), words: [] });
        var modelNameList = indexBarFactoryImpl.Request4ModelName();
        for (var j = 0; j < modelNameList.length; ++j) { 
            var modelName = modelNameList[j];
            var firstLetter = modelName.charAt(0).toUpperCase(); 
            var index = firstLetter.charCodeAt(0) - 65; 
            if (index >= 0 && index < 26)
                wordModel.get(index).words.append({ showText: modelName }); // That is so fucking important to use the attribute name "showText"!!!
            // �����Ҫ��̬������ͼ���ƺ���Ҫ��model�õ�����set����ǿ�Ƹ��µ���ͼ
        }
    }

    ListView {
        id: listView
        width: parent.width
        height: parent.height
        spacing: 0
        clip: true
        model: wordModel

        delegate: ComboBox {
            id: comboBox
            width: listView.width
            height: Math.max(40, implicitHeight)
            visible: keyWord.length === 0 ? true : letter === keyWord.toUpperCase()[0]
            displayText: letter
            model: words
            clip: true
            delegate: Rectangle {
                width: comboBox.width
                height: comboBox.height
                color: comboBox.currentIndex === index ? "lightBlue" : "white" // Highlight text color for the selected item
                Text {
                    anchors.fill: parent
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    text: model.showText
                }
                MouseArea { 
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton|Qt.RightButton
                    onClicked: function(mouse) { 
                        if (mouse.button === Qt.RightButton) { 
                            comboBox.currentIndex = index; 
                            previewItemSelected(currentText);
                        } 
                        else if (mouse.button === Qt.LeftButton) { 
                            engineItemSelected(currentText);
                        } 
                    }
                }
            }
        }
    }

    
    onKeyWordChanged: {
        console.log("KeyWord changed to:", keyWord);

        if (keyWord.length === 0) {
            console.log("Empty keyWord, no visibility changes.");
            return;
        }

        var firstLetter = keyWord[0].toUpperCase();
        if (firstLetter.charCodeAt(0) < 65 || firstLetter.charCodeAt(0) > 90) {
            console.log("Invalid keyWord, no visibility changes.");
            return;
        }

        for (var i = 0; i < wordModel.count; i++) {
            if (wordModel.get(i).letter === firstLetter) {
                pin2TopInListView(listView, i);
                openComboBoxInListView(listView, i);
                focusItemInComboBoxByKeyword(listView, i);
                break;
            }
        }
    }
    function pin2TopInListView(list_view, indice) {
        list_view.currentIndex = indice;
        list_view.positionViewAtIndex(indice, ListView.Beginning);
    }
    function openComboBoxInListView(list_view, indice) {
        listView.itemAtIndex(indice).popup.open()
    }
    function focusItemInComboBoxByKeyword(list_view, indice) {
        for (var j = 0; j < list_view.itemAtIndex(indice).count; j++) {
            var searchText = list_view.itemAtIndex(indice).textAt(j);
            if (searchText === keyWord) {
                list_view.itemAtIndex(indice).currentIndex = j; 
                break;
            }
        }
    }
    function addWord(word) {
        if (!word || word.length === 0) {
            console.log("Empty word entered");
            return;
        }

        var firstLetter = word[0].toUpperCase();
        var index = firstLetter.charCodeAt(0) - 65;

        if (index < 0 || index > 25) {
            console.log("Invalid word entered");
            return;
        }

        var dataItem = wordModel.get(index);

        if (!dataItem.words) {
            console.error("words is undefined for letter", dataItem.letter);
            return;
        }

        for (var i = 0; i < dataItem.words.count; i++) {
            if (dataItem.words.get(i).showText === word) {
                console.log("Duplicating word entered");
                return;
            }
        }

        dataItem.words.append({ showText: word });
        console.log("Updated words for letter", dataItem.letter, ":", dataItem.words);
    }
function delItem(selectedItem) {
    if (!selectedItem || selectedItem.length === 0) {
        console.log("Empty selectedItem");
        return;
    }

    var firstLetter = selectedItem[0].toUpperCase();
    var index = firstLetter.charCodeAt(0) - 65;

    if (index < 0 || index > 25) {
        console.log("Invalid selectedItem");
        return;
    }

    var dataItem = wordModel.get(index);

    if (!dataItem.words) {
        console.error("words is undefined for letter", dataItem.letter);
        return;
    }

    for (var i = 0; i < dataItem.words.count; i++) {
        if (dataItem.words.get(i).showText === selectedItem) {
            dataItem.words.remove(i, 1);
            console.log("Deleted item:", selectedItem);
            break;
        }
    }

    // ǿ�Ƹ��²���
    listView.forceLayout();
}

}