import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 2.12
import QtQuick.Controls.Material 2.15

Rectangle {

    property string text: "Button"
    property string tag: ""
    property bool selected: false
    signal clicked(string tag)

    Text {
        id: name
        text: parent.text
        anchors.centerIn: parent
        color: selected || ma.containsMouse ? "#FFFFFF" : "#333333"
        font.pointSize: 10
        font.bold: true
    }

    color: selected || ma.containsMouse ? Material.accent : "#FFFFFF"

    MouseArea {
        id: ma
         anchors.fill: parent
         hoverEnabled: true
         onEntered: {
         }

        onExited: {
        }

        onPressed: {

        }

        onReleased: {
            selected = true
            parent.clicked(parent.tag);
        }
    }
}
