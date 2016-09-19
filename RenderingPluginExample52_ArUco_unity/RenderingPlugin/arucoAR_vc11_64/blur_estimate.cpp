
#include "blur_estimate.h"
#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>

#define EXPANDING 10

using namespace cv;

//global to cache last shift
Point2d g_last_shift(0,0);

Mat extrace_roi(Mat& src, aruco::Board& last_pattern)
{
	/*double tic=(double)cvGetTickCount();
	Mat src_draw = src.clone();*/

	std::vector<Point2f>	contours;

	for(size_t i=0; i<last_pattern.size(); i++ )
	{
		for(int p = 0; p < 4; p++)
		{
			contours.push_back(last_pattern[i][p]);
		}
	}

	//Mat contour_mat(contours);

	RotatedRect vertices = minAreaRect(Mat(contours));
	Rect bounding_rect = vertices.boundingRect();
	
	//rectangle(src_draw, bounding_rect, Scalar(255,255,0));

	Rect expand_bounding(bounding_rect);
	expand_bounding+Size(EXPANDING,EXPANDING);

	if(expand_bounding.x < 0)
	{
		expand_bounding.x = 0;
	}
	if(expand_bounding.y < 0)
	{
		expand_bounding.y = 0;
	}

	if(expand_bounding.x + expand_bounding.width >= src.cols)
	{
		expand_bounding.width = src.cols - expand_bounding.x - 1;
	}
	if(expand_bounding.y + expand_bounding.height >= src.rows)
	{
		expand_bounding.height = src.rows - expand_bounding.y -1;
	}



	//expand_bounding.x = std::max(0, expand_bounding.x - EXPANDING);
	//expand_bounding.y = std::max(0, expand_bounding.y - EXPANDING);
	//if( expand_bounding.x + expand_bounding.width + 2*EXPANDING < src.cols)
	//{
	//	expand_bounding.width = expand_bounding.width + 2*EXPANDING;
	//}
	//else
	//{
	//	expand_bounding.width = src.cols - expand_bounding.x;
	//}
	//if( expand_bounding.y + expand_bounding.height + 2*EXPANDING < src.rows)
	//{
	//	expand_bounding.height = expand_bounding.height + 2*EXPANDING;
	//}
	//else
	//{
	//	expand_bounding.height = src.rows - expand_bounding.y;
	//}

	//rectangle(src_draw, expand_bounding, Scalar(0,255,0));
	Mat src_roi(src,expand_bounding);
	src_roi = src_roi.clone();
	//double toc=(double)cvGetTickCount();
	//double detectionTime = (toc-tic)/((double) cvGetTickFrequency()*1000);
	//cout << "estimation time: " << detectionTime << endl;
	return src_roi;
}


cv::Mat extrace_roi(cv::Mat& src, aruco::Marker& last_pattern)
{
	//Mat src_draw = src.clone();

	std::vector<Point2f>	contours;
	for (int p = 0; p < 4; p++)
	{
		contours.push_back(last_pattern[p]);
	}

	RotatedRect vertices = minAreaRect(Mat(contours));
	Rect bounding_rect = vertices.boundingRect();

	Rect expand_bounding(bounding_rect);
	expand_bounding + Size(EXPANDING, EXPANDING);

	if (expand_bounding.x < 0)
	{
		expand_bounding.x = 0;
	}
	if (expand_bounding.y < 0)
	{
		expand_bounding.y = 0;
	}

	if (expand_bounding.x + expand_bounding.width >= src.cols)
	{
		expand_bounding.width = src.cols - expand_bounding.x - 1;
	}
	if (expand_bounding.y + expand_bounding.height >= src.rows)
	{
		expand_bounding.height = src.rows - expand_bounding.y - 1;
	}

	Mat src_roi(src, expand_bounding);
	src_roi = src_roi.clone();

	//imshow("src", src);
	//imshow("roi", src_roi);
	//waitKey();

	return src_roi;
}


