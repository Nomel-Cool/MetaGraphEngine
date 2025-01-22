import QtQuick 2.15
import QtQuick.Controls 2.15
import "."

Rectangle {
    id: root
    visible: true
    width: 800
    height: 600
    signal filmNameRecorded(string filmName)
    signal windowSeted(int w, int h, var cameraX, var cameraY, var cameraZ, bool viewType, bool lockType)
    signal deliveredListCompiled()
    signal deliveredListLaunched()
    signal deliveredListRealTimeRendered()
    signal deliveredListPlayInGL(var nameList)
    signal deliveredListStopped()
    signal deliveredListCleared()

    property alias deliveredModelList: graphList.modelList

    GraphList {
        id: graphList
        visible: parent.visible
        width: root.width
        height: 30
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        onListEmpty: {
            buttonToolList.listCountForbid()
        }

        onListNotEmpty: {
            buttonToolList.listCountEngage()
        }
    }
    
    ButtonToolList {
        id: buttonToolList
        visible: parent.visible
        width: 100
        height: parent.height - graphList.height
        anchors.left: parent.left
        anchors.top: graphList.bottom
        anchors.bottom: parent.bottom
        onFilmNameDelivered: (filmName) => {
            filmNameRecorded(filmName);
        }
        onListCompiled: {
            deliveredListCompiled();
        }
        onListLaunched: {
            deliveredListLaunched();
        }
        onListRealTimeRendered: {
            deliveredListRealTimeRendered();
        }
        onListPlayInGL: {
            var filmNameList = filmSelector.getFilm();
            console.log("filmNames: ", JSON.stringify(filmNameList))
            deliveredListPlayInGL(filmNameList);
        }
        onListStopped: {
            deliveredListStopped();
        }
        onListCleared: {
            graphList.cleanModel();
            deliveredListCleared();
        }
    }

    FilmSelector {
        id: filmSelector
        visible: parent.visible
        height:  (parent.height - graphList.height) / 2
        width: parent.width - buttonToolList.width
        anchors.top: graphList.bottom
        anchors.left: buttonToolList.right
        onReadyForRendering: {
            if (isReady) {
                buttonToolList.glPlayAvailable = true;
            }
        }
    }
    
    WindowSettings {
        id: windowSettings
        visible: parent.visible
        height:  (parent.height - graphList.height) / 2
        width: parent.width - buttonToolList.width
        anchors.top: filmSelector.bottom
        anchors.left: buttonToolList.right
        onSetting: (w,h,cx,cy,cz,viewType,isLock) => {
            windowSeted(w,h,cx,cy,cz,viewType,isLock);
            buttonToolList.winSetEngage();
        }
    }

    function resetBtnStatus() {
        buttonToolList.reset();
        buttonToolList.winSetForbid();
    }

    function resetWinStatus() {
        windowSettings.reset();
    }

    function addModelToGraphList(modelName) {
        graphList.addModel(modelName);
    }

    function addFilmToSelector(filmName) {
        filmSelector.addFilm(filmName);
    }
}
