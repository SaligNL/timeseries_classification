
#include "falldetectionfunc.h"

int Fall_Detection::falldetectiondistance(double vertical_dis)
{
	int fall_det;
	if (pre_vertical_dis != vertical_dis)
	{
		frame_counter = 0; 
	}else 
	{
		frame_counter++;
	}
	int fontFace = FONT_HERSHEY_SIMPLEX;
	double fontScale = 1;
	int thickness = 3;  
	cv::Point textOrg(20, 80);

	if (vertical_dis < fall_threshold) 
	{
		fall_det = 1;
	}else 
	{
		fall_det = 0;
	}

	if (fall_temp != fall_det)
	{
		fall_counter = 0; 
	} else 
	{
		fall_counter++; 
	}

	fall_temp = fall_det; 
	pre_vertical_dis = vertical_dis; 

	if ((fall_counter > (fall_duration*25)) && (fall_det==1)&&(vertical_dis!= 0))  
	{
		fall_output_dis = 1; 	
	}else 
	{
		fall_output_dis = 0; 	
	}
	return 0;
}

//int falldetectionvelocisy(vector<vector<double>>& vertical_velocity, 
//	                      int& Nframes, int& fall_output_vel, IplImage* color)
//{
//	double joint_velocity[20];  
//	double Ncounter = 0; 
//	double avg_joint_velocity = 0; 
//	double Jointcounter = 0; 
//	if (vertical_velocity[0].size() > Nframes)
//	{
//	     for (int j = 0; j < NUI_SKELETON_POSITION_COUNT; j++)
//         {
//			 joint_velocity[j] = 0; 
//			 for (int i = 0; i < Nframes; i++ )
//			 {
//                if (vertical_velocity[j][i] != 0)
//				{
//					joint_velocity[j] =  joint_velocity[j] + vertical_velocity[j][i]; 
//					Ncounter ++; 
//				}
//			 }
//			 joint_velocity[j] = joint_velocity[j]/Ncounter;
//			 if (joint_velocity[j]!= 0) 
//			 {
//				avg_joint_velocity = avg_joint_velocity + joint_velocity[j]; 
//				Jointcounter ++; 
//			 }
//		 }
//		 avg_joint_velocity = 1000 * avg_joint_velocity/Jointcounter; 
//		 int fontFace = FONT_HERSHEY_SIMPLEX;
//		 double fontScale = 1;
//		 int thickness = 3;  
//		 cv::Point textOrg(120, 50);
//		 std::stringstream ss;
//		 ss << avg_joint_velocity;
//		 std::string txt_avg_dis = ss.str();
//		 // cv::putText(Mat(color), txt_avg_dis, textOrg, fontFace, fontScale,
//		 // CV_RGB(0, 0, 255), thickness, 8);	
//	}
//	return 0;
//}