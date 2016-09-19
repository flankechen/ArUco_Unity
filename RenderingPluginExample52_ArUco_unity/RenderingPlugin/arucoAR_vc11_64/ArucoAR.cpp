
#include "ArucoAR.h"

ArucoAR::ArucoAR() :TheMarkerSize(1), isVertical(true), last_found(-1), mbFinishRequested(false), mbFinished(false)
{
	//frame_rectify = cv::Mat(FRAME_HEIGHT, FRAME_WIDTH, CV_8UC3, cv::Scalar(0, 0, 0));
}

ArucoAR::~ArucoAR()
{

}

bool ArucoAR::init_board(std::string intrin_path, std::string calib_path, std::string roi_path, std::string board_config_path)
{
	mDectTarget = detect_target::board;

	try{
		//CameraParams.readFromXMLFile("out_camera_data.xml");
		CameraParams.readStereoFromXML(intrin_path, calib_path,roi_path );
		TheBoardConfig.readFromFile(board_config_path);
	}catch (std::exception &ex) {
	  cout<<ex.what()<<endl;
	  return false;
    }

	

	image_chop_up = MAX(CameraParams.validroi1.at<int>(0,1),CameraParams.validroi2.at<int>(0,1));
	image_chop_down = MAX(CameraParams.validroi1.at<int>(0,3),CameraParams.validroi2.at<int>(0,3));
	image_chop_left = MAX(CameraParams.validroi1.at<int>(0,0),CameraParams.validroi2.at<int>(0,0));
	image_chop_right = MAX(CameraParams.validroi1.at<int>(0,2),CameraParams.validroi2.at<int>(0,2));

	/// CREATE UNDISTORTED CAMERA PARAMS
    CameraParamsUnd=CameraParams;
    CameraParamsUnd.Distorsion=cv::Mat::zeros(4,1,CV_32F);  
	CameraParamsUnd.Distorsion_2=cv::Mat::zeros(4,1,CV_32F);

	/// SET BOARD DETECTOR PARAMETERS
    TheBoardDetector.setParams(TheBoardConfig,CameraParamsUnd,TheMarkerSize);
	TheBoardDetector.getMarkerDetector().setCornerRefinementMethod(MarkerDetector::HARRIS);

	return true;
}

bool ArucoAR::init_marker(std::string intrin_path, std::string calib_path, std::string roi_path)
{
	mDectTarget = detect_target::marker;

	try{
		//CameraParams.readFromXMLFile("out_camera_data.xml");
		CameraParams.readStereoFromXML(intrin_path, calib_path,roi_path );
	}catch (std::exception &ex) {
	  cout<<ex.what()<<endl;
	  return false;
    }


	
	image_chop_up = MAX(CameraParams.validroi1.at<int>(0,1),CameraParams.validroi2.at<int>(0,1));
	image_chop_down = MAX(CameraParams.validroi1.at<int>(0,3),CameraParams.validroi2.at<int>(0,3));
	image_chop_left = MAX(CameraParams.validroi1.at<int>(0,0),CameraParams.validroi2.at<int>(0,0));
	image_chop_right = MAX(CameraParams.validroi1.at<int>(0,2),CameraParams.validroi2.at<int>(0,2));

	/// CREATE UNDISTORTED CAMERA PARAMS
    CameraParamsUnd=CameraParams;
    CameraParamsUnd.Distorsion=cv::Mat::zeros(4,1,CV_32F);  
	CameraParamsUnd.Distorsion_2=cv::Mat::zeros(4,1,CV_32F);

	return true;
}

