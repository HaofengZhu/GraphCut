#include "graphcut.h"
#include <cmath>
#include <stdexcept>
#include <iostream>

#define FOREGROUD_LABEL 1
#define BACKGROUD_LABEL 2

GraphCut::GraphCut()
{

}

//void GraphCut::setImage(QImage img)
//{
//    this->img=img;
//}

void GraphCut::setBackPositions(std::vector<std::pair<int,int> > poses)
{
    backPositions=poses;
}

void GraphCut::setForePositions(std::vector<std::pair<int,int> > poses)
{
    forePositions=poses;
}

void GraphCut::loadImage(std::string img_path)
{
    img = cv::imread(img_path);
}
//
//QImage GraphCut::compute()
//{
//    return QImage();
//}

void GraphCut::clear()
{
    backPositions.clear();
    forePositions.clear();
}

void GraphCut::mincut(std::vector<std::pair<int, int> > fpos, std::vector<std::pair<int, int> > bpos)
{
    int row = img.rows;
    int col = img.cols;

    typedef Graph<double, double, double> GraphType;
    GraphType *graph = new GraphType(row * col, 2 * row * col);
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
    maxWeight = 1e10;
    cv::Mat fore_probs(img.rows, img.cols, CV_64FC1);
    cv::Mat back_probs(img.rows, img.cols, CV_64FC1);
    fitGMMProb(fpos, bpos, fore_probs, back_probs);
	cv::Mat seed = cv::Mat::zeros(img.rows, img.cols, CV_8UC1);
    seedMap(fpos, bpos, seed);

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
				s_weight = -log(back_probs.at<double>(y, x));
				t_weight = -log(fore_probs.at<double>(y, x));
			}
			graph->add_tweights(x * row + y, s_weight, t_weight);
		}
	}

    
    double flow = graph->maxflow();
    printf("flow = %d\n", flow);

}


void GraphCut::seedMap(std::vector<std::pair<int, int>>& fpos, std::vector<std::pair<int, int>>& bpos, cv::Mat& seed)
{
    for(int i = 0; i < fpos.size(); ++i)
        seed.at<uchar>(fpos[i].first, fpos[i].second) = FOREGROUD_LABEL;
    for(int i = 0; i < bpos.size(); ++i)
        seed.at<uchar>(bpos[i].first, bpos[i].second) = BACKGROUD_LABEL;
}


void GraphCut::fitGMMProb(std::vector<std::pair<int, int> > &fpos, std::vector<std::pair<int, int> > &bpos,
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

    Mat fore_labels;
//    Mat fore_log_probs;
    cv::Ptr<EM> fore_gmm_model = EM::create();
    fore_gmm_model->setClustersNumber(em_n_cluster);
    fore_gmm_model->setCovarianceMatrixType(EM::COV_MAT_DIAGONAL);
    fore_gmm_model->trainEM(foreMat, cv::noArray(), fore_labels, cv::noArray());

    Mat back_labels;
//    Mat back_log_probs;
    cv::Ptr<EM> back_gmm_model = EM::create();
    back_gmm_model->setClustersNumber(em_n_cluster);
    back_gmm_model->setCovarianceMatrixType(EM::COV_MAT_DIAGONAL);
    back_gmm_model->trainEM(backMat, cv::noArray(), back_labels, cv::noArray());


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

