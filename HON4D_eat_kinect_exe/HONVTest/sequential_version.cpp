
#include "stdafx.h"
#include "sequential_version.h"

#include <fstream>
#include <set>
#include <map>
#include <algorithm>

using namespace std;

//// some tool functions
string int2string(int o)
{
	string rtn;
	_Longlong number = o;
	rtn = std::to_string(number);
	return rtn;
}


int Sequential::SearchDirectory(std::vector<wstring> &refvecFiles,
	const std::wstring        &refcstrRootDirectory,
	const std::wstring        &refcstrExtension,
	bool                     bSearchSubdirectories)
{
	std::wstring     strFilePath;             // Filepath
	std::wstring     strPattern;              // Pattern
	std::wstring     strExtension;            // Extension
	HANDLE          hFile;                   // Handle to file
	WIN32_FIND_DATA FileInformation;         // File information


	strPattern = refcstrRootDirectory + L"\\*.*";

	hFile = ::FindFirstFile((LPCWSTR)strPattern.c_str(), &FileInformation);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if(FileInformation.cFileName[0] != '.')
			{
				strFilePath.erase();
				strFilePath = refcstrRootDirectory + L"\\" + FileInformation.cFileName;

				if(FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if(bSearchSubdirectories)
					{
						// Search subdirectory
						int iRC = SearchDirectory(refvecFiles,
							strFilePath,
							refcstrExtension,
							bSearchSubdirectories);
						if(iRC)
							return iRC;
					}
				}
				else
				{
					// Check extension
					strExtension = FileInformation.cFileName;
					strExtension = strExtension.substr(strExtension.rfind(L".") + 1);

					if(strExtension == refcstrExtension)
					{
						// Save filename
						refvecFiles.push_back(strFilePath);
					}
				}
			}
		} while(::FindNextFile(hFile, &FileInformation) == TRUE);

		// Close handle
		::FindClose(hFile);

		DWORD dwError = ::GetLastError();
		if(dwError != ERROR_NO_MORE_FILES)
			return dwError;
	}

	//for(int i=0; i<10; i++)
	//	std::wcout << refvecFiles[i]<<std::endl;

	return 0;
}

int Sequential::read_data_kinect()
{
	int rtn = 0;


	return rtn;
}

int Sequential::get_all_files()
{
	int rtn = 0;

	if (_mode == 1)
	{
		get_all_files_png();
	}
	else if(_mode == 2)
	{
		get_all_files_bin();
		preread_images_bin();
	}
	else if (_mode == 3)
	{
		read_data_kinect();
	}
	else 
	{
		cout<<"error: no such mode!"<<endl;
		exit(1);
	}

	return rtn;
}

int Sequential::get_all_files_png()
{
	int rtn = 0;

	ifstream openFile;
	openFile.open(_FileName);
	string line;

	int count = 0;
	while ( openFile.good() )
	{
		getline (openFile,line);
		if (line.length()<2)
			continue;


		wstring vidPath(line.length(), L' ');
		copy(line.begin(), line.end(), vidPath.begin());
		pair<string, std::vector<std::wstring> > vecFiles;
		vecFiles.first = line;
		SearchDirectory(vecFiles.second,vidPath, L"png");
		_vecFiles.push_back(vecFiles);

		count ++;
	}

	/// select the min-size video and max-size video.
	set<int> size_set;
	for (int i=0; i<(int)_vecFiles.size(); i++)
	{
		if (i==0)
		{
			_min_video_size = (int)_vecFiles[i].second.size();

			_max_video_size = (int)_vecFiles[i].second.size();
		}
		else
		{
			if (_min_video_size > (int)_vecFiles[i].second.size())
			{
				_min_video_size = (int)_vecFiles[i].second.size();
			}

			if (_max_video_size < (int)_vecFiles[i].second.size())
			{
				_max_video_size = (int)_vecFiles[i].second.size();
			}

		}

		size_set.insert(_vecFiles[i].second.size() );
	}

	/// for test
	//for ( set<int>::iterator it = size_set.begin(); it!= size_set.end(); it++)
	//{
	//	cout<<*it<<endl;
	//}

	cout<<"the min size of video: "<<_min_video_size<<endl;
	cout<<"the max size of video: "<<_max_video_size<<endl;


	split_trainingset_testset_png();
	//	generate_new_test_vecFile();

	//_sliding_windows_width = _min_video_size;
	//_sliding_windows_width = 80;

	return 0;
}

