#include "curveplot.h"

CurvePlot::CurvePlot(QWidget *parent) :
    QWidget(parent)
{
    Margin = QMargin(0);
    Background = QColor(255, 255, 255, 2);
    Pen = QPen(QBrush(QColor(89, 254, 222)), 1);
    PenSpect = QPen(QBrush(QColor(34, 106, 217)), 15);
    iAdjustVal = 0;
}

void CurvePlot::outPut()
{
    int w = width() - Margin.Left - Margin.Right;
    int h = height() - Margin.Top - Margin.Bottom;

    pixmap = QPixmap(w, h);
    pixmap.fill(Background);
//    QPainter painter(&pixmap);
//    painter.setPen(Pen);
//    painter.drawLine(QPoint(Margin.Left, height()/2), QPoint(Margin.Left + w, height()/2));
    update();
}

void CurvePlot::transformPointsSpect(QVector<float>&data, int w, int h, QVector<QPointF> &points)
{   // 获取极值
    float max = data[0], min = data[0], sec = data[0];
    for (int i= 1; i< data.size(); i++)
    {
        if (max < data[i]) {
            sec = max;
            max = data[i];
        }
        if (sec < data[i])
            sec = data[i];
        if (min > data[i])
            min = data[i];
    }
    // 转化成当前屏幕的内的坐标大小
    max = sec;
    max += iAdjustVal;
    min -= iAdjustVal;
    float diffVal = max - min;
    for (int i= 0; i< data.size(); i++) {
        if (data[i] * 20  > diffVal)
            points.append(QPointF(i * w / data.size() * 10, h - (data[i] - min) / diffVal * h));
        else
            points.append(QPointF(i * w / data.size() * 10, h - 1));
    }
}

void CurvePlot::transformPoints(QVector<float>&data, int w, int h, QVector<QPointF> &points)
{   // 获取极值
    float max = data[0], min = data[0];
    for (int i= 1; i< data.size(); i++)
    {
        if (max < data[i])
            max = data[i];
        if (min > data[i])
            min = data[i];
    }
    // 转化成当前屏幕的内的坐标大小
    max += iAdjustVal;
    min -= iAdjustVal;
    float diffVal = max - min;
    for (int i= 0; i< data.size(); i++)
        points.append(QPointF(i * w / data.size(), h - (data[i] - min) / diffVal * h));
}

void CurvePlot::outPut(QVector<float>&data)
{
    int w = width() - Margin.Left - Margin.Right;
    int h = height() - Margin.Top - Margin.Bottom;

    pixmap = QPixmap(w, h);
    pixmap.fill(Background);
    QPainter painter(&pixmap);

    QVector<QPointF> points;
    transformPoints(data, w, h, points);
    painter.setPen(Pen);
    int step = points.size()*2/w;
    for (int i= 0; i< points.size()-1; i+=step)
        painter.drawLine(points[i], points[i] + QPoint(0, h - points[i].y()*2));

    update();
}

void CurvePlot::outPutSpect(QVector<float> &data)
{
    int w = width() - Margin.Left - Margin.Right;
    int h = height() - Margin.Top - Margin.Bottom;

    pixmap = QPixmap(w, h);
    pixmap.fill(Background);
    QPainter painter(&pixmap);

    QVector<QPointF> points;
    transformPointsSpect(data, w, h, points);
    painter.setPen(PenSpect);
    int cnt = points.size()/10;
    for (int i= 0; i< cnt; i++)
        painter.drawLine(points[i], QPoint(points[i].x(), h));

    update();
}

void CurvePlot::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    int w = width() - Margin.Left - Margin.Right;
    int h = height() - Margin.Top - Margin.Bottom;
    painter.drawPixmap(Margin.Left, Margin.Top, w, h, pixmap);
}

void CurvePlot::resizeEvent(QResizeEvent *)
{
    outPut();
}

QSize CurvePlot::minimumSizeHint() const
{
    return QSize(Margin.Left + Margin.Right + 40, Margin.Top + Margin.Bottom + 40);
}
