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
                            //"file:///C:/Program Files (x86)/Steam/appcache/librarycache/2184340_header.jpg";
                            source: "file:///" + CoverUrlRole
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
                            radius: 7
                            visible: false
                        }
                        OpacityMask {
                            id: target_mask
                            anchors.fill: parent
                            source: image_source
                            maskSource: mask
                        }
                        Rectangle {
                            width: item_base_rect.width
                            height: item_base_rect.height
                            radius: 7
                            color: ma.containsMouse ? "#99FFFFFF" : "#00000000"
                        }
                        Rectangle {
                            id: name_background
                            width: item_base_rect.width
                            height: 30
                            radius: 7
                            y: item_base_rect.height - height
                            color: "#CC333333"
                            RowLayout {
                                width: parent.width
                                height: parent.height
                                Rectangle {
                                    width: 5
                                    height:1
                                    color: "#00000000"
                                }
                                Text {
                                    text: NameRole;
                                    color: "#FFFFFF"
                                    font.bold: true
                                }

                                Item {
                                    Layout.fillWidth: true
                                    Layout.minimumHeight: 1 // 设定一个最小高度
                                }
                            }
                        }
                        // Rectangle {
                        //     width: item_base_rect.width
                        //     height: 10
                        //     y: item_base_rect.height - name_background.height
                        //     color: "#99333333"
                        // }

                        // FastBlur {
                        //     id: blurEffect
                        //     anchors.fill: image_source
                        //     source: target_mask
                        //     radius: ma.containsMouse ? 18 : 0 // When mouse is over, set blur radius to 10; otherwise 0
                        // }
                        //
                        // states: [
                        //     State {
                        //         name: "blurred"
                        //         when: mouseArea.containsMouse
                        //         PropertyChanges { target: blurEffect; radius: 18 }
                        //     },
                        //     State {
                        //         name: "normal"
                        //         when: !mouseArea.containsMouse
                        //         PropertyChanges { target: blurEffect; radius: 0 }
                        //     }
                        // ]
                        // transitions: [
                        //     Transition {
                        //         from: "normal"
                        //         to: "blurred"
                        //         NumberAnimation { properties: "radius"; duration: 200 }
                        //     },
                        //     Transition {
                        //         from: "blurred"
                        //         to: "normal"
                        //         NumberAnimation { properties: "radius"; duration: 200 }
                        //     }
                        // ]
                    }

                    // ColumnLayout {
                    //     Text {
                    //         text: NameRole;
                    //         color: "#FFFFFF"
                    //     }
                    //     Text {
                    //         text: AppIdRole;
                    //         color: "#FFFFFF"
                    //     }
                    // }

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
