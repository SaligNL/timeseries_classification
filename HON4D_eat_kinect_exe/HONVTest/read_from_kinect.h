
#ifndef READ_FROM_KINECT_H
#define READ_FROM_KINECT_H

#include <iostream>
#include <vector>
#include <time.h>

/// opencv and kinect lib
#include <Windows.h>
#include <opencv2/opencv.hpp>  
#include "opencv2/highgui/highgui.hpp"
#include <NuiApi.h>
#include <NuiSensor.h>
#include <NuiSkeleton.h>

#include "myMatrix.h"
#include "..\\fall_down_detection\Falldetectionfunc.h"


using namespace std;
using namespace cv;

#define COLOR_WIDTH 640    
#define COLOR_HIGHT 480    
#define DEPTH_WIDTH 320    
#define DEPTH_HIGHT 240    
#define SKELETON_WIDTH 320    
#define SKELETON_HIGHT 240    
#define CHANNEL 3
#define FALL_TEXT_WIDTH 640
#define FALL_TEXT_HIGHT 300
#define EAT_TEXT_WIDTH 640
#define EAT_TEXT_HIGHT 300

// const int SAMPLE_RATE = 2; /// select one frame out of 2

class Read_Kinect
{
public:
	Read_Kinect()
	{
		_current_real_depth.resize(DEPTH_HIGHT, DEPTH_WIDTH);
		_current_skeleton_pixel_coords.resize(NUI_SKELETON_POSITION_COUNT);
		_current_skeleton_realworld_coords.resize(NUI_SKELETON_POSITION_COUNT);
		_if_skeleton = false;
		_avg_dis = 1000;
		_if_start_cache = 0;
		_remove_num = 0;
		_if_shutdown = false;
		_sample_index = 1;
		_if_eat = false;
		_time_index = 0;
		_current_process_frame = 0;
		_if_bottom_right_change = false;
		_record_video_length = 5000;
		_total_frames = 0;
		SAMPLE_RATE = 2; // default value, select one frame out of 2
	}

	int drawColor(HANDLE h);
	int drawDepth(HANDLE h);
	int drawSkeleton(Mat & skeleton);
	int drawSkeleton();

	int control_initialize();
	int control_running();
	int control_finish();
	int control();

	void visualization_setup();
	void visualization_update();
	const std::vector<MyCoords>& get_skeleton_realworld_coords() {return _pre_skeleton_realworld_coords;}

	void remove_cache();
	bool synchronize_remove_test();
	void sample();
	void record_video();

public:
	HANDLE h1, h2, h3, h4, h5;
	BYTE buf[DEPTH_WIDTH * DEPTH_HIGHT * CHANNEL];
	Mat _color;
	Mat _depth;
	Mat _skeleton;

	vector<myMatrix> _real_depth_cache;
	vector<vector<pair<int, int> > > _skeleton_pixel_coords_cache;
	vector<vector<MyCoords> > _skeleton_realworld_coords_cache;
	int _cache_size;
	
	myMatrix _current_real_depth; /// for calculating the LocalHON4D
	/// pixel coordinates and real-world coordinate for current frame
	vector<pair<int, int> > _current_skeleton_pixel_coords;
	std::vector<MyCoords> _current_skeleton_realworld_coords;
	/// pixel coordinates and real-world coordinate for previous frame
	/// if _if_skeleton == true and the skeleton info cannot be detected in current frame, then the value is assigned by the value of the 
	/// previous frame
	vector<pair<int, int> > _pre_skeleton_pixel_coords;
	std::vector<MyCoords> _pre_skeleton_realworld_coords;
	bool _if_skeleton; /// if could detect the skeleton
	int _if_start_cache;
	bool _if_shutdown;
	int _remove_num;

	int _sample_index;

	/// for visualization
	Mat _dispImg;
	Mat _top_left_img;
	Mat _top_right_img;
	Mat _bottom_left_img;
	Mat _bottom_right_img;
	
	vector<Mat> _color_cache;
	bool _if_bottom_right_change;


	/// for eating detection
	bool _if_eat;
	clock_t _start_time_display;
	clock_t _end_time_display;
	int _time_index;
	int _current_process_frame;
	long double _current_decision_value;

	/// for fall down detection
	double _avg_dis;
	int _if_fall_down;
	Fall_Detection fall_detection;

	int _record_video_length;
	int _total_frames;
	VideoWriter _vwr;
	int SAMPLE_RATE;
};

#endif