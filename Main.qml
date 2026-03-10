import QtQuick
import QtQuick.Controls.Basic 2.15
import QtQuick.Controls.Material
import QtQuick.Layouts
import mclient

ApplicationWindow {
    id: window
    width: 360
    height: 640
    visible: true
    title: "Screen"
    visibility: Window.Maximized

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
            MClient.disconnectFromServer();
            stackView.pop()
            window.visibility = Window.Maximized
        }
    }
    Connections {
        target: MClient
        function onAuthorized(){
            stackView.push("MonitorPage.qml")
            window.visibility = Window.FullScreen
        }
        function onDisconneted(){
            if (stackView.currentItem.pageTitle === "Monitor"){
                stackView.pop();
            }
        }
    }
}
