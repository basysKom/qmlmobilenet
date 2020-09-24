#include "cocodetectionfilter.h"
#include "bitmap_helpers_impl.h"

#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QElapsedTimer>

Q_LOGGING_CATEGORY(objectdetector, "video.tensorflow.objectdetector")

const auto PathToMachineLearningModels = QStringLiteral("Development/MachineLearning/Models");
const auto CocoModelSSD = QStringLiteral("ssd_mobilenet_v1_1_metadata_1.tflite");

// ToDo: should be an QML property
const float Threshold = 0.5;

CocoDetectionFilter::CocoDetectionFilter( QObject* parent )
    : QAbstractVideoFilter( parent )
{
}

QVideoFilterRunnable* CocoDetectionFilter::createFilterRunnable()
{
    return new CocoDetectionFilterRunnable();
}


CocoDetectionFilterRunnable::CocoDetectionFilterRunnable()
{
    const auto modelFile = QDir(PathToMachineLearningModels).filePath(CocoModelSSD);
    initializeModel(QStandardPaths::locate(QStandardPaths::HomeLocation, modelFile));
}

void CocoDetectionFilterRunnable::initializeModel(const QString& filename)
{
    qCInfo(objectdetector) << "Loading model ...";
    m_model = tflite::FlatBufferModel::BuildFromFile(filename.toLocal8Bit());

    if (m_model == nullptr) {
        qCWarning(objectdetector) << "Could not load model";
        return;
    }

    qCInfo(objectdetector) << "Building interpreter ...";
    tflite::ops::builtin::BuiltinOpResolver resolver;
    tflite::InterpreterBuilder builder(*m_model, resolver);
    builder(&m_interpreter);

    if (m_interpreter == nullptr) {
        qCWarning(objectdetector) << "Could not build interpreter ...";
        return;
    }

    if (m_interpreter->AllocateTensors() != kTfLiteOk) {
        qCWarning(objectdetector) << "Could not allocate tensors";
        return;
    }

    m_interpreter->SetNumThreads(4);

    qCInfo(objectdetector) << "Interpreter state:";
    tflite::PrintInterpreterState(m_interpreter.get());

    qCInfo(objectdetector) << "****************************************************";
    qCInfo(objectdetector) << "Tensors size: " << m_interpreter->tensors_size();
    qCInfo(objectdetector) << "Nodes size: " << m_interpreter->nodes_size();
    qCInfo(objectdetector) << "Number of Inputs: " << m_interpreter->inputs().size();
    qCInfo(objectdetector) << "Input IDs: " << m_interpreter->inputs();
    qCInfo(objectdetector) << "Input(0) name: " << m_interpreter->GetInputName(0);
    qCInfo(objectdetector) << "Number of Outputs: " << m_interpreter->outputs().size();
    qCInfo(objectdetector) << "Output IDs: " << m_interpreter->outputs();

    int counter = 0;
    for (auto tensorInput : m_interpreter->inputs()) {
        TfLiteIntArray* dims = m_interpreter->tensor(tensorInput)->dims;
        if (dims && dims->size > 3) {
            qCInfo(objectdetector) << m_interpreter->GetInputName(counter) << ": Input Batch Size: " << dims->data[0];
            qCInfo(objectdetector) << m_interpreter->GetInputName(counter) << ": Input Height: " << dims->data[1];
            qCInfo(objectdetector) << m_interpreter->GetInputName(counter) << ": Input Width:" << dims->data[2];
            qCInfo(objectdetector) << m_interpreter->GetInputName(counter) << ": Channel Width:" << dims->data[3];
            // we are interested only in the first input
            if (counter == 0) {
                m_requestedInputHeight = dims->data[1];
                m_requestedInputWidth = dims->data[2];
                m_requestedInputChannels = dims->data[3];
            }
        }
        counter++;
    }

    counter = 0;
    for (auto tensorOutput : m_interpreter->outputs()) {
        TfLiteIntArray* dims = m_interpreter->tensor(tensorOutput)->dims;
        if (dims) {
            QVector<int> outputDims;
            for (int j = 0; j < dims->size; j++) {
                outputDims << dims->data[j];
            }
            qCInfo(objectdetector) << m_interpreter->GetOutputName(counter) << ": Output dimensions: " << outputDims;
        }
        counter++;
    }

}

float* CocoDetectionFilterRunnable::extractOutputAsFloat(int tensorIndex) const
{
    TfLiteTensor* tensor = m_interpreter->tensor(tensorIndex);
    Q_ASSERT(tensor && tensor->type == kTfLiteFloat32);
    return tensor->data.f;
}