void blur_estimate(cv::Mat& src, aruco::BoardDetector& b_detector, aruco::Board& last_pattern, cv::Mat& last_roi, const cv::Mat& cameraMatrix, const cv::Mat& distortions)
{
	//make the roi and find the edge
   	Mat src_roi = extrace_roi(src, last_pattern);

	Mat src_gray;
	cvtColor( src_roi, src_gray, CV_RGB2GRAY );
	Mat last_roi_gray;
	cvtColor( last_roi, last_roi_gray, CV_RGB2GRAY);
	
	Mat previoud_roi64;
	Mat current_roi64;
	last_roi_gray.convertTo(previoud_roi64, CV_64FC1);
	src_gray.convertTo(current_roi64, CV_64FC1);
	Mat hann;
	createHanningWindow(hann, current_roi64.size(), CV_64FC1);
	double correct_respose;
	Point2d shift = phaseCorrelateRes(previoud_roi64, current_roi64, hann, &correct_respose);
	//double radius = cv::sqrt(shift.x*shift.x + shift.y*shift.y);

	//calculate the size of the markers in meters if expressed in pixels

	/*if(correct_respose > 0.1)*/
	{
		//Mat src_draw = src.clone();
		//for(size_t i=0; i<last_pattern.size(); i++ )
		//{
		//	for(int p = 0; p < 4; p++)
		//	{
		//		line( src_draw, last_pattern[i][p], last_pattern[i][(p+1)%4], Scalar(0,255,0), 1, 8 );
		//	}
		//}
		//
		double marker_meter_per_pix = 0.01;
		vector<cv::Point3f> objPoints;
		vector<cv::Point2f> imagePoints;
		for(size_t i=0; i<last_pattern.size(); i++ )
		{
			for(int p = 0; p < 4; p++)
			{
				if(correct_respose > 0.1)
				{
					last_pattern[i][p].x += shift.x;
					last_pattern[i][p].y += shift.y;

					g_last_shift = shift;
				}
				else
				{
					last_pattern[i][p].x += g_last_shift.x;
					last_pattern[i][p].y += g_last_shift.y;
				}


				imagePoints.push_back ( last_pattern[i][p] );
				const aruco::MarkerInfo &Minfo=last_pattern.conf.getMarkerInfo ( last_pattern[i].id );
                objPoints.push_back ( Minfo[p]*marker_meter_per_pix );
			}
		}

		//for(size_t i=0; i<last_pattern.size(); i++ )
		//{
		//	for(int p = 0; p < 4; p++)
		//	{
		//		line( src_draw, last_pattern[i][p], last_pattern[i][(p+1)%4], Scalar(0,0,255), 1, 8 );
		//	}
		//}

		cv::Mat rvec,tvec;
        cv::solvePnP ( objPoints,imagePoints,cameraMatrix,distortions,rvec,tvec );
        rvec.convertTo ( last_pattern.Rvec,CV_32FC1 );
        tvec.convertTo ( last_pattern.Tvec,CV_32FC1 );

		last_pattern.is_estimated = true;
		b_detector.set_board(last_pattern);

	}
}


void blur_estimate(cv::Mat& src, std::vector<aruco::Marker>& markers, aruco::Marker& last_pattern, cv::Mat& last_roi, const cv::Mat& cameraMatrix, const cv::Mat& distortions)
{
	//make the roi and find the edge
	Mat src_roi = extrace_roi(src, last_pattern);

	Mat src_gray;
	cvtColor(src_roi, src_gray, CV_RGB2GRAY);
	Mat last_roi_gray;
	cvtColor(last_roi, last_roi_gray, CV_RGB2GRAY);

	Mat previoud_roi64;
	Mat current_roi64;
	last_roi_gray.convertTo(previoud_roi64, CV_64FC1);
	src_gray.convertTo(current_roi64, CV_64FC1);
	Mat hann;
	createHanningWindow(hann, current_roi64.size(), CV_64FC1);
	double correct_respose;
	Point2d shift = phaseCorrelateRes(previoud_roi64, current_roi64, hann, &correct_respose);

	double marker_meter_per_pix = 1.0;
	vector<cv::Point3f> objPoints;
	vector<cv::Point2f> imagePoints;


	for (int p = 0; p < 4; p++)
	{
		if (correct_respose > 0.1)
		{
			last_pattern[p].x += shift.x;
			last_pattern[p].y += shift.y;

			g_last_shift = shift;
		}
		else
		{
			last_pattern[p].x += g_last_shift.x;
			last_pattern[p].y += g_last_shift.y;
		}

		//imagePoints.push_back ( last_pattern[p] );
		/*const aruco::MarkerInfo &Minfo=last_pattern.conf.getMarkerInfo ( last_pattern[i].id );
		objPoints.push_back ( Minfo[p]*marker_meter_per_pix );*/
	}

	last_pattern.is_estimated = true;
	last_pattern.calculateExtrinsics(marker_meter_per_pix, cameraMatrix, distortions,false);
	markers.push_back(last_pattern);
}
