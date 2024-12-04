#include "cocodetectionfilter.h"
#include "bitmap_helpers_impl.h"

#include <QDebug>
#include <QDir>
#include <QStandardPaths>

Q_LOGGING_CATEGORY(objectdetector, "tensorflow.cocodetectionfilter")

const auto PathToMachineLearningModels = QStringLiteral("../model");
const auto CocoModelSSD = QStringLiteral("ssd_mobilenet_v1_1_metadata_1.tflite");
const auto CocoLabels = QStringLiteral("coco_labels.txt");

CocoDetectionFilter::CocoDetectionFilter( QObject* parent )
    : QAbstractVideoFilter( parent )
{
    auto labelFile = QDir(PathToMachineLearningModels).filePath(CocoLabels);
    m_detectionModel = new CocoDetectionModel(labelFile, this);
}

QVideoFilterRunnable* CocoDetectionFilter::createFilterRunnable()
{
    auto modelFile = QDir(PathToMachineLearningModels).filePath(CocoModelSSD);
    return new CocoDetectionFilterRunnable(modelFile, m_detectionModel);
}


CocoDetectionModel* CocoDetectionFilter::detectionModel() const
{
    return m_detectionModel;
}


CocoDetectionFilterRunnable::CocoDetectionFilterRunnable(const QString &modelFilename, CocoDetectionModel* detectionModel)
{
    m_detectionWorker = std::unique_ptr<CocoDetectionWorker>(new CocoDetectionWorker(modelFilename));
    m_detectionWorker->setDetectionModel(detectionModel);

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


QVideoFrame CocoDetectionFilterRunnable::run(QVideoFrame *input, const QVideoSurfaceFormat &surfaceFormat, RunFlags flags )
{
    Q_UNUSED( flags )

    if ( !input ) {
        return QVideoFrame();
    }

    const QImage::Format imageFormat = QVideoFrame::imageFormatFromPixelFormat(input->pixelFormat());
    input->map(QAbstractVideoBuffer::ReadOnly);
    QImage image;

    if (imageFormat != QImage::Format_Invalid) {
    QImage image = QImage(input->bits(),
                 input->width(),
                 input->height(),
                 input->bytesPerLine(),
                 imageFormat).copy();
    } else {
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        image = input->image();
#endif
    }

    input->unmap();

    if (m_detectionWorkerIsBusy) {
        return image;
    }

    qCInfo(objectdetector) << "Prepare next image";
    auto sourceImage = image;
    if (surfaceFormat.scanLineDirection() == QVideoSurfaceFormat::BottomToTop) {
        sourceImage = sourceImage.mirrored();
    }

    if (sourceImage.format() != QImage::Format_RGB888) {
        sourceImage = sourceImage.convertToFormat(QImage::Format_RGB888);
    }

    if (sourceImage.format() != QImage::Format_Invalid) {
        emit imageReady(sourceImage);
        m_detectionWorkerIsBusy = true;
    }

    return image;
}