void ArucoAR::process_frame(cv::Mat& src)
{
	//double tic=(double)cvGetTickCount();
	//cap >> TheInputImage;
	//flip(TheInputImage.t(), src_flip, 0);
	//remap(TheInputImage, frame_rectify, CameraParams.mx1, CameraParams.my1, CV_INTER_LINEAR);
	//frame_rectify = frame_rectify.adjustROI(-image_chop_up, -image_chop_down, -image_chop_left, -image_chop_right).clone();
	//resize(frame_rectify,frame_rectify,src_flip.size(),INTER_LINEAR );

	//cap_right>>TheInputImage_right;
	//flip(TheInputImage_right.t(), src_flip_right, 0);
	//remap(TheInputImage_right, frame_rectify_right, CameraParams.mx2, CameraParams.my2, CV_INTER_LINEAR);
	//frame_rectify_right = frame_rectify_right.adjustROI(-image_chop_up, -image_chop_down, -image_chop_left, -image_chop_right).clone();  
	//resize(frame_rectify_right,frame_rectify_right,src_flip_right.size(),INTER_LINEAR );

	frame_rectify = src;

	//detect marker
	if (mDectTarget == detect_target::marker)
	{
		TheMarkerDetector.detect(frame_rectify, TheMarkers, CameraParamsUnd, TheMarkerSize);

		if (TheMarkers.size() == 0)
		{
			mProb = 0.0;
		}
		else
		{
			mProb = 1.0;
		}
	}

	/// DETECT BOARD
	if (mDectTarget == detect_target::board)
	{
		mProb = TheBoardDetector.detect(frame_rectify);
		printf("prob detect: %f\n", mProb);
	}

	//double toc=(double)cvGetTickCount();
	//double detectionTime = (toc-tic)/((double) cvGetTickFrequency()*1000);
	//cout << "Detection time: " << detectionTime << endl;

	if (mProb < 0.2 && last_found < 10 && last_found >= 0)
		{
			last_found++;
			if(last_found == 10-1)
			{
				last_found = -1;
			}
			else
			{
				//do motion estimation here
				if(mDectTarget == detect_target::board)
				{
					blur_estimate(frame_rectify, TheBoardDetector, last_pattern ,last_roi, CameraParamsUnd.CameraMatrix, CameraParamsUnd.Distorsion);
				
					if(TheBoardDetector.getDetectedBoard().size() == 0)
					{
						last_found = -1;
					}
					last_roi = extrace_roi(frame_rectify, last_pattern);
				}
				else if(mDectTarget == detect_target::marker)
				{
					blur_estimate(frame_rectify, TheMarkers, last_marker ,last_roi, CameraParamsUnd.CameraMatrix, CameraParamsUnd.Distorsion);

					if(TheMarkers.size() == 0)
					{
						last_found = -1;
					}
					last_roi = extrace_roi(frame_rectify, last_marker);
				}
			}
		}

		 //UPDATE SCENE
		if(mDectTarget == detect_target::board)
		{
			if ( mProb>0.2 || TheBoardDetector.getDetectedBoard().size() > 2) 
			{
				if(!TheBoardDetector.getDetectedBoard().is_estimated)
				{
					last_found =0;
				}
				last_roi = extrace_roi(frame_rectify, TheBoardDetector.getDetectedBoard());
				last_pattern =  TheBoardDetector.getDetectedBoard();
			}


			TheBoardDetector.getDetectedBoard().OgreGetPoseParameters(mPosition, mOrientation);
			TheBoardDetector.getDetectedBoard().glGetModelViewMatrix(mModelview_matrix);
			TheBoardDetector.getDetectedBoard().draw(frame_rectify, Scalar(255,0,0));
		}
		else if(mDectTarget == detect_target::marker)
		{
			if(mProb>0.0 || TheMarkers.size() > 0)
			{
				if(!TheMarkers[0].is_estimated)
				{
					last_found =0;
				}
				last_roi = extrace_roi(frame_rectify, TheMarkers[0]);
				last_marker =  TheMarkers[0];
			}

			for(int i = 0; i<TheMarkers.size();i++)
			{
				TheMarkers[i].draw(frame_rectify,Scalar(0,255,0));
			}

			if (TheMarkers.size() > 0)
			{ 
				TheMarkers[0].glGetModelViewMatrix(mModelview_matrix);
			}
				
		}





#ifdef CV_WINDOW
	imshow("frame_rectify", frame_rectify);
#ifdef FRAME_BY_FRAME
	waitKey();
#else
	waitKey(30);
#endif
#endif

		//cout<<"position"<<mPosition[0]<<" "<<mPosition[1]<<" "<<mPosition[2]<<endl;

		//for(int i=0; i<3; i++)
		//{
		//	position[i] = mPosition[i];
		//}

		//for(int i=0; i<4; i++)
		//{
		//	orientation[i] = mOrientation[i];
		//}
}

void ArucoAR::get_pose(double* position, double* orientation)
{
	for(int i=0; i<3; i++)
		{
			position[i] = mPosition[i];
		}

		for(int i=0; i<4; i++)
		{
			orientation[i] = mOrientation[i];
		}
}

double* ArucoAR::get_position()
{
	unique_lock<mutex> lock(mMutex);
	return mPosition;
}

double* ArucoAR::get_orientation()
{
	unique_lock<mutex> lock(mMutex);
	return mOrientation;
}

double* ArucoAR::get_modelview_matrix()
{
	unique_lock<mutex> lock(mMutex);
	return mModelview_matrix;
}


void ArucoAR::update(Mat& pSrc)
{
	unique_lock<mutex> lock(mMutex);
	pSrc.copyTo(frame_rectify);
}

void ArucoAR::run()
{

	while (1)
	{
		//imshow("rectify", frame_rectify);
		//waitKey();

		if (frame_rectify.empty())
			continue;

		process_frame(frame_rectify);

		if (checkFinish())
			break;
	}

	setFinish();
}

void ArucoAR::requestFinish()
{
	unique_lock<mutex> lock(mMutexFinish);
	mbFinishRequested = true;
}

bool ArucoAR::checkFinish()
{
	unique_lock<mutex> lock(mMutexFinish);
	return mbFinishRequested;
}

void ArucoAR::setFinish()
{
	unique_lock<mutex> lock(mMutexFinish);
	mbFinished = true;
}

bool ArucoAR::isFinished()
{
	unique_lock<mutex> lock(mMutexFinish);
	return mbFinished;
}


//ArucoAR* _cdecl create_ArucoAR()
//{
//	ArucoAR* return_ptr = new ArucoAR();
//	if(return_ptr->init("./save_param/intrinsics.yml", "./save_param/calib_para.yml","./save_param/validRoi.yml","boardConfig.yml"))
//		return return_ptr;
//	else return NULL;
//}
//
//void process_frame(ArucoAR* pAruchAR)
//{
//	pAruchAR->process_frame();
//}
//
//double* get_position(ArucoAR* pAruchAR)
//{
//	return pAruchAR->mPosition;
//}
//
//double* get_orientation(ArucoAR* pAruchAR)
//{
//	return pAruchAR->mOrientation;
//}
//
//void delete_arucoAR(ArucoAR* pAruchAR)
//{
//	delete pAruchAR;
//}

//viewer::viewer()
//{
//	mIm = cv::Mat(480, 640, CV_8UC3, cv::Scalar(0, 0, 0));
//}
//
//void viewer::run()
//{
//	namedWindow("rectify");
//
//	while (1)
//	{
//		imshow("rectify", mIm);
//		waitKey(500);
//	}
//}
//
//void viewer::update(ArucoAR* pAruco)
//{
//	unique_lock<mutex> lock(mMutex);
//	pAruco->frame_rectify.copyTo(mIm);
//}