//
//  ViewController.m
//  MobileLSD
//
//  Created by Guanhang Wu on 11/24/16.
//  Copyright © 2016 Guanhang Wu. All rights reserved.
//
#import "ViewController.h"
#import "RenderViewController.h"

#include <boost/thread.hpp>
#include "util/settings.h"
#include "util/globalFuncs.h"
#include "opencv2/opencv.hpp"
#include <iostream>
#include "SlamSystem.h"

#include "SlamSystem.h"
#include "LightfieldClass.h"


@interface ViewController (){
    UIImageView *imageView_; // Setup the image view
    UIImageView *arrowView; // Setup the arrows
    NSArray *arrows;
    //UITextView *fpsView_; // Display the current FPS
    //UITextView *avgfpsView_; // Display the current FPS
    //UITextView *translation_; // Display the current FPS
    int64 curr_time_; // Store the current time
    lsd_slam::SlamSystem* system_;
    //AJB-added
    LightfieldClass* lightfield_;
    //float max_fps_;
    //float avg_fps_;
    int cam_width;
    int cam_height;
    int runningIdx_;
    int count_;
    //cv::Mat depthMap;
    //cv::Mat displayImage;
    //cv::Mat colorImage;
    UIButton *resetButton_;
    UIButton *renderButton_;
    int skip_frame_; //buffer size for the camera to start
}
@end

@implementation ViewController

@synthesize videoCamera;

void getK(Sophus::Matrix3f& K){
    
    //cv::fisheye::calibrate for iPad2
    float fx = 1.1816992757731507e+03;
    float fy = 3.3214250594664935e+02;
    float cx = 0;
    float cy = 0;
    K << fx, 0.0, cx, 0.0, fy, cy, 0.0, 0.0, 1.0;
    
}

- (void)viewDidLoad {
    
    
    [super viewDidLoad];
    
    // Do any additional setup after loading the view, typically from a nib.
    
    //background color black
    UIColor *myColor = [UIColor colorWithRed:(0.0 / 255.0) green:(0.0 / 255.0) blue:(0.0 / 255.0) alpha: 1];
    self.view.backgroundColor = myColor;
    
    int arrow_height = 70;
    int arrow_width = 70;
    int x = self.view.frame.size.width/2 - 35;
    int y = self.view.frame.size.height/2 - 35;

    //add initial arrow to the app
    arrowView = [[UIImageView alloc] initWithFrame:CGRectMake(x, y, arrow_height, arrow_width)];
    arrows = [[NSArray alloc] initWithObjects: @"arrow_up.png", @"arrow_down.png", @"arrow_left.png", @"arrow_right.png", @"arrow_down_left.png", @"arrow_down_right.png", @"arrow_up_left.png", @"arrow_up_right.png", nil];
    
    cam_width = 352;
    cam_height = 288;
    
    // Take into account_ size of camera input
    int view_width = self.view.frame.size.width;
    //int view_height = (int)(cam_height*self.view.frame.size.width/cam_width)/2;
    int view_height = self.view.frame.size.height;
    int offset = (self.view.frame.size.height - view_height)/2;
    
    imageView_ = [[UIImageView alloc] initWithFrame:CGRectMake(0.0, offset, view_width, view_height)];
    
    //[imageView_ setContentMode:UIViewContentModeScaleAspectFill]; (does not work)
    [self.view addSubview:imageView_]; // Add the view
    [self.view addSubview:arrowView];
    
    // Initialize the video camera
    self.videoCamera = [[CvVideoCamera alloc] initWithParentView:imageView_];
    self.videoCamera.delegate = self;
    self.videoCamera.defaultAVCaptureDevicePosition = AVCaptureDevicePositionBack;
    self.videoCamera.defaultAVCaptureVideoOrientation = AVCaptureVideoOrientationLandscapeRight;
    self.videoCamera.defaultFPS = 10; // Set the frame rate
    //self.videoCamera.grayscaleMode = YES; // Get grayscale
    self.videoCamera.rotateVideo = YES; // Rotate video so everything looks correct
    
    // Choose these depending on the camera input chosen
    self.videoCamera.defaultAVCaptureSessionPreset = AVCaptureSessionPreset352x288;
    resetButton_ = [self simpleButton:@"Reset" buttonColor:[UIColor blackColor]];
    // Important part that connects the action to the member function buttonWasPressed
    [resetButton_ addTarget:self action:@selector(restartWasPressed) forControlEvents:UIControlEventTouchUpInside];
    
    renderButton_ = [self simpleButton:@"Render" buttonColor:[UIColor blackColor]];
    // Important part that connects the action to the member function buttonWasPressed
    [renderButton_ addTarget:self action:@selector(renderWasPressed) forControlEvents:UIControlEventTouchUpInside];
    
    runningIdx_ = 0;
    Sophus::Matrix3f K;
    getK(K);
    system_ = new lsd_slam::SlamSystem(cam_width, cam_height, K);
    lightfield_ = new LightfieldClass();
    
    count_ = 0;
    curr_time_ = cv::getTickCount();
    //max_fps_ = 0;
    //avg_fps_ = 0;
    skip_frame_ = 20;
    [videoCamera start];
    
}


