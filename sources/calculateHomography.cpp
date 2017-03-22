#include <vector>
#include <string>
#include <iostream>
#include <stdio.h>

#include "LightfieldClass.h"

#include "opencv2/imgcodecs.hpp"
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/calib3d.hpp"

using namespace std;
using namespace cv;

//helper function to find rotation
Mat buildA(Matx<double, 3, 3> &R, vector<pair<Vec3d, Vec3d>> dirs)
{
	int pointCount = dirs.size;
	Mat A(pointCount, 3, DataType<double>::type);
	Vec3d *a = (Vec3d *)A.data;
	for (int i = 0; i < pointCount; i++)
	{
		a[i] = (dirs[i].first).cross(R * (dirs[i].second));
		double length = norm(a[i]);
		if (length == 0.0)
		{
			CV_Assert(false);
		}
		else
		{
			a[i] *= (1.0 / length);
		}
	}
	return A;
}

void eval(Vec3f &X, Mat &residues, Mat &weights, vector<pair<Vec3d, Vec3d>> dirs)
{

	//For every pair of image points observed in both images, we compute the 
	//directions of their corresponding rays in world coordinates and store them in a 2d-array dirs

	Matx<double, 3, 3> R2Ref = eulerAnglesToRotationMatrix(X); // Map the 3x1 euler angle to a rotation matrix
	Mat A = buildA(R2Ref, dirs); // Compute the A matrix that measures the distance between ray pairs
	Vec3d c;
	Mat cMat(c, false);
	SVD::solveZ(A, cMat); // Find the optimum camera centre of the second camera at distance 1 from the first camera
	residues = A*cMat; // Compute the  output vector whose length we are minimizing
	weights.setTo(1.0);
}


// Calculates rotation matrix given euler angles.
Mat eulerAnglesToRotationMatrix(Vec3f &theta)
{
	// Calculate rotation about x axis
	Mat R_x = (Mat_<double>(3, 3) <<
		1, 0, 0,
		0, cos(theta[0]), -sin(theta[0]),
		0, sin(theta[0]), cos(theta[0])
		);

	// Calculate rotation about y axis
	Mat R_y = (Mat_<double>(3, 3) <<
		cos(theta[1]), 0, sin(theta[1]),
		0, 1, 0,
		-sin(theta[1]), 0, cos(theta[1])
		);

	// Calculate rotation about z axis
	Mat R_z = (Mat_<double>(3, 3) <<
		cos(theta[2]), -sin(theta[2]), 0,
		sin(theta[2]), cos(theta[2]), 0,
		0, 0, 1);


	// Combined rotation matrix
	Mat R = R_z * R_y * R_x;

	return R;

}

/**
 * Function that calculates the homography from the first image to the second.
 * 
 * TODO- when there is no match, need to return FAILURE
 **/
int LightFieldClass::calculateHomography(Mat& img_object, Mat& img_scene, Mat& H) {

	//-- Step 1: Detect the keypoints using ORB Detector, compute the descriptors
	int minHessian = 400;

	std::vector<KeyPoint> keypoints_1, keypoints_2;
	Mat descriptors_1, descriptors_2;

	// Initiate ORB detector
	Ptr<FeatureDetector> detector = ORB::create(minHessian);

	// find the keypoints and descriptors with ORB
	detector->detect(img_object, keypoints_1);
	detector->detect(img_scene, keypoints_2);

	Ptr<DescriptorExtractor> extractor = ORB::create();
	extractor->compute(img_object, keypoints_1, descriptors_1);
	extractor->compute(img_object, keypoints_1, descriptors_2);

	// Flann needs the descriptors to be of type CV_32F
	descriptors_1.convertTo(descriptors_1, CV_32F);
	descriptors_2.convertTo(descriptors_2, CV_32F);

	std::vector<std::vector<cv::DMatch>> matches;
	cv::BFMatcher matcher;
	matcher.knnMatch(descriptors_1, descriptors_2, matches, 10);  // Find two nearest matches
																 //look whether the match is inside a defined area of the image
																 //only 25% of maximum of possible distance
	double tresholdDist = 0.25 * 
		sqrt(double(img_object.size().height*img_object.size().height +
					img_object.size().width*img_object.size().width));

	vector< DMatch > good_matches;
	good_matches.reserve(matches.size());
	for (size_t i = 0; i < matches.size(); ++i)
	{
		for (int j = 0; j < matches[i].size(); j++)
		{
			Point2f from = keypoints_1[matches[i][j].queryIdx].pt;
			Point2f to = keypoints_2[matches[i][j].trainIdx].pt;

			//calculate local distance for each possible match
			double dist = sqrt((from.x - to.x) * (from.x - to.x) + (from.y - to.y) * (from.y - to.y));

			//save as best match if local distance is in specified area and on same height
			if (dist < tresholdDist && abs(from.y - to.y)<5)
			{
				good_matches.push_back(matches[i][j]);
				j = matches[i].size();
			}
		}
	}
	
	Mat img_matches;
	drawMatches(img_object, keypoints_1, img_scene, keypoints_2,
		good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
		vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

	/*for (int i = 0; i < (int)good_matches.size(); i++){
		printf("-- Good Match [%d] Keypoint 1: %d  -- Keypoint 2: %d  \n", i, good_matches[i].queryIdx, good_matches[i].trainIdx);
	}

	*/

	//-- Show detected matches
	imshow("Good Matches", img_matches);
	waitKey(0);

	//-- Localize the object
	vector<Point2f> obj;
	vector<Point2f> scene;

	for (size_t i = 0; i < good_matches.size(); i++) {
		//-- Get the keypoints from the good matches
		obj.push_back(keypoints_1[good_matches[i].queryIdx].pt);
		scene.push_back(keypoints_2[good_matches[i].trainIdx].pt);
	}
	H = findHomography(obj, scene, RANSAC);

	/**





	//pos is the position of the camera expressed in the global frame (the same frame the 
	//objectPoints are expressed in). R is an attitude matrix DCM which is a good form 
	//to store the attitude in. 
	_, rVec, tVec = cv2.solvePnP(objectPoints, imagePoints, cameraMatrix, distCoeffs = NULL)
	Rt = cv2.Rodrigues(rvec)
	R = Rt.transpose()
	pos = -R * tVec
	
	*/

	
	return 0;

}