import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 2.12
//import QtQuick.Controls.Universal 2.12
import QtQuick.Controls.Material 2.15

Item {
    Column {

        Rectangle {
            width: 100;
            height: 100;
            color: "#090989";
        }

        MouseArea {

        }

        Row {
            Slider {
                id: slider
                width: 200
                value: 0
            }

            Label {

                text: Math.floor(slider.value)
            }
        }

        RangeSlider {
            from: 1
            to: 100
            first.value: 25
            second.value: 75
        }

        CheckBox {

        }

        Row {
            RadioButton {
                text: "R1"
            }
            RadioButton {
                text: "R2"
            }
        }

        Button {
            id: control
            width: 100
            height: 45
            text: "Good"
            //flat: true
            //highlighted: true

            background: Rectangle {
                   implicitWidth: 100
                   implicitHeight: 40
                   opacity: enabled ? 1 : 0.3
                   border.color: control.down ? "#17001a" : "#21be2b"
                   border.width: 1
                   color: control.down ? "#17001a" : "#21be2b"
                   radius: 10
               }
        }

        Button {
            id: test
            width: 100
            height: 45
            text: qsTr("确定")
            background: Rectangle {
                       id: bg
                       color: Material.accent
                       radius: height / 2 // 圆角设置为高度的一半
                   }
            MouseArea {
                id: ma
                 anchors.fill: parent
                 hoverEnabled: true
                 onEntered: {
                    test.background.color = Material.primary
                     console.log(".enter...");
                 }

                 onExited: {
                    test.background.color = Material.accent
                     console.log(".leave...");
                 }

                 onPressed: {
                    test.background.color = Material.primaryTextColor
                 }

                 onReleased: {
                    test.background.color = Material.accent
                 }
            }
        }

        Button {
            text: qsTr("取消")
        }
        Switch {
            checked: true
            scale: 0.8
            onCheckedChanged: {
                console.log("Switch is " + (checked ? "on" : "off"));
                if (checked) {
                    layout.currentIndex = 0;
                }
                else {
                    layout.currentIndex = 1;
                }
            }
        }

        Image {
            id: name
            source: "https://ydlunacommon-cdn.nosdn.127.net/1ed7a7858eabd4d407370a83d9209838.png"
        }

        StackLayout {
            id: layout
            // anchors.fill: parent
            currentIndex: 0
            Rectangle {
                color: 'teal'
                implicitWidth: 200
                implicitHeight: 200
            }
            Rectangle {
                color: 'plum'
                implicitWidth: 300
                implicitHeight: 200
            }
        }

        TCButton {
            width: 200
            height: 40
            text: "取消"
        }

    }
}
