import QtQuick 2.15
import QtQuick.Controls 2.15
import "qrc:/resources/qmls/"

/// <summary>
/// 主要功能：具有根据历史记录按tab补全的输入框，用信号与将输入内容传递到其它控件使用
/// </summary>
/// <property name="textCommitted">文本发送信号，可以被其它控件捕获</param>
/// <property name="text">暴露给客户读取的文本</param>
/// <property name="searchHistoryModel">历史记录容器</param>
/// 
/// <signal name="textCommitted">
/// <param name="text">输入框的字符串</param>
/// </signal>
///
/// <function name="autoComplete">
/// <summary>按tab键补全的js函数</summary>
/// </function>
///
/// <function name="addSearchHistory">
/// <summary>按钮触发的把输入框文本计入历史记录</summary>
/// <param name="text">当前输入框文本</param>
/// </function>
Item {
    id: searchBar
    signal textCommitted(string text)
    readonly property alias text: searchInput.text
    property var searchHistoryModel: ListModel { id: searchHistoryModel }

    TextField {
        id: searchInput
        anchors.fill: parent
        wrapMode: TextInput.WordWrap
        cursorVisible: true
        clip: true
        leftPadding: 2
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        validator: RegularExpressionValidator {
            regularExpression: /^[a-zA-Z0-9_-]{0,32}$/
        }
        Keys.onTabPressed: {
            autoComplete()
        }
        Keys.onReturnPressed: {
            textCommitted(searchInput.text)
            addSearchHistory(searchInput.text)
        }
    }

    function autoComplete() {
        var input = searchInput.text.toLowerCase()
        for (var i = 0; i < searchHistoryModel.count; i++) {
            var item = searchHistoryModel.get(i).text.toLowerCase()
            if (item.startsWith(input)) {
                searchInput.text = searchHistoryModel.get(i).text
                searchHistoryModel.setProperty(i, "count", searchHistoryModel.get(i).count + 1)
                break
            }
        }
    }

    function addSearchHistory(text) {
        for (var i = 0; i < searchHistoryModel.count; i++) {
            if (searchHistoryModel.get(i).text === text) {
                return
            }
        }
        if (searchHistoryModel.count >= 100) {
            var minIndex = 0
            var minQuote = searchHistoryModel.get(0).quote
            for (var i = 1; i < searchHistoryModel.count; i++) {
                if (searchHistoryModel.get(i).quote < minQuote) {
                    minQuote = searchHistoryModel.get(i).quote
                    minIndex = i
                }
            }
            searchHistoryModel.remove(minIndex)
        }
        searchHistoryModel.append({ "text": text, "quote": 0 })
    }
}
