#ifndef FALL_DETECTION_H
#define FALL_DETECTION_H

#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"


using namespace cv;
using namespace std;

const double fall_threshold = 0.4; 

class Fall_Detection
{
public:
	Fall_Detection()
	{
		fall_counter = 0;  
		fall_temp = 0; 
		fall_output_dis = 0; 
		fall_output_vel = 0; 
		vertical_dis = 0; 
		pre_vertical_dis = 0; 
		frame_counter = 0;  

		fall_duration = 1; 
		savedemo = true;  
	}
	int falldetectiondistance(double vertical_dis);

	//int falldetectionvelocisy(vector<vector<double>>& vertical_velocity, 
	//	                      int& Nframes,
	//						  int& fall_output_vel, 
	//						  IplImage* color); 
	int get_fall_output_dis(){return fall_output_dis;}
protected:
private:
	int fall_output_dis; /// final decision 
	int fall_counter;  
	int fall_temp; 
	int fall_output_vel; 
	double vertical_dis; 
	double pre_vertical_dis; 
	int frame_counter;  

	int fall_duration; 
	bool savedemo; 
};


#endif
