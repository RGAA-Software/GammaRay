import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 2.15
import QtQuick.Controls.Material 2.15

Rectangle {

    property int gridCellWidth: parent.width/numberOfColumns
    property int numberOfColumns: 5


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
        model: model
        cellWidth: gridCellWidth
        cellHeight: 165

        delegate: Rectangle {
            id: item
            width: gridCellWidth
            height: gridView.cellHeight

            //color: "#eeaaee"

            RowLayout {
                width: 200
                height: 150
                spacing: 0
                anchors.centerIn: parent
                // Rectangle{
                //     width: (gridCellWidth - 150)/2
                //     height: 5
                //     color: "#ff55ee"
                // }
                Rectangle {
                    width: parent.width
                    height: 150
                    color: ma.containsMouse ? "#cccccc" : "#dddddd"
                    radius: 10

                    ColumnLayout {
                        spacing: 0

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
                            text: name;

                        }
                    }
                }
                // Rectangle{
                //     width: (gridCellWidth - 150)/2
                //     height: 5
                //     color: "#0055ee"
                // }
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
            gridCellWidth = parent.width/numberOfColumns
            console.log("width1: ", gridCellWidth);
        }

        onWidthChanged: {
            gridCellWidth = parent.width/numberOfColumns
            console.log("width: ", gridCellWidth);

        }

    }

}
