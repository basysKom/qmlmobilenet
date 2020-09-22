import QtQuick 2.6
import QtMultimedia 5.5
import QtQuick.Controls 1.0

import machine.learning 1.0

ApplicationWindow {
    visible: true
    width: 1024
    height: 800
    title: qsTr("Qml Detector")

    Item {
        anchors.fill: parent

        Camera {
            id: camera
        }

        VideoOutput {
            source: camera
            anchors.fill: parent
            filters: [detectionFilter]
        }

        CocoDetectionFilter {
            id: detectionFilter
        }
    }
}
