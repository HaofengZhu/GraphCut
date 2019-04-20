#include "graphcut.h"
#include <cmath>
#include <stdexcept>
#include <iostream>

GraphCut::GraphCut()
{
    graph = nullptr;
}


void GraphCut::loadImage(std::string img_path)
{
    img = cv::imread(img_path);
}

void GraphCut::loadImage(cv::Mat& img)
{
    this->img = img;
}

void GraphCut::saveBinarizedImg()
{
    int row = res_mask.size(), col = res_mask[0].size();
    std::string imgPath("C:/Projects/GraphCut/image/res.jpg");
    cv::Mat binImg(row, col, CV_8UC3);
    for(int i = 0; i < row; ++i)
    {
        for(int j = 0; j < col; ++j)
        {
            if(res_mask[i][j] == BACKGROUD_LABEL)
                binImg.at<cv::Vec3b>(i,j) = cv::Vec3b(0, 0, 0);
            else
                binImg.at<cv::Vec3b>(i,j) = cv::Vec3b(255,255,255);
        }
    }
    cv::imwrite(imgPath, binImg);
}

std::vector<std::vector<int>>& GraphCut::getLabelMask()
{
    return this->res_mask;
}

void GraphCut::clear()
{
    delete graph;
    graph = nullptr;
    res_mask.resize(0);
    fore_gmm_model.release();
    back_gmm_model.release();
}

void GraphCut::run(PointList& fpos, PointList& bpos)
{
    int row = img.rows;
    int col = img.cols;

    // if graph not constructed, create one and assign n_weight at the first time
    if(graph == nullptr)
    {
        graph = new GraphType(row * col, 2 * row * col);
        graph->add_node(row * col);

        double maxWeight = -1e20;
        for (int x = 0; x < col; ++x)
        {
            for (int y = 0; y < row; ++y)
            {
                int upperPointx = x;
                int upperPointy = y - 1;
                int leftPointx = x - 1;
                int leftPointy = y;
                double n_weight = 0;
                if (upperPointy >= 0 && upperPointy < row)
                {
                    double sqr_diff = 0;

                    cv::Vec3b cur_rgb = img.at<cv::Vec3b>(y, x);
                    cv::Vec3b upp_rgb = img.at<cv::Vec3b>(upperPointy, upperPointx);
                    for (int i = 0; i < 3; ++i)
                    {
                        double diff = cur_rgb[i] - upp_rgb[i];
                        sqr_diff += diff * diff;
                    }

                    n_weight = lambda * exp(-sqr_diff / (2 * sigma * sigma));
                    int pIdx = x * row + y;
                    int qIdx = upperPointx * row + upperPointy;

                    graph->add_edge(qIdx, pIdx, n_weight, n_weight);
                }
                if (leftPointx >= 0 && leftPointx < col)
                {
                    double sqr_diff = 0;

                    cv::Vec3b cur_rgb = img.at<cv::Vec3b>(y, x);
                    cv::Vec3b left_rgb = img.at<cv::Vec3b>(leftPointy, leftPointx);
                    for (int i = 0; i < 3; ++i)
                    {
                        double diff = double(cur_rgb[i] - left_rgb[i]);
                        sqr_diff += diff * diff;
                    }
                    n_weight = lambda * exp(-sqr_diff / (2 * sigma * sigma));
                    int pIdx = x * row + y;
                    int qIdx = leftPointx * row + leftPointy;
                    graph->add_edge(qIdx, pIdx, n_weight, n_weight);
                }
                if (n_weight > maxWeight) maxWeight = n_weight;
            }
        }
    }

    // compute and assign s_weight, t_weight in the graph

//    double maxWeight = 1e10;
//    cv::Mat fore_probs(img.rows, img.cols, CV_64FC1);
//    cv::Mat back_probs(img.rows, img.cols, CV_64FC1);
//    fitGMMProb(fpos, bpos, fore_probs, back_probs);
//    updateSeed(fpos, bpos);

//	for (int x = 0; x < col; ++x)
//	{
//		for (int y = 0; y < row; ++y)
//		{
//			uchar label = seed.at<uchar>(y, x);
//			double s_weight = 0;
//			double t_weight = 0;
//			if (label == FOREGROUD_LABEL)
//				s_weight = maxWeight;
//			else if (label == BACKGROUD_LABEL)
//				t_weight = maxWeight;
//			else
//			{
//				s_weight = -log(back_probs.at<double>(y, x));
//				t_weight = -log(fore_probs.at<double>(y, x));
//			}
//            int idx = x * row + y;
//            graph->add_tweights(idx, s_weight, t_weight);
//		}
//	}
    double maxWeight = 1e10;
    double_mat fore_probs(img.rows, std::vector<double>(img.cols, 0));
    double_mat back_probs(img.rows, std::vector<double>(img.cols, 0));
    fitGMMProb(fpos, bpos, fore_probs, back_probs);
    updateSeed(fpos, bpos);

    for (int x = 0; x < col; ++x)
    {
        for (int y = 0; y < row; ++y)
        {
            uchar label = seed.at<uchar>(y, x);
            double s_weight = 0;
            double t_weight = 0;
            if (label == FOREGROUD_LABEL)
                s_weight = maxWeight;
            else if (label == BACKGROUD_LABEL)
                t_weight = maxWeight;
            else
            {
                s_weight = -log(back_probs[y][x]);
                t_weight = -log(fore_probs[y][x]);
            }
            int idx = x * row + y;
            graph->add_tweights(idx, s_weight, t_weight);
        }
    }

    double flow = graph->maxflow();
    printf("flow = %f\n", flow);


    //generate label mask
    if(res_mask.empty())
    {
        res_mask = std::vector<std::vector<int>>(row, std::vector<int>(col, 0));
    }
    for(int x = 0; x < col; ++x)
    {
        for(int y = 0; y < row; ++y)
        {
            int idx = x * row + y;
            if(graph->what_segment(idx) == GraphType::SINK)
                res_mask[y][x] = BACKGROUD_LABEL;
            else
                res_mask[y][x] = FOREGROUD_LABEL;
        }
    }
    saveBinarizedImg();
}


