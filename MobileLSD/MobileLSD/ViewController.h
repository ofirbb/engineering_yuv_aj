// All rights reserved.

#import <opencv2/videoio/cap_ios.h>

#import <UIKit/UIKit.h>

using namespace cv;

@interface ViewController : UIViewController<CvVideoCameraDelegate> {
  CvVideoCamera *videoCamers;
}

@property (nonatomic, retain) CvVideoCamera *videoCamera;


@end
