import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QSpcPlay

ApplicationWindow {
    width: 640
    height: 480
    visible: true
    title: "%1 %2".arg(qsTr("Hello World from Qt")).arg(state.qt_version)

    menuBar: MenuBar {
        Menu {
            title: qsTr("&File")
            Action { text: qsTr("&New...") }
            Action { text: qsTr("&Open...") }
            Action { text: qsTr("&Save") }
            Action { text: qsTr("Save &As...") }
            MenuSeparator { }
            Action {
                text: qsTr("&Quit")
                onTriggered: close()
            }
        }
        Menu {
            title: qsTr("&Edit")
            Action { text: qsTr("Cu&t") }
            Action { text: qsTr("&Copy") }
            Action { text: qsTr("&Paste") }
        }
        Menu {
            title: qsTr("&Help")
            Action { text: qsTr("&About") }
        }
    }

    AppState {
        id: state
        onError: (message) => {
            error_dialog.contentItem.text = message;
            error_dialog.visible = true;
        }
    }

    Component.onCompleted: state.on_loaded()

    Dialog {
        id: error_dialog
        anchors.centerIn: parent
        title: "Error"
        contentItem: Text { }
        standardButtons: Dialog.Ok
        modal: true
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onClicked: {
            if (mouse.button === Qt.RightButton)
                contextMenu.popup()
        }
        onPressAndHold: {
            if (mouse.source === Qt.MouseEventNotSynthesized)
                contextMenu.popup()
        }

        Menu {
            id: contextMenu
            MenuItem { text: "Cut" }
            MenuItem { text: "Copy" }
            MenuItem { text: "Paste" }
        }
    }
}