void ReadAllImagesBin(FILE * fp, int noImages, int ncols, int nrows, vector<myMatrix>& one_bin )
{
	//read each frame
	int f; //frame index
	for(f=0; f<noImages; f++)
	{
		CDepthMapSkt depthMap;
		depthMap.SetSize(ncols, nrows); //it allocates space
		//the data will be stored in <depthMap>
		int max_v = ReadDepthMapSktBinFileNextFrame(fp, ncols, nrows, depthMap);

		myMatrix matd(nrows, ncols);
		for (int i=0; i<nrows; i++)
			//for (int i=DetWin.height-1; i>-1; i--)
			for(int j=0; j<ncols; j++)
			{
				int val = depthMap.GetItem(i,j);
				matd.data[i][j] = val;
			}
			one_bin[f] = matd;

	}

	fp=NULL;


}

/// each read three videos
int Sequential::preread_images_bin()
{
	int rtn  = 0;
	/// check if read needed
	if (_vec_image_value.size() - _current_index_bin > _sliding_windows_width)
	{
		cout<<"size of _vec_image_value: "<<_vec_image_value.size()<<" : "<<_current_index_bin + _sliding_windows_width<<endl;
		return 1;
	}

	cout<<"_vec size: "<<_vec_image_value.size()<<endl;
	cout<<"current_index_bin: "<<_current_index_bin<<endl;
	/// remove the preceding elements
	_vec_image_value.erase(_vec_image_value.begin(),  _vec_image_value.begin()+_current_index_bin);
	cout<<"_vec size after erase: "<<_vec_image_value.size()<<endl;
	_total_erase_items += _current_index_bin;

	cout<<"start prepread:"<<endl;
	int iend = My_Min(_current_video_ind+_pre_read_size, (int)_vec_file_path_testing.size());
	cout<<"iend: "<<iend<<endl;
	for(int i=_current_video_ind; i<iend; i++)
	{
		int noImages = _vec_file_path_testing[i].second;

		vector<myMatrix> one_bin;
		one_bin.resize(noImages);

		const string& line = _vec_file_path_testing[i].first;
		cout<<line<<" size: "<<_vec_file_path_testing[i].second<<endl;
		const char* depthFileName = line.c_str();
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
		if (nofs != noImages)
		{
			cout<<"error: nofs != noImages"<<endl;
			exit(1);
		}
		ReadAllImagesBin(fp, noImages, ncols, nrows, one_bin);
		_vec_image_value.insert(_vec_image_value.end(), one_bin.begin(), one_bin.end());
		fclose(fp);


	}
	_current_video_ind += My_Min(_pre_read_size, (int)_vec_file_path_testing.size()-_current_video_ind);

	cout<<"_vec size after preread: "<<_vec_image_value.size()<<endl;
	cout<<"current_index_bin: "<<_current_index_bin<<endl;


	_current_index_bin =  0;

	if (_total_erase_items != _current_index)
	{
		cout<<"error: _total_erase_items != _current_index"<<endl;
		exit(1);
	}

	cout<<"prepread successfully!"<<endl;

	return rtn;
}

int Sequential::get_all_files_bin()
{
	int rtn = 0;

	ifstream openFile;
	openFile.open(_FileName);
	string line;

	int count = 0;
	while (getline (openFile,line)  )
	{
		count ++;
		if (line.length()<2)
			continue;

		cout<<count<<": "<<line<<endl;
		const char* depthFileName = line.c_str();
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

		int noImages = nofs;
		_vec_image_number.push_back(noImages);
		_vec_file_path.push_back(line);
		cout<<count<<"   "<<"nImages: "<<noImages<<endl;
		fclose(fp);
		fp = NULL;

	}

	/// select the min-size video and max-size video.

	_min_video_size = *min_element(_vec_image_number.begin(), _vec_image_number.end());
	_max_video_size = *max_element(_vec_image_number.begin(), _vec_image_number.end());

	set_min_video_size(100);

	cout<<"the min size of video: "<<_min_video_size<<endl;
	cout<<"the max size of video: "<<_max_video_size<<endl;


	split_trainingset_testset_bin();


	return 0;
}


