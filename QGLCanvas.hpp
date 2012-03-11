#ifndef QGLCANVAS_HPP
#define QGLCANVAS_HPP

#include <QGLWidget>

class QGLCanvas : public QGLWidget
{
    Q_OBJECT

public:
    QGLCanvas(QWidget* parent = NULL);
    void setImage(const QImage& image);

protected:
    void paintEvent(QPaintEvent*);

private:
    QImage img;
};

#endif // QGLCANVAS_HPP
