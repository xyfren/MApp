import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import mclient
import QtCore

Page {
    signal exitRequested()

    property string pageTitle: "Monitor"

    MPlayerOutput{
        id:mplayer
        anchors.fill: parent
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.preferredHeight: 200

    }

    Shortcut {
        sequence: "Back"
        onActivated: exitRequested()
    }


}
