#include "QGLCanvas.hpp"

QGLCanvas::QGLCanvas(QWidget* parent)
    : QGLWidget(parent)
{
}

void QGLCanvas::setImage(const QImage& image)
{
    img = image;
}

void QGLCanvas::renewFrame( uint8_t const * aPpmBuffer, int aPpmSize )
{
    QImage image = QImage::fromData( aPpmBuffer, aPpmSize, "PPM" );
    setImage( image );
    update();
}

void QGLCanvas::clear()
{
    // QImage image( 100, 100, QImage::Format_RGB32 );
    QImage image( 1, 1, QImage::Format_RGB32 );
    QRgb value;
    value = qRgb( 0, 0, 0 ); // 0xffbd9527
    image.setPixel( 0, 0, value );

    setImage( image );
    update();
}

void QGLCanvas::paintEvent(QPaintEvent*)
{
    QPainter p(this);

    //Set the painter to use a smooth scaling algorithm.
    p.setRenderHint(QPainter::SmoothPixmapTransform, 1);

    p.drawImage(this->rect(), img);
}

