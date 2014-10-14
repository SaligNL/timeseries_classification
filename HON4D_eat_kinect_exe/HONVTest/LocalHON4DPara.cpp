#include <iostream>
#include <fstream>
#include <string>
#include <vector>


#include "LocalHON4D.h"
#include "..\pwjTools\file_operations.h"

using namespace std;

LocalHON4D::LocalHON4D(int mode, int if_3D_joint_position, int nCellWidth, int nCellHeight, int nWinWidth, int nWinHeight, int patch_w, int patch_h, int patch_depth,
	wstring vidPath, int cell_t, int hdiff,CvMat* proj,int binsz, const vector<vector<pair<int, int> > >& skeleton_info, 
	const vector<vector<MyCoords> >& joint_realworld)
{
	_if_sequential = false;
	cell.width = nCellWidth;
	cell.height = nCellHeight;
	cell.depth = patch_depth / cell_t;
	DetWin.width = nWinWidth;
	DetWin.height = nWinHeight;
	_patch.width = patch_w;
	_patch.height = patch_h;
	_patch.depth = patch_depth;
	_if_3D_joint_position = if_3D_joint_position;

	int iRC = 0;
	if (mode == 1)
	{
		iRC = File_Operations::SearchDirectory(vecFiles,vidPath, L"png");
		noImages = vecFiles.size();
		cout<<"nImages: "<<noImages<<endl;
		_images_value.resize(noImages);
	}
	else if(mode == 3)
	{
		iRC = File_Operations::SearchDirectory(vecFiles,vidPath, L"txt");
		noImages = vecFiles.size();
		cout<<"nImages: "<<noImages<<endl;
		_images_value.resize(noImages);
	}
	else if(mode == 2)
	{
		string file(vidPath.begin(), vidPath.end());
		const char* depthFileName = file.c_str();
		FILE * fp = fopen(depthFileName, "rb");

		if(fp == NULL)
		{
			cout<<"error: cannot open the file:" << depthFileName<<endl;
			exit(1);
		}


		int nofs = 0; //number of frames conatined in the file (each file is a video sequence of depth maps)
		int ncols = 0;
		int nrows = 0;
		ReadDepthMapSktBinFileHeader(fp, nofs, ncols, nrows);

		noImages = nofs;
		//		cout<<depthFileName<<endl;
		cout<<"nImages: "<<noImages<<endl;		
		_images_value.resize(noImages);
		ReadAllImagesBin(fp);
	}
	else
	{
		cout<<"error: mode"<<endl;
		exit(1);
	}

	SetDefaultParam();

	HON4D_difference = hdiff;
	PixBegin = 1;

	_skeleton_info = skeleton_info;
	_joint_number = (int)_skeleton_info[0].size();
	_joints_realworld_coords = joint_realworld;

	_mode = mode;



	//preparePMat();
	cell.bin_n_HON4D = binsz;
	p =  cvCreateMat(cell.bin_n_HON4D,4,CV_32FC1);
	cvCopy(proj,p);
	binThreshold =1.3090;// 3.5674 for 600 // 0.5 for 24//  1.3090 for 120 


	UpdateEnviorment();

}

