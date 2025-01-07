import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root

    border.width: 1
    color: "white"

    ListModel {
        id: leftListModel
    }

    ListModel {
        id: rightListModel
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 5

        Text {
            text: "Film Selector"
            font.pointSize: 16
            Layout.alignment: Qt.AlignHCenter
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 1
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredWidth: 140
                Layout.fillHeight: true
                border.width: 1
                ListView {
                    id: leftList
                    anchors.fill: parent
                    model: leftListModel
                    delegate: Rectangle {
                        width: parent.width
                        height: 30
                        color: leftList.currentIndex === index ? "lightblue" : "transparent"
                        border.color: "gray"
                        Text {
                            text: model.name
                            anchors.centerIn: parent
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                leftList.currentIndex = index
                                console.log("Selected Left Index: ", leftList.currentIndex)
                            }
                        }
                    }
                    clip: true
                    highlight: Rectangle {
                        color: "transparent"
                        border.width: 0
                    }
                    interactive: true
                    onCurrentIndexChanged: console.log("Left List Current Index:", currentIndex)
                }
            }

            ColumnLayout {
                spacing: 5
                Layout.alignment: Qt.AlignVCenter

                Button {
                    text: ">>"
                    onClicked: {
                        if (leftList.currentIndex !== -1) {
                            let item = leftList.model.get(leftList.currentIndex)
                            rightList.model.append(item)
                            leftList.model.remove(leftList.currentIndex)
                        }
                    }
                }

                Button {
                    text: "<<"
                    onClicked: {
                        if (rightList.currentIndex !== -1) {
                            let item = rightList.model.get(rightList.currentIndex)
                            leftList.model.append(item)
                            rightList.model.remove(rightList.currentIndex)
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredWidth: 140
                Layout.fillHeight: true
                border.width: 1
                ListView {
                    id: rightList
                    anchors.fill: parent
                    model: rightListModel
                    delegate: Rectangle {
                        width: parent.width
                        height: 30
                        color: rightList.currentIndex === index ? "lightgreen" : "transparent"
                        border.color: "gray"
                        Text {
                            text: model.name
                            anchors.centerIn: parent
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                rightList.currentIndex = index
                                console.log("Selected Right Index: ", rightList.currentIndex)
                            }
                        }
                    }
                    clip: true
                    highlight: Rectangle {
                        color: "transparent"
                        border.width: 0
                    }
                    interactive: true
                    onCurrentIndexChanged: console.log("Right List Current Index:", currentIndex)
                }
            }
        }
    }

    function addFilm(filmName) {
        console.log("adding: ", filmName)
        leftList.model.append({name: filmName})
    }

    function getFilm() {
        let films = []
        for (let i = 0; i < rightListModel.count; i++) {
            films.push(rightListModel.get(i).name)
        }
        return films
    }
}
