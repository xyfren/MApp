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

        RowLayout{
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: 50
            Layout.alignment: Qt.AlignCenter

            MButton{
                Layout.alignment: Qt.AlignCenter

                text: "Настройки"
                implicitWidth : 200
                implicitHeight: 50
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
