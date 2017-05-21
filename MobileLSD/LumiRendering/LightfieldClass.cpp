#include "LightfieldClass.h"

bool paircomp(pair<int, double> p, pair<int, double> q) { return (p.second < q.second); }

LightfieldClass::~LightfieldClass(void){
    delete[] ImgDataSeq;
    ImgDataSeq = nullptr;
}

LightfieldClass::LightfieldClass(void) : ImgDataSeq(NULL)
, kth(0)
, proxyWidth(0)
, maxNumImages(100)
, numImages(0)

{
    this->ImgDataSeq = (unsigned char*)malloc(sizeof(unsigned char) * IMAGE_RESOLUTION_X * IMAGE_RESOLUTION_Y * maxNumImages * 3);
    if(this->ImgDataSeq == nullptr) {
        std::cout << "error: malloc" << std::endl;
    }

	this->kth = 1;

    //cv::fisheye::calibrate for iPad2
    float fx = 1.1816992757731507e+03;
    float fy = 3.3214250594664935e+02;
    float cy = 0.0;
    float cx = 0.0;
    this->Camera_K = (Mat1d(3, 3) << fx, 0, cx, 0, fy, cy, 0, 0, 1);
    
	//Set parameters
	proxyWidth = 5;
	proxyHeight = 5;

	//Proxy info-DONE
	for (int j = 0; j < proxyHeight; ++j) {
		for (int k = 0; k < proxyWidth; ++k) {
			int z = 10;
            cv::Point3d tempProxy;
			tempProxy.x = k;
			tempProxy.y = j;
			tempProxy.z = z;
			proxyData.push_back(tempProxy);
		}
	}
        
}




void printMat(cv::Mat mat) {
    std::cout << "printing mat" << std::endl;
    std::cout << mat << std::endl;
}

vector<pair<int, double>> LightfieldClass::GetWeights(Point3d proxyPoint, Point3d VirtualCameraLoc,
													  vector<Point3d> AllCameraLocs)
{
	vector<pair<int, double>> weights;
	vector<pair<int, double>> penalities;


    cv::Point3d VirCamVec = VirtualCameraLoc - proxyPoint;
	double dist_VirCam = norm(VirCamVec);
        
	for (unsigned int i = 0; i < AllCameraLocs.size(); i++)
	{
		Vec3d curpenalities;

		//Angle Penalty: Compute Angles
        cv::Point3d CurCamVec = AllCameraLocs[i] - proxyPoint;
		double dist_CurCam = norm(CurCamVec);
		double curangle = CurCamVec.dot(VirCamVec);
		curangle = curangle / (dist_VirCam * dist_CurCam);
		curangle = acos(curangle);
		curpenalities[0] = (curangle);

		//Resolution Penalty
		curpenalities[1] = (dist_CurCam - dist_CurCam > 0) ? (dist_CurCam - dist_CurCam) : 0;

		//Field of View Penalty
		// project to current camera plane
		//decomposition

		Matx33d R(AllCameraMat[i](0, 0), AllCameraMat[i](0, 1), AllCameraMat[i](0, 2),
			AllCameraMat[i](1, 0), AllCameraMat[i](1, 1), AllCameraMat[i](1, 2),
			AllCameraMat[i](2, 0), AllCameraMat[i](2, 1), AllCameraMat[i](2, 2));

		//Vec3d T;
		Mat T(3, 1, cv::DataType<double>::type);
        vector<cv::Point3d> tempProxy;

		T.at<double>(0, 0) = AllCameraMat[i](0, 3);
		T.at<double>(1, 0) = AllCameraMat[i](1, 3);
		T.at<double>(2, 0) = AllCameraMat[i](2, 3);
        //print T
		tempProxy.push_back(proxyPoint);
		//Vec2d curimgPoint;
        vector<cv::Point2d> curimgPoint;
		//decomposeProjectionMatrix(AllCameraMat[i], K, R, T);
		Mat_<double> K = this->Camera_K;
		//Mat K(3, 3, cv::DataType<double>::type);
		//K = Camera_K.clone();
        

		cv::Mat rvecR(3, 1, cv::DataType<double>::type);//rodrigues rotation matrix
		cv::Rodrigues(R, rvecR);
		try
		{
            /**
            C++: void projectPoints(InputArray objectPoints, InputArray rvec, InputArray tvec, InputArray cameraMatrix, InputArray distCoeffs, OutputArray imagePoints, OutputArray jacobian=noArray(), double aspectRatio=0 )*/
            //print mats
            printMat(rvecR);
            printMat(T);
            projectPoints(tempProxy, rvecR, T, K, this->discoeff, curimgPoint);
		}
		catch (cv::Exception & e)
		{
			cout << e.msg << endl;
		}
		//int test = 1;
		if (curimgPoint[0].x > IMAGE_RESOLUTION_X || curimgPoint[0].y > IMAGE_RESOLUTION_Y)
			curpenalities[2] = DBL_MAX_EXP;
		else
			curpenalities[2] = 0;

		//total penalty
		double totalcurpenalty;
		double alpha = 0.4, beta = 0.3, gamma = 0.3;
		totalcurpenalty = alpha * curpenalities[0] + 
						  beta * curpenalities[1] + 
						  gamma * curpenalities[2];

		penalities.push_back(make_pair(i, totalcurpenalty));
	}

	// find the first kth smallest element in penalities
	nth_element(penalities.begin(), penalities.begin() + kth, penalities.end(), paircomp);
	double totalweights = 0;
	for (int i = 0; i < this->kth; i++)
	{
		weights.push_back(penalities[i]);
		weights[i].second = 1 - weights[i].second / 
							(penalities.begin() + this->kth)->second;
		totalweights += weights[i].second;
	}
	//normalize
	for (int i = 0; i < this->kth; i++)
	{
		if (totalweights != 0)
		{
			weights[i].second /= totalweights;
		}
		else
		{
			int test = 1;
		}
	}
	return weights;
}

