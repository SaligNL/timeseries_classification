

#ifndef SEQUENTIAL_H_
#define SEQUENTIAL_H_

#include "..\\DepthMapSktIO\DepthMapSktBinFileIO.h"
#include "myMatrix.h"

#include <iostream>
#include <vector>
#include <string>
#include <windows.h>


class Sequential
{
public:
	Sequential(){}
	Sequential(const std::string& filename, int mode=1):_FileName(filename)
	{
		_step_width = 0;
		_current_index = 0;
		_last_index = -1;
		_up_bound = 250;
		_prob_threshold = 0.35;
		_start_point = 0;
		_current_video_ind = 0;
		_mode = mode;
		_pre_read_size = 4;
		_current_index_bin = 0;
		_total_erase_items = 0;
		_eat_threshold = -0.6677;
		//_eat_threshold = -0.66;

		_window_start_step = 70;
		_window_size_step = 10;
	}

	Sequential(int mode=3)
	{
		_step_width = 0;
		_current_index = 0;
		_last_index = -1;
		_up_bound = 150;
		_prob_threshold = 0.35;
		_start_point = 0;
		_current_video_ind = 0;
		_mode = mode;
		_pre_read_size = 4;
		_current_index_bin = 0;
		_total_erase_items = 0;
		_eat_threshold = -0.6677;
	//	_eat_threshold = -0.70;

		_window_start_step = 60;
		_window_size_step = 10;
		_min_video_size = 50;
	}

	~Sequential(){}

	int get_all_files();
	int get_all_files_png();
	int get_all_files_bin();
	int preread_images_bin();
	int read_data_kinect();

	int SearchDirectory(std::vector<std::wstring> &refvecFiles,
		const std::wstring        &refcstrRootDirectory,
		const std::wstring        &refcstrExtension,
		bool                     bSearchSubdirectories = true);

	int split_trainingset_testset_png();
	int split_trainingset_testset_bin();

	/// still have some problems, hence no use temporarily 
	int generate_new_test_vecFile();

	/// generate one new test vecFile which is to give HON4D
	int generate_one_new_test_vecFile_png(std::vector<std::wstring>& new_test_vecFile, int& label, int& max_label);
	int generate_one_new_test_vecFile_bin(int& label, int& max_label);


	int get_test_sequence_length() {return (int)_new_test_frames_sequence.size();}
	int get_windows_width() const{ return _sliding_windows_width;}

	int set_sliding_window_width(int flag);

	void set_step_width(int step) {_step_width = step;}

	int get_step_width(){return _step_width;}
	void set_sliding_window_width_plus1(){_sliding_windows_width++;}
	void set_sliding_window_width_minus1(){_sliding_windows_width--;}
	void set_sliding_window_width_plus_k(){_sliding_windows_width += _window_size_step;}
	void set_sliding_window_width_minus_k(){_sliding_windows_width -= _window_size_step;}
	

	int get_current_index() const {return _current_index;}
	int get_current_index_bin() const{return _current_index_bin;}
	void set_current_index(int current){_current_index = current; _last_index = _current_index-1;}
	void set_start_point(int start) {_start_point = start; _current_index=start; _last_index = _current_index -1;}

	void set_predict_result(int label, double prob) 
	{
		if (_predict_labels.size() > _current_index+_up_bound+_window_size_step)
		{
			cout<<"error: _predict_labels.size() > _current_index+_up_bound+_window_size_step"<<endl;
			exit(1);
		}
		if (_predict_labels.size() > _current_index+_sliding_windows_width)
		{
			return;
		}
		_predict_labels.push_back(label);
		_predict_probs.push_back(prob);

		if (_current_index+_sliding_windows_width > (int)_predict_labels.size())
		{
			int numm = _current_index+_sliding_windows_width - (int)_predict_labels.size();
			for (int i=0; i<numm; i++)
			{
				_predict_labels.push_back(label);
				_predict_probs.push_back(prob);
			}
		}
		std::cout<<"size of predict_labels: "<<_predict_labels.size()<<std::endl;
		//for (int i=0; i<_predict_labels.size(); i++)
		//{
		//	std::cout<<i<<": "<<_predict_labels[i]<<"  ";
		//}
		//std::cout<<std::endl;
		//for (int i=0; i<_predict_labels.size(); i-++)
		//{
		//	std::cout<<i<<": "<<_predict_probs[i]<<"  ";
		//}
		//std::cout<<std::endl;

	}

