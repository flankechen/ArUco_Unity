#include <aruco/aruco.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "blur_estimate.h"
#include <mutex>

#define FRAME_WIDTH 960
#define FRAME_HEIGHT 1080
#define RENDER_WIDTH 1920
#define RENDER_HEIGHT 1080

//#define LOCAL_VIDEO
#ifdef LOCAL_VIDEO

#endif
//#define DRAW_LINE

//#define FRAME_BY_FRAME
//
//#define CV_WINDOW

//#ifdef ARUCODLL_EXPORT
//#define ARUCODLL_API __declspec(dllexport) 
//#else
//#define ARUCODLL_API __declspec(dllimport) 
//#endif

using namespace cv;
using namespace aruco;

//class viewer;

extern "C"
{
	class ArucoAR{
	public:
		cv::Mat TheInputImage, TheInputImageUnd;
		cv::Mat TheInputImage_right, TheInputImageUnd_right;
		cv::Mat src_flip, src_flip_right;
		cv::Mat frame_rectify, frame_rectify_right;
		aruco::CameraParameters CameraParams, CameraParamsUnd;
		float TheMarkerSize;
		aruco::MarkerDetector TheMarkerDetector;
		std::vector<aruco::Marker> TheMarkers;
		aruco::BoardConfiguration TheBoardConfig;
		aruco::BoardDetector TheBoardDetector; 
		bool isVertical;
		//cv::VideoCapture cap;
		//cv::VideoCapture cap_right;
		Mat last_roi;
		Board last_pattern;
		Marker last_marker;
		int last_found;
		/*image valid boundary after image rectification*/
		int image_chop_left,image_chop_right,image_chop_up,image_chop_down; 
		double mPosition[3], mOrientation[4];
		double mModelview_matrix[16];
		//detect probability.
		float mProb;
		//
		enum detect_target{board, marker};
		detect_target mDectTarget;

		ArucoAR();
		~ArucoAR();

		std::mutex mMutex;
		std::mutex mMutexFinish;

		bool mbFinishRequested;
		bool mbFinished;

		void update(Mat& pSrc);
		void run();
		void requestFinish();
		bool checkFinish();
		void setFinish();
		bool isFinished();

		//init function for a board detection
		bool init_board(std::string intrin_path, std::string calib_path, std::string roi_path, std::string board_config_path);
		//init function for a marker detection
		bool init_marker(std::string intrin_path, std::string calib_path, std::string roi_path);
		void get_image(cv::Mat& pframe_rectify);
		void process_frame(cv::Mat& src);
		void get_pose(double* position, double* orientation);
		double* get_position();
		double* get_orientation();
		//return the 4*4 gl model view matrix
		double* get_modelview_matrix();
	};


	 //ArucoAR* _cdecl create_ArucoAR();
	 //void process_frame(ArucoAR* pAruchAR, cv::Mat& src, cv::Mat& src_right);
	 //double* get_position(ArucoAR* pAruchAR);
	 //double* get_orientation(ArucoAR* pAruchAR);
	 //void delete_arucoAR(ArucoAR* pAruchAR);

}

//class viewer
//{
//public:
//	viewer();
//
//	std::mutex mMutex;
//	Mat mIm;
//
//	void run();
//	void update(ArucoAR* pAruco);
//};