- (void)restartWasPressed {
    [videoCamera stop];
    count_ = 0;
    runningIdx_ = 0;
    [videoCamera start];
}

- (void)renderWasPressed {
    [videoCamera stop];
    [self performSegueWithIdentifier:@"moveToRenderSegue" sender:self];
    
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (UIButton *) simpleButton:(NSString *)buttonName buttonColor:(UIColor *)color
    {
        UIButton *button = [UIButton buttonWithType:UIButtonTypeCustom]; // Initialize the button
        // Bit of a hack, but just positions the button at the bottom of the screen
        int button_width = 100; int button_height = 30; // Set the button height and width (heuristic)
        // Botton position is adaptive as this could run on a different device (iPAD, iPhone, etc.)
        int button_x; // Position of top-left of button
        int button_y;
        
        if ([buttonName isEqualToString:@"Reset"]) {
            
            button_x = (self.view.frame.size.width - button_width)/4;
            button_y = self.view.frame.size.height - 90;
            
        }
        else {
            button_x = 3 * (self.view.frame.size.width - button_width)/4;
            button_y = self.view.frame.size.height - 90;
            
        }
        
        button.layer.borderWidth = 1.0f;
        button.layer.borderColor = [[UIColor darkGrayColor] CGColor];
        button.layer.cornerRadius = 10;
        button.backgroundColor = [UIColor lightGrayColor];
        
        
        button.frame = CGRectMake(button_x, button_y, button_width, button_height); // Position the button
        [button setTitle:buttonName forState:UIControlStateNormal]; // Set the title for the button
        [button setTitleColor:color forState:UIControlStateNormal]; // Set the color for the title
        
        [self.view addSubview:button]; // Important: add the button as a subview
        //[button setEnabled:bflag]; [button setHidden:(!bflag)]; // Set visibility of the button
        return button; // Return the button pointer
    }


- (void) processImage:(cv:: Mat &)image
{

    
    
    if (count_ < skip_frame_){
        count_++;
        return;
    }
    
    
    if(runningIdx_ == 0 || !system_->trackingIsGood){
        if (runningIdx_ != 0) system_->reinit();
        system_->randomInit(image.data, curr_time_, runningIdx_);
    }
    else{
        std::cout<<"tracking"<<std::endl;
        system_->trackFrame(image.data, runningIdx_,true,curr_time_);
    }
    
    //display arrow
    dispatch_sync(dispatch_get_main_queue(), ^{
        arrowView.image = [UIImage imageNamed:[arrows objectAtIndex:arc4random_uniform((uint32_t)
            [arrows count])]];
    });
    
    auto sim3mat = system_->getSim3Mat();
    auto transmat = sim3mat.translation();
    auto rotmat = sim3mat.rxso3().rotationMatrix();
    
    lightfieldStructUnit newUnit;
    
//    system_->addNewDataMutex.lock();
//    if (lightfield_->numImages == lightfield_->maxNumImages) {
//        [self performSegueWithIdentifier:@"moveToRenderSegue" sender:self];
//        
//    }
//    
//    unsigned char *input = (unsigned char *)(image.data);
//    //int de = image.channels();
//    std::memcpy(lightfield_->ImgDataSeq + 3 * image.cols * image.rows + lightfield_->numImages,
//                input, 3 * image.cols * image.rows);
//    
//    Matx34d tempPose = Matx34d(rotmat(0, 0), rotmat(0, 1), rotmat(0, 2), transmat(0),
//                               rotmat(1, 0), rotmat(1, 1), rotmat(1, 2), transmat(1),
//                               rotmat(2, 0), rotmat(2, 1), rotmat(2, 2), transmat(2));
//    
//    lightfield_->AllCameraMat.push_back(tempPose);
//    lightfield_->images.push_back(image);
//    ++(lightfield_->numImages);
//    
//    lightfield_->currImage = image;
//    lightfield_->currPose = tempPose;
//    system_->addNewDataMutex.unlock();
    runningIdx_++;
    
    //Finally estimate the frames per second (FPS)
    int64 next_time = cv::getTickCount(); // Get the next time stamp
    curr_time_ = next_time; // Update the time
    
}
    
-(void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender{
    if([segue.identifier isEqualToString:@"moveToRenderSegue"]){
        RenderViewController *controller = [segue destinationViewController];
        //(RenderViewController*)segue.destinationViewController;
        controller.lightfield_= lightfield_;
    }
}
@end