int Sequential::set_sliding_window_width(int flag)
{
	int rtn = 0;

	if (flag == 1)
	{
//		_min_video_size = 30;
		_sliding_windows_width = _min_video_size;

	}
	else if (flag == 2)
	{
		_sliding_windows_width = _average_test_length;
	}
	else
		_sliding_windows_width = _max_video_size;


	return rtn;
}

int Sequential::split_trainingset_testset_png()
{
	int rtn = 0;


	for(int i=0; i<(int)_vecFiles.size(); i++)
	{
		string ss = "";
		int s_ind = -1;
		string& name = _vecFiles[i].first;
		//		cout<<name<<endl;

		for ( int j=name.length()-1;j>=0; j--)
		{
			if (name[j]=='s' && name[j+1]<58 && name[j+1]>47)
			{
				ss += name[j+1];
				ss += name[j+2];
				s_ind = atoi(ss.c_str());
				if (s_ind < 0)
				{
					cout<<"error: s_ind < 0"<<endl;
				}
				//			cout<<"s_ind: "<<s_ind<<" ss: "<<ss<<endl;

				break;
			}
		}

		if (s_ind%2 == 1)
		{
			_training_vecFiles.push_back(_vecFiles[i]);
		}
		else
			_test_vecFiles.push_back(_vecFiles[i]);
	}





	/// for test
	cout<<"training set size: "<<_training_vecFiles.size()<<endl;
	//for (int i=0; i<10; i++)
	//{
	//	cout<<_training_vecFiles[i].first<<endl;
	//}

	cout<<"test set size: "<<_test_vecFiles.size()<<endl;
	//for (int i=0; i<100; i++)
	//{
	//	cout<<_test_vecFiles[i].first<<endl;
	//}

	/// randomly shuffle the elements in test sequence
	random_shuffle(_test_vecFiles.begin(), _test_vecFiles.end());

	/// generate _new_test_frames_sequence
	for (int i=0; i<(int) _test_vecFiles.size(); i++)
	{
		for (int j=0; j<(int) _test_vecFiles[i].second.size(); j++)
		{
			_new_test_frames_sequence.push_back(_test_vecFiles[i].second[j]);
		}
	}
	cout<<"the size of total test sequence: "<<_new_test_frames_sequence.size()<<endl;
	//for (int i=0; i<100; i++)
	//{
	//	wcout<<_new_test_frames_sequence[i]<<endl;
	//}

	_average_test_length = _new_test_frames_sequence.size() / _test_vecFiles.size();
	cout<<"average test length per activity: "<< _average_test_length <<endl;


	return rtn;
}

