//
//  RenderViewController.m
//  MobileLSD
//
//  Created by AJ Bruce on 4/12/17.
//  Copyright © 2017 Guanhang Wu. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "ViewController.h"
#import "RenderViewController.h"
#include <boost/thread.hpp>
#include "util/settings.h"
#include "util/globalFuncs.h"
#include "opencv2/opencv.hpp"
#include <iostream>
#include "SlamSystem.h"
#include "LightfieldClass.h"

#include <iostream>

#include "SlamSystem.h"
#include "LightfieldClass.h"



@interface RenderViewController () {
    //xform xf;
    UIButton *saveButton_;
    int renderNum;
    

}
@end

@implementation RenderViewController

@synthesize lightfield_;
@synthesize renderedImageView;

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
    }
    return self;
}


- (cv::Mat)cvMatFromUIImage:(UIImage *)image
{
    CGColorSpaceRef colorSpace = CGImageGetColorSpace(image.CGImage);
    CGFloat cols = image.size.width;
    CGFloat rows = image.size.height;
    
    cv::Mat cvMat(rows, cols, CV_8UC4); // 8 bits per component, 4 channels (color channels + alpha)
    
    CGContextRef contextRef = CGBitmapContextCreate(cvMat.data,                 // Pointer to  data
                                                    cols,                       // Width of bitmap
                                                    rows,                       // Height of bitmap
                                                    8,                          // Bits per component
                                                    cvMat.step[0],              // Bytes per row
                                                    colorSpace,                 // Colorspace
                                                    kCGImageAlphaNoneSkipLast |
                                                    kCGBitmapByteOrderDefault); // Bitmap info flags
    
    CGContextDrawImage(contextRef, CGRectMake(0, 0, cols, rows), image.CGImage);
    CGContextRelease(contextRef);
    
    
    Point2f src_center(cvMat.cols/2.0F, cvMat.rows/2.0F);
    cv::Mat rot_mat = getRotationMatrix2D(src_center, 180, 1.0);
    cv::Mat dst;
    warpAffine(cvMat, dst, rot_mat, cvMat.size());
    cvMat = dst;
    
    
    return cvMat;
}



-(UIImage *)UIImageFromCVMat:(cv::Mat)cvMat

{
    Point2f src_center(cvMat.cols/2.0F, cvMat.rows/2.0F);
    cv::Mat rot_mat = getRotationMatrix2D(src_center, 180, 1.0);
    cv::Mat dst;
    warpAffine(cvMat, dst, rot_mat, cvMat.size());
    cvMat = dst;
    
    NSData *data = [NSData dataWithBytes:cvMat.data length:cvMat.elemSize()*cvMat.total()];
    CGColorSpaceRef colorSpace;
    CGBitmapInfo bitmapInfo;
    if (cvMat.elemSize() == 1) {
        colorSpace = CGColorSpaceCreateDeviceGray();
        bitmapInfo = kCGImageAlphaNone | kCGBitmapByteOrderDefault;
        
    } else {
        colorSpace = CGColorSpaceCreateDeviceRGB();
        bitmapInfo = kCGBitmapByteOrder32Little | (cvMat.elemSize() == 3? kCGImageAlphaNone : kCGImageAlphaNoneSkipFirst);
    }
    CGDataProviderRef provider = CGDataProviderCreateWithCFData((__bridge CFDataRef)data);
    // Creating CGImage from cv::Mat
    CGImageRef imageRef = CGImageCreate(cvMat.cols,                 //width
                                        cvMat.rows,                 //height
                                        8,                          //bits per component
                                        8 * cvMat.elemSize(),       //bits per pixel
                                        cvMat.step[0],              //bytesPerRow
                                        colorSpace,                 //colorspace
                                        bitmapInfo,                 // bitmap info
                                        provider,                   //CGDataProviderRef
                                        NULL,                       //decode
                                        false,                      //should interpolate
                                        kCGRenderingIntentDefault   //intent
                                        );
    // Getting UIImage from CGImage
    UIImage *finalImage = [UIImage imageWithCGImage:imageRef];
    CGImageRelease(imageRef);
    CGDataProviderRelease(provider);
    CGColorSpaceRelease(colorSpace);
    return finalImage;
}




