import QtQuick
import QtQuick.Window
import QtQuick.Controls.Basic 2.15
import QtQuick.Controls.Material
import QtQuick.Layouts
import mapp
import androidtools

ApplicationWindow {
    id: window
    width: 360
    height: 640
    visible: true
    title: "Screen"
    flags: Qt.FramelessWindowHint | Qt.Window | Qt.ExpandedClientAreaHint | Qt.NoTitleBarBackgroundHint
    visibility: Window.Maximized
    property bool isFullScreen: false

    onContentOrientationChanged: function (o){
        console.log(o)
        console.log("изм")

    }
    onSafeAreaMarginsChanged: function(margins){
        if (isFullScreen){
            window.leftPadding = 0;
            window.rightPadding = 0;
            window.topPadding = 0;
            window.bottomPadding = 0;
            window.visibility = Window.FullScreen

        }
        else{
            window.leftPadding = margins.left;
            window.rightPadding =  margins.right;
            window.topPadding =  margins.top;
            window.bottomPadding =  margins.bottom;
            window.visibility = Window.Maximized
        }
    }

    function setFullScreen(enabled){
        window.isFullScreen = enabled;
        if (enabled){
            window.flags =  Qt.Window | Qt.ExpandedClientAreaHint | Qt.NoTitleBarBackgroundHint
            window.leftPadding = 0;
            window.rightPadding = 0;
            window.topPadding = 0;
            window.bottomPadding = 0;
            window.visibility = Window.FullScreen

        }
        else{
            window.flags = Qt.FramelessWindowHint | Qt.Window
            window.leftPadding = SafeArea.margins.left;
            window.rightPadding =  SafeArea.margins.right;
            window.topPadding =  SafeArea.margins.top;
            window.bottomPadding =  SafeArea.margins.bottom;
            window.visibility = Window.Maximized
        }
    }

    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: "MainPage.qml"
    }
    Connections {
        target: stackView.currentItem
        ignoreUnknownSignals: true
        function onExitRequested() {
            window.setFullScreen(false)
            stackView.pop()
            MClient.disconnectFromServer();
        }
    }
    Connections {
        target: MClient
        function onAuthorized(){
            window.setFullScreen(true)
            stackView.push("MonitorPage.qml")
        }
        function onDisconnected(){
            if (stackView.currentItem.pageTitle === "Monitor"){
                window.setFullScreen(false)
                stackView.pop();
                MClient.disconnectFromServer();
            }
        }
    }


}
