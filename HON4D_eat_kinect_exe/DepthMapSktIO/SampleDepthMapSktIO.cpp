// SampleDepthMapSktIO.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "DepthMapSktBinFileIO.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>


using namespace std;

//This is a sample program to load a depth video file (*.bin). 
//Authors: Zicheng Liu, Jiang Wang.
//A depth video file consists of a sequence of depth maps. Each frame corresponds to one depth map.
//A depth map is a matrix of depth values (see CDepthMapSkt in DepthMapSkt.h). For each pixel, its skeleton ID
//is also stored.
//I purposely did not create a data structure to hold all the frames of the depth video because the implementation
//of such data structure is better left to the application developers.

/// video index --> frame index --> skeleton index --> skeleton coodinate of x and y  
vector<vector<vector<pair<int, int> > > > _skeleton_info;


int read_skeleton_information()
{
	int rtn = 0; 

///	system("dir /a-d /s /b  E:\\kinect\\MSR_activity_dataset\\MSRDailyAct3D\\*.txt* >..\\..\\..\\skeleton_all.txt");


	ifstream input;
	input.open("..\\..\\..\\skeleton_all.txt");
	if (input.fail())
	{
		cout<<"error: cannot open file"<<"..\\..\\..\\skeleton_all.txt"<<endl;
		exit(1);
	}
	vector<string> skeleton_vec;
	string str;
	int index = 0;
	while (getline(input, str))
	{
		index ++;
		skeleton_vec.push_back(str);
	}

	cout<<"the size of skeleton_vec: "<<skeleton_vec.size()<<endl;

	_skeleton_info.resize(skeleton_vec.size());

	int max_xd = 0;
	int max_yd = 0;

	int image_width = 320;
	int image_height= 240;
	ifstream skeleton_input;

	int total_frames = 0;
	int x_bigger_160 = 0;

	for (int i=0; i<skeleton_vec.size(); i++)
	{
		skeleton_input.open(skeleton_vec[i].c_str());
		int frame_number;
		int joints_number;
		skeleton_input >> frame_number >> joints_number;
		total_frames += frame_number;
		cout<<i<<"  frame number: "<<frame_number<< " joint number: "<<joints_number<<endl;
		vector<vector<pair<int, int> > > skeleton_one_video;
		skeleton_one_video.resize(frame_number);
		for (int k = 0; k<frame_number; k++)
		{
			int lines;
			skeleton_input  >>  lines;
			if (lines == 0)
			{
				skeleton_one_video[k] = skeleton_one_video[k-1];
				continue;
			}
			vector<pair<int, int> > skeleton;
			skeleton.resize(lines/2);
			int min_x = image_width;
			int min_y = image_height;
			int max_x = 0;
			int max_y = 0;

			for ( int j=0; j<lines; j++)
			{
				if (j % 2 == 1)
				{
					double x, y, depth, nouse;
					skeleton_input >> x >> y >> depth >> nouse;
					if (x == 0 && y == 0 && depth == 0)
					{
						continue;
					}
					if (y < 0)
					{
						y = 0;
						//cout<<x<<" "<<y<<endl;						
						//cout<<skeleton_vec[i]<<" "<<k<<" "<<j<<" "<<y<<endl;
					}
					else if (y > 1)
					{
						y = 1;
					}
					if (x < 0)
					{
						x = 0;
					}
					else if (x > 1)
					{
						x = 1;
					}
					int image_x, image_y;
					image_x = (int)(image_width*x);
					image_y = (int)(image_height* y);
					pair<int, int> axis = make_pair(image_x, image_y);
					int index = j / 2;
					skeleton[index] = axis;

					if (min_x > image_x)
					{
						min_x = image_x;
					}
					if (max_x < image_x)
					{
						max_x = image_x;
					}
					if (min_y > image_y)
					{
						min_y = image_y;
					}
					if (max_y < image_y)
					{
						max_y = image_y;
					}
				}
				else
				{
					double nouse;
					skeleton_input >> nouse >> nouse >> nouse >> nouse;
				}
			}
			skeleton_one_video[k] = skeleton;

			if (max_x-min_x > 160)
			{
				x_bigger_160 ++;
			}

			if (max_xd < max_x - min_x)
			{
				max_xd = max_x - min_x;
				//cout<<skeleton_vec[i]<<"  "<<k<<"  max_xd: "<<max_xd<<"  max_yd: "<<max_yd<<"   "<<max_x<<" "<<min_x<<endl;

			}
			if (max_yd < max_y - min_y)
			{
				max_yd = max_y - min_y;
				//cout<<skeleton_vec[i]<<" max_xd: "<<max_xd<<"  max_yd: "<<max_yd<<"   "<<max_y<<" "<<min_y<<endl;

			}



		}

		_skeleton_info[i] = skeleton_one_video;
		



		skeleton_input.close();
	}

	input.close();

	cout<<"load skeleton information finished!"<<endl;
	cout<<"max_xd: "<<max_xd<<"  max_yd: "<<max_yd<<endl;
	cout<<"total frames: "<<total_frames<<endl;
	cout<<"x_bigger_160: "<<x_bigger_160<<endl;


	return rtn;
}


