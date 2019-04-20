#ifndef GRAPHCUT_H
#define GRAPHCUT_H
#include<vector>

#include <opencv2/opencv.hpp>

#include "maxflow-v3.04.src/graph.h"

class GraphCut
{
public:
    GraphCut();
//    void setImage(QImage img);
    void setBackPositions(std::vector<std::pair<int,int> > poses);
    void setForePositions(std::vector<std::pair<int,int> > poses);
    void loadImage(std::string img_path);
    void mincut(std::vector<std::pair<int,int>> fpos, std::vector<std::pair<int,int>> bpos);
    //QImage compute();
    void clear();

private:
    //manual parameter for energy model
    double lambda = 10.0;
	double sigma = 40.0;

    //manual parameter for EM GMM model w.r.t. region probability weights
    int em_n_cluster = 3;
    double em_sigma = 0.1;

    std::vector<std::pair<int,int> > backPositions;
    std::vector<std::pair<int,int> > forePositions;
//    QImage img;
    cv::Mat img;
    std::string img_path;

    void fitGMMProb(std::vector<std::pair<int,int>>& fpos, std::vector<std::pair<int,int>>& bpos,
                    cv::Mat& fore_probs, cv::Mat& back_probs);
    void seedMap(std::vector<std::pair<int,int>>& fpos, std::vector<std::pair<int,int>>& bpos, cv::Mat& seed);

};

#endif // GRAPHCUT_H
