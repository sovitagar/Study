//============================================================================
// Name        : Dip2.cpp
// Author      : Ronny Haensch
// Version     : 2.0
// Copyright   : -
// Description :
//============================================================================

#include "Dip2.h"
#include <iostream>
#include <algorithm>

#define ONE_OVER_TWO_PI 0.159154943

// convolution in spatial domain
/*
src:     input image
kernel:  filter kernel
return:  convolution result
*/
Mat Dip2::spatialConvolution(Mat& src, Mat& kernel){
  // TO DO !!
  Mat result = Mat(src.size(), src.type());
  for(int y=0; y < src.rows; y++){
    for(int x=0; x < src.cols; x++ ) {
      float pixel = 0;
      for (int y1=0; y1 < kernel.rows; y1++){
        for (int x1=0; x1 < kernel.cols; x1++){
          pixel += src.at<float>(
                min(max(y+y1-kernel.rows/2,0),src.rows-1),  // asume center of kernel is anchor; use border pixels outside of image (border handling)
                min(max(x+x1-kernel.cols/2,0),src.cols-1))
            * kernel.at<float>(
                kernel.rows-1-y1,   //flipping kernel
                kernel.cols-1-x1);
        }
      }

      result.at<float>(y,x) = pixel;
    }
  }

  return result;
}

// the average filter
// HINT: you might want to use Dip2::spatialConvolution(...) within this function
/*
src:     input image
kSize:   window size used by local average
return:  filtered image
*/
Mat Dip2::averageFilter(Mat& src, int kSize){
   Mat kernel = Mat(kSize, kSize, CV_32FC1, 1./((float)(kSize*kSize)));
   return spatialConvolution(src, kernel);
}

// the median filter
/*
src:     input image
kSize:   window size used by median operation
return:  filtered image
*/
Mat Dip2::medianFilter(Mat& src, int kSize){
  Mat result = Mat(src.size(), src.type());
  for(int y=0; y < src.rows; y++){
    for(int x=0; x < src.cols; x++ ) {
      float elements[kSize*kSize];
      for (int y1=0; y1 < kSize; y1++){
        for (int x1=0; x1 < kSize; x1++){
          elements[y1 * kSize + x1] = src.at<float>(
                min(max(y+y1-kSize/2,0),src.rows-1),  // asume center of kernel is anchor; use border pixels outside of image (border handling)
                min(max(x+x1-kSize/2,0),src.cols-1));
        }
      }

      std::sort(elements, elements+kSize*kSize);
      result.at<float>(y,x) = elements[kSize*kSize/2];
    }
  }

  return result;
}

// the bilateral filter
/*
src:     input image
kSize:   size of the kernel --> used to compute std-dev of spatial kernel
sigma_rad:   standard-deviation of the radiometric kernel
return:  filtered image
*/
Mat Dip2::bilateralFilter(Mat& src, int kSize, double sigma_rad){
  float sigma_sp = 0.3 * ((kSize-1) * 0.5 - 1) + 0.8;

  Mat result = Mat(src.size(), src.type());
  for(int y=0; y < src.rows; y++){
    for(int x=0; x < src.cols; x++ ) {
      float pixel = 0; // weighted sum of pixel values
      float Z = 0;     // sum of weihgts
      float pixel_value = src.at<float>(y,x); //value of center pixel (for comparison)
      for (int y1=0; y1 < kSize; y1++){
        if (y+y1-kSize/2 < 0) continue;           // |
        if (y+y1-kSize/2 >= src.rows) continue;   // |
        for (int x1=0; x1 < kSize; x1++){         // } if outside of image then skip
          if (x+x1-kSize/2 < 0) continue;         // | (equivalent to rediometric weight=0)
          if (x+x1-kSize/2 >= src.cols) continue; // |

          float value = src.at<float>(y+y1-kSize/2, x+x1-kSize/2); // value of curent pixel

          float h_sp  = -((y1-kSize/2)*(y1-kSize/2) + (x1-kSize/2)*(x1-kSize/2))/(2.*sigma_sp *sigma_sp ); // spartial weight
          float h_rad = -((pixel_value-value)*(pixel_value-value)              )/(2.*sigma_rad*sigma_rad); // rediometric weight
          float h = std::exp(h_sp) * std::exp(h_rad);

          //printf("h_sp: %f, h_rad: %f, h: %f\n", h_sp, h_rad, h);

          Z += h; // add weight to sum
          pixel += value * h; // add weighted pixel to sum

        }
      }

      result.at<float>(y,x) = pixel/Z; // compute weighted average
    }
  }

  return result;
}