void GraphCut::updateSeed(PointList& fpos, PointList& bpos)
{
    if(seed.empty())
    {
        seed = cv::Mat::zeros(img.rows, img.cols, CV_8UC1);
    }
    for(int i = 0; i < fpos.size(); ++i)
        seed.at<uchar>(fpos[i].first, fpos[i].second) = FOREGROUD_LABEL;
    for(int i = 0; i < bpos.size(); ++i)
        seed.at<uchar>(bpos[i].first, bpos[i].second) = BACKGROUD_LABEL;
}


void GraphCut::fitGMMProb(PointList& fpos, PointList &bpos,
                          cv::Mat& fore_probs, cv::Mat& back_probs)
{
    using namespace cv::ml;
    using cv::Mat;
    using cv::Vec3b;

    int channel = img.channels();
    em_n_cluster = channel;

    Mat foreMat = Mat::zeros(fpos.size(), channel, CV_64FC1);
    Mat backMat = Mat::zeros(bpos.size(), channel, CV_64FC1);

    for(int i = 0; i < fpos.size(); ++i)
    {
        Vec3b rgb = img.at<Vec3b>(fpos[i].first, fpos[i].second);
        foreMat.at<double>(i,0) = static_cast<double>(rgb[0]);
        foreMat.at<double>(i,1) = static_cast<double>(rgb[1]);
        foreMat.at<double>(i,2) = static_cast<double>(rgb[2]);
    }

    for(int i = 0; i < bpos.size(); ++i)
    {
        Vec3b rgb = img.at<Vec3b>(bpos[i].first, bpos[i].second);
        backMat.at<double>(i,0) = static_cast<double>(rgb[0]);
        backMat.at<double>(i,1) = static_cast<double>(rgb[1]);
        backMat.at<double>(i,2) = static_cast<double>(rgb[2]);
    }

    if(fore_gmm_model.empty() || back_gmm_model.empty())
    {
        fore_gmm_model = EM::create();
        fore_gmm_model->setClustersNumber(em_n_cluster);
        fore_gmm_model->setCovarianceMatrixType(EM::COV_MAT_DIAGONAL);

        back_gmm_model = EM::create();
        back_gmm_model->setClustersNumber(em_n_cluster);
        back_gmm_model->setCovarianceMatrixType(EM::COV_MAT_DIAGONAL);
    }

    Mat fore_labels;
//    fore_gmm_model->trainEM(foreMat, cv::noArray(), fore_labels, cv::noArray());
    fore_gmm_model->trainEM(foreMat);

    Mat back_labels;
//    back_gmm_model->trainEM(backMat, cv::noArray(), back_labels, cv::noArray());
    back_gmm_model->trainEM(backMat);

    Mat sample(1, channel, CV_64FC1);
    for(int x = 0; x < img.rows; ++x)
    {
        for(int y = 0; y < img.cols; ++y)
        {
            Vec3b rgb = img.at<Vec3b>(x,y);
            sample.at<double>(0,0) = static_cast<double>(rgb[0]);
            sample.at<double>(0,1) = static_cast<double>(rgb[1]);
            sample.at<double>(0,2) = static_cast<double>(rgb[2]);
			double fore_pred = fore_gmm_model->predict2(sample, cv::noArray())[0];
			double back_pred = back_gmm_model->predict2(sample, cv::noArray())[0];
            fore_probs.at<double>(x,y) = exp(fore_pred);
            back_probs.at<double>(x,y) = exp(back_pred);
        }
    }
}

