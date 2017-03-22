#include <vector>
#include <string>
#include <iostream>
#include "LightfieldClass.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

/**
*Function that adds the data from the image to the
*lightfield.  Uses homography helper functions to find the
*pose of the camera.  Then puts that information into the
*lightfield. If there is already data there, then it takes the
*average
*/
int LightFieldClass::makeTheFrame(void) {
	LightFieldClass * currField = this;
	Mat H;

	int res = calculateHomography(*(currField->frameImages.at(0)), 
								  *(currField->frameImages.at(0)), H);

	cout << H << endl;

	if (res == FAILURE) {
		std::cout << "failed to find homography of: first" << std::endl;

		return res;
	}
	currField->homographiesOfFrameImages.push_back(H);

	for (int i = 1; i < NUM_FRAMING_IMAGES; ++i) {

		Mat H;
		res = calculateHomography(*(currField->frameImages.at(i)),
								  *(currField->frameImages.at(i - 1)), H);

		if (res == FAILURE) {
			std::cout << "failed to find homography of: " << i << std::endl;
			return res;
		}
		currField->homographiesOfFrameImages.push_back(H);
	}

	//now find total homographies.  first one is obviously 0. second one stays the same.	
	for (int j = 2; j < NUM_FRAMING_IMAGES; ++j) {

		Mat * H = new Mat(currField->homographiesOfFrameImages.at(j - 1) *
			currField->homographiesOfFrameImages.at(j));
		
		vector<Point2f> imageCenterAndCorners(5);
		imageCenterAndCorners[0] = cvPoint(IMAGE_RESOLUTION_X / 2, IMAGE_RESOLUTION_Y / 2);
		imageCenterAndCorners[1] = cvPoint(0, 0);
		imageCenterAndCorners[2] = cvPoint(0, IMAGE_RESOLUTION_Y);
		imageCenterAndCorners[3] = cvPoint(IMAGE_RESOLUTION_X, 0);
		imageCenterAndCorners[4] = cvPoint(IMAGE_RESOLUTION_X, IMAGE_RESOLUTION_Y);

		vector<Point2f> cameraCenterAndCorners(5);
		perspectiveTransform(imageCenterAndCorners, cameraCenterAndCorners, *H);

		lightfieldStructUnit newUnit;
		//newUnit.position = cameraCenterAndCorners[0];
		newUnit.image = frameImages[j];
		newUnit.homography = H;

		//cameraCenterAndCorners.erase(cameraCenterAndCorners.begin());
		//newUnit.corners = cameraCenterAndCorners;

		currField->lightfield.push_back(newUnit);


		//check that this matrix mult is in the correct order
		currField->homographiesOfFrameImages.at(j) =
			currField->homographiesOfFrameImages.at(j - 1) *
			currField->homographiesOfFrameImages.at(j);

	}

	////find total poses
	//for (int k = 0; k < NUM_FRAMING_IMAGES; ++k) {
	//	Mat pose;
	//	if (poseFromHomography(currField->homographiesOfFrameImages.at(k), pose) == FAILURE) {
	//		return FAILURE;
	//	}
	//	currField->posesOfFrameImages.at(k) = pose;
	//}

	return SUCCESS;

}