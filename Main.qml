import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 2.15
import QtQuick.Controls.Material 2.15

Window {
    width: 150
    height: 80
    visible: true
    minimumWidth: 150
    minimumHeight: 80
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