vector<Matx33d> LightfieldClass::CalculateFundamentalMat(vector<Matx34d> allcameraMat, 
														 Matx34d curP)
{
	vector<Matx33d> F;
	Matx41d C(0, 0, 0, 1);
	for (unsigned int i = 0; i<allcameraMat.size(); i++)
	{
		Matx33d temp_F;
		Matx31d P2C = allcameraMat[i] * C;
		Matx33d P2Cx(0, -P2C(2, 0), P2C(1, 0),
			P2C(2, 0), 0, -P2C(0, 0),
			-P2C(1, 0), P2C(0, 0), 0);
		SVD svd(allcameraMat[i]);
		Mat_<double> pinvP(4, 3);
		pinvP = svd.vt.t() * Mat::diag(1. / svd.w)*svd.u.t();
		Matx43d temp_P(pinvP(0, 0), pinvP(0, 1), pinvP(0, 2),
			pinvP(1, 0), pinvP(1, 1), pinvP(1, 2),
			pinvP(2, 0), pinvP(2, 1), pinvP(2, 2),
			pinvP(3, 0), pinvP(3, 1), pinvP(3, 2));

		temp_F = P2Cx * curP * temp_P;
		F.push_back(temp_F);
	}
	return F;
}


Mat LightfieldClass::RenderImage(vector<cv::Point3d> proxyPoint, cv::Point3d VirtualCameraLoc,
							     vector<cv::Point3d> AllCameraLocs, Matx33d VirtualRot,
								 Vec3d VirtualTrans)
{
	//Mat Img(s_height,t_width,CV_32FC3);
	Mat Img(IMAGE_RESOLUTION_Y, IMAGE_RESOLUTION_X, CV_8UC3);
	vector<Vec2d> ProxyProjectPoint;

	for (unsigned int numP = 0; numP<proxyPoint.size(); numP++)
	{
		CurrFrameWeights = this->GetWeights(proxyPoint[numP], VirtualCameraLoc, AllCameraLocs);
		double addWeightInten_R = 0;
		double addWeightInten_G = 0;
		double addWeightInten_B = 0;
        Mat K = this->Camera_K; //AJB
        printMat(this->Camera_K);
		for (int i = 0; i<this->kth; i++)
		{
            printMat(K);
			//vector<Point2d> curimgPoint;
			int Num = CurrFrameWeights[i].first;

			Matx33d R(AllCameraMat[i](0, 0), AllCameraMat[i](0, 1), AllCameraMat[i](0, 2),
				AllCameraMat[i](1, 0), AllCameraMat[i](1, 1), AllCameraMat[i](1, 2),
				AllCameraMat[i](2, 0), AllCameraMat[i](2, 1), AllCameraMat[i](2, 2));
			Mat T(3, 1, cv::DataType<double>::type);
			vector<cv::Point3d> tempProxy;

			T.at<double>(0, 0) = AllCameraMat[i](0, 3);
			T.at<double>(1, 0) = AllCameraMat[i](1, 3);
			T.at<double>(2, 0) = AllCameraMat[i](2, 3);
			tempProxy.push_back(proxyPoint[numP]);
			//Vec2d curimgPoint;
			vector<cv::Point2d> curimgPoint;
			//decomposeProjectionMatrix(AllCameraMat[i], K, R, T);
			
			cv::Mat rvecR(3, 1, cv::DataType<double>::type);//rodrigues rotation matrix
			cv::Rodrigues(R, rvecR);
            Mat K = this->Camera_K;
			//projectPoints((Vec3d)proxyPoint[numP], K, R, T, NULL, curimgPoint);
			projectPoints(tempProxy, rvecR, T, K, this->discoeff, curimgPoint);
			if (curimgPoint[0].x > IMAGE_RESOLUTION_X || curimgPoint[0].y > IMAGE_RESOLUTION_Y)
				continue;

			unsigned char curimgIntensity_R = this->ImgDataSeq[IMAGE_RESOLUTION_X * IMAGE_RESOLUTION_Y * 3 * Num +
                                                               (int)curimgPoint[0].y * IMAGE_RESOLUTION_X * 3 +
                                                               (int)curimgPoint[0].x];
			unsigned char curimgIntensity_G = this->ImgDataSeq[IMAGE_RESOLUTION_X * IMAGE_RESOLUTION_Y * 3 * Num +
                                                               (int)curimgPoint[0].y * IMAGE_RESOLUTION_X * 3 +
                                                               (int)curimgPoint[0].x + 1];
			unsigned char curimgIntensity_B = this->ImgDataSeq[IMAGE_RESOLUTION_X * IMAGE_RESOLUTION_Y * 3 * Num +
                                                               (int)curimgPoint[0].y * IMAGE_RESOLUTION_X * 3 +
                                                               (int)curimgPoint[0].x + 2];
			addWeightInten_R += curimgIntensity_R * CurrFrameWeights[i].second;
			addWeightInten_G += curimgIntensity_G * CurrFrameWeights[i].second;
			addWeightInten_B += curimgIntensity_B * CurrFrameWeights[i].second;
		}

		vector<cv::Point2d> virtualPoint;
		vector<cv::Point3d> tempProxy;
		tempProxy.push_back(proxyPoint[numP]);

		Mat vR(3, 1, cv::DataType<double>::type);//rodrigues rotation matrix
		Rodrigues(VirtualRot, vR);
		Mat vT(3, 1, cv::DataType<double>::type);
		vT.at<double>(0, 0) = VirtualTrans[0];
		vT.at<double>(1, 0) = VirtualTrans[1];
		vT.at<double>(2, 0) = VirtualTrans[2];
		//int Num = CurrFrameWeights[i].first;
		//decomposeProjectionMatrix(VirtualP, K1, R1, T1);
		//projectPoints((tempProxy, this->Camera_K, VirtualRot, VirtualTrans, this->discoeff, virtualPoint);
		projectPoints(tempProxy, vR, vT, K, this->discoeff, virtualPoint);
        
        printMat(vR);
        printMat(vT);
        printMat(K);
        printMat(this->discoeff);
        
		if (virtualPoint[0].x > IMAGE_RESOLUTION_X || virtualPoint[0].y > IMAGE_RESOLUTION_Y)
			continue;
		//assign intesity to Img
		try
		{
			*(Img.data + Img.step[0] * (int)virtualPoint[0].y + Img.step[1] * (int)virtualPoint[0].x) = addWeightInten_R;
			*(Img.data + Img.step[0] * (int)virtualPoint[0].y + Img.step[1] * (int)virtualPoint[0].x + 1) = addWeightInten_G;
			*(Img.data + Img.step[0] * (int)virtualPoint[0].y + Img.step[1] * (int)virtualPoint[0].x + 2) = addWeightInten_B;
			/*Img.at<float>((int)virtualPoint[0].y,(int)virtualPoint[0].x,1) = addWeightInten_R;
			Img.at<float>((int)virtualPoint[0].y,(int)virtualPoint[0].x,2) = addWeightInten_G;
			Img.at<float>((int)virtualPoint[0].y,(int)virtualPoint[0].x,3) = addWeightInten_B;*/

		}
		catch (cv::Exception & e)
		{
			cout << e.msg << endl;
		}
		Vec2d myPoint;
		myPoint[0] = virtualPoint[0].y;
		myPoint[1] = virtualPoint[0].x;
        std::cout <<"x: " << virtualPoint[0].x << std::endl;
        std::cout <<"y: " << virtualPoint[0].y << std::endl;
        
		ProxyProjectPoint.push_back(myPoint);
	}
	Mat RenderedImg = this->InterpolateRenderImage(Img, ProxyProjectPoint);
	//cvNamedWindow("test2");
	//imshow("test2",Img);
	return RenderedImg;
}