void CocoDetectionFilterRunnable::predict(const QImage &image) const
{
    if (Q_UNLIKELY(m_interpreter->inputs().size() <= 0)) {
        qCWarning(objectdetector) << "Model not loaded - Detection does not work!";
        return;
    }

    const int channels = 3; // we enfoce QImage::Format_RGB888

    int imageInput = m_interpreter->inputs()[0];
    TfLiteType inputType = m_interpreter->tensor(imageInput)->type;

    // tflite::label_image:::resize: the method resizes, normalizes and assigns image to input tensor
    switch(inputType) {
    case kTfLiteFloat32:
        tflite::label_image::resize(m_interpreter->typed_tensor<float>(imageInput),
                                    image.constBits(), image.height(), image.width(), channels,
                                    m_requestedInputHeight, m_requestedInputWidth,
                                    m_requestedInputChannels, inputType);
        break;
    case kTfLiteInt8:
        tflite::label_image::resize(m_interpreter->typed_tensor<int8_t>(imageInput),
                                    image.constBits(), image.height(), image.width(), channels,
                                    m_requestedInputHeight, m_requestedInputWidth,
                                    m_requestedInputChannels, inputType);
        break;
    case kTfLiteUInt8:
        tflite::label_image::resize(m_interpreter->typed_tensor<uint8_t>(imageInput),
                                    image.constBits(), image.height(), image.width(), channels,
                                    m_requestedInputHeight, m_requestedInputWidth,
                                    m_requestedInputChannels, inputType);
        break;
     default:
        qCWarning(objectdetector) << "Cannot handle input type " << inputType << " - Incompatible Model loaded?";
        return;
    }

    qDebug() << "before invoke";

    // finally run the network :-)
    QElapsedTimer timer;
    timer.start();
    TfLiteStatus status = m_interpreter->Invoke();

    if (status != kTfLiteOk) {
        qCWarning(objectdetector) << "Failed to inference the image" << status;
        return;
    }

    qCInfo(objectdetector) << "after invoke - Returned with status" << status << "in" << timer.elapsed() << "ms";

    // inspired by https://github.com/YijinLiu/tf-cpu/blob/master/benchmark/obj_detect_lite.cc
    const auto outputTensors = m_interpreter->outputs();
    const float* locations          = extractOutputAsFloat(outputTensors[0]);
    const float* outputClasses      = extractOutputAsFloat(outputTensors[1]);
    const float* outputScores       = extractOutputAsFloat(outputTensors[2]);
    const float* foundDetections    = extractOutputAsFloat(outputTensors[3]);

    for (int detectionIndex = 0; detectionIndex < static_cast<int>(*foundDetections); detectionIndex++) {

        const int classIndex = static_cast<int>(outputClasses[detectionIndex]);
        const float score = outputScores[detectionIndex];

        const int top    = static_cast<int>(locations[4 * detectionIndex]      * image.height());
        const int left   = static_cast<int>(locations[4 * detectionIndex + 1]  * image.width());
        const int bottom = static_cast<int>(locations[4 * detectionIndex + 2]  * image.height());
        const int right  = static_cast<int>(locations[4 * detectionIndex + 3]  * image.width());

        const QRectF boundingRect(left, top, right - left, bottom - top);

        qCInfo(objectdetector) << "Found object with score at:" << classIndex << score << boundingRect;

    }

}


QVideoFrame CocoDetectionFilterRunnable::run( QVideoFrame *input, const QVideoSurfaceFormat &surfaceFormat, RunFlags flags )
{
    Q_UNUSED( flags )
    Q_UNUSED( surfaceFormat )

    //ToDo
    // move prediction to a thread
    static int frameCounter = 1;

    if ( !input ) {
        return QVideoFrame();
    }

    const QImage::Format imageFormat = QVideoFrame::imageFormatFromPixelFormat(input->pixelFormat());
    input->map(QAbstractVideoBuffer::ReadWrite);
    QImage image = QImage(input->bits(),
                 input->width(),
                 input->height(),
                 input->bytesPerLine(),
                 imageFormat).copy();

   if (frameCounter % 100 == 0) {
       if (image.format() != QImage::Format_RGB888) {
           predict(image.convertToFormat(QImage::Format_RGB888));
       } else {
           predict(image);
       }
   }

   frameCounter++;

   input->unmap();
   return image;
}