int Sequential::split_trainingset_testset_bin()
{
	int rtn = 0;

	std::vector<std::vector<vector<pair<int, int> > > > test_skeleton_info;
	std::vector<std::vector<std::vector<MyCoords> > > test_joint_realworld_coords;

	for(int i=0; i<(int)_vec_file_path.size(); i++)
	{
		string ss = "";
		int s_ind = -1;
		string& name = _vec_file_path[i];

		for ( int j=name.length()-1;j>=0; j--)
		{
			if (name[j]=='s' && name[j+1]<58 && name[j+1]>47)
			{
				ss += name[j+1];
				ss += name[j+2];
				s_ind = atoi(ss.c_str());
				if (s_ind < 0)
				{
					cout<<"error: s_ind < 0"<<endl;
				}
				//			cout<<"s_ind: "<<s_ind<<" ss: "<<ss<<endl;

				break;
			}
		}

		if (s_ind%2 == 1)
		{
			_vec_file_path_training.push_back(make_pair(_vec_file_path[i], _vec_image_number[i]));
		}
		else
		{	
			_vec_file_path_testing.push_back(make_pair(_vec_file_path[i], _vec_image_number[i]));
			test_skeleton_info.push_back(_skeleton_info[i]);
			test_joint_realworld_coords.push_back(_joint_realworld_coords[i]);
		}
	}


	/// preprocess the skeleton information, since some skeleton frame number is not equal to the depth  frame number
	for (int i=0; i<_vec_file_path_testing.size(); i++)
	{
		if (_vec_file_path_testing[i].second != test_joint_realworld_coords[i].size())
		{
			cout<<"unequal: "<<i<<"  "<<_vec_file_path_testing[i].first<<"  "<<_vec_file_path_testing[i].second<<"  "<<test_joint_realworld_coords[i].size()<<endl;
			if (_vec_file_path_testing[i].second < test_joint_realworld_coords[i].size())
			{
				test_joint_realworld_coords[i].pop_back();
				test_skeleton_info[i].pop_back();
			}
			else
			{
				test_joint_realworld_coords[i].push_back(test_joint_realworld_coords[i][test_joint_realworld_coords[i].size()-1]);
				test_skeleton_info[i].push_back(test_skeleton_info[i][test_skeleton_info[i].size()-1]);
			}
		}
	}

	/// for test
	cout<<"training set size: "<<_vec_file_path_training.size()<<endl;
	//for (int i=0; i<10; i++)
	//{
	//	cout<<_training_vecFiles[i].first<<endl;
	//}

	cout<<"test set size: "<<_vec_file_path_testing.size()<<endl;
	//for (int i=0; i<100; i++)
	//{
	//	cout<<_test_vecFiles[i].first<<endl;
	//}

	/// randomly shuffle the elements in test sequence
	vector<int> temp(_vec_file_path_testing.size());
	for (int i=0; i<temp.size(); i++)
	{
		temp[i] = i;
	}
	random_shuffle(temp.begin(), temp.end());
	std::vector<std::pair<std::string, int> > temp_vec_file_path_testing(_vec_file_path_testing.size());
	for (int i=0; i<temp.size(); i++)
	{
		temp_vec_file_path_testing[i] = _vec_file_path_testing[temp[i]];
	}
	_vec_file_path_testing = temp_vec_file_path_testing;

	/// for test
	for (int i=0; i<_vec_file_path_testing.size(); i++)
	{
		cout<<i<<"  "<<_vec_file_path_testing[i].first<<endl;
	}


	for (int i=0; i<test_joint_realworld_coords.size();  i++)
	{
		int k = temp[i];
		if (test_joint_realworld_coords[k].size() != test_skeleton_info[k].size())
		{
			cout<<"error: test_joint_realworld_coords[k].size() != test_skeleton_info[k].size()"<<endl;
			exit(1);
		}
		_test_skeleton_info.insert(_test_skeleton_info.end(), test_skeleton_info[k].begin(), test_skeleton_info[k].end());
		_test_joint_realworld_coords.insert(_test_joint_realworld_coords.end(), test_joint_realworld_coords[k].begin(), 
			test_joint_realworld_coords[k].end());
	}

	_total_test_length = 0;
	for (int i=0; i<_vec_file_path_testing.size(); i++)
	{
		_total_test_length += _vec_file_path_testing[i].second;
	}



	if (_total_test_length != _test_joint_realworld_coords.size() || _total_test_length != _test_skeleton_info.size())
	{
		cout<<"error: _total_test_length != _test_joint_realworld_coords.size()!"<<endl;
		cout<<"total test length: "<<_total_test_length<<endl;
		cout<<"size of test_joint_realworld: "<<_test_joint_realworld_coords.size()<<endl;
		cout<<"size of test_skeleton: "<<_test_skeleton_info.size()<<endl;
		exit(1);
	}

	_average_test_length = _total_test_length / (int)_vec_file_path_testing.size();
	cout<<"average test length: "<<_average_test_length<<endl;
	cout<<"total test length: "<<_total_test_length<<endl;


	///// for  test
	//for (int i=0; i<10; i++)
	//{
	//	cout<<i<<": "<<_vec_file_path_testing[i].first<<endl;
	//}

	return rtn;
}


/// still have some problems, hence no use temporarily 
int Sequential::generate_new_test_vecFile()
{
	int rtn = 0;

	int index = 1;
	pair<string, std::vector <std::wstring> > temp_vec;
	temp_vec.first = int2string(index);

	for (int i=0; i<(int) _test_vecFiles.size(); i++)
	{
		for (int j=0; j<(int) _test_vecFiles[i].second.size(); j++)
		{
			temp_vec.second.push_back(_test_vecFiles[i].second[j]);
			if (int(temp_vec.second.size()) == _sliding_windows_width)
			{
				_new_test_vecFiles.push_back(temp_vec);
				index ++;
				temp_vec.first = int2string(index);
				temp_vec.second.clear();
			}
		}
	}

	/// for test
	cout<<"size of new_test vecFile: "<<_new_test_vecFiles.size()<<endl;
	for (int i=0; i<10; i++)
	{
		cout<<_new_test_vecFiles[i].first<<" : "<<endl;
		for ( int j=0; j<(int)_new_test_vecFiles[i].second.size(); j++)
		{
			wcout<<_new_test_vecFiles[i].second[j]<<endl;
		}
	}

	return rtn;

}