// the non-local means filter
/*
src:   		input image
searchSize: size of search region
sigma: 		Optional parameter for weighting function
return:  	filtered image
*/
Mat Dip2::nlmFilter(Mat& src, int searchSize, double sigma){

    return src.clone();

}

/* *****************************
  GIVEN FUNCTIONS
***************************** */

// function loads input image, calls processing function, and saves result
void Dip2::run(void){

   // load images as grayscale
	cout << "load images" << endl;
	Mat noise1 = imread("noiseType_1.jpg", 0);
   if (!noise1.data){
	   cout << "noiseType_1.jpg not found" << endl;
      cout << "Press enter to exit"  << endl;
      cin.get();
	   exit(-3);
	}
   noise1.convertTo(noise1, CV_32FC1);
	Mat noise2 = imread("noiseType_2.jpg",0);
	if (!noise2.data){
	   cout << "noiseType_2.jpg not found" << endl;
      cout << "Press enter to exit"  << endl;
      cin.get();
	   exit(-3);
	}
   noise2.convertTo(noise2, CV_32FC1);
	cout << "done" << endl;

   // apply noise reduction
	// TO DO !!!
	// ==> Choose appropriate noise reduction technique with appropriate parameters
	// ==> "average" or "median"? Why?
	// ==> try also "bilateral" (and if implemented "nlm")
	cout << "reduce noise" << endl;
	Mat restorated1 = noiseReduction(noise1, "median", 5);  // noise1 is shot-noise that can best be reduced using a median filter
	Mat restorated2 = noiseReduction(noise2, "average", 15); // noise2 is uniform gausian noise that can best be reduced with averaging
	Mat restorated3 = noiseReduction(noise2, "bilateral", 15, 100); // noise2 is uniform gausian noise that can best be reduced with averaging
	cout << "done" << endl;

	// save images
	cout << "save results" << endl;
	imwrite("median.jpg", restorated1);
	imwrite("average.jpg", restorated2);
	imwrite("bilateral.jpg", restorated3);
	cout << "done" << endl;

}

// noise reduction
/*
src:     input image
method:  name of noise reduction method that shall be performed
	     "average" ==> moving average
         "median" ==> median filter
         "bilateral" ==> bilateral filter
         "nlm" ==> non-local means filter
kSize:   (spatial) kernel size
param:   if method == "bilateral", standard-deviation of radiometric kernel; if method == "nlm", (optional) parameter for similarity function
         can be ignored otherwise (default value = 0)
return:  output image
*/
Mat Dip2::noiseReduction(Mat& src, string method, int kSize, double param){

   // apply moving average filter
   if (method.compare("average") == 0){
      return averageFilter(src, kSize);
   }
   // apply median filter
   if (method.compare("median") == 0){
      return medianFilter(src, kSize);
   }
   // apply bilateral filter
   if (method.compare("bilateral") == 0){
      return bilateralFilter(src, kSize, param);
   }
   // apply adaptive average filter
   if (method.compare("nlm") == 0){
      return nlmFilter(src, kSize, param);
   }

   // if none of above, throw warning and return copy of original
   cout << "WARNING: Unknown filtering method! Returning original" << endl;
   cout << "Press enter to continue"  << endl;
   cin.get();
   return src.clone();

}

// generates and saves different noisy versions of input image
/*
fname:   path to the input image
*/
void Dip2::generateNoisyImages(string fname){

   // load image, force gray-scale
   cout << "load original image" << endl;
   Mat img = imread(fname, 0);
   if (!img.data){
      cout << "ERROR: file " << fname << " not found" << endl;
      cout << "Press enter to exit"  << endl;
      cin.get();
      exit(-3);
   }

   // convert to floating point precision
   img.convertTo(img,CV_32FC1);
   cout << "done" << endl;

   // save original
   imwrite("original.jpg", img);

   // generate images with different types of noise
   cout << "generate noisy images" << endl;

   // some temporary images
   Mat tmp1(img.rows, img.cols, CV_32FC1);
   Mat tmp2(img.rows, img.cols, CV_32FC1);
   // first noise operation
   float noiseLevel = 0.15;
   randu(tmp1, 0, 1);
   threshold(tmp1, tmp2, noiseLevel, 1, CV_THRESH_BINARY);
   multiply(tmp2,img,tmp2);
   threshold(tmp1, tmp1, 1-noiseLevel, 1, CV_THRESH_BINARY);
   tmp1 *= 255;
   tmp1 = tmp2 + tmp1;
   threshold(tmp1, tmp1, 255, 255, CV_THRESH_TRUNC);
   // save image
   imwrite("noiseType_1.jpg", tmp1);

   // second noise operation
   noiseLevel = 50;
   randn(tmp1, 0, noiseLevel);
   tmp1 = img + tmp1;
   threshold(tmp1,tmp1,255,255,CV_THRESH_TRUNC);
   threshold(tmp1,tmp1,0,0,CV_THRESH_TOZERO);
   // save image
   imwrite("noiseType_2.jpg", tmp1);

	cout << "done" << endl;
	cout << "Please run now: dip2 restorate" << endl;

}

