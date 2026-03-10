import QtQuick
import QtQuick.Controls.Basic 2.15


Button {
        width: 200
        height: 50

        text: "Нажми меня"

        background: Rectangle {
            color: parent.pressed ? "darkred" : "red"
            radius: 10
            border.color: "black"
            border.width: 2
        }

        contentItem: Text {
                text: parent.text
                color: "white"  // ← ТЕКСТ БЕЛЫЙ
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

// onClicked: androidTools.showToast("HELLO WORLD")
}

