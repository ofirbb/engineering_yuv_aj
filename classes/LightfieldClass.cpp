#include "LightfieldClass.h"

using namespace std;
using namespace cv;
using namespace cv::xfeatures2d;


	/**
	*Function that adds the data from the image to the
	*lightfield.  Uses homography helper functions to find the
	*pose of the camera.  Then puts that information into the
	*lightfield. If there is already data there, then it takes the
	*average
	*/
	int LightFieldClass::getTheData(LightFieldClass * currField) {


		vector<Mat *>::iterator it = currField->samplingPath.begin();

		for (it; it != currField->samplingPath.end(); ++it) {

			Mat H;
			int res = FAILURE;
			int i;

			Mat * image = *it;

			for (i = 0; i < NUM_FRAMING_IMAGES; ++i) {
				res = calculateHomography(image, currField->frameImages.at(i), H);
				if (res == SUCCESS) {
					break;
				}
			}
			if (res == FAILURE) {
				return res;
			}

			//find the total homography of current image from the first frame
			//unclear if this matrix mult is in the correct order
			Mat totalH = currField->homographiesOfFrameImages.at(i) * H;
			Mat pose;

			res = poseFromHomography(totalH, pose);

			if (res == FAILURE) {
				return FAILURE;
			}

			////call rayFromImage function	
			//if (rayFromImage(currField, image, totalH, pose) == FAILURE) {
			//	return FAILURE;
			//}
		}
		return SUCCESS;
	}


/**
 * Function that calculates the homography from the first image to the second.
 * 
 * TODO- when there is no match, need to return FAILURE
 **/
int LightFieldClass::calculateHomography(Mat* img_object, Mat* img_scene, Mat & H) {

	//-- Step 1: Detect the keypoints and extract descriptors using SURF
	int minHessian = 400;
	Ptr<SURF> detector = SURF::create( minHessian );
	vector<KeyPoint> keypoints_object, keypoints_scene;
	Mat descriptors_object, descriptors_scene;
	detector->detectAndCompute( *img_object, Mat(), keypoints_object, descriptors_object );
	detector->detectAndCompute( *img_scene, Mat(), keypoints_scene, descriptors_scene );

	//-- Step 2: Matching descriptor vectors using FLANN matcher
	FlannBasedMatcher matcher;
	vector< DMatch > matches;
	matcher.match( descriptors_object, descriptors_scene, matches );
	double max_dist = 0; 
	double min_dist = 100; 

	//-- Quick calculation of max and min distances between keypoints
	for( int i = 0; i < descriptors_object.rows; i++ ){ 
		double dist = matches[i].distance;
		if( dist < min_dist ) min_dist = dist;
		if( dist > max_dist ) max_dist = dist;
	}

	//-- Draw only "good" matches (i.e. whose distance is less than 3*min_dist )
	vector< DMatch > good_matches;
	for( int i = 0; i < descriptors_object.rows; i++ ){ 
		if( matches[i].distance <= 3*min_dist ){ 
			good_matches.push_back( matches[i]); 
		}
	}

	//-- Localize the object
	vector<Point2f> obj;
	vector<Point2f> scene;

	for( size_t i = 0; i < good_matches.size(); i++ ){
		//-- Get the keypoints from the good matches
		obj.push_back(keypoints_object[ good_matches[i].queryIdx ].pt);
		scene.push_back(keypoints_scene[ good_matches[i].trainIdx ].pt);
	}
	H = (findHomography( obj, scene, RANSAC));

	return SUCCESS;

}


int LightFieldClass::makeTheFrame(LightFieldClass * currField) {

	Mat H;

	int res = calculateHomography(currField->frameImages.at(0), currField->frameImages.at(0), H);

	if (res == FAILURE) {
		std::cout << "failed to find homography of: first" << std::endl;
		return res;
	}
	currField->homographiesOfFrameImages.at(0) = H;

	for (int i = 1; i < NUM_FRAMING_IMAGES; ++i) {

		Mat H;
		//opencv
		res = calculateHomography(currField->frameImages.at(i),
			currField->frameImages.at(i - 1), H);

		if (res == FAILURE) {
			std::cout << "failed to find homography of: " << i << std::endl;
			return res;
		}
		currField->homographiesOfFrameImages.at(i) = H;
	}

	//now find total homographies.  first one is obviously 0. second one stays the same.	
	for (int j = 2; j < NUM_FRAMING_IMAGES; ++j) {

		//check that this matrix mult is in the correct order
		currField->homographiesOfFrameImages.at(j) =
			currField->homographiesOfFrameImages.at(j - 1) *
			currField->homographiesOfFrameImages.at(j);

	}

	//find total poses
	for (int k = 0; k < NUM_FRAMING_IMAGES; ++k) {
		Mat pose;
		if (poseFromHomography(currField->homographiesOfFrameImages.at(k), pose) == FAILURE) {
			return FAILURE;
		}
		currField->posesOfFrameImages.at(k) = pose;
	}

	return SUCCESS;

}