Mat LightfieldClass::InterpolateRenderImage(Mat Img, vector<Vec2d> proxy2DPoint)
{

	for (unsigned int numP = 0; numP<proxy2DPoint.size() - 2; numP++)
	{
		int Width = numP + proxyWidth;
		if ((numP + proxyWidth) > proxy2DPoint.size() - 2)
		{
			Width = (int)proxy2DPoint.size() - 2;
		}
		for (int y = proxy2DPoint[numP][0]; y<proxy2DPoint[Width][0]; y++)
		{
			for (int x = proxy2DPoint[numP][1]; x<proxy2DPoint[numP + 1][1]; x++)
			{
				double x1 = proxy2DPoint[numP][1];
				double y1 = proxy2DPoint[numP][0];
				double x2 = proxy2DPoint[numP + 1][1];
				double y2 = proxy2DPoint[numP + 1][0];
				double x3 = proxy2DPoint[Width][1];
				double y3 = proxy2DPoint[Width][0];
				double x4 = proxy2DPoint[Width + 1][1];
				double y4 = proxy2DPoint[Width + 1][0];
				double Q11 = *(Img.data + Img.step[0] * (int)y1 + Img.step[1] * (int)x1);
				double Q12 = *(Img.data + Img.step[0] * (int)y2 + Img.step[1] * (int)x2);
				double Q21 = *(Img.data + Img.step[0] * (int)y3 + Img.step[1] * (int)x3);
				double Q22 = *(Img.data + Img.step[0] * (int)y4 + Img.step[1] * (int)x4);
				double k_12 = (y2 - y1) / (x2 - x1); // slope of upper and lower line
				double alpha = ((y - y3) - k_12 * (x - x3)) / ((y1 - y3) - k_12 * (x1 - x3)); //ratio between upper and lower
				double Qleft = alpha * Q11 + (1 - alpha) * Q21;// left point intensity
				double Qright = alpha * Q12 + (1 - alpha) * Q22; //right point intensity
				double k_13 = (y3 - y1) / (x3 - x1); // slope of left line
				double beta = ((y - y1) - k_13 * (x - x1)) / ((y2 - y1) - k_13 * (x2 - x1)); // ratio between left and right
				int P = (1 - beta) * Qleft + beta * Qright; // final result
															//				double R1 = (double)(x2-x)/(x2-x1)*Q11 + (double)(x-x1)/(x2-x1)*Q12;
															//				double R2 = (double)(x4-x)/(x4-x3)*Q21 + (double)(x-x3)/(x4-x3)*Q22;
															//				int P = (double)(y3-y)/(y3-y1)*R1 + (double)(y-y2)/(y4-y2)*R2;
				*(Img.data + Img.step[0] * (int)y + Img.step[1] * (int)x) = (unsigned char)P;
				*(Img.data + Img.step[0] * (int)y + Img.step[1] * (int)x + 1) = (unsigned char)P;
				*(Img.data + Img.step[0] * (int)y + Img.step[1] * (int)x + 2) = (unsigned char)P;
				//Img.data[Img.step*(int)y + Img.channels()*(int)x] = 255;
				//Img.data[Img.step*(int)y + Img.channels()*(int)x + 1] = 255;

			}
		}
	}
	cv::Size size(1024, 768);
	Mat test;
	resize(Img, test, size);
	//cvNamedWindow("test3");
	//imshow("test3", test);

	return Img;
}