int Sequential::set_true_labels_png()
{
	int rtn = 0;

	_true_labels.resize(_new_test_frames_sequence.size());

	for (int i=0; i<(int)_new_test_frames_sequence.size(); i++)
	{
		const wstring& name = _new_test_frames_sequence[i];
		string ss = "";
		int a_ind = -1;
		for ( int j=name.length()-1;j>=0; j--)
		{
			if (name[j]=='a' && name[j+1]<58 && name[j+1]>47)
			{
				ss += name[j+1];
				ss += name[j+2];
				a_ind = atoi(ss.c_str());
				if (a_ind < 0)
				{
					cout<<"error: a_ind < 0"<<endl;
				}

				break;
			}
		}

		_true_labels[i] = a_ind;
	}

	///// for test
	//for (int i=0; i<100; i++)
	//{
	//	cout<<i<<" "<<_true_labels[i]<<endl;
	//}


	return rtn;
}

int Sequential::set_true_labels_bin()
{
	int rtn = 0;
	int index = 0;

	_true_labels.resize(_total_test_length);

	for (int i=0; i<(int)_vec_file_path_testing.size(); i++)
	{
		const string& name = _vec_file_path_testing[i].first;
		string ss = "";
		int a_ind = -1;
		for ( int j=name.length()-1;j>=0; j--)
		{
			if (name[j]=='a' && name[j+1]<58 && name[j+1]>47)
			{
				ss += name[j+1];
				ss += name[j+2];
				a_ind = atoi(ss.c_str());
				if (a_ind < 0)
				{
					cout<<"error: a_ind < 0"<<endl;
				}

				break;
			}
		}

		if (a_ind == 2)
		{
			a_ind = +1;
		}
		else
			a_ind = -1;
		for (int j=index; j<index+_vec_file_path_testing[i].second; j++)
		{

			_true_labels[j] = a_ind;
		}
		index += _vec_file_path_testing[i].second;
	}

	///// for test
	//for (int i=0; i<300; i++)
	//{
	//	cout<<i<<" "<<_true_labels[i]<<endl;
	//}


	return rtn;
}

void Sequential::write_true_labels_to_files()
{
	const char* filename = "..\\Release\\eat_true_label.txt";
	ofstream out_file;
	out_file.open(filename);

	if (out_file.fail())
	{
		cout<<"error: cannot open files: true_labels.txt."<<endl;
		exit(1);
	}

	for (int i=0; i<(int)_true_labels.size(); i++)
	{
		if(i<_true_labels.size()-1)
			out_file << _true_labels[i] << endl;
		else
			out_file << _true_labels[i];

	}

	out_file.close();

}


int Sequential::generate_one_new_test_vecFile_png(vector<wstring>& new_test_vecFile, int& rtn_label, int& max_label)
{
	int rtn = 0;
	map<int, int> labels;


	new_test_vecFile.resize(_sliding_windows_width);

	for (int i=0; i<_sliding_windows_width; i++)
	{
		int k = i+_current_index;
		new_test_vecFile[i] = _new_test_frames_sequence[k]; 


		/// for calculate the rtn_label 
		const wstring& name = _new_test_frames_sequence[k];
		string ss = "";
		int a_ind = -1;
		for ( int j=name.length()-1;j>=0; j--)
		{
			if (name[j]=='a' && name[j+1]<58 && name[j+1]>47)
			{
				ss += name[j+1];
				ss += name[j+2];
				a_ind = atoi(ss.c_str());
				if (a_ind < 0)
				{
					cout<<"error: a_ind < 0"<<endl;
				}
				//			cout<<"s_ind: "<<s_ind<<" ss: "<<ss<<endl;

				break;
			}
		}

		map<int, int>::iterator it = labels.find(a_ind);
		if (it == labels.end())
		{
			labels[a_ind] = 1;
		}
		else
			labels[a_ind] ++;

		/// for test 
		//		wcout<<name<<" label: "<< a_ind<<endl;

	}
	//	_current_index += _sliding_windows_width;
	_current_index += _step_width;
	//	_current_index ++;

	/// for test
	//if (labels.size() > 1)
	//{
	//	for (map<int,int>::iterator it = labels.begin(); it!=labels.end(); it++)
	//	{
	//		cout<<it->first<<" "<<it->second<<endl;
	//	}

	//}

	/// take the max value in the map to be the last label of this test sequence (vecFile) 
	int max_v = 0;
	int tmp_label = 0;
	for (map<int,int>::iterator it = labels.begin(); it!=labels.end(); it++)
	{
		if (max_v < it->second)
		{
			tmp_label = it->first;
			max_v = it->second;
		}
	}

	max_label = max_v;
	rtn_label = tmp_label;

	//	cout<<"rtn_label: "<<rtn_label<<endl;


	return rtn;
}

