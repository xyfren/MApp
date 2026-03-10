import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import mclient
import QtCore
import QtMultimedia

Page {
    id: page

    property string pageTitle: "Main"

    signal nextPage(string url)

    Component.onCompleted: {
        console.log("Страница загружена")
        MClient.startFindServer();
    }

    ColumnLayout{
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10
        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            Layout.alignment: Qt.AlignCenter
            MButton {
                id: btn_disconnect
                text: "Отключиться"
                Layout.preferredWidth: 100
                Layout.preferredHeight: 40
                Layout.alignment: Qt.AlignCenter

                onClicked: {
                    MClient.disconnectFromServer();
                }
            }
        }
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            TextField {
                id: textInput
                Layout.fillWidth: true
                Layout.preferredHeight: 40
                font.pixelSize: 14

                onAccepted: btn.clicked()
            }

            MButton {
                id: btn_addText
                text: "Добавить"
                Layout.preferredWidth: 100
                Layout.preferredHeight: 40

                onClicked: {
                    var text = textInput.text.trim();
                    logArea.append(text)
                    MClient.sendMessage(text)
                }
            }

        }

        ServerList{
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: 200
            id:serverList

            onServerClicked: function(serverAddress,connectionPort,dataPort){
                console.log("Подключаемся к:", serverAddress, connectionPort)

                MClient.setup(serverAddress,connectionPort,dataPort);
                MClient.connectToServer();
            }
        }

        LogArea {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: 200
            id:logArea
        }

    }

    Connections {
        target:MClient
        function onAddLog(text){
            logArea.append(text)
        }
        function onErrorOccurred(text){
            logArea.append("Error: " + text)
        }
        function onServerFound(serverAddress,connectionPort,dataPort){
            serverList.append(serverAddress,connectionPort,dataPort)
        }

    }

}
