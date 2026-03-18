import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import mapp

Rectangle {
    id: root
    border.color: "gray"
    border.width: 1
    radius: 3
    color:"black"

    MPlayer {
        id: player
        videoOutput: videoOutput
    }

    VideoOutput {
        id: videoOutput
        anchors.fill: parent
        fillMode: VideoOutput.PreserveAspectCrop
    }
}
