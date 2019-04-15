#ifndef GRAPHCUT_H
#define GRAPHCUT_H
#include<vector>
#include<QImage>
class GraphCut
{
public:
    GraphCut();
    void setImage(QImage img);
    void setBackPositions(std::vector<std::vector<int> > poses);
    void setForePositions(std::vector<std::vector<int> > poses);
    QImage compute();
    void clear();

private:
    std::vector<std::vector<int> > backPositions;
    std::vector<std::vector<int> > forePositions;
    QImage img;
};

#endif // GRAPHCUT_H
