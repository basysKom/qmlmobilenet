/**
 * SPDX-FileCopyrightText: 2024 basysKom GmbH
 * SPDX-FileContributor: Berthold Krevert <berthold.krevert@basyskom.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "cocodetectionmodel.h"

#include <QStringList>
#include <QMutexLocker>
#include <QFile>
#include <QTextStream>
#include <QDebug>

CocoDetectionModel::CocoDetectionModel(const QString &labelsFilename, QObject *parent )
    : QAbstractListModel(parent)
{
    loadLabels(labelsFilename);
    createPalette();
    // avoid deadlocks
    connect(this, &CocoDetectionModel::detectionObjectsChanged, this, [this]() {
        beginResetModel();
        endResetModel();
    }, Qt::QueuedConnection);
}


QHash<int, QByteArray> CocoDetectionModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[BoundingRect] = QByteArray("boundingRect");
    roles[DetectedObjectName] = QByteArray("detectedObject");
    roles[Score] = QByteArray("score");
    roles[BoundingRectColor] = QByteArray("boundingRectColor");
    return roles;
}

QVariant CocoDetectionModel::data(const QModelIndex &index, int role) const
{

    QMutexLocker locker(&m_detectedObjectsMutex);

    if (index.row() < 0 || index.row() >= m_detectedObjects.count()) {
        return QVariant();
    }

    const DetectedObject& detectedObject = m_detectedObjects.at(index.row());

    switch(role) {
    case BoundingRect:
        return detectedObject.boundingRect;
    case DetectedObjectName:
        return m_labels.value(detectedObject.classIndex, QString::number(detectedObject.classIndex));
    case Score:
        return detectedObject.score;
    case BoundingRectColor:
        return m_palette.at(index.row() % m_palette.length());
    default:
        return QVariant();
    }

    return QVariant();
}

int CocoDetectionModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    QMutexLocker locker(&m_detectedObjectsMutex);
    return m_detectedObjects.count();
}

void CocoDetectionModel::setDetectedObjects(const QVector<DetectedObject> &detectedObjects)
{
    QMutexLocker locker(&m_detectedObjectsMutex);
    m_detectedObjects = detectedObjects;
    emit detectionObjectsChanged();
}

void CocoDetectionModel::loadLabels(const QString &labelsFilename)
{
    m_labels.clear();
    QFile labelsFile(labelsFilename);
    if (labelsFile.open(QIODevice::ReadOnly)) {
        QTextStream stream(&labelsFile);
        while (!stream.atEnd()) {
            QStringList line = stream.readLine().split("  ", Qt::SkipEmptyParts);
            if (line.length() != 2) {
                continue;
            }

            bool ok = false;
            int classIndex = line[0].trimmed().toInt(&ok);
            if (!ok) {
                continue;
            }

            QString className = line[1].trimmed();
            if (className.length() <= 0) {
                continue;
            }
            className = className[0].toUpper() + className.mid(1);

            m_labels.insert(classIndex, className);

        }
    }

    labelsFile.close();

}

void CocoDetectionModel::createPalette()
{
    // ssd_mobilenet_v1_1_metadata_1 can detect a maximum of ten locations
    m_palette.clear();
    m_palette << QColor::fromRgb(0xC70039)
              << QColor::fromRgb(0x111D5E)
              << QColor::fromRgb(0x438A5E)
              << QColor::fromRgb(0xCF7500)
              << QColor::fromRgb(0xF9D56E)
              << QColor::fromRgb(0x87431D)
              << QColor::fromRgb(0xE11D74)
              << QColor::fromRgb(0x91D18B)
              << QColor::fromRgb(0x30475E)
              << QColor::fromRgb(0xD7385E);
}

