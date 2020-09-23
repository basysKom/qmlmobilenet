#ifndef __COCO_DETECTION_FILTER__
#define __COCO_DETECTION_FILTER__

#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/optional_debug_tools.h"

#include <QLoggingCategory>
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

class CocoDetectionFilterRunnable : public QVideoFilterRunnable
{
public:
    CocoDetectionFilterRunnable();
    QVideoFrame run( QVideoFrame *input, const QVideoSurfaceFormat &surfaceFormat, RunFlags flags ) override;

private:
    void initializeModel(const QString &filename);
    std::unique_ptr<tflite::FlatBufferModel> m_model = nullptr;
    std::unique_ptr<tflite::Interpreter> m_interpreter = nullptr;
};

#endif // __COCO_DETECTION_FILTER__
