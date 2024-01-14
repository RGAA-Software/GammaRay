import QtQuick 2.12
import QtQuick.Controls 2.12
//import QtQuick.Controls.Universal 2.12
import QtQuick.Controls.Material 2.15

Window {
    width: 1280
    height: 768
    visible: true
    minimumWidth: 1280
    minimumHeight: 768
    title: qsTr("Hello World")

// 设置 Material Design 主题
    Material.theme: Material.Light // 选择 Light 或 Dark 主题
    Material.accent: Material.Orange // 设置强调色（可以选择预定义的颜色）
    Material.primary: Material.DeepOrange // 设置主色（可以选择预定义的颜色）


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
            }
        }

    }

}
