#ifndef __COCO_DETECTION_MODEL__
#define __COCO_DETECTION_MODEL__

#include <QAbstractListModel>
#include <QVector>
#include <QRectF>
#include <QHash>
#include <QMutex>
#include <QObject>

class CocoDetectionModel : public QAbstractListModel
{
    Q_OBJECT
public:
    struct DetectedObject {
        int classIndex;
        float score;
        QRectF boundingRect;
    };

    enum DetectedObjectRole {
        LocationX = Qt::UserRole + 1,
        LocationY,
        LocationWidth,
        LocationHeight,
        DetectedObjectName,
        Score
    };
    Q_ENUMS(DetectedObjectRole)

    explicit CocoDetectionModel(const QString &labelsFilename, QObject *parent = nullptr);
    ~CocoDetectionModel() = default;

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    void setDetectedObjects(const QVector<DetectedObject> &detectedObjects);

private:
    void loadLabels(const QString &labelsFilename);

    mutable QMutex m_detectedObjectsMutex;
    QHash<int, QString> m_labels;
    QVector<DetectedObject> m_detectedObjects;


};


#endif // __COCO_DETECTION_MODEL__