-(void)debugFunction {
    
    NSString* filePath = [[NSBundle mainBundle]
                          pathForResource:@"Building5" ofType:@"png"];
    UIImage* input1 = [UIImage imageWithContentsOfFile:filePath];
    
    cv::Mat src1 = [self cvMatFromUIImage:input1];
    std::cout << "width:1" << src1.rows << std::endl;
    std::cout << "height: " << src1.cols << std::endl;
    
    //resize image1
    cv::Size size(288, 352);//the dst image size,e.g.100x100
    cv::Mat image1;//dst image
    resize(src1,image1,size);//resize image
    
    
    Matx34d tempPose1 = Matx34d(1. ,0. ,0. ,0. ,0. ,1. ,0. ,0. ,0. ,0. ,1. ,0.);
    double x = tempPose1(0,3);
    double y = tempPose1(1,3);
    
    cout << x << endl;
    cout << y << endl;
    
    unsigned char *input = (unsigned char *)(image1.data);
    //int de = image.channels();
    std::memcpy(lightfield_->ImgDataSeq + 3 * image1.cols * image1.rows + lightfield_->numImages,
                input, 3 * image1.cols * image1.rows);
    
//    lightfield_->AllCameraMat.push_back(tempPose1);
//    lightfield_->images.push_back(image1);
//    lightfield_->numImages++;

    lightfieldStructUnit newUnit;
    newUnit.image = image1;
    newUnit.pose = tempPose1;
    lightfield_->imagesAndPoses.push_back(newUnit);

    
    NSString* filePath2 = [[NSBundle mainBundle]
                           pathForResource:@"Building6" ofType:@"png"];
    UIImage* input2 = [UIImage imageWithContentsOfFile:filePath2];
    cv::Mat src2 = [self cvMatFromUIImage:input2];
    
    //resize image2
    cv::Mat image2;//dst image
    resize(src2,image2,size);//resize image
    
    
    
    Matx34d tempPose2 = Matx34d(
                                9.9907816355645818e-001 ,-1.3467530285943021e-002,
                                4.0760872569797033e-002 ,-9.9748073436781359e-001,
                                1.3421995009699820e-002 ,9.9990895399863011e-001,
                                1.3905981897279823e-003 ,-1.6508926480737356e-002,
                                -4.0775889378572898e-002 ,-8.4222405741551628e-004,
                                9.9916796260890162e-001 ,6.8990143582258051e-002);
    
    input = (unsigned char *)(image2.data);
    //int de = image.channels();
    std::memcpy(lightfield_->ImgDataSeq + 3 * image2.cols * image2.rows + lightfield_->numImages,
                input, 3 * image1.cols * image1.rows);
    
    
    lightfield_->AllCameraMat.push_back(tempPose2);
    lightfield_->images.push_back(image2);
    ++(lightfield_->numImages);
    
    lightfield_->currImage = image1;
    lightfield_->currPose = tempPose1;
    
    lightfieldStructUnit newUnit2;
    newUnit2.image = image2;
    newUnit2.pose = tempPose2;
    lightfield_->imagesAndPoses.push_back(newUnit2);
    
    
    
    
}



- (void)viewDidLoad
{
    [super viewDidLoad];
    //[self debugFunction];
    
    
    //initially, set image to the most recent image
    [self updateRenderImage];
    renderNum = 0;
//    
//    saveButton_ = [self simpleButton:@"Save" buttonColor:[UIColor blackColor]];
//    // Important part that connects the action to the member function buttonWasPressed
//    [saveButton_ addTarget:self action:@selector(saveWasPressed) forControlEvents:UIControlEventTouchUpInside];


    UIPanGestureRecognizer *panGestureRecognizer = [[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(moveViewWithGestureRecognizer:)];
    [self.view addGestureRecognizer:panGestureRecognizer];
    
//    UITapGestureRecognizer *singleTapGestureRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(handleSingleTapGesture:)];
//    [self.view addGestureRecognizer:singleTapGestureRecognizer];
//    
//    UITapGestureRecognizer *doubleTapGestureRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(handleDoubleTapGesture:)];
//    doubleTapGestureRecognizer.numberOfTapsRequired = 2;
//    doubleTapGestureRecognizer.numberOfTouchesRequired = 2;
//    [self.view addGestureRecognizer:doubleTapGestureRecognizer];

}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)updateRenderImage {
    
    UIImage * image = [self UIImageFromCVMat:lightfield_->currImage];
    
    int height = self.view.frame.size.width;
    int width = self.view.frame.size.height;
    
    self.renderedImageView = [[UIImageView alloc] initWithFrame:CGRectMake(0,0,height, width)];
    self.renderedImageView.image = image;
    [self.view addSubview:renderedImageView];
    
    
    saveButton_ = [self simpleButton:@"Save" buttonColor:[UIColor blackColor]];
    // Important part that connects the action to the member function buttonWasPressed
    [saveButton_ addTarget:self action:@selector(saveWasPressed) forControlEvents:UIControlEventTouchUpInside];

    
}

- (UIButton *) simpleButton:(NSString *)buttonName buttonColor:(UIColor *)color
{
    UIButton *button = [UIButton buttonWithType:UIButtonTypeCustom]; // Initialize the button
    // Bit of a hack, but just positions the button at the bottom of the screen
    int button_width = 100; int button_height = 30; // Set the button height and width (heuristic)
    // Botton position is adaptive as this could run on a different device (iPAD, iPhone, etc.)
    int button_x = (self.view.frame.size.width - button_width)/2;

    int button_y = self.view.frame.size.height - 90;
    
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



- (void)saveWasPressed {
    UIImageWriteToSavedPhotosAlbum(renderedImageView.image, nil, nil, nil);
}


-(void)moveViewWithGestureRecognizer:(UIPanGestureRecognizer *)panGestureRecognizer{
   
    double movement = 0.2;
    
    double transX = 0;
    double transY = 0;
    
    
    CGPoint vel = [panGestureRecognizer velocityInView:self.view];
    if(vel.x > 0) {
        transX = movement;
    }
    else if (vel.x < 0){
        transX = -movement;
    }
    if(vel.y > 0) {
        transY = movement;
    }
    else if (vel.y < 0){
        transY = -movement;
    }
    

    Matx33d vP_rot = Matx33d(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
    Vec3d vP_trans = Vec3d(transX, transY, 0.0);
    Point3d vCameraLoc(0.0, 0.0, 0.0);
    lightfield_->currentTranslation[0] += transX;
    lightfield_->currentTranslation[1] += transY;
    
    cout << "translation: " << lightfield_->currentTranslation << endl;

    int res = lightfield_->DrawImage(vCameraLoc, vP_rot, lightfield_->currentTranslation);
    if(res == SUCCESS) {
        [self updateRenderImage];
    }
    else {
        std::cout << "DrawImage did not succeed" << std::endl;
    }
}

@end
