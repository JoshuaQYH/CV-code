#pragma once 
#include <ctime>  
#include <opencv2/opencv.hpp>  
#include <opencv/cv.h>  
#include <iostream> 
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp>  
#include <opencv2/ml/ml.hpp>  

#include "CImg.h"
using namespace std;
using namespace cimg_library;




int ReverseInt(int i);

cv::Mat read_Mnist_Label(std::string filename);

cv::Mat read_Mnist_Image(std::string filename);

bool loadSVM(cv::Ptr<cv::ml::SVM>* mySvm);

int predictWithSVM(cv::Mat digitImagem, cv::Ptr<cv::ml::SVM> svm) ;


int trainAndPredict();