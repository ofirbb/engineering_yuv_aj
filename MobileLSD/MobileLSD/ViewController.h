//
//  ViewController.h
//  MobileLSD
//
//  Created by Guanhang Wu on 11/24/16.
//  Copyright © 2016 Guanhang Wu. All rights reserved.
//

#include <opencv2/videoio/cap_ios.h>
#include "LightfieldClass.h"
#import <UIKit/UIKit.h>


//#import <opencv2/highgui/ios.h>
//#import <opencv2/highgui/cap_ios.h>
//#import <opencv2/opencv.hpp>
//#import <opencv2/imgcodecs/ios.h>

using namespace cv;

@interface ViewController : UIViewController<CvVideoCameraDelegate>
{
    
    CvVideoCamera *videoCamers;
}

@property (nonatomic, retain) CvVideoCamera *videoCamera;


@end