//Mat LightfieldClass::DrawImage(xform xf)
int LightfieldClass::DrawImage(Point3d vCameraLoc, Matx33d vP_rot, Vec3d vP_trans)
{
//	Point3d vCameraLoc;
//	vCameraLoc.x = xf[12];
//	vCameraLoc.y = xf[13];
//	vCameraLoc.z = xf[14];
//	Matx33d vP_rot(xf[0], xf[4], xf[8],
//		xf[1], xf[5], xf[9],
//		xf[2], xf[6], xf[10]);
//	Vec3d vP_trans;
//	vP_trans[0] = xf[3];
//	vP_trans[1] = xf[7];
//	vP_trans[2] = xf[11];
    
	vector<Point3d> AllCameraLocs;
	for (unsigned int i = 0; i<this->AllCameraMat.size(); i++)
	{
		Matx33d K, R;
		Vec3d T;
		Point3d vLoc;
		//decomposeProjectionMatrix(this->AllCameraMat[i], K, R, T);
		vLoc.x = AllCameraMat[i](0, 3);
		vLoc.y = AllCameraMat[i](1, 3);
		vLoc.z = AllCameraMat[i](2, 3);
		AllCameraLocs.push_back(vLoc);

	}

	Mat myImg = this->RenderImage(this->proxyData, vCameraLoc, AllCameraLocs, 
								  vP_rot, vP_trans);
	cv::Size size(1024, 768);
	Mat test;
	resize(myImg, test, size);
//	imshow("image", test);
//	waitKey(0);
//	destroyAllWindows();
//	//cvNamedWindow("test1");
	//imshow("test1", test);

    currImage = myImg;
	return SUCCESS;

}