int Sequential::generate_one_new_test_vecFile_bin( int& rtn_label, int& max_label)
{
	int rtn = 0;
	map<int, int> labels;


	//new_test_vecFile.resize(_sliding_windows_width);

	for (int i=0; i<_sliding_windows_width; i++)
	{

		/// for calculate the rtn_label 
		int a_ind = -2;
		a_ind = _true_labels[i+_current_index];
		map<int, int>::iterator it = labels.find(a_ind);
		if (it == labels.end())
		{
			labels[a_ind] = 1;
		}
		else
			labels[a_ind] ++;

	}
	//	_current_index += _sliding_windows_width;
	_current_index += _step_width;
	//	_current_index ++;

	/// for test
	//if (labels.size() > 1)
	//{
	//	for (map<int,int>::iterator it = labels.begin(); it!=labels.end(); it++)
	//	{
	//		cout<<it->first<<" "<<it->second<<endl;
	//	}

	//}

	/// take the max value in the map to be the last label of this test sequence (vecFile) 
	int max_v = 0;
	int tmp_label = 0;
	for (map<int,int>::iterator it = labels.begin(); it!=labels.end(); it++)
	{
		if (max_v < it->second)
		{
			tmp_label = it->first;
			max_v = it->second;
		}
	}

	max_label = max_v;
	rtn_label = tmp_label;

//	cout<<"rtn_label: "<<rtn_label<<endl;


	return rtn;
}


int Sequential::next_decision1()
{
	int rtn = 0;
	int size = (int)_predict_labels.size();
	if (size>1)
	{
		if (_predict_labels[size-2] != _predict_labels[size-1] || _sliding_windows_width == _max_video_size+1 )
		{
			double max_v = _predict_probs[_last_index+_min_video_size];
			cout<<"last_index before: "<<_last_index<<endl;
			int temp_max_p = _last_index+_min_video_size;
			for (int i=temp_max_p+1; i<size-1; i++)
			{
				if (_predict_probs[i] >= max_v)
				{
					max_v = _predict_probs[i];
					temp_max_p = i;
				}
			}
			_current_index = temp_max_p+1;
			_last_index = temp_max_p;
			cout<<"last_index new: "<<_last_index<<endl;
			cout<<"current index new: "<<_current_index<<endl;

			vector<int> label_temp;
			label_temp.assign(_predict_labels.begin(), _predict_labels.begin()+_last_index+1);
			_predict_labels.clear();
			_predict_labels = label_temp;

			vector<double> prob_temp;
			prob_temp.assign(_predict_probs.begin(), _predict_probs.begin()+_last_index+1);
			_predict_probs.clear();
			_predict_probs = prob_temp;

			set_sliding_window_width(1);
			set_sliding_window_width_minus1();

			//for (int i=0; i<_predict_labels.size(); i++)
			//{
			//	cout<<i<<": "<<_predict_labels[i]<<"  ";
			//}
			//cout<<endl;
			//for (int i=0; i<_predict_labels.size(); i++)
			//{
			//	cout<<i<<": "<<_predict_probs[i]<<"  ";
			//}
			//cout<<endl;

			/// calculate the accuracy
			calculate_accuracy();
		}
		//else if (_predict_probs[_predict_probs.size()-1] > _prob_threshold)
		//{

		//}

	}

	return rtn;
}

