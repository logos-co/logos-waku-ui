import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import WakuBackend 1.0

Rectangle {
    id: root
    color: _d.backgroundColor

    QtObject {
        id: _d
        
        // Colors
        readonly property color backgroundColor: "#1e1e1e"
        readonly property color surfaceColor: "#2d2d2d"
        readonly property color surfaceDarkColor: "#252525"
        readonly property color borderColor: "#5d5d5d"
        readonly property color textColor: "#ffffff"
        readonly property color textSecondaryColor: "#a0a0a0"
        readonly property color textDisabledColor: "#808080"
        readonly property color buttonColor: "#4d4d4d"
        readonly property color hoverColor: "#3d3d3d"
        
        // Dimensions
        readonly property int primaryFontSize: 12
        
        function getStatusString(status) {
            switch(status) {
                case WakuBackend.NotStarted: return "Not started";
                case WakuBackend.Starting: return "Starting Waku...";
                case WakuBackend.Running: return "Waku started";
                case WakuBackend.Stopping: return "Stopping Waku...";
                case WakuBackend.Stopped: return "Waku stopped";
                case WakuBackend.Error: return "Error";
                default: return "Unknown";
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 10

        // Button row
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Button {
                text: "Start Waku"
                enabled: backend.status === WakuBackend.NotStarted || backend.status === WakuBackend.Stopped || backend.status === WakuBackend.Error
                font.family: "Courier"
                font.pointSize: _d.primaryFontSize
                Layout.preferredWidth: 140
                onClicked: backend.startWaku()
                
                contentItem: Text {
                    text: parent.text
                    font: parent.font
                    color: parent.enabled ? _d.textColor : _d.textDisabledColor
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                
                background: Rectangle {
                    implicitWidth: 140
                    implicitHeight: 36
                    color: parent.enabled ? (parent.pressed ? _d.hoverColor : _d.buttonColor) : _d.surfaceColor
                    radius: 4
                    border.color: parent.enabled ? _d.borderColor : _d.hoverColor
                    border.width: 1
                }
            }

            Button {
                text: "Stop Waku"
                enabled: backend.status === WakuBackend.Running
                font.family: "Courier"
                font.pointSize: _d.primaryFontSize
                Layout.preferredWidth: 140
                onClicked: backend.stopWaku()
                
                contentItem: Text {
                    text: parent.text
                    font: parent.font
                    color: parent.enabled ? _d.textColor : _d.textDisabledColor
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                
                background: Rectangle {
                    implicitWidth: 140
                    implicitHeight: 36
                    color: parent.enabled ? (parent.pressed ? _d.hoverColor : _d.buttonColor) : _d.surfaceColor
                    radius: 4
                    border.color: parent.enabled ? _d.borderColor : _d.hoverColor
                    border.width: 1
                }
            }

            Button {
                text: "Refresh Peers"
                enabled: backend.status === WakuBackend.Running
                font.family: "Courier"
                font.pointSize: _d.primaryFontSize
                Layout.preferredWidth: 140
                onClicked: backend.refreshPeers()
                
                contentItem: Text {
                    text: parent.text
                    font: parent.font
                    color: parent.enabled ? _d.textColor : _d.textDisabledColor
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                
                background: Rectangle {
                    implicitWidth: 140
                    implicitHeight: 36
                    color: parent.enabled ? (parent.pressed ? _d.hoverColor : _d.buttonColor) : _d.surfaceColor
                    radius: 4
                    border.color: parent.enabled ? _d.borderColor : _d.hoverColor
                    border.width: 1
                }
            }

            Button {
                text: "Refresh Metrics"
                enabled: backend.status === WakuBackend.Running
                font.family: "Courier"
                font.pointSize: _d.primaryFontSize
                Layout.preferredWidth: 140
                onClicked: backend.refreshMetrics()
                
                contentItem: Text {
                    text: parent.text
                    font: parent.font
                    color: parent.enabled ? _d.textColor : _d.textDisabledColor
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                
                background: Rectangle {
                    implicitWidth: 140
                    implicitHeight: 36
                    color: parent.enabled ? (parent.pressed ? _d.hoverColor : _d.buttonColor) : _d.surfaceColor
                    radius: 4
                    border.color: parent.enabled ? _d.borderColor : _d.hoverColor
                    border.width: 1
                }
            }

            Item {
                Layout.fillWidth: true
            }
        }

                // Status label
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 36
            border.color: _d.borderColor
            border.width: 1
            color: _d.surfaceColor
            radius: 4

            Text {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: 5
                text: "Status: " + _d.getStatusString(backend.status)
                font.family: "Courier"
                font.pointSize: _d.primaryFontSize
                color: _d.textColor
            }
        }

        // Connected Peers section
        Text {
            text: "Connected Peers: " + backend.peersList.length
            font.family: "Courier"
            font.pointSize: _d.fontSize
            color: _d.textColor
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.preferredHeight: 150
            Layout.maximumHeight: 150
            clip: true
            
            background: Rectangle {
                color: _d.surfaceColor
                radius: 4
                border.color: _d.borderColor
                border.width: 1
            }

            ListView {
                id: peersListView
                anchors.fill: parent
                anchors.margins: 8
                model: backend.peersList
                spacing: 4
                delegate: PeerListItemDelegate {
                    width: peersListView.width
                }
            }
        }

        // Metrics section
        Text {
            text: "Metrics:"
            font.family: "Courier"
            font.pointSize: _d.fontSize
            color: _d.textColor
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            
            background: Rectangle {
                color: _d.surfaceColor
                radius: 4
                border.color: _d.borderColor
                border.width: 1
            }

            TextArea {
                id: metricsTextArea
                anchors.fill: parent
                anchors.margins: 8
                readOnly: true
                text: backend.metrics
                font.family: "Courier"
                font.pointSize: _d.primaryFontSize
                wrapMode: TextArea.Wrap
                selectByMouse: true
                color: _d.textColor
                padding: 8
                background: Rectangle {
                    color: "transparent"
                }
            }
        }
    }
}
