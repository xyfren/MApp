import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtCore
import QtMultimedia
import mapp

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

        ServerList{
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: 200
            id:serverList

            onServerClicked: function(serverAddress,connectionPort,dataPort,connType){
                console.log("Подключаемся к:", serverAddress, connectionPort,connType)
                MAppSettings.connection = connType;
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

        Row{
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: 25
            MButton{
                anchors.centerIn: parent
                text: "Настройки"
                width: parent.width / 2
                height: parent.height
                onClicked:{
                    nextPage("SettingsPage.qml")
                }
            }
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
        function onServerFound(serverAddress,connectionPort,dataPort,connType){
            serverList.append(serverAddress,connectionPort,dataPort,connType)
        }

    }

}