void GraphCut::fitGMMProb(PointList &fpos, PointList &bpos, double_mat &fore_probs, double_mat &back_probs)
{
    using namespace cv::ml;
    using cv::Mat;
    using cv::Vec3b;

    int channel = img.channels();
    em_n_cluster = channel;



    Mat foreMat = Mat::zeros(fpos.size(), channel, CV_64FC1);
    Mat backMat = Mat::zeros(bpos.size(), channel, CV_64FC1);

    for(int i = 0; i < fpos.size(); ++i)
    {
        Vec3b rgb = img.at<Vec3b>(fpos[i].first, fpos[i].second);
        double* data = foreMat.ptr<double>(i);
        data[0] = static_cast<double>(rgb[0]);
        data[1] = static_cast<double>(rgb[1]);
        data[2] = static_cast<double>(rgb[2]);
    }

    for(int i = 0; i < bpos.size(); ++i)
    {
        Vec3b rgb = img.at<Vec3b>(bpos[i].first, bpos[i].second);
        double* data = backMat.ptr<double>(i);
        data[0] = static_cast<double>(rgb[0]);
        data[1] = static_cast<double>(rgb[1]);
        data[2] = static_cast<double>(rgb[2]);
    }

    if(fore_gmm_model.empty() || back_gmm_model.empty())
    {
        fore_gmm_model = EM::create();
        fore_gmm_model->setClustersNumber(em_n_cluster);
        fore_gmm_model->setCovarianceMatrixType(EM::COV_MAT_DIAGONAL);

        back_gmm_model = EM::create();
        back_gmm_model->setClustersNumber(em_n_cluster);
        back_gmm_model->setCovarianceMatrixType(EM::COV_MAT_DIAGONAL);
    }

    Mat fore_labels;
    fore_gmm_model->trainEM(foreMat);

    Mat back_labels;
    back_gmm_model->trainEM(backMat);

    Mat sample(1, channel, CV_64FC1);
    for(int x = 0; x < img.rows; ++x)
    {
        for(int y = 0; y < img.cols; ++y)
        {
            Vec3b rgb = img.at<Vec3b>(x,y);
            double* data = sample.ptr<double>(0);
            data[0] = static_cast<double>(rgb[0]);
            data[1] = static_cast<double>(rgb[1]);
            data[2] = static_cast<double>(rgb[2]);
            double fore_pred = fore_gmm_model->predict2(sample, cv::noArray())[0];
            double back_pred = back_gmm_model->predict2(sample, cv::noArray())[0];
            fore_probs[x][y] = exp(fore_pred);
            back_probs[x][y] = exp(back_pred);
        }
    }
}

