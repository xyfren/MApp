import QtQuick
import QtQuick.Controls.Basic 2.15
import QtQuick.Controls.Material
import QtQml.Models
import QtQuick.Dialogs
import mclient


Rectangle {
    border.color: "gray"
    border.width: 1
    radius: 3
    clip:true

    signal serverClicked(string serverAddress,int connectionPort,int dataPort)

    ListModel {
        id: serverModel
    }


    Dialog {
        id: connectionDialog
        title: "Подтверждение"

        property string serverAddress: ""
        property int connectionPort: 0
        property int dataPort: 0

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
            serverClicked(serverAddress,connectionPort,dataPort)
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
            required property int index  // Добавляем index
            anchors.horizontalCenter: parent.horizontalCenter
            width: ListView.view.width - 20
            height: 48
            radius: 6
            color: "transparent"


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
                        text: "🌐"
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
                    connectionDialog.serverAddress = serverAddress
                    connectionDialog.connectionPort = connectionPort
                    connectionDialog.dataPort = dataPort
                    connectionDialog.open()

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

    function contains(serverAddress) {
        for (var i = 0; i < serverModel.count; i++) {
            if (serverModel.get(i).serverAddress === serverAddress)
                return true;
        }
        return false;
    }

    function append(serverAddress, connectionPort, dataPort) {
        if (!contains(serverAddress)) {
            serverModel.append({
                "serverAddress": serverAddress,
                "connectionPort": connectionPort,
                "dataPort": dataPort
            });
        }
    }
}
