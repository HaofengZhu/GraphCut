#include "graphcut.h"

GraphCut::GraphCut()
{

}

void GraphCut::setImage(QImage img)
{
    this->img=img;
}

void GraphCut::setBackPositions(std::vector<std::vector<int> > poses)
{
    backPositions=poses;
}

void GraphCut::setForePositions(std::vector<std::vector<int> > poses)
{
    forePositions=poses;
}

QImage GraphCut::compute()
{
    return QImage();
}

void GraphCut::clear()
{
    backPositions.clear();
    forePositions.clear();
    img=QImage();
}

