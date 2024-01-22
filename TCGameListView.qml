import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 2.15
import QtQuick.Controls.Material 2.15
import Qt5Compat.GraphicalEffects

Rectangle {

    property int gridCellWidth: 270
    property int numberOfColumns: parent.width/gridCellWidth
    property real header_radio: 460.0/ 215.0


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
        model: installed_game_model
        cellWidth: gridCellWidth
        cellHeight: gridCellWidth/header_radio + 10

        delegate: Rectangle {
            id: item
            width: gridCellWidth
            height: gridView.cellHeight

            RowLayout {
                width: 250
                height: width / header_radio
                spacing: 0
                anchors.centerIn: parent

                Rectangle {
                    id: item_base_rect
                    width: parent.width
                    height: parent.height
                    color: ma.containsMouse ? "#EEEEEE" : "#FFFFFF"
                    radius: 10

                    Item {
                        width: item_base_rect.width
                        height: item_base_rect.height
                        Image {
                            id: image_source
                            source: "file:///C:/Program Files (x86)/Steam/appcache/librarycache/2184340_header.jpg";
                            width: parent.width;
                            height: parent.height;
                            // fillMode: Image.PreserveAspectCrop
                            smooth: true
                            visible: false
                            // clip: true
                        }
                        Rectangle{
                            id: mask
                            anchors.fill: parent
                            radius: 5
                            visible: false
                        }
                        OpacityMask {
                            anchors.fill: parent
                            source: image_source
                            maskSource: mask
                        }
                    }

                    ColumnLayout {
                        Text {
                            text: NameRole;
                            color: "#FFFFFF"
                        }
                        Text {
                            text: AppIdRole;
                            color: "#FFFFFF"
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
                onDoubleClicked: {
                    installed_game_model.OnItemClicked(AppIdRole);
                }
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