	void set_skeleton_info(const std::vector<std::vector<vector<pair<int, int> > > >& skeleton_info,
	const std::vector<std::vector<std::vector<MyCoords> > >& joint_realworld_coords)
	{
		_skeleton_info = skeleton_info;
		_joint_realworld_coords = joint_realworld_coords;
	}

	int next_decision1();

	int next_decision2();
	int next_decision_kinect(int predict_label, double cprob);

	int set_true_labels_png();
	int set_true_labels_bin();

	void write_true_labels_to_files();

	double calculate_accuracy();

	const std::vector<myMatrix>& get_vec_image_value_bin() const {return _vec_image_value;}

	int get_total_test_length() {return _total_test_length;}

	int get_current_test_total_length() {return (int)_vec_image_value.size();}

	const std::vector<std::vector<std::pair<int, int> > >& get_test_skeleton() {return _test_skeleton_info;}
	const std::vector<std::vector<MyCoords> >& get_test_skeleton_real_world() {return _test_joint_realworld_coords;}

	void set_min_video_size(int k) {_min_video_size = k;}

	void print_predicted_label();
	int get_sliding_window_size() const {return _sliding_windows_width;}

private:
	// Path for a text file which has a list of the videos to be processed. (For an example file, see "Data\MSRAction\FileNames_01.txt")
	const std::string _FileName;

	/// depth information get from png file
	std::vector<std::pair<std::string, std::vector <std::wstring> > > _vecFiles;

	/// depth information got from bin file
	std::vector<myMatrix> _vec_image_value;
	std::vector<int> _vec_image_number;
	std::vector<std::string> _vec_file_path;
	int _current_video_ind;

	int _min_video_size;
	int _max_video_size;

	int _sliding_windows_width;

	int _step_width; /// the step width for sliding widow

	std::vector<std::pair<std::string, std::vector <std::wstring> > > _training_vecFiles;
	std::vector<std::pair<std::string, std::vector <std::wstring> > > _test_vecFiles;

	/// from bin file
	std::vector<std::pair<std::string, int> > _vec_file_path_training;
	std::vector<std::pair<std::string, int> > _vec_file_path_testing;

	/// newly segmented vecfile in one sliding window based on _sliding_widows_width
	std::vector<std::pair<std::string, std::vector <std::wstring> > > _new_test_vecFiles;
	std::vector <std::wstring> _new_test_frames_sequence;
	int _current_index;/// record the current process position of silding windows, corresponding to _new_test_frames_sequence

	int _average_test_length;
	int _total_test_length;
	
	/// for prediction
	std::vector<int> _predict_labels;
	std::vector<double> _predict_probs;
	std::vector<int> _true_labels;
	int _last_index; /// indicate the separated position for the last activity 
	double _current_accuracy;
	int _up_bound;
	double _prob_threshold;

	int _start_point;

	/// _mode == 1: read image from png files
	/// _mode == 2: read image from bin files
	/// _mode == 3: read image from kinect in realtime
	int _mode;


	/// there is a corresponding between _current_index and _current_index_bin
	int _current_index_bin;

	/// the size of videos read in one time
	int _pre_read_size;

	/// pixel coordinates and real-world coordinate
	std::vector<std::vector<vector<pair<int, int> > > > _skeleton_info;
	std::vector<std::vector<std::vector<MyCoords> > > _joint_realworld_coords;

	std::vector<std::vector<std::pair<int, int> > > _test_skeleton_info;
	std::vector<std::vector<MyCoords> > _test_joint_realworld_coords;

	int _total_erase_items;

	double _eat_threshold;
	int _window_start_step;
	int _window_size_step;
	int _definite_point;

};


#endif