#ifndef __COCO_DETECTION_FILTER__
#define __COCO_DETECTION_FILTER__

#include "cocodetectionworker.h"

#include <QLoggingCategory>
#include <QThread>
#include <QAbstractVideoFilter>
#include <QVideoFilterRunnable>

Q_DECLARE_LOGGING_CATEGORY(objectdetector)

class CocoDetectionFilter : public QAbstractVideoFilter
{
    Q_OBJECT
public:
    CocoDetectionFilter( QObject* parent = nullptr );
    QVideoFilterRunnable* createFilterRunnable() override;
};

class CocoDetectionFilterRunnable : public QObject, public QVideoFilterRunnable
{
    Q_OBJECT
public:
    CocoDetectionFilterRunnable();
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
