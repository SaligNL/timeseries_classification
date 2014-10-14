#include "stdafx.h"

/// internal lib
#include "HON4D.h"
#include "sequential_version.h"
#include "..\libsvm\mysvm.h"
#include "LocalHON4D.h"
#include "..\DepthMapSktIO\DepthMapSktBinFileIO.h"
#include "read_from_kinect.h"

/// general lib
#include <iostream>
#include <vector>
#include <conio.h>
#include <string>
#include <fstream>
#include <exception>
#include <WinDef.h>
#include <Windows.h>
#include <time.h>
#include <stdlib.h>


using namespace std;
using namespace cv;

const int THRESHOLD_DELAY = 2000; /// 1000 frames delay means the processing speed is not real time 
int OUTPUT_RESULT_TIME_SLOT = 5000; /// the time interval between two output result (in millisecond)


int occurence_times(const vector<int>& vec, int find_value)
{
	int rtn = 0;

	for (int i=0; i<(int)vec.size(); i++)
	{
		if (vec[i] == find_value)
		{

			rtn ++;
		}
	}

	return rtn;
}

// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
const std::string currentDateTime() 
{
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

	return buf;
}

int readProjectors(int pNo,string projectorsPath, CvMat* &p)
{
	ifstream pfile;
	char buffer[4];
	sprintf(buffer,"%03d",pNo);
	string projFile = projectorsPath + buffer + ".txt";
	pfile.open(projFile);
	string line;
	int ind = 0;
	float v;

	getline (pfile,line);
	int sz = atof(line.c_str());
	p =  cvCreateMat(sz,4,CV_32FC1);

	while ( pfile.good() )
	{
		getline (pfile,line);
		if (line.length()<2)
			continue;
		//cout << line << endl;


		stringstream   linestream(line);
		string         value;

		getline(linestream,value,',');
		v = atof(value.c_str());
		cvSet2D(p,ind,0,cvScalar(v));

		getline(linestream,value,',');
		v = atof(value.c_str());
		cvSet2D(p,ind,1,cvScalar(v));


		getline(linestream,value,',');
		v = atof(value.c_str());
		cvSet2D(p,ind,2,cvScalar(v));

		getline(linestream,value,',');
		v = atof(value.c_str());
		cvSet2D(p,ind,3,cvScalar(v));


		ind++;
	}
	pfile.close();
	return sz;
}

/// global variable for multi-thread programing
Read_Kinect _read_kinect;

/// for multi_threads
DWORD WINAPI thread_read_kinect(LPVOID lpParamter)
{
	_read_kinect.control();
	return 0;
}