int Sequential::next_decision2()
{
	int rtn = 0;
	int size = (int)_predict_labels.size();
	double p1 = _predict_probs[size-2];
	double p2 = _predict_probs[size-1];
	//	if (_sliding_windows_width==_up_bound || (p1>_prob_threshold  && p1 > p2))
	if (_sliding_windows_width >=_up_bound)
	{
		double max_v = _predict_probs[_current_index-1+_min_video_size];
		cout<<"last_index before: "<<_last_index<<endl;
		int last_before = _last_index;
		int temp_max_p = _current_index-1+_min_video_size;
		for (int i=_current_index+_min_video_size; i<size; i++)
		{
			if (_predict_probs[i] >= max_v)
			{
				max_v = _predict_probs[i];
				temp_max_p = i;
			}
		}

		if (max_v >= _eat_threshold)
		{
			int last_current = _current_index;
			//_current_index = temp_max_p+1;
			_current_index += _window_start_step;
			if (_current_index < last_current)
			{
				cout<<"error: current_index < last_current"<<endl;
				exit(1);
			}
			_current_index_bin = _current_index - last_current + _current_index_bin;
			if(_last_index < temp_max_p)
				_last_index = temp_max_p;
			cout<<"last_index new: "<<_last_index<<endl;
			cout<<"current index new: "<<_current_index<<endl;


			for (int i=last_before+1; i<_last_index; i++)
			{
				_predict_labels[i] = +1;
			}
			cout<<"finally predicted label: +1"<<"  max_decision: "<<max_v<<"max index: "<<temp_max_p<<endl;

			vector<int> label_temp;
			label_temp.assign(_predict_labels.begin(), _predict_labels.begin()+_last_index+1);
			_predict_labels.clear();
			_predict_labels = label_temp;

			vector<double> prob_temp;
			prob_temp.assign(_predict_probs.begin(), _predict_probs.begin()+_last_index+1);
			_predict_probs.clear();
			_predict_probs = prob_temp;

			set_sliding_window_width(1);
			set_sliding_window_width_minus_k();
		}
		else
		{
			int last_current = _current_index;
			_current_index += _window_start_step;
	
			_current_index_bin = _current_index - last_current + _current_index_bin;
			if(_last_index < _current_index-1)
				_last_index = _current_index - 1;
			cout<<"last_index new: "<<_last_index<<endl;
			cout<<"current index new: "<<_current_index<<endl;


			for (int i=last_before+1; i<_last_index; i++)
			{
				_predict_labels[i] = -1;
			}
			cout<<"finally predicted label: -1"<<"  max_decision: "<<max_v<<"max index: "<<temp_max_p<<endl;

			vector<int> label_temp;
			label_temp.assign(_predict_labels.begin(), _predict_labels.begin()+_last_index+1);
			_predict_labels.clear();
			_predict_labels = label_temp;

			vector<double> prob_temp;
			prob_temp.assign(_predict_probs.begin(), _predict_probs.begin()+_last_index+1);
			_predict_probs.clear();
			_predict_probs = prob_temp;

			set_sliding_window_width(1);
			set_sliding_window_width_minus_k();
		}
		

		//for (int i=0; i<_predict_labels.size(); i++)
		//{
		//	cout<<i<<": "<<_predict_labels[i]<<"  ";
		//}
		//cout<<endl;
		//for (int i=0; i<_predict_labels.size(); i++)
		//{
		//	cout<<i<<": "<<_predict_probs[i]<<"  ";
		//}
		//cout<<endl;

		/// calculate the accuracy
		calculate_accuracy();

	}
	preread_images_bin();


	return rtn;
}

int Sequential::next_decision_kinect(int predict_label, double prob)
{
	int rtn = 0;

	if (_sliding_windows_width >=_up_bound || predict_label == 1)
	{
		_current_index += _window_start_step;
		set_sliding_window_width(1);
		set_sliding_window_width_minus_k();

		rtn = _window_start_step;

	}

	return rtn;
}

double Sequential::calculate_accuracy()
{
	int correct = 0;
	for (int i=_start_point; i<=_last_index; i++)
	{
		if (_true_labels[i] == _predict_labels[i])
		{
			correct ++;
		}
	}
	_current_accuracy = (double)correct/(double)(_last_index+1.0);
	cout<<"current accuracy: "<<_current_accuracy<<"  "<<correct<<" // "<<_last_index+1<<endl;

	return _current_accuracy;
}

void Sequential::print_predicted_label()
{
	ofstream outfile;
	string file_p = "..\\predicted_label.dat";
	outfile.open(file_p.c_str());
	for (int i=0; i<_predict_labels.size(); i++)
	{
		if(i != _predict_labels.size()-1)
			outfile << _predict_labels[i]<<endl;
		else
			outfile << _predict_labels[i];
	}
	outfile.close();
}