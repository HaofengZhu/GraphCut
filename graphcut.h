#ifndef GRAPHCUT_H
#define GRAPHCUT_H
#include<vector>

#include <opencv2/opencv.hpp>

#include "maxflow-v3.04.src/graph.h"


class GraphCut
{
    //vector for QPoint storage. pair(row, column) <==> QPoint(y, x)
    typedef std::vector<std::pair<int, int>> PointList;
    typedef Graph<double, double, double> GraphType;

public:
    GraphCut();
    void loadImage(std::string img_path);
    void run(PointList& fpos, PointList& bpos);
    std::vector<std::vector<int>>& getLabelMask();
    void clear();

private:
    //manual parameter for energy model
    double lambda = 10.0;
	double sigma = 40.0;

    //manual parameter for EM GMM model w.r.t. region probability weights
    int em_n_cluster = 3;
    double em_sigma = 0.1;

    cv::Ptr<cv::ml::EM> fore_gmm_model;
    cv::Ptr<cv::ml::EM> back_gmm_model;

    GraphType *graph;

    cv::Mat img;
    cv::Mat seed;
    std::string img_path;

    std::vector<std::vector<int>> res_mask;

    void fitGMMProb(PointList& fpos, PointList& bpos,
                    cv::Mat& fore_probs, cv::Mat& back_probs);
    void updateSeed(PointList& fpos, PointList& bpos);

    void saveBinarizedImg();
};

#endif // GRAPHCUT_H
