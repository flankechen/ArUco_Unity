
#include <aruco/aruco.h>


cv::Mat extrace_roi(cv::Mat& src, aruco::Board& last_pattern);
cv::Mat extrace_roi(cv::Mat& src, aruco::Marker& last_pattern);

void blur_estimate(cv::Mat& src, aruco::BoardDetector& b_detector, aruco::Board& last_pattern, cv::Mat& last_roi, const cv::Mat& cameraMatrix, const cv::Mat& distortions);
void blur_estimate(cv::Mat& src, std::vector<aruco::Marker>& markers, aruco::Marker& last_pattern, cv::Mat& last_roi, const cv::Mat& cameraMatrix, const cv::Mat& distortions);

