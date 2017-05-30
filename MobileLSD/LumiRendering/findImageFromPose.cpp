
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <opencv2/calib3d/calib3d.hpp>
//#opencv2/imgcodecs/ios.h
//#include "opencv2/nonfree/features2d.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <stdio.h>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <math.h>
#include <algorithm>
#include <utility>
#include <iostream>
#include <stdlib.h>


#include "ReadInDataSet.h"
#include "LightfieldClass.h"

using namespace std;
using namespace cv;


//Mat LightfieldClass::DrawImage(xform xf)
int LightfieldClass::DrawImage(Point3d vCameraLoc, Matx33d vP_rot, Vec3d vP_trans)
{
    double virtual_X = vP_trans[0];
    double virtual_Y = vP_trans[1];
    
    
    //find closest neighbors
    int i = 0;
    int closestNeighbor1 = 0;
    double closestDist1 = 1000;
    double diffX1 = 0;
    double diffY1 = 0;
    
    int closestNeighbor2 = 0;
    double closestDist2 = 1000;
    double diffX2 = 0;
    double diffY2 = 0;
    
    for(i; i < this->imagesAndPoses.size(); ++i) {
        double x = this->imagesAndPoses[i].pose(0,3);
        double y = this->imagesAndPoses[i].pose(1,3);
        
        cout << "x: " << x << endl;
        cout << "y: " << y << endl;
        
        double dist = sqrt(pow(virtual_X - x,2) + pow(virtual_Y - y,2));
        
        cout << "dist: " << dist << endl;
        
        if(dist < closestDist2) {
            if(dist < closestDist1) {
//                cout << "new closest" << endl;
                closestDist2 = closestDist1;
                diffX2 = diffX1;
                diffY2 = diffY1;
                closestNeighbor2 = closestNeighbor1;
                
                closestDist1 = dist;
                closestNeighbor1 = i;
                diffX1 = virtual_X - x;
                diffY1 = virtual_Y - y;
            }
            else {
//                cout << "new second closest" << endl;
                closestDist2 = dist;
                closestNeighbor2 = i;
                diffX2 = virtual_X - x;
                diffY2 = virtual_Y - y;
            }
        }
    }
    
    cout << "closest neighbor 1: " << closestNeighbor1 << endl;
    cout << "closest neighbor 2: " << closestNeighbor2 << endl;
    
    
    Mat Img(IMAGE_RESOLUTION_HEIGHT, IMAGE_RESOLUTION_WIDTH, CV_8UC4);
    bool outofrange1 = false;
    bool outofrange2 = false;
    //interpolate the patches
    int patch_size = 5;
    int u,v;
    for(v = 0; (v+1) * patch_size < IMAGE_RESOLUTION_HEIGHT; ++v) {
        for(u = 0; (u+1) * patch_size < IMAGE_RESOLUTION_WIDTH; ++u) {
            //check if patch is inside the neighbors field of view
            //set alpha (reset if outof range)
            double alpha = closestDist1 / (closestDist1 + closestDist2);
            
            //closest neighbor:
            if(u * patch_size +  diffX1 < 0 || (u+1) * patch_size + diffX1 >= IMAGE_RESOLUTION_WIDTH
               || v * patch_size + diffY1 < 0 || (v+1) * patch_size + diffY1 >= IMAGE_RESOLUTION_HEIGHT) {
                //outside range: make black
                outofrange1 = true;
                alpha = 0;
            }
            //second closest neighbor:
            if(u * patch_size +  diffX2 < 0 || (u+1) * patch_size + diffX2 >= IMAGE_RESOLUTION_WIDTH
               || v * patch_size + diffY2 < 0 || (v+1) * patch_size + diffY2 >= IMAGE_RESOLUTION_HEIGHT) {
                if(outofrange1) {
                    //set patch to be black
                    Mat patch(patch_size, patch_size, CV_8UC4, Scalar(0,0,0,0));
                    patch.copyTo(Img(Rect(u * patch_size,v * patch_size, patch_size, patch_size)));
                    continue;
                }
                outofrange2 = true;
                alpha = 1;
            }
            
            Mat patch(patch_size, patch_size, CV_8UC4);
            Mat patch1(patch_size, patch_size, CV_8UC4, Scalar(0,0,0));
            Mat patch2(patch_size, patch_size, CV_8UC4, Scalar(0,0,0));
//            
//            cout << "before patch 1" << endl;

            
            if(!outofrange1){
                patch1 = this->imagesAndPoses[closestNeighbor1].image(Rect(u * patch_size +  diffX1,
                                                                           v * patch_size +  diffY1,
                                                                           patch_size, patch_size));
            }
            
//            
//            cout << "(u+1) * patch_size +  diffX2: " << (u+1) * patch_size +  diffX2 << endl;
//            cout << "(v+1) * patch_size +  diffY2: " << (v+1) * patch_size +  diffY2 << endl;
//            cout << "image 2 width" << this->imagesAndPoses[closestNeighbor2].image.cols << endl;
//            
//            cout << "image 2 height" << this->imagesAndPoses[closestNeighbor2].image.rows << endl;
//            cout << "WIDTH " << IMAGE_RESOLUTION_WIDTH << endl;
//            cout << "Height " << IMAGE_RESOLUTION_HEIGHT << endl;
//            
//            
//            cout << "before patch 2" << endl;
            if(!outofrange2){
                patch2 = this->imagesAndPoses[closestNeighbor2].image(Rect(u * patch_size +  diffX2,
                                                                           v * patch_size +  diffY2,
                                                                           patch_size, patch_size));
            }
//            cout << "before blend" << endl;
            
            patch = patch1 * alpha + patch2 * (1-alpha);
            
//            
//            cout << "u * patchsize: " << u*patch_size << endl;
//            cout << "v* patchsize: " << v*patch_size << endl;
//            cout << "Img.rows " << Img.rows << endl;
//            cout << "Img.cols " << Img.cols << endl;
//            cout <<"Img chanels "<<Img.channels() << endl;
//            cout <<"patch chanels "<<patch.channels() << endl;
//            

            
//            cout << "before copy to" << endl;
            
            patch.copyTo(Img(Rect(u * patch_size, v * patch_size, patch_size, patch_size)));
//            cout << "after copy to" << endl;
            
        }
    }

    this->currImage = Img;
    
    return SUCCESS;
    
}
