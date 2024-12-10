import QtQuick 2.15
import QtQuick.Controls 2.15
import "qrc:/resources/qmls/"

/// <summary>
/// ��Ҫ���ܣ����и�����ʷ��¼��tab��ȫ����������ź��뽫�������ݴ��ݵ������ؼ�ʹ��
/// </summary>
/// <property name="textCommitted">�ı������źţ����Ա������ؼ�����</param>
/// <property name="text">��¶���ͻ���ȡ���ı�</param>
/// <property name="searchHistoryModel">��ʷ��¼����</param>
/// 
/// <signal name="textCommitted">
/// <param name="text">�������ַ���</param>
/// </signal>
///
/// <function name="autoComplete">
/// <summary>��tab����ȫ��js����</summary>
/// </function>
///
/// <function name="addSearchHistory">
/// <summary>��ť�����İ�������ı�������ʷ��¼</summary>
/// <param name="text">��ǰ������ı�</param>
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
