

#ifndef _LFC_H_
#define _LFC_H_


#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <opencv2/calib3d/calib3d.hpp>
//#opencv2/imgcodecs/ios.h
//#include "opencv2/nonfree/features2d.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <stdio.h>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <math.h>
#include <algorithm>
#include <utility> 
#include <iostream>
#include <stdlib.h>

//#include <GLFW/glfw3.h>
//#include <glut.h>
#include "XForm.h"
#include "ReadInDataSet.h"
//#include "ViewController.h"


using namespace std;
using namespace cv;

#define FAILURE -1
#define SUCCESS 0
#define NUM_FRAMING_IMAGES 3
//#define IMAGE_RESOLUTION_X 480
//#define IMAGE_RESOLUTION_Y 640
#define IMAGE_RESOLUTION_X 288
#define IMAGE_RESOLUTION_Y 352

//#define POSE_RESOLUTION_X 120
//#define POSE_RESOLUTION_Y 160

//essentially defines a ray bundle
//have to calculate dimensions of the patch
//typedef pair<Point2f, int> pointAndPixel;
//typedef map<Point2f, pointAndPixel> lightfieldStruct;

struct lightfieldStructUnit {

	//Point2f position;
	Mat image;
	Matx34d pose;
	//vector<Point2f> corners;

};

class LightfieldClass {

public:
	int maxNumImages;
    int numImages;
	vector<lightfieldStructUnit> imagesAndPoses;
	
    Mat currImage;
    vector<Mat> images;
    Matx34d currPose;
	unsigned char*ImgDataSeq;
	vector<pair<int, double>> CurrFrameWeights;
	vector<Point3d> proxyData;
	vector <Matx34d> AllCameraMat;
    
    String fullPathData;

	Mat Camera_K; // Camera Intrinsic Matrix
	Mat discoeff;

	LightfieldClass(void);
	
    ~LightfieldClass(void);
	
	vector<pair<int, double>> GetWeights(Point3d proxyPoint, Point3d VirtualCameraLoc, vector<Point3d> AllCameraLocsP);
	vector<Matx33d> CalculateFundamentalMat(vector<Matx34d> allcameraMat, Matx34d curP);
	int kth;
	Mat RenderImage(vector<Point3d> proxyPoint, Point3d VirtualCameraLoc, vector<Point3d> AllCameraLocs, Matx33d VirtualRot, Vec3d VirtualTrans);
//	Mat DrawImage(xform xf);
    int DrawImage(Point3d vCameraLoc, Matx33d vP_rot, Vec3d vP_trans);
	Mat InterpolateRenderImage(Mat Img, vector<Vec2d> proxy2DPoint);
	int proxyWidth;
	int proxyHeight;

	//int calculateHomography(Mat& img_object, Mat& img_scene, Mat & H);
	//int getTheData(void);
	//int makeTheFrame(void);
	//int poseFromHomography(const Mat& H, Mat& pose);

	int findImageFromPose(void);
    
    


};



#endif
