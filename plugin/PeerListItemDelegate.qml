import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ItemDelegate {
    id: root
    
    QtObject {
        id: _d
        
        // Colors
        readonly property color textColor: "#ffffff"
        readonly property color textSecondaryColor: "#a0a0a0"
        readonly property color surfaceColor: "#2d2d2d"
        readonly property color surfaceDarkColor: "#252525"
        readonly property color hoverColor: "#3d3d3d"
        
        // Dimensions
        readonly property int primaryFontSize: 12
    }
    
    contentItem: RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        anchors.topMargin: 8
        anchors.bottomMargin: 8
        spacing: 16
        
        Text {
            text: (index + 1).toString() + "."
            font.family: "Courier"
            font.pointSize: _d.primaryFontSize
            color: _d.textSecondaryColor
            Layout.preferredWidth: 30
            Layout.alignment: Qt.AlignTop | Qt.AlignLeft
        }
        
        Text {
            text: "Peer ID:"
            font.family: "Courier"
            font.pointSize: _d.primaryFontSize
            font.bold: true
            color: _d.textSecondaryColor
            Layout.alignment: Qt.AlignTop
        }
        
        Text {
            text: modelData
            font.family: "Courier"
            font.pointSize: _d.primaryFontSize
            color: _d.textColor
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }
    }
    
    background: Rectangle {
        color: {
            if (root.hovered || root.pressed) {
                return _d.hoverColor
            }
            return root.ListView.index % 2 === 0 ? _d.surfaceColor : _d.surfaceDarkColor
        }
        radius: 2
    }
}
