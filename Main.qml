import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 2.15
import QtQuick.Controls.Material 2.15


Window {
    width: 1280
    height: 768
    visible: true
    minimumWidth: 1280
    minimumHeight: 768
    title: qsTr("Thunder Cloud")

    Material.theme: Material.Light
    Material.accent: Material.DeepPurple
    Material.primary: Material.DeepOrange

    TCTabView {
        windowHeight: parent.height
    }

    // Test {

    // }

}