LocalHON4D::LocalHON4D(int mode, int if_3D_joint_position, int nCellWidth, int nCellHeight, int nWinWidth, int nWinHeight, int patch_w, int patch_h, int patch_depth,
	int cell_t, int hdiff,CvMat* proj,int binsz, const vector<vector<pair<int, int> > >& skeleton_info, 
	const vector<vector<MyCoords> >& joint_realworld, const Sequential& sequ, int pre_start_frame, int pre_end_frame)
{
	_if_sequential = true;
	cell.width = nCellWidth;
	cell.height = nCellHeight;
	cell.depth = patch_depth / cell_t;
	DetWin.width = nWinWidth;
	DetWin.height = nWinHeight;
	_patch.width = patch_w;
	_patch.height = patch_h;
	_patch.depth = patch_depth;
	_if_3D_joint_position = if_3D_joint_position;
	_pre_start_frame = pre_start_frame;
	_pre_end_frame = pre_end_frame;
	set_start_end_frames(sequ.get_current_index(), sequ.get_windows_width()+sequ.get_current_index()-1);
	compute_overlap_area();

	int iRC = 0;
	if (mode == 1)
	{
		cout<<"nImages: "<<noImages<<endl;
		_images_value.resize(noImages);
	}
	else if(mode == 3) /// for data from kinect
	{
		noImages = sequ.get_windows_width();
		cout<<"nImages: "<<noImages<<endl;	
		_images_value.resize(noImages);

		for (int i=My_Max(_start_frame, _overlap_end-_patch.depth); i<=_end_frame; i++)
		{
			int ind = sequ.get_current_index_bin() + i - _start_frame;
			_images_value[i-_start_frame] = sequ.get_vec_image_value_bin()[ind];
		}

		_skeleton_info.assign(skeleton_info.begin(), skeleton_info.begin()+_end_frame+1);
		_joint_number = (int)_skeleton_info[0].size();
		_joints_realworld_coords.assign(joint_realworld.begin(), joint_realworld.begin()+_end_frame+1);
	}
	else if(mode == 2)
	{
		noImages = sequ.get_windows_width();
		cout<<"nImages: "<<noImages<<endl;	

		_images_value.resize(noImages);

		for (int i=My_Max(0, _overlap_end-_patch.depth); i<=_end_frame; i++)
		{
			int ind = sequ.get_current_index_bin() + i - _start_frame;
			if(i >= _start_frame)
				_images_value[i-_start_frame] = sequ.get_vec_image_value_bin()[ind];
			else
				continue;

		}
		_skeleton_info.assign(skeleton_info.begin()+_start_frame, skeleton_info.begin()+_end_frame+1);
		_joint_number = (int)_skeleton_info[0].size();
		_joints_realworld_coords.assign(joint_realworld.begin()+_start_frame, joint_realworld.begin()+_end_frame+1);
	}
	else
	{
		cout<<"error: mode"<<endl;
		exit(1);
	}

	SetDefaultParam();

	HON4D_difference = hdiff;
	PixBegin = 1;


	cout<<"size of joint_realworld: "<<_joints_realworld_coords.size()<<endl;
	cout<<"size of skeleton: "<<_skeleton_info.size()<<endl;
	_mode = mode;



	//preparePMat();
	cell.bin_n_HON4D = binsz;
	p =  cvCreateMat(cell.bin_n_HON4D,4,CV_32FC1);
	cvCopy(proj,p);
	binThreshold =1.3090;// 3.5674 for 600 // 0.5 for 24//  1.3090 for 120 


	UpdateEnviorment();

}

/// for kinect version
LocalHON4D::LocalHON4D(int if_3D_joint_position, int nCellWidth, int nCellHeight, int nWinWidth, int nWinHeight, int patch_w, int patch_h, int patch_depth,
	int cell_t, int hdiff,CvMat* proj,int binsz, const Sequential& sequ, const Read_Kinect& read_kinect, int pre_start_frame, int pre_end_frame)
{
	_mode = 3;
	_if_sequential = true;
	cell.width = nCellWidth;
	cell.height = nCellHeight;
	cell.depth = patch_depth / cell_t;
	DetWin.width = nWinWidth;
	DetWin.height = nWinHeight;
	_patch.width = patch_w;
	_patch.height = patch_h;
	_patch.depth = patch_depth;
	_if_3D_joint_position = if_3D_joint_position;
	_pre_start_frame = pre_start_frame;
	_pre_end_frame = pre_end_frame;
	set_start_end_frames(sequ.get_current_index(), sequ.get_windows_width()+sequ.get_current_index()-1);
	compute_overlap_area();


	noImages = sequ.get_windows_width();
	//cout<<"nImages: "<<noImages<<endl;	
	_images_value.resize(noImages);

	for (int i=My_Max(_start_frame, _overlap_end-_patch.depth); i<=_end_frame; i++)
	{
		int ind = i - _start_frame;
		_images_value[ind] = read_kinect._real_depth_cache[ind];
	}

	_skeleton_info.assign(read_kinect._skeleton_pixel_coords_cache.begin(), read_kinect._skeleton_pixel_coords_cache.begin()+sequ.get_sliding_window_size());
	_joint_number = (int)_skeleton_info[0].size();
	if (_joint_number < 20)
	{
		cout<<"error: _joint_number < 20"<<endl;
		exit(1);
	}
	
	_joints_realworld_coords.assign(read_kinect._skeleton_realworld_coords_cache.begin(), read_kinect._skeleton_realworld_coords_cache.begin()+sequ.get_sliding_window_size());

	SetDefaultParam();

	HON4D_difference = hdiff;
	PixBegin = 1;


	//cout<<"size of joint_realworld: "<<_joints_realworld_coords.size()<<endl;
	//cout<<"size of skeleton: "<<_skeleton_info.size()<<endl;

	//preparePMat();
	cell.bin_n_HON4D = binsz;
	p =  cvCreateMat(cell.bin_n_HON4D,4,CV_32FC1);
	cvCopy(proj,p);
	binThreshold =1.3090;// 3.5674 for 600 // 0.5 for 24//  1.3090 for 120 


	UpdateEnviorment();

}


