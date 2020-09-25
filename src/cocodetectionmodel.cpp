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
}


QHash<int, QByteArray> CocoDetectionModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[LocationX] = QByteArray("locationX");
    roles[LocationY] = QByteArray("locationY");
    roles[LocationWidth] = QByteArray("locationWidth");
    roles[LocationHeight] = QByteArray("locationHeight");
    roles[DetectedObjectName] = QByteArray("detectedObject");
    roles[Score] = QByteArray("score");
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
    case LocationX:
        return detectedObject.boundingRect.left();
    case LocationY:
        return detectedObject.boundingRect.top();
    case LocationWidth:
        return detectedObject.boundingRect.width();
    case LocationHeight:
        return detectedObject.boundingRect  .height();
    case DetectedObjectName:
        return m_labels.value(detectedObject.classIndex, QString::number(detectedObject.classIndex));
    case Score:
        return detectedObject.score;
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
    beginResetModel();
    m_detectedObjects = detectedObjects;
    endResetModel();
}

void CocoDetectionModel::loadLabels(const QString &labelsFilename)
{
    m_labels.clear();
    QFile labelsFile(labelsFilename);
    if (labelsFile.open(QIODevice::ReadOnly)) {
        QTextStream stream(&labelsFile);
        while (!stream.atEnd()) {
            QStringList line = stream.readLine().split(" ", Qt::SkipEmptyParts);
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

