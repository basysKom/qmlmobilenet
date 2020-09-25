#include "cocodetectionfilter.h"
#include "bitmap_helpers_impl.h"

#include <QDebug>
#include <QDir>
#include <QStandardPaths>

Q_LOGGING_CATEGORY(objectdetector, "tensorflow.cocodetectionfilter")

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
    auto modelFile = QDir(PathToMachineLearningModels).filePath(CocoModelSSD);
    modelFile = QStandardPaths::locate(QStandardPaths::HomeLocation, modelFile);
    m_detectionWorker = std::unique_ptr<CocoDetectionWorker>(new CocoDetectionWorker(modelFile));

    m_workerThread = new QThread;
    m_detectionWorker->moveToThread(m_workerThread);

    connect(this, &CocoDetectionFilterRunnable::imageReady, m_detectionWorker.get(), &CocoDetectionWorker::predict);
    connect(m_detectionWorker.get(), &CocoDetectionWorker::finishedPrediction, this, [this]() {
        m_detectionWorkerIsBusy = false;
    });

    m_workerThread->start();
}

CocoDetectionFilterRunnable::~CocoDetectionFilterRunnable()
{
    m_workerThread->quit();
    m_workerThread->deleteLater();
}


QVideoFrame CocoDetectionFilterRunnable::run( QVideoFrame *input, const QVideoSurfaceFormat &surfaceFormat, RunFlags flags )
{
    Q_UNUSED( flags )

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

    input->unmap();

    if (m_detectionWorkerIsBusy) {
        return image;
    }

    qCInfo(objectworker) << "Prepare next image";
    auto sourceImage = image;
    if (surfaceFormat.scanLineDirection() == QVideoSurfaceFormat::BottomToTop) {
        sourceImage = sourceImage.mirrored();
    }

    if (sourceImage.format() != QImage::Format_RGB888) {
        sourceImage = sourceImage.convertToFormat(QImage::Format_RGB888);
    }

    emit imageReady(sourceImage);

    m_detectionWorkerIsBusy = true;
    return image;
}