// function calls some basic testing routines to test individual functions for correctness
void Dip2::test(void){

	test_spatialConvolution();
   test_averageFilter();
   test_medianFilter();

   cout << "Press enter to continue"  << endl;
   cin.get();

}

// checks basic properties of the convolution result
void Dip2::test_spatialConvolution(void){

   Mat input = Mat::ones(9,9, CV_32FC1);
   input.at<float>(4,4) = 255;
   Mat kernel = Mat(3,3, CV_32FC1, 1./9.);

   Mat output = spatialConvolution(input, kernel);

   if ( (input.cols != output.cols) || (input.rows != output.rows) ){
      cout << "ERROR: Dip2::spatialConvolution(): input.size != output.size --> Wrong border handling?" << endl;
      return;
   }
  if ( (sum(output.row(0) < 0).val[0] > 0) ||
           (sum(output.row(0) > 255).val[0] > 0) ||
           (sum(output.row(8) < 0).val[0] > 0) ||
           (sum(output.row(8) > 255).val[0] > 0) ||
           (sum(output.col(0) < 0).val[0] > 0) ||
           (sum(output.col(0) > 255).val[0] > 0) ||
           (sum(output.col(8) < 0).val[0] > 0) ||
           (sum(output.col(8) > 255).val[0] > 0) ){
         cout << "ERROR: Dip2::spatialConvolution(): Border of convolution result contains too large/small values --> Wrong border handling?" << endl;
         return;
   }else{
      if ( (sum(output < 0).val[0] > 0) ||
         (sum(output > 255).val[0] > 0) ){
            cout << "ERROR: Dip2::spatialConvolution(): Convolution result contains too large/small values!" << endl;
            return;
      }
   }
   float ref[9][9] = {{0, 0, 0, 0, 0, 0, 0, 0, 0},
                      {0, 1, 1, 1, 1, 1, 1, 1, 0},
                      {0, 1, 1, 1, 1, 1, 1, 1, 0},
                      {0, 1, 1, (8+255)/9., (8+255)/9., (8+255)/9., 1, 1, 0},
                      {0, 1, 1, (8+255)/9., (8+255)/9., (8+255)/9., 1, 1, 0},
                      {0, 1, 1, (8+255)/9., (8+255)/9., (8+255)/9., 1, 1, 0},
                      {0, 1, 1, 1, 1, 1, 1, 1, 0},
                      {0, 1, 1, 1, 1, 1, 1, 1, 0},
                      {0, 0, 0, 0, 0, 0, 0, 0, 0}};
   for(int y=1; y<8; y++){
      for(int x=1; x<8; x++){
         if (abs(output.at<float>(y,x) - ref[y][x]) > 0.0001){
            cout << "ERROR: Dip2::spatialConvolution(): Convolution result contains wrong values!" << endl;
            return;
         }
      }
   }
   input.setTo(0);
   input.at<float>(4,4) = 255;
   kernel.setTo(0);
   kernel.at<float>(0,0) = -1;
   output = spatialConvolution(input, kernel);
   if ( abs(output.at<float>(5,5) + 255.) < 0.0001 ){
      cout << "ERROR: Dip2::spatialConvolution(): Is filter kernel \"flipped\" during convolution? (Check lecture/exercise slides)" << endl;
      return;
   }
   if ( ( abs(output.at<float>(2,2) + 255.) < 0.0001 ) || ( abs(output.at<float>(4,4) + 255.) < 0.0001 ) ){
      cout << "ERROR: Dip2::spatialConvolution(): Is anchor point of convolution the centre of the filter kernel? (Check lecture/exercise slides)" << endl;
      return;
   }
   cout << "Message: Dip2::spatialConvolution() seems to be correct" << endl;
}

