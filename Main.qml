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
    flags: Qt.Window | Qt.ExpandedClientAreaHint | Qt.NoTitleBarBackgroundHint
    visibility: Window.Maximized
    property bool isFullScreen: false

    Component.onCompleted: {
        AndroidTools.setOrientation(1);
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
            window.flags = Qt.Window
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
        function onExitRequested(page) {
            if (page === "Monitor"){
                window.setFullScreen(false)
                AndroidTools.setOrientation(1);
                AndroidTools.setKeepScreenOn(false);
                stackView.pop()
                MClient.disconnectFromServer();
            }
            else if(page === "Settings"){
                stackView.pop()
            }
            else {
                stackView.pop()
            }
        }
        function onNextPage(url){
            stackView.push(url)
        }
    }
    Connections {
        target: MClient
        function onAuthorized(){
            window.setFullScreen(true)
            AndroidTools.setOrientation(-1);
            AndroidTools.setKeepScreenOn(true);
            stackView.push("MonitorPage.qml")
        }
        function onDisconnected(){
            if (stackView.currentItem.pageTitle === "Monitor"){
                window.setFullScreen(false)
                AndroidTools.setOrientation(1);
                AndroidTools.setKeepScreenOn(false);
                stackView.pop();
                MClient.disconnectFromServer();
            }
        }
    }
}
