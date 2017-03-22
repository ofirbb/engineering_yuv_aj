#import "Camera.h"
#import "UIImage+OpenCV.h"


@implementation Camera

- (instancetype)initWithCameraView:(UIImageView *)view{
    self = [super init];
    if (self) {
        self.videoCamera = [[CvVideoCamera alloc] initWithParentView:view];
        self.videoCamera.defaultAVCaptureDevicePosition = AVCaptureDevicePositionBack;
        self.videoCamera.defaultAVCaptureSessionPreset = AVCaptureSessionPreset640x480;
        self.videoCamera.defaultAVCaptureVideoOrientation = AVCaptureVideoOrientationPortrait;
        self.videoCamera.defaultFPS = 30;
        self.videoCamera.grayscaleMode = NO;
        self.videoCamera.delegate = self;
   
        
        /**NSString *faceCascadePath = [[NSBundle mainBundle] pathForResource:@"haarcascade_frontalface_alt2"
                                                                    ofType:@"xml"];
        
        const CFIndex CASCADE_NAME_LEN = 2048;
        char *CASCADE_NAME = (char *) malloc(CASCADE_NAME_LEN);
        CFStringGetFileSystemRepresentation( (CFStringRef)faceCascadePath, CASCADE_NAME, CASCADE_NAME_LEN);
        
        _faceDetector.load(CASCADE_NAME);
        
        free(CASCADE_NAME);*/
        
        //        NSString *eyesCascadePath = [[NSBundle mainBundle] pathForResource:@"haarcascade_eye_tree_eyeglasses"
        //                                                                    ofType:@"xml"];
        //
        //        CFStringGetFileSystemRepresentation( (CFStringRef)eyesCascadePath, CASCADE_NAME, CASCADE_NAME_LEN);
        //
        //        eyesDetector.load(CASCADE_NAME);
    }
    
    return self;
}


- (void)startCapture {
    [self.videoCamera start];
}

- (void)stopCapture; {
    [self.videoCamera stop];
}

- (void)processImage:(cv::Mat&)image {
    // Do some OpenCV stuff with the image

    //[self detectAndDrawFacesOn:image scale:_scale];
}
    

@end
