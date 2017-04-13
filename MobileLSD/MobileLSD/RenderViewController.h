//
//  RenderViewController.h
//  MobileLSD
//
//  Created by AJ Bruce on 4/12/17.
//  Copyright © 2017 Guanhang Wu. All rights reserved.
//

#include "LightfieldClass.h"
#import <UIKit/UIKit.h>


@interface RenderViewController : UIViewController

@property(nonatomic) LightfieldClass *lightfield_;

@property (weak, nonatomic) IBOutlet UIView *testView;

@property (weak, nonatomic) IBOutlet UILabel *horizontalVelocityLabel;

@property (weak, nonatomic) IBOutlet UILabel *verticalVelocityLabel;

@end

