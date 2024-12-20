import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import CustomModules 1.0
/// <summary>
/// 用26个ComboBox存储A~Z开头的单词，并支持关键词搜索后高亮显示到置顶
/// </summary>
/// <property name="keyWord">用户可指定的搜索关键词</param>
///
/// <signal name="itemSelected">
/// <param name="selectedItem">选中的关键词</param>
/// </signal>
///
/// <function name="pin2TopInListView">
/// <summary>将关键词关联的ComboBox置顶</summary>
/// <param name="list_view">ComboBox所在的ListView的id</param>
/// <param name="indice">ListView中ComboBox的索引</param>
/// </function>
///
/// <function name="openComboBoxInListView">
/// <summary>将ListView中被选中的ComboBox展开</summary>
/// <param name="list_view">ComboBox所在的ListView的id</param>
/// <param name="indice">ListView中ComboBox的索引</param>
/// </function>
///
/// <function name="focusItemInComboBoxByKeyword">
/// <summary>若关键词命中，则高亮展开的ComboBox里的项</summary>
/// <param name="list_view">ComboBox所在的ListView的id</param>
/// <param name="indice">ListView中ComboBox的索引</param>
/// </function>
///
/// <function name="addWord">
/// <summary>前端添加模型名称到图元栏的接口</summary>
/// <param name="word">所添加的模型名称</param>
/// </function>

Item {
    property string keyWord: ""
    signal previewItemSelected(string selectedItem)
    signal engineItemSelected(string selectedItem)

    ListModel {
        id: wordModel
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

        // 强制更新布局
        listView.forceLayout();
    }
    function updateModel(nameList) {
        wordModel.clear();
        for (var i = 0; i < 26; i++)
            wordModel.append({ letter: String.fromCharCode(65 + i), words: [] });
        var modelNameList = nameList;
        for (var j = 0; j < modelNameList.length; ++j) { 
            var modelName = modelNameList[j];
            var firstLetter = modelName.charAt(0).toUpperCase(); 
            var index = firstLetter.charCodeAt(0) - 65; 
            if (index >= 0 && index < 26)
                wordModel.get(index).words.append({ showText: modelName }); // That is so fucking important to use the attribute name "showText"!!!
            // 如果需要动态更新视图，似乎需要对model用到它的set方法强制更新到视图
        }
    }
}
