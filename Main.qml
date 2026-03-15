import QtQuick
import QtQuick.Controls.Basic 2.15
import QtQuick.Controls.Material
import QtQuick.Layouts
import mclient
import androidtools

ApplicationWindow {
    id: window
    width: 360
    height: 640
    visible: true
    title: "Screen"
    visibility: Window.Maximized

    // flags: Qt.FramelessWindowHint | Qt.Window

    StackView {
        id: stackView
        anchors.fill: parent
        // Устанавливаем начальную страницу
        initialItem: "MainPage.qml"
    }
    Connections {
        target: stackView.currentItem
        ignoreUnknownSignals: true
        function onExitRequested() {
            console.log("Monitor запросила выход. Выполняем код...")
            stackView.pop()
            AndroidTools.setFullScreen(false)
            window.visibility = Window.Maximized
            MClient.disconnectFromServer();
        }
    }
    Connections {
        target: MClient
        function onAuthorized(){
            window.visibility = Window.FullScreen
            // AndroidTools.setFullScreen(true)
            stackView.push("MonitorPage.qml")
            AndroidTools.setFullScreen(true)
        }
        function onDisconnected(){
            if (stackView.currentItem.pageTitle === "Monitor"){
                stackView.pop();
                AndroidTools.setFullScreen(false)
                window.visibility = Window.Maximized
                MClient.disconnectFromServer();
            }
        }
    }
}
