#include "cocodetectionworker.h"
#include "bitmap_helpers_impl.h"

#include <QImage>
#include <QElapsedTimer>

Q_LOGGING_CATEGORY(objectworker, "tensorflow.cocodetectionworker")

// ToDo: should be an QML property
const float Threshold = 0.5;

CocoDetectionWorker::CocoDetectionWorker(const QString& tfLiteFile)
{
    initializeModel(tfLiteFile);
}

void CocoDetectionWorker::setDetectionModel(CocoDetectionModel* detectionModel)
{
    m_detectionModel = QPointer<CocoDetectionModel>(detectionModel);
}

void CocoDetectionWorker::initializeModel(const QString& filename)
{
    qCInfo(objectworker) << "Loading model ...";
    m_model = tflite::FlatBufferModel::BuildFromFile(filename.toLocal8Bit());

    if (m_model == nullptr) {
        qCWarning(objectworker) << "Could not load model";
        return;
    }

    qCInfo(objectworker) << "Building interpreter ...";
    tflite::ops::builtin::BuiltinOpResolver resolver;
    tflite::InterpreterBuilder builder(*m_model, resolver);
    builder(&m_interpreter);

    if (m_interpreter == nullptr) {
        qCWarning(objectworker) << "Could not build interpreter ...";
        return;
    }

    if (m_interpreter->AllocateTensors() != kTfLiteOk) {
        qCWarning(objectworker) << "Could not allocate tensors";
        return;
    }

    m_interpreter->SetNumThreads(4);

    qCInfo(objectworker) << "Interpreter state:";
    tflite::PrintInterpreterState(m_interpreter.get());

    qCInfo(objectworker) << "****************************************************";
    qCInfo(objectworker) << "Tensors size: " << m_interpreter->tensors_size();
    qCInfo(objectworker) << "Nodes size: " << m_interpreter->nodes_size();
    qCInfo(objectworker) << "Number of Inputs: " << m_interpreter->inputs().size();
    qCInfo(objectworker) << "Input IDs: " << m_interpreter->inputs();
    qCInfo(objectworker) << "Input(0) name: " << m_interpreter->GetInputName(0);
    qCInfo(objectworker) << "Number of Outputs: " << m_interpreter->outputs().size();
    qCInfo(objectworker) << "Output IDs: " << m_interpreter->outputs();

    int counter = 0;
    for (auto tensorInput : m_interpreter->inputs()) {
        TfLiteIntArray* dims = m_interpreter->tensor(tensorInput)->dims;
        if (dims && dims->size > 3) {
            qCInfo(objectworker) << m_interpreter->GetInputName(counter) << ": Input Batch Size: " << dims->data[0];
            qCInfo(objectworker) << m_interpreter->GetInputName(counter) << ": Input Height: " << dims->data[1];
            qCInfo(objectworker) << m_interpreter->GetInputName(counter) << ": Input Width:" << dims->data[2];
            qCInfo(objectworker) << m_interpreter->GetInputName(counter) << ": Channel Width:" << dims->data[3];
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
            qCInfo(objectworker) << m_interpreter->GetOutputName(counter) << ": Output dimensions: " << outputDims;
        }
        counter++;
    }

}

float* CocoDetectionWorker::extractOutputAsFloats(int tensorIndex) const
{
    TfLiteTensor* tensor = m_interpreter->tensor(tensorIndex);
    Q_ASSERT(tensor && tensor->type == kTfLiteFloat32);
    return tensor->data.f;
}

void CocoDetectionWorker::predict(const QImage &image) const
{
    if (Q_UNLIKELY(m_interpreter->inputs().size() <= 0)) {
        qCWarning(objectworker) << "Model not loaded - Detection does not work!";
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
        qCWarning(objectworker) << "Cannot handle input type " << inputType << " - Incompatible Model loaded?";
        return;
    }

    // finally run the network :-)
    QElapsedTimer timer;
    timer.start();
    TfLiteStatus status = m_interpreter->Invoke();

    if (status != kTfLiteOk) {
        qCWarning(objectworker) << "Failed to inference the image" << status;
        return;
    }

    qCInfo(objectworker) << "Inference Done - Returned with status" << status << "in" << timer.elapsed() << "ms";

    // inspired by https://github.com/YijinLiu/tf-cpu/blob/master/benchmark/obj_detect_lite.cc
    const auto outputTensors = m_interpreter->outputs();
    const float* locations          = extractOutputAsFloats(outputTensors[0]);
    const float* outputClasses      = extractOutputAsFloats(outputTensors[1]);
    const float* outputScores       = extractOutputAsFloats(outputTensors[2]);
    const float* foundDetections    = extractOutputAsFloats(outputTensors[3]);

    QVector<CocoDetectionModel::DetectedObject> detectedObjects;

    for (int detectionIndex = 0; detectionIndex < static_cast<int>(*foundDetections); detectionIndex++) {

        const float score = outputScores[detectionIndex];

        if (score < Threshold) {
            continue;
        }

        const int classIndex = static_cast<int>(outputClasses[detectionIndex]);
        const float top    = locations[4 * detectionIndex];
        const float left   = locations[4 * detectionIndex + 1];
        const float bottom = locations[4 * detectionIndex + 2];
        const float right  = locations[4 * detectionIndex + 3];

        const QRectF boundingRect(left, top, right - left, bottom - top);
        qCInfo(objectworker) << "Found object" << classIndex << "with score" << score << "at:" << boundingRect;

        CocoDetectionModel::DetectedObject detectedObject;
        detectedObject.classIndex = classIndex;
        detectedObject.score = score;
        detectedObject.boundingRect = boundingRect;

        detectedObjects << detectedObject;

    }

    if (!m_detectionModel.isNull()) {
        m_detectionModel->setDetectedObjects(detectedObjects);
    }


    emit finishedPrediction();
}