/// for multi_threads
int thread_classification(int argc, const char* argv[])
{
	if (argc < 3)
	{
		cout<<"error: the input arguments should be 3."<<endl;
		cout<<"usage: sample rate(default 2), rest port(default: http://localhost:27080), output_time_slot(default 5000 ms)"<<endl;
		exit(1);
	}
	//_read_kinect.SAMPLE_RATE = atoi(argv[1]);
	string rest_port = string(argv[1]);
	OUTPUT_RESULT_TIME_SLOT = atoi(argv[2]);
	//cout<<"sample rate: "<<_read_kinect.SAMPLE_RATE<<endl;
	cout<<"rest port: "<<rest_port<<endl;
	cout<<"output time slot: "<<OUTPUT_RESULT_TIME_SLOT<<endl;

	// Path for the 4D projectors for the polychoron (provided in data folder)
	string projectorsPath = "..\\Data\\Projectors\\";  

	int mode = 3;

	/// if use 3D joint position feature
	int if_3D_joint_position = 2;

	int cell_x = 3;
	int cell_y = 3;
	int cell_t = 1;
	int hdiff = 1;
	//	int pNo = 0;
	int pNo = 0;

	//	initialize model size
	int patch_w,patch_h, patch_depth, window_w, window_h, x0,y0;

	//	define HON4D class
	LocalHON4D * local_HON4D;

	//	compute feature length
	int HON4DFeatLen;

	//	define feature vector
	float *pfHON4DFeat = NULL;

	// read the projectors
	CvMat* p= NULL;
	int projDim = readProjectors(pNo,projectorsPath,p);

	string line;
	patch_w = 12;
	patch_h = 12;
	patch_depth = 4;
	int cellwidth = floor(double(patch_w/cell_x));
	int cellheight = floor(double(patch_h/cell_y));
	int celldepth = floor(double(patch_depth/cell_t));
	patch_w = cellwidth*cell_x;
	patch_h = cellheight*cell_y;
	patch_depth = celldepth*cell_t;
	x0 = 0;
	y0 = 0;
	window_w = DEPTH_WIDTH;
	window_h = DEPTH_HIGHT;

	cout<<"Start to set up:"<<endl<<endl;
	/// the file path of the training svm file

	/// if_half == 0: use entire frames in the video 
	/// if_half == 0: use half frames (one out of two ) in the video 
	int if_half = 1;  
	string train_data;
	wstring train_path;

	if(if_half)
	{
		train_data = "..\\data\\train_eat_svm_half.dat";
		train_path = L"..\\Data\\training_set_if3d_2_half\\";
	}
	else
	{
		train_data = "..\\data\\train_eat_svm.dat";
		train_path = L"..\\Data\\training_set_if3d_2\\";
	}

	MySVM mySVM(train_data, if_half);	

	/// check if need to train model
	ifstream fin;
	if(if_half)
	{
		fin.open("..\\data\\svm_eat_model_half.dat");
	}
	else
	{
		fin.open("..\\data\\svm_eat_model.dat");
	}
	if (!fin)
	{
		mySVM.scale_format_convertion_data(train_path, train_data.c_str(), 1);
		cout<<"training set scale operation and format conversion is completed!"<<endl;
		mySVM.control_train();
		cout<<"the svm model training is completed!"<<endl;
	}
	
	mySVM.load_training_min_max();
	mySVM.load_training_model();

	Sequential sequ(mode);
	sequ.set_sliding_window_width(1);
	sequ.set_start_point(0);

	int pre_start_frame = -1;
	int pre_end_frame = -1;

	/// the local-HON4D feature of previous frame, which is used to avoid overlap calculation for current frame
	vector<vector<float> > pre_HON4D_feature;

	int count = 0;
	clock_t start, end;
	start = clock();

	_read_kinect._if_start_cache ++;

	/// initialize the database
	string remove = "curl --data \"criteria={}\" \"" + rest_port + "/foo/bar/_remove\"";
	system(remove.c_str());cout<<endl;
	//const char eating[] = "curl --data \"docs={\"\"eating\"\":0}\" \"http://localhost:27080/foo/bar/_insert\""; // default value: non-eating
	//system(eating);cout<<endl;
	//const char falling[] = "curl --data \"docs={\"\"falling\"\":0}\" \"http://localhost:27080/foo/bar/_insert\""; // default value: non-falling
	//system(falling); cout<<endl;

	cout<<"set up finish!"<<endl<<endl;

	vector<int> eating_result;
	vector<int> falling_result;
	bool if_start_flag = false;
	clock_t start_t;

	for ( int i=0; ; i++)
	{
		if (_read_kinect._if_shutdown)
		{
			exit(0);
		}

		if (_read_kinect._if_start_cache < 2)
		{
			continue;
		}
		if (!if_start_flag)
		{
			if_start_flag = true;
			cout<<"start to monitor..."<<endl;
			start_t = clock();
		}
		cout<<endl<<endl;
		//cout<<"new iteration!"<<endl;

		//exit
		int c = waitKey(1);
		if (c == 27 || c == 'q' || c == 'Q')
			break;


		while (_read_kinect._remove_num != 0)
		{
			cout<<"_remove_num != 0"<<endl;
			Sleep(100);
		}
		//cout<<"cache size: "<<_read_kinect._cache_size<<endl;
		while (sequ.get_sliding_window_size() > _read_kinect._cache_size)
		{
			cout<<"sequ.get_sliding_window_size() > _read_kinect._cache_size! Waiting more depth image!"<<endl;
			Sleep(1000);
		}
		if (_read_kinect._cache_size > THRESHOLD_DELAY)
		{
			cout<<"there are over "<<THRESHOLD_DELAY<<" depth frames stored in the cache. the processing speed cannot be real-time."<<endl;
			exit(1);
		}

		clock_t t0, t1, t2, t3, t4, t5, t6;
		t0 = clock();


		//cout<<"sliding window width: "<<sequ.get_windows_width()<<endl;
		//cout<<"current_index: "<<sequ.get_current_index()<<endl;

		t1 = clock();

		_read_kinect._current_process_frame = sequ.get_sliding_window_size()-1;
		_read_kinect._if_bottom_right_change = true;
		local_HON4D = new LocalHON4D(if_3D_joint_position, cellwidth, cellheight, window_w, window_h, patch_w, patch_h, patch_depth,
			cell_t,hdiff,p,projDim, sequ, _read_kinect, pre_start_frame, pre_end_frame);
		HON4DFeatLen=local_HON4D->get_feature_length();
		pfHON4DFeat = new float[HON4DFeatLen];
		
		t2 = clock();

		if(local_HON4D->GetFeature(pfHON4DFeat, pre_HON4D_feature)!= HON4DFeatLen){
			printf("get hon4d feature error!\n");
			return 0;
		}
		t3 = clock();
		//cout<<"stage1: "<<(t3-t0)/1000.0<<endl;

		local_HON4D->set_pre_vector_feature(pre_HON4D_feature);
		pre_start_frame = local_HON4D->get_start_frame();
		pre_end_frame = local_HON4D->get_end_frame();
		t4 = clock();
		delete local_HON4D;

		/// scale
		mySVM.scale_one_sample(HON4DFeatLen, pfHON4DFeat);
		//cout<<"scaling completed!"<<endl;

		/// predict
		int eating_predict_label;
		double cprob;
		mySVM.predict_one_sample( pfHON4DFeat, HON4DFeatLen, eating_predict_label, cprob);
		_read_kinect._current_decision_value = cprob;
		t5 = clock();
		//cout<<"time for svm prediction: "<<(t5-t4)/1000.0<<endl;

		if (eating_predict_label == 1)
		{
			_read_kinect._if_eat = true;
		}
		eating_result.push_back(eating_predict_label);
		falling_result.push_back(_read_kinect._if_fall_down);

		if (t5 - start_t >= OUTPUT_RESULT_TIME_SLOT)
		{
			start_t = clock();
			int eating_times  = occurence_times(eating_result, 1);
			int falling_times = occurence_times(falling_result, 1);
			int eating_final_predict = (eating_times>0) ? 1 : 0;
			int falling_final_predict = (falling_times>0) ? 1 : 0;
			eating_result.clear();
			falling_result.clear();

			string now_time = currentDateTime();
			/// update the database	
			if (eating_final_predict==1)
			{
				string eating = "curl --data \"docs={\"\"eating\"\":1, \"\"time\"\":\"\"" + now_time + "\"\"}\" \"" + rest_port + "/foo/bar/_insert\"";
				system(eating.c_str());cout<<endl;
				cout<<"predicted label: Eating!!!!!!!!!!!!!!!"<<endl;
			}
			else
			{
				string eating = "curl --data \"docs={\"\"eating\"\":0, \"\"time\"\":\"\"" + now_time + "\"\"}\" \"" + rest_port + "/foo/bar/_insert\"";
				system(eating.c_str());cout<<endl;
				cout<<"non-eating!"<<endl;
			}
			if (falling_final_predict==1)
			{
				string falling = "curl --data \"docs={\"\"falling\"\":1, \"\"time\"\":\"\"" + now_time + "\"\"}\" \"" + rest_port + "/foo/bar/_insert\"";
				system(falling.c_str());cout<<endl;
				cout<<"Fall down!!!!!!!!!!!!!!!"<<endl;
			}
			else
			{
				string falling = "curl --data \"docs={\"\"falling\"\":0, \"\"time\"\":\"\"" + now_time + "\"\"}\" \"" + rest_port + "/foo/bar/_insert\"";
				system(falling.c_str());cout<<endl;
				cout<<"Normal (no fall down)!"<<endl;
			}

		}
		
		
		/// next decision
		int remove_num = sequ.next_decision_kinect(eating_predict_label, cprob);
		if (remove_num > 0)
		{
			_read_kinect._remove_num = remove_num;
		}
		delete [] pfHON4DFeat;

		count ++;
		end = clock();
		//std::cout<<"the running time is "<<(end-start)/1000.0<<std::endl; 

		sequ.set_sliding_window_width_plus_k();
		t6 = clock();
		//cout<<"total time for this video: "<<(t6-t0)/1000.0<<endl;
		cout<<endl<<endl;

	}

	mySVM.free_train_model();
	return 0;
}

int main(int argc, const char* argv[])
{	
	/// multi-thread programing
	/// thread 1: running kinect to get realtime dataset 
	HANDLE hThread1 = CreateThread(NULL, 0, thread_read_kinect, NULL, 0, NULL);
	CloseHandle(hThread1);

	/// thread 2: classification with Local HON4D feature and SVM classifier
	thread_classification(argc, argv);

	return 0;	
}
