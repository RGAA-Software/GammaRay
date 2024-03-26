import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 2.15
import QtQuick.Controls.Material 2.15

Window {
    width: 1298
    height: 768
    visible: true
    minimumWidth: 1298
    minimumHeight: 768
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
