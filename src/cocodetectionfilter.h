#ifndef __COCO_DETECTION_FILTER__
#define __COCO_DETECTION_FILTER__

#include "cocodetectionworker.h"
#include "cocodetectionmodel.h"

#include <QLoggingCategory>
#include <QThread>
#include <QAbstractItemModel>
#include <QAbstractVideoFilter>
#include <QVideoFilterRunnable>

Q_DECLARE_LOGGING_CATEGORY(objectdetector)

class CocoDetectionFilter : public QAbstractVideoFilter
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel* detectionModel READ detectionModel CONSTANT)
public:
    CocoDetectionFilter( QObject* parent = nullptr );
    QVideoFilterRunnable* createFilterRunnable() override;

    CocoDetectionModel* detectionModel() const;

private:
    CocoDetectionModel* m_detectionModel = nullptr;
};

class CocoDetectionFilterRunnable : public QObject, public QVideoFilterRunnable
{
    Q_OBJECT
public:
    CocoDetectionFilterRunnable(const QString &modelFilename, CocoDetectionModel* detectionModel = nullptr);
    ~CocoDetectionFilterRunnable();
    QVideoFrame run( QVideoFrame *input, const QVideoSurfaceFormat &surfaceFormat, RunFlags flags ) override;

signals:
    void imageReady(const QImage &image);

private:
    bool m_detectionWorkerIsBusy = false;
    std::unique_ptr<CocoDetectionWorker> m_detectionWorker = nullptr;
    QThread* m_workerThread = nullptr;
};

#endif // __COCO_DETECTION_FILTER__
