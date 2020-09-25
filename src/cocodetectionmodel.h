#ifndef __COCO_DETECTION_MODEL__
#define __COCO_DETECTION_MODEL__

#include <QAbstractListModel>
#include <QVector>
#include <QRectF>
#include <QHash>
#include <QMutex>
#include <QColor>
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
        BoundingRect = Qt::UserRole + 1,
        DetectedObjectName,
        Score,
        BoundingRectColor
    };
    Q_ENUMS(DetectedObjectRole)

    explicit CocoDetectionModel(const QString &labelsFilename, QObject *parent = nullptr);
    ~CocoDetectionModel() = default;

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    void setDetectedObjects(const QVector<DetectedObject> &detectedObjects);

signals:
    void detectionObjectsChanged();

private:
    void loadLabels(const QString &labelsFilename);
    void createPalette();

    mutable QMutex m_detectedObjectsMutex;
    QHash<int, QString> m_labels;
    QVector<DetectedObject> m_detectedObjects;
    QVector<QColor> m_palette;


};


#endif // __COCO_DETECTION_MODEL__



