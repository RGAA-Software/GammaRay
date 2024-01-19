import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 2.15
import QtQuick.Controls.Material 2.15
import Qt5Compat.GraphicalEffects

Rectangle {

    property int gridCellWidth: 250
    property int numberOfColumns: parent.width/gridCellWidth


    ListModel {
       id:model
       ListElement{name: "张三";number: "555 3264"}
       ListElement{name: "李四";number: "555 8426"}
       ListElement{name: "张三";number: "555 3264"}
       ListElement{name: "李四";number: "555 8426"}
       ListElement{name: "张三";number: "555 3264"}
       ListElement{name: "李四";number: "555 8426"}
       ListElement{name: "李四";number: "555 8426"}
       ListElement{name: "张三";number: "555 3264"}
       ListElement{name: "李四";number: "555 8426"}
    }

    GridView {
        id: gridView
        anchors.fill: parent
        model: game_model
        cellWidth: gridCellWidth
        cellHeight: 165

        delegate: Rectangle {
            id: item
            width: gridCellWidth
            height: gridView.cellHeight

            RowLayout {
                width: 230
                height: 150
                spacing: 0
                anchors.centerIn: parent

                Rectangle {
                    width: parent.width
                    height: 150
                    color: ma.containsMouse ? "#EEEEEE" : "#FFFFFF"
                    radius: 10

                    ColumnLayout {
                        spacing: 0
                        anchors.centerIn: parent
                        Item {
                            width: 50
                            height: 50
                            Image {
                                source: "qrc:/resources/tc_icon.png";
                                width: parent.width;
                                height: parent.height;
                                fillMode: Image.Stretch
                            }
                        }
                        Text {
                            text: NameRole;

                        }
                    }

                    layer.enabled: true
                    layer.effect: DropShadow {
                        color: "#CCCCCC"
                        transparentBorder: true
                        horizontalOffset: 0
                        verticalOffset: 0
                        radius: 4
                    }
                }
            }

            MouseArea {
                id: ma
                anchors.fill: parent
                hoverEnabled: true
            }

        }
        //highlight: Rectangle { color: "lightsteelblue"; radius: 5 }
        focus: true

        Component.onCompleted: {
            // gridCellWidth = parent.width/numberOfColumns
            // console.log("width1: ", gridCellWidth);
        }

        onWidthChanged: {
            // gridCellWidth = parent.width/numberOfColumns
            // console.log("width: ", gridCellWidth);

        }

    }

}