void LocalHON4D::SetDefaultParam()
{
	images = NULL;

	HON4D_dx_image=NULL;
	HON4D_dy_image=NULL;
	HON4D_dz_image=NULL;
	HON4D_difference = 5;
	_layer_number = 1;
	if(_layer_number < 3)
		_top_n = 10 * _layer_number;
	else if(_layer_number == 3)
		_top_n = 28;

}

void LocalHON4D::UpdateEnviorment()
{
	total_hist_of_cells = new float [cell.bin_n_HON4D];

	cell.numR = _patch.width/cell.width;
	cell.numC = _patch.height/cell.height;
	cell.numD = _patch.depth/cell.depth;

	_patch.featLen = cell.numD*cell.numR * cell.numC * cell.bin_n_HON4D;
	//cout<<cell.numR<<" "<<cell.numC<<" "<<cell.numD<<" "<<cell.bin_n_HON4D<<endl;
	//	cout<<"_patch_featLen: "<<_patch.featLen<<endl;
	if (_if_3D_joint_position == 0)
		DetWin.featLen = _top_n * _joint_number * cell.numD*cell.numR * cell.numC * cell.bin_n_HON4D;
	else if(_if_3D_joint_position == 1)
		DetWin.featLen =3 * _joint_number * _joint_number * _top_n;
	else
		DetWin.featLen =3 * _joint_number * _joint_number * _top_n + _top_n * _joint_number * cell.numD*cell.numR * cell.numC * cell.bin_n_HON4D;


	//cout<<"featLen: "<<DetWin.featLen<<endl;


	PixBegin = 1;
	if(NULL != HON4D_dx_image){
		delete [] HON4D_dx_image;
		HON4D_dx_image = NULL;
	}
	if(NULL != HON4D_dy_image){
		delete [] HON4D_dy_image;
		HON4D_dy_image = NULL;
	}
	if(NULL != HON4D_dz_image){
		delete [] HON4D_dz_image;
		HON4D_dz_image = NULL;
	}


}

LocalHON4D::~LocalHON4D()
{

	// delete the images
	if (_mode == 1)
	{
		for (int i=0;i<noImages;i++)
			SafeImgDel(images[i]);
	}


	SafeMatDel(p);


	if(NULL != HON4D_dx_image){
		delete [] HON4D_dx_image;
	}
	if(NULL != HON4D_dy_image){
		delete [] HON4D_dy_image;
	}

	if(NULL != HON4D_dz_image){
		delete [] HON4D_dz_image;
	}

	if(NULL != total_hist_of_cells){
		delete [] total_hist_of_cells;
	}
}


