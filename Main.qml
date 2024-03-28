import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 2.15
import QtQuick.Controls.Material 2.15

Window {
    width: 1450
    height: 800
    visible: true
    minimumWidth: 1450
    minimumHeight: 800
    title: qsTr("GammaRay Server")

    Material.theme: Material.Light
    Material.accent: "#9adfff"
    Material.primary: "#69f0ff"

    TCTabView {
        windowHeight: parent.height
    }



    // Test {

    // }

}
