//
//  RenderViewController.h
//  MobileLSD
//
//  Created by AJ Bruce on 4/12/17.
//  Copyright © 2017 Guanhang Wu. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "LightfieldClass.h"

@interface RenderViewController : UIViewController

@property (nonatomic) LightfieldClass *lightfield_;

@property (nonatomic, retain) IBOutlet UIImageView *renderedImageView;

@property (weak, nonatomic) IBOutlet UIView *testView;

@property (weak, nonatomic) IBOutlet UILabel *horizontalVelocityLabel;

@property (weak, nonatomic) IBOutlet UILabel *verticalVelocityLabel;

@end