int LocalHON4D::read_skeleton_information(vector<vector<vector<pair<int, int> > > >& skeleton_info, vector<vector<vector<MyCoords> > > & joint_realworld_coord)
{
	int rtn = 0; 

	system("dir /a-d /s /b  ..\\..\\..\\..\\MSR_activity_dataset\\MSRDailyAct3D\\*.txt* >..\\..\\Data\\MSRDailyAct3D\\skeleton_all.txt");

	int joint_number = 20;

	ifstream input;
	input.open("..\\..\\Data\\MSRDailyAct3D\\skeleton_all.txt");
	if (input.fail())
	{
		cout<<"error: cannot open file"<<"..\\..\\Data\\MSRDailyAct3D\\skeleton_all.txt"<<endl;
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

	skeleton_info.resize(skeleton_vec.size());
	joint_realworld_coord.resize(skeleton_vec.size());

	int max_xd = 0;
	int max_yd = 0;

	int image_width = 320;
	int image_height= 240;
	ifstream skeleton_input;

	int total_frames = 0;

	for (int i=0; i<skeleton_vec.size(); i++)
	{
		skeleton_input.open(skeleton_vec[i].c_str());
		int total_lines = 0;
		/// get the total line numbers
		string tmp;
		while (getline(skeleton_input, tmp))
		{
			total_lines ++;
		}
		skeleton_input.close();
		skeleton_input.open(skeleton_vec[i].c_str());


		int frame_number;
		int joints_number;
		skeleton_input >> frame_number >> joints_number;
		total_frames += frame_number;
		//cout<<i<<"  frame number: "<<frame_number<< " joint number: "<<joints_number<<endl;
		vector<vector<pair<int, int> > > skeleton_one_video;
		skeleton_one_video.resize(frame_number);
		vector<vector<MyCoords> > realWorldCoords_one_video;
		realWorldCoords_one_video.resize(frame_number);

		int line_index = 1;
		for (int k = 0; k<frame_number; k++)
		{
			int lines;
			skeleton_input  >>  lines;
			if (lines != 0 && lines!= 40 && lines != 80)
			{
				cout<<"error: line != 0 && line!= 40 && line != 80"<<endl;
				cout<<i+1<<" "<<k+1<<"  "<<skeleton_vec[i].c_str()<<endl;
				cout<<line_index<<":  "<<lines<<endl;
				exit(1);
			}

			line_index ++;


			if (lines < joint_number*2)
			{
				skeleton_one_video[k] = skeleton_one_video[k-1];
				realWorldCoords_one_video[k] = realWorldCoords_one_video[k-1];
				continue;
			}


			vector<pair<int, int> > skeleton;
			skeleton.resize(joint_number);
			vector<MyCoords> realWorldCoord;
			realWorldCoord.resize(joint_number);

			for ( int j=0; j<lines; j++)
			{
				if (j>joint_number*2-1)
				{
					double nouse;
					skeleton_input >> nouse >> nouse >> nouse >> nouse;
					line_index ++;
					continue;
				}

				if (j % 2 == 1 && j<joint_number*2)
				{
					double x_v, y_v, depth, nouse;
					skeleton_input >> x_v >> y_v >> depth >> nouse;
					line_index ++;

					//if (x_v == 0 && y_v == 0 && depth == 0)
					//{
					//	skeleton_one_video[k] = skeleton_one_video[k-1];
					//	cout<<"error: x == 0 && y == 0 && depth == 0"<<endl;
					//	break;
					//}
					if (y_v < 0)
					{
						y_v = 0;
					}
					else if (y_v > 1)
					{
						y_v = 1;
					}
					if (x_v < 0)
					{
						x_v = 0;
					}
					else if (x_v > 1)
					{
						x_v = 1;
					}

					int image_x, image_y;
					image_x = (int)(image_width* x_v+0.5);
					image_y = (int)(image_height* y_v+0.5);
					pair<int, int> axis = make_pair(image_x, image_y);
					int ind_j = j / 2;
					skeleton[ind_j] = axis;
				}
				else if(j<joint_number*2)
				{
					double x_v, y_v, z_v, nouse;
					skeleton_input >> x_v >> y_v >> z_v >> nouse;
					int ind_j = j / 2;
					MyCoords coord(x_v, y_v, z_v);
					realWorldCoord[ind_j] = coord;

					line_index ++;

				}
			}
			skeleton_one_video[k] = skeleton;
			realWorldCoords_one_video[k] = realWorldCoord;
		}

		if (line_index != total_lines)
		{
			cout<<"error: line_index != total lines"<<endl;
			cout<<"total: "<<total_lines<<"  line_index: "<<line_index<<endl;
			exit(1);

		}
		skeleton_info[i] = skeleton_one_video;
		joint_realworld_coord[i] = realWorldCoords_one_video;

		skeleton_input.close();
	}

	input.close();

	//cout<<"load skeleton information finished!"<<endl;
	//cout<<"total frames: "<<total_frames<<endl;


	return rtn;
}


