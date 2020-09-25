#ifndef __COCO_DETECTION_WORKER__
#define __COCO_DETECTION_WORKER__


#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/optional_debug_tools.h"

#include <QObject>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(objectworker)

class CocoDetectionWorker : public QObject {
    Q_OBJECT
public:
    CocoDetectionWorker(const QString& tfLiteFile);

public slots:
    void predict(const QImage& image) const;

signals:
    void finishedPrediction() const;

private:
    void initializeModel(const QString &filename);
    float* extractOutputAsFloats(int tensorIndex) const;

    int m_requestedInputHeight = 0;
    int m_requestedInputWidth = 0;
    int m_requestedInputChannels = 0;

    std::unique_ptr<tflite::FlatBufferModel> m_model = nullptr;
    std::unique_ptr<tflite::Interpreter> m_interpreter = nullptr;

};


#endif // __COCO_DETECTION_WORKER__
