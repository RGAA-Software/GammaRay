import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 2.15
import QtQuick.Controls.Material 2.15

RowLayout {

    property int btnWidth: 150
    property int btnHeight: 38
    property int btnRadius: btnHeight/2
    property int windowHeight: 220

    width: parent.width
    height: parent.height

    Rectangle {width: 20; height:1}

    ColumnLayout {
        spacing: 0
        Rectangle{width: 1; height: 10;}

        Item {
            width: 150
            height: 150
            Image {
                width: 100
                height: 100
                fillMode: Image.Stretch
                source: "qrc:/resources/tc_icon.png"
                anchors.centerIn: parent
            }
        }

        TCButton {
            id: nav_steam_game
            tag: "nav_steam_game"
            text: qsTr("Steam游戏")
            width: btnWidth
            height: btnHeight
            radius: btnRadius
            selected: true
            onClicked: tag => {
                console.log("steam...", tag)
                content_layout.currentIndex = 0

                nav_added_game.selected = false
                nav_settings.selected = false
            }
        }
        Rectangle {width: 1; height: 3}

        TCButton {
            id: nav_added_game
            tag: "nav_added_game"
            text: qsTr("添加的游戏")
            width: btnWidth
            height: btnHeight
            radius: btnRadius
            onClicked: tag => {
                console.log("added...", tag)
                content_layout.currentIndex = 1

               nav_steam_game.selected = false
               nav_settings.selected = false
            }
        }
        Rectangle {width: 1; height: 3}

        TCButton {
            id: nav_settings
            tag: "nav_settings"
            text: qsTr("软件设置")
            width: btnWidth
            height: btnHeight
            radius: btnRadius
            onClicked: tag => {
                console.log("settings...", tag)
                content_layout.currentIndex = 2

                nav_steam_game.selected = false
                nav_added_game.selected = false
                nav_steam_game.update()
                nav_added_game.update()
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.minimumHeight: 1 // 设定一个最小高度
        }

        Rectangle {
            width: btnWidth
            height: btnHeight
            //color: "#999888"
            anchors.bottom: parent.bottom
            Text {
                id: name
                width: btnWidth
                height: btnHeight
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                text: qsTr("V:1.0.0")
                color: "#707070"
            }
        }
    }

    Rectangle {width: 20; height:1}

    StackLayout {
        id: content_layout
        //anchors.fill: parent
        width: parent.width - 190
        height: parent.height
        currentIndex: 0
        Rectangle {
            color: 'teal'
            width: content_layout.width
            height: content_layout.height
        }
        Rectangle {
            color: 'plum'
            width: content_layout.width
            height: content_layout.height
        }
        Rectangle {
            color: '#909876'
            width: content_layout.width
            height: content_layout.height
        }
    }

}
