#ifndef CURVEPLOT_H
#define CURVEPLOT_H

#include <QWidget>
#include <QPainter>
#include <QPaintEvent>

class QMargin
{
public:
    QMargin(int left, int top, int right, int bottom):
        Left(left),Top(top),Right(right),Bottom(bottom){}

    QMargin(int h, int v):
        Left(h),Top(v),Right(h),Bottom(v){}

    QMargin(int d = 0):
        Left(d),Top(d),Right(d),Bottom(d){}

    int Left;
    int Top;
    int Right;
    int Bottom;
};

class CurvePlot : public QWidget
{
    Q_OBJECT
public:
    explicit CurvePlot(QWidget *parent = 0);
    void outPut();
    void outPut(QVector<float> &data);
    void outPutSpect(QVector<float> &data);
    void setAdjustVal(float val){iAdjustVal = val;}
    QSize minimumSizeHint() const;

    QMargin Margin;
    QPen Pen, PenSpect;
    QColor Background;

protected:
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *);

private:
    void transformPointsSpect(QVector<float>&data, int w, int h, QVector<QPointF> &points);
    void transformPoints(QVector<float> &data, int w, int h, QVector<QPointF> &points);
    QPixmap pixmap;
    float iAdjustVal;
};

#endif // CURVEPLOT_H
