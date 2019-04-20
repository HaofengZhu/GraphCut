#ifndef DRAWWIDGET_H
#define DRAWWIDGET_H

#include <QWidget>
#include"graphcut.h"

class DrawWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DrawWidget(int width,int height,QWidget *parent=nullptr);
    void setImage(QImage& img);
    void setImagePath(std::string img_path);
    void setForeOrBack(bool isFore);
    void setLineWidth(int w);
    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void compute();
    void clear();
    void paintEvent (QPaintEvent *e);
signals:

public slots:

private:
    bool isFore;
    QColor foreColor;
    QColor backColor;
    int width;
    int height;
    int lineWidth;
    std::string img_path;
    QImage origin_img;
    QPixmap draw_img;
    QImage result_img;
    QPoint startPos;
    QVector<QPoint> forePos;
    QVector<QPoint> backPos;
    GraphCut gc;

    int img_wid;
    int img_hei;

    bool dropout(QPoint point);
};

#endif // DRAWWIDGET_H
