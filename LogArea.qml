import QtQuick
import QtQuick.Controls.Basic 2.15
import QtQuick.Controls.Material

ScrollView {
    focusPolicy: Qt.NoFocus
    ScrollBar.vertical.policy: ScrollBar.AlwaysOff
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    ScrollBar.horizontal.interactive: false
    ScrollBar.vertical.interactive: false
    anchors.margins: 0
    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0
    Component.onCompleted: {
        contentItem.flickableDirection = Flickable.VerticalFlick
        contentItem.boundsBehavior = Flickable.DragOverBounds
    }
    background: Rectangle { border.color: "gray"; border.width: 1; radius: 3 }
    clip:true

    TextEdit {
        id: logArea

        width: parent.width
        readOnly: true
        wrapMode: TextEdit.WordWrap
        font.pixelSize: 14

        selectByMouse: true
        focus: false

        padding: 10
    }
    // Функция добавления текста с автопрокруткой
    function append(text) {
        if (text !== "") {
            var timestamp = new Date().toLocaleTimeString();
            logArea.text += "[" + timestamp.substring(0,9) + "] " + text + "\n";
            // Автопрокрутка вниз
            logArea.cursorPosition = logArea.text.length;
        }
    }

}
