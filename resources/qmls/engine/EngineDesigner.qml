import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "."

Rectangle {
	Text {
		id: temporary_display
		anchors.fill: parent
		text: "Pixel Space"
		font.pixelSize: 24
		font.bold: true
		horizontalAlignment: Text.AlignHCenter
		verticalAlignment: Text.AlignVCenter
	}
}