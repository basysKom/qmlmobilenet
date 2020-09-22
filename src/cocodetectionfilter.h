#ifndef __COCO_DETECTION_FILTER__
#define __COCO_DETECTION_FILTER__

#include <QAbstractVideoFilter>
#include <QVideoFilterRunnable>

class CocoDetectionFilter : public QAbstractVideoFilter
{
    Q_OBJECT
public:
    CocoDetectionFilter( QObject* parent = nullptr );
    QVideoFilterRunnable* createFilterRunnable() override;
};

class CocoDetectionFilterRunnable : public QVideoFilterRunnable
{
public:
    CocoDetectionFilterRunnable();
    QVideoFrame run( QVideoFrame *input, const QVideoSurfaceFormat &surfaceFormat, RunFlags flags ) override;
};

#endif // __COCO_DETECTION_FILTER__
