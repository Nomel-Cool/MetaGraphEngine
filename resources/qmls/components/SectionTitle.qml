import QtQuick
import "../Theme.js" as Theme

Text {
    property string subtitle: ""
    text: ""
    color: Theme.text
    font.family: Theme.fontFamily
    font.pixelSize: 16
    font.weight: Font.DemiBold
}
