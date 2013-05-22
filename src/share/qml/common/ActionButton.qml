import QtQuick 1.1

Item {
    id: actionRoot

    property bool isHighlight
    property bool isStrongHighlight

    property bool isCurrent

    property string actionIconURL
    property bool actionIconScales: true

    property bool hoverScalesIcons: true
    property bool transparentStyle: false
    property real decoOpacity: transparentStyle ? 0 : 1

    property alias isHovered: actionMouseArea.containsMouse

    property string textTooltip

    property string overlayText
    property string orientation: "horizontal"
    property bool isVert: orientation == "vertical"

    signal triggered()
    signal hovered()
    signal hoverLeft()
    signal held()

    Rectangle {
        id: actionRect

        radius: Math.min(width, height) / 10
        smooth: true

        anchors.fill: parent
        anchors.margins: hoverScalesIcons ? 2 : 0
        border.width: isStrongHighlight ? 2 : 1
        border.color: colorProxy.setAlpha(colorProxy.color_ToolButton_BorderColor, decoOpacity)

        gradient: Gradient {
            GradientStop {
                position: 0
                color: colorProxy.setAlpha(colorProxy.color_ToolButton_TopColor, decoOpacity)
            }
            GradientStop {
                position: 1
                color: colorProxy.setAlpha(colorProxy.color_ToolButton_BottomColor, decoOpacity)
            }
        }

        states: [
            State {
                name: "current"
                when: actionRoot.isCurrent && !actionMouseArea.containsMouse
                PropertyChanges { target: actionRect; anchors.margins: 0 }
            },
            State {
                name: "hovered"
                when: actionMouseArea.containsMouse && !actionMouseArea.pressed
                PropertyChanges { target: actionRect; border.color: colorProxy.color_ToolButton_HoveredBorderColor; anchors.margins: 0 }
            },
            State {
                name: "pressed"
                when: actionMouseArea.containsMouse && actionMouseArea.pressed
                PropertyChanges { target: actionRect; border.color: colorProxy.color_ToolButton_PressedBorderColor }
            }
        ]

        transitions: [
            Transition {
                from: ""
                to: "hovered,current"
                reversible: true
                PropertyAnimation { properties: "border.color,anchors.margins"; duration: 200 }
            }
        ]

        Image {
            id: actionImageElem

            anchors.fill: parent

            source: actionIconScales ? (actionIconURL + '/' + width) : actionIconURL
            smooth: true
            cache: false
        }

        Common { id: buttCommon }

        MouseArea {
            id: actionMouseArea

            anchors.fill: parent
            hoverEnabled: true

            onClicked: actionRoot.triggered()
            onPressAndHold: actionRoot.held()
            onEntered: {
                actionRoot.hovered();
                if (textTooltip.length > 0)
                    buttCommon.showTextTooltip(actionMouseArea, textTooltip);
            }
            onExited: actionRoot.hoverLeft()
        }
    }

    Text {
        id: overlayTextLabel

        z: parent.z + 2
        anchors.right: parent.right
        anchors.rightMargin: parent.width / 10
        anchors.bottom: parent.bottom
        anchors.bottomMargin: parent.height / 10

        text: overlayText
        visible: overlayText.length > 0

        color: "white"
        font.pixelSize: parent.height / 3
        font.italic: true
        smooth: true

        verticalAlignment: Text.AlignBottom
        horizontalAlignment: Text.AlignRight
    }

    Rectangle {
        z: overlayTextLabel.z - 1
        anchors.fill: overlayTextLabel
        anchors.topMargin: -2
        anchors.leftMargin: -parent.width / 10
        anchors.bottomMargin: -1

        visible: overlayTextLabel.visible

        color: "#E01919"
        smooth: true
        radius: 4
        border.color: "white"
        border.width: 1
    }

    Rectangle {
        id: highlightOpenedRect

        opacity: isHighlight ? 1 : 0
        Behavior on opacity { PropertyAnimation {} }

        width: parent.height / 18
        height: parent.width / 2
        radius: 1

        rotation: isVert ? 0 : 90

        anchors.horizontalCenter: isVert ? undefined : parent.horizontalCenter
        anchors.left: isVert ? parent.left : undefined
        anchors.verticalCenter: isVert ? parent.verticalCenter : parent.bottom
        anchors.verticalCenterOffset: isVert ? 0 : (-width / 2)

        gradient: Gradient {
            GradientStop {
                position: 0
                color: colorProxy.setAlpha(colorProxy.color_ToolButton_SelectedBorderColor, 0)
            }
            GradientStop {
                position: 0.5
                color: colorProxy.color_ToolButton_SelectedBorderColor
            }
            GradientStop {
                position: 1
                color: colorProxy.setAlpha(colorProxy.color_ToolButton_SelectedBorderColor, 0)
            }
        }
    }

    Rectangle {
        id: highlightCurrentRect

        opacity: isStrongHighlight ? 1 : 0
        Behavior on opacity { PropertyAnimation {} }

        width: parent.height / 18
        height: parent.width / 2
        radius: 1

        rotation: isVert ? 0 : 90

        anchors.horizontalCenter: isVert ? undefined : parent.horizontalCenter
        anchors.right: isVert ? parent.right : undefined
        anchors.verticalCenter: isVert ? parent.verticalCenter : parent.top
        anchors.verticalCenterOffset: isVert ? 0 : (width / 2)

        gradient: Gradient {
            GradientStop {
                position: 0
                color: colorProxy.setAlpha(colorProxy.color_ToolButton_SelectedBorderColor, 0)
            }
            GradientStop {
                position: 0.5
                color: colorProxy.color_ToolButton_SelectedBorderColor
            }
            GradientStop {
                position: 1
                color: colorProxy.setAlpha(colorProxy.color_ToolButton_SelectedBorderColor, 0)
            }
        }
    }
}
