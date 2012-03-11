#include "QGLCanvas.hpp"

QGLCanvas::QGLCanvas(QWidget* parent)
    : QGLWidget(parent)
{
}

void QGLCanvas::setImage(const QImage& image)
{
    img = image;
}

void QGLCanvas::paintEvent(QPaintEvent*)
{
    QPainter p(this);

    //Set the painter to use a smooth scaling algorithm.
    p.setRenderHint(QPainter::SmoothPixmapTransform, 1);

    p.drawImage(this->rect(), img);
}

