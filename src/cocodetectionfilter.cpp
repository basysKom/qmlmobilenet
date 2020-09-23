#include "cocodetectionfilter.h"

#include <QDebug>
#include <QDir>
#include <QStandardPaths>

Q_LOGGING_CATEGORY(objectdetector, "video.tensorflow.objectdetector")

const auto PathToMachineLearningModels = QStringLiteral("Development/MachineLearning/Models");
const auto CocoModelSSD = QStringLiteral("ssd_mobilenet_v1_1_metadata_1.tflite");

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

    qCInfo(objectdetector) << "Interpreter state:";
    tflite::PrintInterpreterState(m_interpreter.get());

}


QVideoFrame CocoDetectionFilterRunnable::run( QVideoFrame *input, const QVideoSurfaceFormat &surfaceFormat, RunFlags flags )
{
    Q_UNUSED( flags )
    Q_UNUSED( surfaceFormat )

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

    int width = image.width();
    int height = image.height();

    for ( int y = 0; y < height; y++ ) {
        uchar* pixel = image.scanLine( y );
        for ( int x = 0; x < width; x++ ) {
            uchar& B = pixel[ 0 ];
            uchar& G = pixel[ 1 ];
            uchar& R = pixel[ 2 ];
            B = G = R = static_cast< uchar >( qGray( R, G, B ) );
            pixel += 4;
        }
    }

    input->unmap();
    return image;
}
