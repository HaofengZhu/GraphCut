#include "drawwidget.h"
#include <QtGui>
#include <QPen>
#include<QMessageBox>
#include<vector>
DrawWidget::DrawWidget(int width,int height,QWidget *parent) : QWidget(parent)
{
    this->width=width;
    this->height=height;
    this->setFixedSize(width,height);
    this->setGeometry(0, 0, 300, 100);
    QPalette pal(this->palette());

    pal.setColor(QPalette::Background, Qt::white);
    this->setAutoFillBackground(true);
    this->setPalette(pal);
    this->show();
    lineWidth=1;
    foreColor=Qt::red;
    backColor=Qt::yellow;
    isFore=true;
}

void DrawWidget::setImage(QImage& img)
{
    result_img=img;//浅拷贝
    draw_img=QPixmap::fromImage(img);

    img_wid = img.size().width();
    img_hei = img.size().height();

    this->setAutoFillBackground(true);
    int width = img.size().width();
    int height = img.size().height();
    if(width > height)
    {
        height = height*this->size().width()/width;
        width=this->size().width();
    }
    else {
        width=width*this->size().height()/height;
        height=this->size().height();
    }

    result_img=img.scaled(width,height, Qt::KeepAspectRatio,
                          Qt::SmoothTransformation);
    origin_img=result_img.copy();
    draw_img=QPixmap::fromImage(origin_img);

    this->setFixedSize(width,height);
    QPalette pal=this->palette();
    pal.setBrush(QPalette::Background,origin_img);
    this->setPalette(pal);

}

void DrawWidget::setImagePath(std::string img_path)
{
    this->img_path = img_path;
}

void DrawWidget::setForeOrBack(bool isFore)
{
    this->isFore=isFore;
}

void DrawWidget::setLineWidth(int w)
{
    this->lineWidth=w;
}

void DrawWidget::mousePressEvent(QMouseEvent *e)
{
    startPos = e->pos ();
}

void DrawWidget::mouseMoveEvent(QMouseEvent *e)
{
    QPainter *painter = new QPainter;            //新建一个QPainter对象
    QPen pen;                                    //新建一个QPen对象
    //设置画笔的线型,style表示当前选择的线型是Qt::PenStyle枚举数据中的第几个元素
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(lineWidth);                       //设置画笔的线宽值

    pen.setColor(isFore?foreColor:backColor);                        //设置画笔的颜色
    /***
     * 以QPixmap对象为QPaintDevice参数绘制,构造一个QPainter对象,
     * 就立即开始对绘画设备进行绘制,此构造QPainter对象是短期的
     * 由于当一个QPainter对象的初始化失败时构造函数不能提供反馈信息,
     * 所以在绘制 外部设备时 应使用begin()和end()(Ps:如打印机外部设备)
    */
    painter->begin(&draw_img);
    painter->setPen (pen);                       //将QPen对象应用到绘制对象当中
    //绘制从startPos到鼠标当前位置的直线
    painter->drawLine (startPos, e->pos ());
    painter->end ();                             //绘制成功返回true
    if(isFore)
    {
        forePos.append(startPos);
        forePos.append(e->pos());
    }
    else {
        backPos.append(startPos);
        backPos.append(e->pos());
    }
    startPos = e->pos ();                        //更新鼠标的当前位置,为下次绘制做准备

    update ();
}

void DrawWidget::compute()
{
//    if(forePos.isEmpty()||backPos.isEmpty())
//    {
//        QMessageBox::about(nullptr, "Warning", "please draw foreground and backgroud");
//    }
    std::vector<std::pair<int,int> > backP;
    std::vector<std::pair<int,int> > foreP;

    int max_x = origin_img.size().width();
    int max_y = origin_img.size().height();
    for(int i=0;i<forePos.size();i++)
    {
        if(!dropout(forePos[i]))
        {
            int reg_x = forePos[i].x() / float(max_x) * (img_wid-1);
            int reg_y = forePos[i].y() / float(max_y) * (img_hei-1);
            foreP.push_back(std::make_pair(reg_y, reg_x));
        }
    }

    for(int i=0;i<backPos.size();i++)
    {
        if(!dropout(backPos[i]))
        {
            int reg_x = backPos[i].x() / float(max_x) * (img_wid-1);
            int reg_y = backPos[i].y() / float(max_y) * (img_hei-1);
            backP.push_back(std::make_pair(reg_y, reg_x));
        }
    }
//    gc.setBackPositions(backP);
//    gc.setForePositions(foreP);
//    gc.setImage(origion_img);
//    result_img=gc.compute();
    gc.loadImage(this->img_path);
    gc.run(foreP, backP);

    this->setAutoFillBackground(true);
    qDebug()<<"start compute.";
    QPalette pal(this->palette());
    pal.setColor(QPalette::Background, Qt::white);
    this->setAutoFillBackground(true);
    this->setPalette(pal);

}

void DrawWidget::clear()
{
    setImage(this->origin_img);
    backPos.clear();
    forePos.clear();
}

void DrawWidget::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.drawPixmap (QPoint(0,0),draw_img);
}

bool DrawWidget::dropout(QPoint point) {
    int x = point.x(), y = point.y();
    if(x < 0 || x > origin_img.size().width()) return true;
    if(y < 0 || y > origin_img.size().height()) return true;
    return false;
}
