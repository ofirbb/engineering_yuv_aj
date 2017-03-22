#import <opencv2/videoio/cap_ios.h>
#import <UIKit/UIKit.h>


@interface Camera : NSObject <CvVideoCameraDelegate>

@property (nonatomic, strong) CvVideoCamera* videoCamera;

- (instancetype)initWithCameraView:(UIImageView *)view;

- (void)startCapture;
- (void)stopCapture;

@end
