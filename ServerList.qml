import QtQuick
import QtQuick.Controls.Basic 2.15
import QtQuick.Controls.Material
import QtQml.Models
import QtQuick.Dialogs
import mapp


Rectangle {
    id:root
    border.color: "gray"
    border.width: 1
    radius: 3
    clip:true

    signal serverClicked(string serverAddress,int connectionPort,int dataPort,int connectionType)
    signal requestTimerRestart(int targetIndex)

    ListModel {
        id: serverModel
    }


    Dialog {
        id: connectionDialog
        title: "Подтверждение"

        property string serverAddress: ""
        property int connectionPort: 0
        property int dataPort: 0
        property int connectionType:0

        standardButtons: Dialog.Ok | Dialog.Cancel

        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        parent: Overlay.overlay

        Component.onCompleted: {
            standardButton(Dialog.Ok).text = "Подключиться"
            standardButton(Dialog.Cancel).text = "Отмена"
        }

        // Текст диалога
        Text {
            text: "Сервер: " + connectionDialog.serverAddress +
                  ":" + connectionDialog.connectionPort
            font.pixelSize: 16
        }

        onAccepted: {
            serverClicked(serverAddress,connectionPort,dataPort,connectionType)
        }
    }
    ListView {
        id: serverList
        anchors.fill: parent
        anchors.bottomMargin: 10

        spacing: 5
        headerPositioning: ListView.OverlayHeader
        clip: true

        header: Rectangle {
            width: serverList.width
            height:40
            color:"white"
            border.color: "gray"
            border.width:1
            topLeftRadius: 3
            topRightRadius: 3

            z:2
            Label {
                anchors.centerIn: parent
                text: "Доступные сервера"
                font.bold: true
                font.pixelSize: 16
                bottomPadding: 8

            }
        }

        model: serverModel
        delegate: serverDelegate
    }

    Component {
        id: serverDelegate

        Rectangle {            
            required property string serverAddress
            required property int connectionPort
            required property int dataPort
            required property int connectionType
            required property int index  // Добавляем index

            anchors.horizontalCenter: parent.horizontalCenter

            width: ListView.view.width - 20
            height: 48
            radius: 6
            color: "transparent"

            Timer {
                id: deleteTimer
                interval: 5000
                running: active
                repeat: true
                onTriggered: {
                    serverModel.remove(index)
                }
            }

            Connections {
                target: root
                function onRequestTimerRestart(targetIndex) {
                    if (targetIndex === index) {
                        deleteTimer.restart()
                        console.log("Restarted timer at index:", targetIndex)
                    }
                }
            }

            Rectangle {
                id:del_rect
                anchors.fill: parent
                anchors.margins: 2
                color: "white"
                border.color: "#cccccc"
                border.width: 1
                radius: 6

                Row {
                    anchors.fill: parent
                    anchors.leftMargin: 16
                    anchors.rightMargin: 16
                    spacing: 8

                    Label {
                        anchors.verticalCenter: parent.verticalCenter
                        text: connectionType == 1 ? "🔌" : connectionType == 2 ? "🌐" : "None"
                        font.pixelSize: 20
                    }

                    Column {
                        anchors.verticalCenter: parent.verticalCenter

                        Label {
                            text: serverAddress
                            font.bold: true
                        }

                        Label {
                            text: "Порт подключения: " + connectionPort + " | данных: " + dataPort
                            color: "#666666"
                            font.pixelSize: 11
                        }
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    serverList.currentIndex = index
                    if (connectionType == 1){
                        connectionDialog.serverAddress = "127.0.0.1"
                        connectionDialog.connectionPort = connectionPort
                        connectionDialog.dataPort = connectionPort // по usb данные передаются на одном сокете
                        connectionDialog.connectionType = connectionType
                        connectionDialog.open()
                    }
                    else if (connectionType == 2){
                        connectionDialog.serverAddress = serverAddress
                        connectionDialog.connectionPort = connectionPort
                        connectionDialog.dataPort = dataPort
                        connectionDialog.connectionType = connectionType
                        connectionDialog.open()
                    }
                }
                onPressed: {
                    del_rect.color = "#c2fdff"
                }
                onReleased: {
                    del_rect.color = "white"
                }
                onCanceled: {
                    // Если нажатие отменено (например, из-за скроллинга)
                    del_rect.color = "white"
                }
            }
        }
    }

    function find(serverAddress) {
        for (var i = 0; i < serverModel.count; i++) {
            if (serverModel.get(i).serverAddress === serverAddress)
                return i;
        }
        return -1;
    }

    function append(serverAddress, connectionPort, dataPort, connType) {
        var res = find(serverAddress)
        if (res < 0) {
            console.log(connType);
            serverModel.append({
                "serverAddress": serverAddress,
                "connectionPort": connectionPort,
                "dataPort": dataPort,
                "connectionType":connType
            });
        }
        else{
            requestTimerRestart(res)
        }
    }
}
