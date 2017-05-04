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
#include "Xform.h"


@interface RenderViewController () {
    xform xf;
    UIButton *saveButton_;
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



-(UIImage *)UIImageFromCVMat:(cv::Mat)cvMat
{
    Point2f src_center(cvMat.cols/2.0F, cvMat.rows/2.0F);
    cv::Mat rot_mat = getRotationMatrix2D(src_center, 180, 1.0);
    cv::Mat dst;
    warpAffine(cvMat, dst, rot_mat, cvMat.size());
    
    cvMat = dst;
    
    NSData *data = [NSData dataWithBytes:cvMat.data length:cvMat.elemSize()*cvMat.total()];
    CGColorSpaceRef colorSpace;
    
    if (cvMat.elemSize() == 1) {
        colorSpace = CGColorSpaceCreateDeviceGray();
    } else {
        colorSpace = CGColorSpaceCreateDeviceRGB();
    }
    
    CGDataProviderRef provider = CGDataProviderCreateWithCFData((__bridge CFDataRef)data);
    
    // Creating CGImage from cv::Mat
    CGImageRef imageRef = CGImageCreate(cvMat.cols, cvMat.rows, 8, 8 * cvMat.elemSize(), cvMat.step[0], colorSpace, kCGImageAlphaNone|kCGBitmapByteOrderDefault, provider, NULL, false, kCGRenderingIntentDefault);
    
    
    // Getting UIImage from CGImage
    UIImage *finalImage = [UIImage imageWithCGImage:imageRef];
    CGImageRelease(imageRef);
    CGDataProviderRelease(provider);
    CGColorSpaceRelease(colorSpace);
    
    return finalImage;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    //initially, set image to the most recent image
    [self updateRenderImage];
    
    saveButton_ = [self simpleButton:@"Save" buttonColor:[UIColor blackColor]];
    // Important part that connects the action to the member function buttonWasPressed
    [saveButton_ addTarget:self action:@selector(saveWasPressed) forControlEvents:UIControlEventTouchUpInside];


    UIPanGestureRecognizer *panGestureRecognizer = [[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(moveViewWithGestureRecognizer:)];
    [self.renderedImageView addGestureRecognizer:panGestureRecognizer];
    
    UITapGestureRecognizer *singleTapGestureRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(handleSingleTapGesture:)];
    [self.renderedImageView addGestureRecognizer:singleTapGestureRecognizer];
    
    UITapGestureRecognizer *doubleTapGestureRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(handleDoubleTapGesture:)];
    doubleTapGestureRecognizer.numberOfTapsRequired = 2;
    doubleTapGestureRecognizer.numberOfTouchesRequired = 2;
    [self.renderedImageView addGestureRecognizer:doubleTapGestureRecognizer];

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
    
    
}

- (UIButton *) simpleButton:(NSString *)buttonName buttonColor:(UIColor *)color
{
    UIButton *button = [UIButton buttonWithType:UIButtonTypeCustom]; // Initialize the button
    // Bit of a hack, but just positions the button at the bottom of the screen
    int button_width = 100; int button_height = 30; // Set the button height and width (heuristic)
    // Botton position is adaptive as this could run on a different device (iPAD, iPhone, etc.)
    int button_x = (self.view.frame.size.width - button_width)/2;

    int button_y = self.view.frame.size.height - 80;
    
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
    CGPoint startLocation;
    CGPoint stopLocation;
    if (panGestureRecognizer.state == UIGestureRecognizerStateBegan) {
        startLocation = [panGestureRecognizer locationInView:self.view];
    }
    
    else if (panGestureRecognizer.state == UIGestureRecognizerStateEnded) {
        stopLocation = [panGestureRecognizer locationInView:self.view];
        
    }
    CGFloat dx = stopLocation.x - startLocation.x;
    CGFloat dy = stopLocation.y - startLocation.y;
    CGFloat distance = sqrt(dx*dx + dy*dy );
    NSLog(@"Distance: %f", distance);
    
    
//    self.testView.center = touchLocation;
    
    
    //find new camera center, mult by GL_PROJECTION
    
    
    //send xform xf to self->lightfield_.DrawImage
    
    
}



-(void)handleSingleTapGesture:(UITapGestureRecognizer *)tapGestureRecognizer{
//    CGFloat newWidth = 100.0;
//    if (self.testView.frame.size.width == 100.0) {
//        newWidth = 200.0;
//    }
//    
//    CGPoint currentCenter = self.testView.center;
//    
//    self.testView.frame = CGRectMake(self.testView.frame.origin.x, self.testView.frame.origin.y, newWidth, self.testView.frame.size.height);
//    self.testView.center = currentCenter;
}


-(void)handleDoubleTapGesture:(UITapGestureRecognizer *)tapGestureRecognizer{
//    CGSize newSize = CGSizeMake(100.0, 100.0);
//    if (self.testView.frame.size.width == 100.0) {
//        newSize.width = 200.0;
//        newSize.height = 200.0;
//    }
//    
//    CGPoint currentCenter = self.testView.center;
//    
//    self.testView.frame = CGRectMake(self.testView.frame.origin.x, self.testView.frame.origin.y, newSize.width, newSize.height);
//    self.testView.center = currentCenter;
}





@end
