import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import mplayer
import mclient

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