// checks basic properties of the filtering result
void Dip2::test_averageFilter(void){

   Mat input = Mat::ones(9,9, CV_32FC1);
   input.at<float>(4,4) = 255;

   Mat output = averageFilter(input, 3);

   if ( (input.cols != output.cols) || (input.rows != output.rows) ){
      cout << "ERROR: Dip2::averageFilter(): input.size != output.size --> Wrong border handling?" << endl;
      return;
   }
  if ( (sum(output.row(0) < 0).val[0] > 0) ||
           (sum(output.row(0) > 255).val[0] > 0) ||
           (sum(output.row(8) < 0).val[0] > 0) ||
           (sum(output.row(8) > 255).val[0] > 0) ||
           (sum(output.col(0) < 0).val[0] > 0) ||
           (sum(output.col(0) > 255).val[0] > 0) ||
           (sum(output.col(8) < 0).val[0] > 0) ||
           (sum(output.col(8) > 255).val[0] > 0) ){
         cout << "ERROR: Dip2::averageFilter(): Border of result contains too large/small values --> Wrong border handling?" << endl;
         return;
   }else{
      if ( (sum(output < 0).val[0] > 0) ||
         (sum(output > 255).val[0] > 0) ){
            cout << "ERROR: Dip2::averageFilter(): Result contains too large/small values!" << endl;
            return;
      }
   }
   float ref[9][9] = {{0, 0, 0, 0, 0, 0, 0, 0, 0},
                      {0, 1, 1, 1, 1, 1, 1, 1, 0},
                      {0, 1, 1, 1, 1, 1, 1, 1, 0},
                      {0, 1, 1, (8+255)/9., (8+255)/9., (8+255)/9., 1, 1, 0},
                      {0, 1, 1, (8+255)/9., (8+255)/9., (8+255)/9., 1, 1, 0},
                      {0, 1, 1, (8+255)/9., (8+255)/9., (8+255)/9., 1, 1, 0},
                      {0, 1, 1, 1, 1, 1, 1, 1, 0},
                      {0, 1, 1, 1, 1, 1, 1, 1, 0},
                      {0, 0, 0, 0, 0, 0, 0, 0, 0}};
   for(int y=1; y<8; y++){
      for(int x=1; x<8; x++){
         if (abs(output.at<float>(y,x) - ref[y][x]) > 0.0001){
            cout << "ERROR: Dip2::averageFilter(): Result contains wrong values!" << endl;
            return;
         }
      }
   }
   cout << "Message: Dip2::averageFilter() seems to be correct" << endl;
}

// checks basic properties of the filtering result
void Dip2::test_medianFilter(void){

   Mat input = Mat::ones(9,9, CV_32FC1);
   input.at<float>(4,4) = 255;

   Mat output = medianFilter(input, 3);

   if ( (input.cols != output.cols) || (input.rows != output.rows) ){
      cout << "ERROR: Dip2::medianFilter(): input.size != output.size --> Wrong border handling?" << endl;
      return;
   }
  if ( (sum(output.row(0) < 0).val[0] > 0) ||
           (sum(output.row(0) > 255).val[0] > 0) ||
           (sum(output.row(8) < 0).val[0] > 0) ||
           (sum(output.row(8) > 255).val[0] > 0) ||
           (sum(output.col(0) < 0).val[0] > 0) ||
           (sum(output.col(0) > 255).val[0] > 0) ||
           (sum(output.col(8) < 0).val[0] > 0) ||
           (sum(output.col(8) > 255).val[0] > 0) ){
         cout << "ERROR: Dip2::medianFilter(): Border of result contains too large/small values --> Wrong border handling?" << endl;
         return;
   }else{
      if ( (sum(output < 0).val[0] > 0) ||
         (sum(output > 255).val[0] > 0) ){
            cout << "ERROR: Dip2::medianFilter(): Result contains too large/small values!" << endl;
            return;
      }
   }
   for(int y=1; y<8; y++){
      for(int x=1; x<8; x++){
         if (abs(output.at<float>(y,x) - 1.) > 0.0001){
            cout << "ERROR: Dip2::medianFilter(): Result contains wrong values!" << endl;
            return;
         }
      }
   }
   cout << "Message: Dip2::medianFilter() seems to be correct" << endl;

}