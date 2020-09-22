#include "cocodetectionfilter.h"

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
}

QVideoFrame CocoDetectionFilterRunnable::run( QVideoFrame *input, const QVideoSurfaceFormat &surfaceFormat, RunFlags flags )
{
    Q_UNUSED( flags )
     Q_UNUSED( surfaceFormat )

    if ( !input ) {
        return QVideoFrame();
    }

    QImage image = input->image();
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

    return image;
}
