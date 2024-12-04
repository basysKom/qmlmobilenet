/**
 * SPDX-FileCopyrightText: 2024 basysKom GmbH
 * SPDX-FileContributor: Berthold Krevert <berthold.krevert@basyskom.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

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
        id: videoItem
        anchors.fill: parent

        Camera {
            id: camera
        }

        VideoOutput {
            id: videoOutput
            source: camera
            anchors.fill: parent
            filters: [detectionFilter]
        }

        CocoDetectionFilter {
            id: detectionFilter
        }

        Repeater {
            id: detectionRepeater
            model: detectionFilter.detectionModel
            Item {
                id: detectionItem
                property rect location: videoOutput.mapNormalizedRectToItem(boundingRect)

                implicitWidth: Math.max(objectLocation.width, objectDescription.width)
                implicitHeight: objectLocation.height + objectDescription.height

                Rectangle {
                    id: objectLocation
                    x: location.x
                    y: location.y
                    width: location.width
                    height: location.height
                    color: "transparent"
                    border.width: 2
                    border.color: boundingRectColor
                }

                Rectangle {
                    id: objectDescription
                    anchors.bottom: objectLocation.top
                    anchors.left: objectLocation.left
                    anchors.right: objectLocation.right
                    height: 25
                    color: objectLocation.border.color

                    Text {
                        id: description
                        anchors.fill: parent
                        anchors.leftMargin: 16
                        anchors.rightMargin: 16
                        verticalAlignment: Text.AlignVCenter
                        color: "white"
                        text: detectedObject + " (" + score.toFixed(2) + ")"
                    }
                }

            }
        }

    }
}
