#include "stdafx.h"
#include "HON4D.h"
#include "sequential_version.h"
#include "..\libsvm\mysvm.h"
#include "LocalHON4D.h"
#include "..\DepthMapSktIO\DepthMapSktBinFileIO.h"

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

/*
/// by Wenjie
string wint2string(int o)
{
	string rtn;
	_Longlong number = o;
	rtn = std::to_string(number);
	return rtn;
}

string convert_specified_length(const string& ss, int len)
{
	string str;
	if(ss.length() < len)
	{
		int leng = len - ss.length();
		for (int i=0; i<leng; i++)
		{
			str += "0";
		}
		str += ss;
	}

	return str;

}


int SearchDirectory(std::vector<std::wstring> &refvecFiles,
	const std::wstring        &refcstrRootDirectory,
	const std::wstring        &refcstrExtension,
	bool                     bSearchSubdirectories = true)
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

	return 0;
}

int strlenTCHAR(const _TCHAR * x)
{
	if(x == NULL)
		return 0;

	int ret=0;
	while(x[ret] != '\0')
	{
		ret++;
	}
	return ret;
}

void strcpyTCHAR(const _TCHAR * src, char * dst )
{
	int length = strlenTCHAR(src);
	for(int i=0; i<length; i++)
	{
		dst[i] = (char) (src[i]);
	}
	dst[length] = '\0';
}

char** parseInputArg(int argc, _TCHAR* argv[])
{
	// Parse inupt argument
	int i ,j;
	//convert _TCHAR to char
	char ** argv_copy;

	argv_copy = new char*[argc];
	for(i=0; i<argc; i++)
	{
		int tempLen = strlenTCHAR(argv[i]);
		argv_copy[i] = new char[tempLen+1];
		strcpyTCHAR(argv[i], argv_copy[i]);
	}
	return argv_copy;
}

void QueryParameters(string FileName,int inputArg,int &startObject,int &cell_x,int &cell_y,int &cell_t,int &hdiff,int &pNo)
{

	// cellx celly cellz object number 

	// read all parameters
	string line;
	ifstream parFile;
	parFile.open(FileName);
	int counter = 0;
	while ( parFile.good() )
	{
		getline(parFile,line);
		if (counter == inputArg)
		{
			//cout << line << endl;
			break;
		}
		else
		{
			counter++;
		} 
	}
	parFile.close();


	stringstream   linestream(line);
	string         value;

	getline(linestream,value,' ');
	cell_x = atoi(value.c_str());
	getline(linestream,value,' ');
	cell_y = atoi(value.c_str());
	getline(linestream,value,' ');
	cell_t = atoi(value.c_str());

	getline(linestream,value,' ');
	hdiff = atoi(value.c_str());

	getline(linestream,value,' ');
	pNo = atoi(value.c_str());

	getline(linestream,value,' ');
	startObject = atoi(value.c_str());
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


int specify_FileNames_01()
{

	string name = "..\\..\\Data\\MSRAction\\FileNames_01.txt";
	const char* filename = name.c_str();
	ofstream outfile;
	outfile.open(filename);
	if(!outfile)
	{
		std::cout<<"error:file cannot be opened!"<<std::endl;
	}

	int action_num = 20;
	int subject_num = 10;
	int perform_times = 3;


	system("dir /a-d /b E:\\kinect\\HON4D\\HON4D\\EvaluationCode\\Action3D\\Desc\\*.* >..\\..\\Data\\MSRAction\\allfiles.txt");

	ifstream infile;

	infile.open("..\\..\\Data\\MSRAction\\allfiles.txt");
	string line;
	while(getline(infile, line))
	{
		string action_i = "";
		string subject_i = "";
		string perform_i = "";

		for(int i=0; i<line.length(); i++)
		{
			if(line[i]=='a')
			{
				action_i += line[i+1];
				action_i += line[i+2];
			}
			else if(line[i]=='s')
			{
				subject_i += line[i+1];
				subject_i += line[i+2];
			}
			else if(line[i]=='e')
			{
				perform_i += line[i+1];
				perform_i += line[i+2];
				break;
			}
		}

		outfile << "..\\\\..\\\\Data\\\\MSRAction\\\\VideosCrop\\\\action"<<action_i<<"\\\\d\_a"<<action_i<<"\_s"<<subject_i<<"\_e"<<perform_i<<"\_sdepth"<<std::endl;
	}



	outfile.close();

	return 0;
}

int specify_FileNames_02()
{
	string name = "..\\..\\Data\\MSRDailyAct3D\\FileNames_02.txt";
	const char* filename = name.c_str();
	ofstream outfile;
	outfile.open(filename);
	if(!outfile)
	{
		std::cout<<"error:file cannot be opened!"<<std::endl;
	}

	int action_num = 16;
	int subject_num = 10;
	int perform_times = 2;


	system("dir /a-d /s /b ..\\..\\Data\\MSRDailyAct3D\\depth_png\\*.* >..\\..\\Data\\MSRDailyAct3D\\allfiles.txt");

	ifstream infile;

	infile.open("..\\..\\Data\\MSRDailyAct3D\\allfiles.txt");
	if (!infile.good())
	{
		cout<<"open file "<<"..\\..\\Data\\MSRDailyAct3D\\allfiles.txt "<<"failed!"<<endl;
	}
	string line;
	while(getline(infile, line))
	{
		string action_i = "";
		string subject_i = "";
		string perform_i = "";

		for(int i=0; i<line.length(); i++)
		{
			if(line[i]=='a')
			{
				action_i += line[i+1];
				action_i += line[i+2];
			}
			else if(line[i]=='s')
			{
				subject_i += line[i+1];
				subject_i += line[i+2];
			}
			else if(line[i]=='e')
			{
				perform_i += line[i+1];
				perform_i += line[i+2];
				break;
			}
		}

		outfile << "..\\\\..\\\\Data\\\\MSRDailyAct3D\\\\depth_png\\"<<"\_a"<<action_i<<"\_s"<<subject_i<<"\_e"<<perform_i<<"\_depth"<<std::endl;
	}



	outfile.close();

	return 0;
}


*/
/*
/// temp main function for specify_FileNames_01
int _tmain(int argc, _TCHAR* argv[])
{
specify_FileNames_02();
return 0;
}
*/
/*

//int _tmain(int argc, const char* argv[])
int main(int argc, const char* argv[])
{
	if (atoi(argv[1]) == 1)
	{
		// Path for the 4D projectors for the polychoron (provided in data folder)
		string projectorsPath = "..\\..\\Data\\Projectors\\";  

		// Path for a text file which has a list of the videos to be processed. (For an example file, see "Data\MSRAction\FileNames_01.txt")
		string FileName = "..\\..\\Data\\MSRAction\\FileNames_01.txt";

		// Path where the final descriptors will be saved
		string descPath = "..\\..\\Data\\MSRAction\\Desc\\";


		int inputArg;
		int cell_x = 5;
		int cell_y = 4;
		int cell_t = 3;
		int startObject = 1;
		int hdiff = 5;
		//	int pNo = 0;
		int pNo = atoi(argv[2]);

		const int MSR_FILE_LEN = 5; // for MSRAction and MSRDailiy and MSRGesture naming convension

		//	initialize model size
		int model_w,model_h,x0,y0;

		//	define HON4D class
		HON4D *cHON4D;// = new HON4D(8, 8, model_w, model_h);

		//	compute feature length
		int HON4DFeatLen;//=cHON4D->DetWin.featLen;

		//	define feature vector
		float *pfHON4DFeat;// = new float[HON4DFeatLen];


		// read the projectors
		CvMat* p= NULL;
		int projDim = readProjectors(pNo,projectorsPath,p);

		string descFileName;
		ifstream openFile;
		string line;
		ofstream savefile;
		string fileNameSub;
		int x;
		model_w = 160;
		model_h = 240;
		int cellwidth = floor(double(model_w/cell_x));
		int cellheight = floor(double(model_h/cell_y));
		model_w = cellwidth*cell_x;
		model_h = cellheight*cell_y;
		x0 = 0;
		y0 = 0;


		openFile.open(FileName);

		string descFold;
		char cell_x_char[5];
		char cell_y_char[5];
		char cell_t_char[5];
		char hdiff_char[5];
		char pNo_char[5];
		sprintf(cell_x_char,"%d",cell_x);
		sprintf(cell_y_char,"%d",cell_y);
		sprintf(cell_t_char,"%d",cell_t);
		sprintf(hdiff_char,"%d",hdiff);
		sprintf(pNo_char,"%d",pNo);

		descFold = descPath + "HON4DDesc-" + cell_x_char + "_" + cell_y_char + "_" + cell_t_char + "_" + hdiff_char + "_" + pNo_char + "\\";

		string cmd = "mkdir " + descFold;
		system(cmd.c_str());

		int count = 0;
		clock_t start, end;
		start = clock();

		while ( openFile.good() )
		{
			getline (openFile,line);
			if (line.length()<2)
				continue;
			cout << line << endl;

			wstring tmp(line.length(), L' ');
			copy(line.begin(), line.end(), tmp.begin());

			cHON4D = new HON4D(cellwidth, cellheight, model_w, model_h,tmp,cell_t,hdiff,p,projDim);
			HON4DFeatLen=cHON4D->DetWin.featLen;
			pfHON4DFeat = new float[HON4DFeatLen];
			cHON4D->ReadAllImages();

			x = line.find_last_of("\\");
			fileNameSub = line.substr(x+1,line.length()-x-MSR_FILE_LEN);
			descFileName = descFold + fileNameSub + ".txt";
			savefile.open(descFileName);
			if(cHON4D->GetFeature(x0, y0, pfHON4DFeat)!=HON4DFeatLen){
				printf("Get HON4D feature error!\n");
				return 0;
			}

			savefile << pfHON4DFeat[0]; 
			for (int i=1;i<HON4DFeatLen;i++){ 
				savefile << "," << pfHON4DFeat[i] ;
			}
			savefile <<  "\n";				
			savefile.close();
			delete cHON4D;
			delete [] pfHON4DFeat;

			count ++;
			if(count % 10 == 0)
			{
				end = clock();
				std::cout<<"have processed "<<count;
				std::cout<<"the running time is "<<(end-start)/1000.0<<std::endl; 
			}
		}
		openFile.close();

	}
	else if (atoi(argv[1])==2)
	{
		cout<<"process the video frames sequentially!"<<endl;


		// Path for the 4D projectors for the polychoron (provided in data folder)
		string projectorsPath = "..\\..\\Data\\Projectors\\";  

		// Path for a text file which has a list of the videos to be processed. (For an example file, see "Data\MSRAction\FileNames_01.txt")
		string FileName = "..\\..\\Data\\MSRAction\\FileNames_01.txt";

		// Path where the final descriptors will be saved
		string descPath = "..\\..\\Data\\MSRAction\\Desc\\";


		int inputArg;
		int cell_x = 5;
		int cell_y = 4;
		int cell_t = 3;
		int startObject = 1;
		int hdiff = 5;
		//	int pNo = 0;
		int pNo = atoi(argv[2]);

		const int MSR_FILE_LEN = 5; // for MSRAction and MSRDailiy and MSRGesture naming convension

		//	initialize model size
		int model_w,model_h,x0,y0;

		//	define HON4D class
		HON4D *cHON4D;// = new HON4D(8, 8, model_w, model_h);

		//	compute feature length
		int HON4DFeatLen;//=cHON4D->DetWin.featLen;

		//	define feature vector
		float *pfHON4DFeat;// = new float[HON4DFeatLen];


		// read the projectors
		CvMat* p= NULL;
		int projDim = readProjectors(pNo,projectorsPath,p);

		string descFileName;
		ofstream savefile;
		string fileNameSub;
		int x;
		model_w = 160;
		model_h = 240;
		int cellwidth = floor(double(model_w/cell_x));
		int cellheight = floor(double(model_h/cell_y));
		model_w = cellwidth*cell_x;
		model_h = cellheight*cell_y;
		x0 = 0;
		y0 = 0;



		string descFold;
		char cell_x_char[5];
		char cell_y_char[5];
		char cell_t_char[5];
		char hdiff_char[5];
		char pNo_char[5];
		sprintf(cell_x_char,"%d",cell_x);
		sprintf(cell_y_char,"%d",cell_y);
		sprintf(cell_t_char,"%d",cell_t);
		sprintf(hdiff_char,"%d",hdiff);
		sprintf(pNo_char,"%d",pNo);

		descFold = descPath + "HON4DDesc-" + cell_x_char + "_" + cell_y_char + "_" + cell_t_char + "_" + hdiff_char + "_" + pNo_char + "\\";

		string cmd = "mkdir " + descFold;
		system(cmd.c_str());

		int count = 0;
		clock_t start, end;
		start = clock();



		Sequential sequ(FileName);
		sequ.get_all_files();
		sequ.set_sliding_window_width(2);
		sequ.set_step_width(20);


		int loops;
		loops = sequ.get_test_sequence_length()/ sequ.get_step_width();
		cout<<"total loops: "<< loops <<endl;
		for (int i=0;  i<loops; i++)
		{
			vector<wstring> new_test_vecFile;
			int label;
			int max_label;
			sequ.generate_one_new_test_vecFile(new_test_vecFile, label,max_label);

			/// for test
			//cout<<i<<endl;
			//for (int k=0; k<(int)new_test_vecFile.size(); k++)
			//{
			//	wcout<<new_test_vecFile[k]<<endl;
			//}



			cHON4D = new HON4D(cellwidth, cellheight, model_w, model_h,cell_t,hdiff,p,projDim, new_test_vecFile);
			HON4DFeatLen=cHON4D->DetWin.featLen;
			pfHON4DFeat = new float[HON4DFeatLen];
			cHON4D->ReadAllImages();

			string sss = wint2string(label);
			if (sss.length() == 1)
			{
				sss = "0" + sss;
			}

			fileNameSub = "d"+ convert_specified_length(wint2string(i), 5) + "_a" + sss + "_s11" + ".txt";
			descFileName = descFold + fileNameSub;
			savefile.open(descFileName);
			if(cHON4D->GetFeature(x0, y0, pfHON4DFeat)!=HON4DFeatLen){
				printf("Get HON4D feature error!\n");
				return 0;
			}

			savefile << pfHON4DFeat[0]; 
			for (int i=1;i<HON4DFeatLen;i++){ 
				savefile << "," << pfHON4DFeat[i] ;
			}
			savefile <<  "\n";				
			savefile.close();
			delete cHON4D;
			delete [] pfHON4DFeat;

			cout<<fileNameSub<<"   finish!"<<endl;
			count ++;
			if(count % 10 == 0)
			{
				end = clock();
				std::cout<<"have processed "<<count;
				std::cout<<"the running time is "<<(end-start)/1000.0<<std::endl; 
			}

		}

	}
	else if (atoi(argv[1])==3)
	{
		string train_data = "..\\..\\Data\\MSRAction\\Desc\\training_set\\train_svm.dat";
		string test_data = "..\\..\\Data\\MSRAction\\Desc\\test_set\\test_svm.dat";
		MySVM mySVM(train_data, test_data);	
		wstring train_path = L"..\\..\\Data\\MSRAction\\Desc\\training_set\\";
		//		mySVM.file_format_convertion(train_path, train_data.c_str());	
		mySVM.scale_format_convertion_data(train_path, train_data.c_str(), 1);
		wstring test_path = L"..\\..\\Data\\MSRAction\\Desc\\test_set\\";
		//		mySVM.file_format_convertion(test_path, test_data.c_str());
		mySVM.scale_format_convertion_data(test_path, test_data.c_str(), 2);

		mySVM.control_train();
		mySVM.control_predict();
		mySVM.free_train_model();
		mySVM.free_test_model();

	}
	else if(atoi(argv[1]) == 4)
	{
		cout<<"process the video frames sequentially, trying all possible length of sliding window!"<<endl;


		// Path for the 4D projectors for the polychoron (provided in data folder)
		string projectorsPath = "..\\..\\Data\\Projectors\\";  

		// Path for a text file which has a list of the videos to be processed. (For an example file, see "Data\MSRAction\FileNames_01.txt")
		string FileName = "..\\..\\Data\\MSRAction\\FileNames_01.txt";

		// Path where the final descriptors will be saved
		string descPath = "..\\..\\Data\\MSRAction\\Desc\\";


		int inputArg;
		int cell_x = 5;
		int cell_y = 4;
		int cell_t = 3;
		int startObject = 1;
		int hdiff = 5;
		//	int pNo = 0;
		int pNo = atoi(argv[2]);

		const int MSR_FILE_LEN = 5; // for MSRAction and MSRDailiy and MSRGesture naming convension

		//	initialize model size
		int model_w,model_h,x0,y0;

		//	define HON4D class
		HON4D *cHON4D;// = new HON4D(8, 8, model_w, model_h);

		//	compute feature length
		int HON4DFeatLen;//=cHON4D->DetWin.featLen;

		//	define feature vector
		float *pfHON4DFeat;// = new float[HON4DFeatLen];


		// read the projectors
		CvMat* p= NULL;
		int projDim = readProjectors(pNo,projectorsPath,p);

		string descFileName;
		ofstream savefile;
		string fileNameSub;
		int x;
		model_w = 160;
		model_h = 240;
		int cellwidth = floor(double(model_w/cell_x));
		int cellheight = floor(double(model_h/cell_y));
		model_w = cellwidth*cell_x;
		model_h = cellheight*cell_y;
		x0 = 0;
		y0 = 0;



		string descFold;
		char cell_x_char[5];
		char cell_y_char[5];
		char cell_t_char[5];
		char hdiff_char[5];
		char pNo_char[5];
		sprintf(cell_x_char,"%d",cell_x);
		sprintf(cell_y_char,"%d",cell_y);
		sprintf(cell_t_char,"%d",cell_t);
		sprintf(hdiff_char,"%d",hdiff);
		sprintf(pNo_char,"%d",pNo);

		descFold = descPath + "HON4DDesc-" + cell_x_char + "_" + cell_y_char + "_" + cell_t_char + "_" + hdiff_char + "_" + pNo_char + "\\";

		string cmd = "mkdir " + descFold;
		system(cmd.c_str());



		string train_data = "..\\..\\Data\\MSRAction\\Desc\\training_set\\train_svm.dat";
		MySVM mySVM(train_data);	
		//wstring train_path = L"..\\..\\Data\\MSRAction\\Desc\\training_set\\";
		//mySVM.scale_format_convertion_data(train_path, train_data.c_str(), 1);
		//cout<<"training set scale operation and format conversion is completed!"<<endl;

		//mySVM.control_train();
		//cout<<"the svm model training is completed!"<<endl;

		mySVM.load_training_min_max();
		mySVM.load_training_model();


		Sequential sequ(FileName);
		sequ.get_all_files();
		sequ.set_sliding_window_width(1);
		sequ.set_step_width(0);
		sequ.set_true_labels();
		sequ.write_true_labels_to_files();

		int current;
		if (argc<5)
		{
			current = 0;
		}
		else
			current = atoi(argv[4]);

		sequ.set_start_point(current);
		cout<<"current_index: "<<sequ.get_current_index()<<endl;


		/// for optimization
		vector<vector<float> > pre_point_dist_bin;
		int pre_start_frame = -1;
		int pre_end_frame = -1;


		int count = 0; 
		clock_t start, end;
		start = clock();

		cout<<endl<<endl<<endl;

		int loops = 4;
		for (int i=0; ; i++)
		{
			if (sequ.get_current_index() + sequ.get_windows_width() > sequ.get_test_sequence_length())
			{
				cout<<"finish!"<<endl;
				break;
			}
			cout<<"sliding window width: "<<sequ.get_windows_width()<<endl;
			cout<<"current_index: "<<sequ.get_current_index()<<endl;

			vector<wstring> new_test_vecFile;
			int label;
			int max_label; /// the number of frames corresponding to the label
			sequ.generate_one_new_test_vecFile(new_test_vecFile, label, max_label);
			wcout<<"the last frame: "<<new_test_vecFile[new_test_vecFile.size()-1]<<endl;


			cHON4D = new HON4D(cellwidth, cellheight, model_w, model_h,cell_t,hdiff,p,projDim, new_test_vecFile);
			cHON4D->set_pre_start_end_frames(pre_start_frame, pre_end_frame);
			cHON4D->set_start_end_frames(sequ.get_current_index(), sequ.get_windows_width()+sequ.get_current_index()-1);
			cHON4D->set_pre_dist_bin(pre_point_dist_bin);

			HON4DFeatLen=cHON4D->DetWin.featLen;
			pfHON4DFeat = new float[HON4DFeatLen];
			cHON4D->ReadAllImages();

			string sss = wint2string(label);
			if (sss.length() == 1)
			{
				sss = "0" + sss;
			}

			fileNameSub = "d"+ convert_specified_length(wint2string(i), 5) + "_a" + sss + "_s11" + ".txt";
			descFileName = descFold + fileNameSub;
			if(cHON4D->GetFeature(x0, y0, pfHON4DFeat)!=HON4DFeatLen){
				printf("Get HON4D feature error!\n");
				return 0;
			}

			pre_point_dist_bin = cHON4D->get_point_dist_bin();
			pre_start_frame = cHON4D->get_start_frame();
			pre_end_frame = cHON4D->get_end_frame();

			pair<int, vector<double> > hon4d_feature;
			hon4d_feature.first = label;
			hon4d_feature.second.resize(HON4DFeatLen);


			for (int i=0;i<HON4DFeatLen;i++){ 
				hon4d_feature.second[i] = pfHON4DFeat[i];
			}

			delete cHON4D;
			delete [] pfHON4DFeat;

			cout<<"Calculating HON4D feature completed!"<<endl;

			/// scale
			mySVM.scale_one_sample(hon4d_feature);
			cout<<"scaling completed!"<<endl;

			cout<<"number of max_label: "<<max_label<<endl;
			/// predict
			int predict_label;
			double cprob;
			mySVM.predict_one_sample(hon4d_feature, predict_label, cprob);

			sequ.set_predict_result(predict_label, cprob);

			/// next decision
			if (atoi(argv[3])==1)
				sequ.next_decision1();
			else
				sequ.next_decision2();

			count ++;
			if(count % 2 == 0)
			{
				end = clock();
				std::cout<<"have processed "<<count;
				std::cout<<" the running time is "<<(end-start)/1000.0<<std::endl; 
			}

			sequ.set_sliding_window_width_plus1();

			cout<<endl<<endl;

		}
		mySVM.free_train_model();

	}

	else if (atoi(argv[1]) == 11)
	{
		// Path for the 4D projectors for the polychoron (provided in data folder)
		string projectorsPath = "..\\..\\Data\\Projectors\\";  

		// Path for a text file which has a list of the videos to be processed. (For an example file, see "Data\MSRAction\FileNames_01.txt")
		string FileName = "..\\..\\Data\\MSRDailyAct3D\\FileNames_02.txt";

		// Path where the final descriptors will be saved
		string descPath = "..\\..\\Data\\MSRDailyAct3D\\Desc_test\\";


		int inputArg;
		int cell_x = 5;
		int cell_y = 4;
		int cell_t = 3;
		int startObject = 1;
		int hdiff = 5;
		//	int pNo = 0;
		int pNo = atoi(argv[2]);

		const int MSR_FILE_LEN = 5; // for MSRAction and MSRDailiy and MSRGesture naming convension

		//	initialize model size
		int model_w,model_h,x0,y0;

		//	define HON4D class
		HON4D *cHON4D;// = new HON4D(8, 8, model_w, model_h);

		//	compute feature length
		int HON4DFeatLen;//=cHON4D->DetWin.featLen;

		//	define feature vector
		float *pfHON4DFeat;// = new float[HON4DFeatLen];


		// read the projectors
		CvMat* p= NULL;
		int projDim = readProjectors(pNo,projectorsPath,p);

		string descFileName;
		ifstream openFile;
		string line;
		ofstream savefile;
		string fileNameSub;
		int x;
		model_w = 320;
		model_h = 240;
		int cellwidth = floor(double(model_w/cell_x));
		int cellheight = floor(double(model_h/cell_y));
		model_w = cellwidth*cell_x;
		model_h = cellheight*cell_y;
		x0 = 0;
		y0 = 0;


		openFile.open(FileName);

		string descFold;
		char cell_x_char[5];
		char cell_y_char[5];
		char cell_t_char[5];
		char hdiff_char[5];
		char pNo_char[5];
		sprintf(cell_x_char,"%d",cell_x);
		sprintf(cell_y_char,"%d",cell_y);
		sprintf(cell_t_char,"%d",cell_t);
		sprintf(hdiff_char,"%d",hdiff);
		sprintf(pNo_char,"%d",pNo);

		descFold = descPath + "HON4DDesc-" + cell_x_char + "_" + cell_y_char + "_" + cell_t_char + "_" + hdiff_char + "_" + pNo_char + "\\";

		string cmd = "mkdir " + descFold;
		system(cmd.c_str());

		int count = 0;
		clock_t start, end;
		start = clock();

		while ( openFile.good() )
		{
			getline (openFile,line);
			if (line.length()<2)
				continue;
			cout << line << endl;

			wstring tmp(line.length(), L' ');
			copy(line.begin(), line.end(), tmp.begin());

			cHON4D = new HON4D(cellwidth, cellheight, model_w, model_h,tmp,cell_t,hdiff,p,projDim);
			HON4DFeatLen=cHON4D->DetWin.featLen;
			pfHON4DFeat = new float[HON4DFeatLen];
			cHON4D->ReadAllImages();

			x = line.find_last_of("\\");
			fileNameSub = line.substr(x+1,line.length()-x-MSR_FILE_LEN);
			descFileName = descFold + fileNameSub + ".txt";
			savefile.open(descFileName);
			if(cHON4D->GetFeature(x0, y0, pfHON4DFeat)!=HON4DFeatLen){
				printf("Get HON4D feature error!\n");
				return 0;
			}

			savefile << pfHON4DFeat[0]; 
			for (int i=1;i<HON4DFeatLen;i++){ 
				savefile << "," << pfHON4DFeat[i] ;
			}
			savefile <<  "\n";				
			savefile.close();
			delete cHON4D;
			delete [] pfHON4DFeat;

			count ++;
			if(count % 10 == 0)
			{
				end = clock();
				std::cout<<"have processed "<<count;
				std::cout<<"the running time is "<<(end-start)/1000.0<<std::endl; 
			}
		}
		openFile.close();

	}

	/// localHON4D 
	/// for segmented video
	/// for MSRDailyAct3D
	else if (atoi(argv[1]) == 21)
	{
		// Path for the 4D projectors for the polychoron (provided in data folder)
		string projectorsPath = "..\\..\\Data\\Projectors\\";  

		string FileName;	
		int mode = 0;
		int if_3D_joint_position = atoi(argv[4]);
		/// use image as input
		if (atoi(argv[3]) == 1)
		{
			// Path for a text file which has a list of the videos to be processed. (For an example file, see "Data\MSRAction\FileNames_01.txt")
			FileName = "..\\..\\Data\\MSRDailyAct3D\\FileNames_02.txt";
			mode = 1;
		}
		/// use bin file as input
		else if(atoi(argv[3]) == 2)
		{
			FileName = "..\\..\\Data\\MSRDailyAct3D\\allfiles.txt";
			mode = 2;
		}
		else 
		{
			FileName = "..\\..\\Data\\MSRDailyAct3D\\FileNames_02_txt.txt";
			mode = 3;
		}
		// Path where the final descriptors will be saved
		string descPath = "..\\..\\Data\\MSRDailyAct3D\\Desc_local_HON4D_bin\\";


		int inputArg;
		int cell_x = 3;
		int cell_y = 3;
		int cell_t = 1;
		int startObject = 1;
		int hdiff = 1;
		//	int pNo = 0;
		int pNo = atoi(argv[2]);

		const int MSR_FILE_LEN = 5; // for MSRAction and MSRDailiy and MSRGesture naming convension

		//	initialize model size
		int patch_w,patch_h, patch_depth, window_w, window_h, x0,y0;

		//	define HON4D class
		LocalHON4D * local_HON4D;// = new HON4D(8, 8, model_w, model_h);

		//	compute feature length
		int HON4DFeatLen;//=cHON4D->DetWin.featLen;

		//	define feature vector
		float *pfHON4DFeat = NULL;// = new float[HON4DFeatLen];


		// read the projectors
		CvMat* p= NULL;
		int projDim = readProjectors(pNo,projectorsPath,p);

		string descFileName;
		ifstream openFile;
		string line;
		ofstream savefile;
		string fileNameSub;
		int x;
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
		window_w = 320;
		window_h = 240;

		/// read skeleton information
		vector<vector<vector<pair<int, int> > > > skeleton_info;
		vector<vector<vector<MyCoords> > > joint_realworld_coords;
		LocalHON4D::read_skeleton_information(skeleton_info, joint_realworld_coords);

		string train_data = "..\\..\\Data\\MSRDailyAct3D\\Desc_local_HON4D_bin\\training_set\\train_svm.dat";
		wstring test_path = L"..\\..\\Data\\MSRDailyAct3D\\Desc_local_HON4D_bin\\test_set\\";
		string test_data = "..\\..\\Data\\MSRDailyAct3D\\Desc_local_HON4D_bin\\test_set\\test_svm.dat";
		wstring train_path = L"..\\..\\Data\\MSRDailyAct3D\\Desc_local_HON4D_bin\\training_set\\";

		MySVM mySVM(train_data, test_data);	
		mySVM.scale_format_convertion_data(train_path, train_data.c_str(), 1);
		cout<<"training set scale operation and format conversion is completed!"<<endl;
		//mySVM.control_train();
		//cout<<"the svm model training is completed!"<<endl;

		mySVM.load_training_min_max();
		mySVM.load_training_model();

		mySVM.scale_format_convertion_data(test_path, test_data.c_str(), 2);
		mySVM.control_predict();
		mySVM.free_train_model();
		mySVM.free_test_model();


		Sequential sequ(FileName);
		sequ.get_all_files();
		sequ.set_sliding_window_width(1);
		sequ.set_step_width(0);
		sequ.set_true_labels();
		sequ.write_true_labels_to_files();

		int current;
		if (argc<5)
		{
			current = 0;
		}
		else
			current = atoi(argv[4]);

		sequ.set_start_point(current);
		cout<<"current_index: "<<sequ.get_current_index()<<endl;


		openFile.open(FileName);
		if (openFile.fail())
		{
			cout<<"error: cannot open file"<<FileName<<endl;
			exit(1);
		}

		string descFold;
		char cell_x_char[5];
		char cell_y_char[5];
		char cell_t_char[5];
		char hdiff_char[5];
		char pNo_char[5];
		char if_3D_joint_char[5];
		sprintf(cell_x_char,"%d",cell_x);
		sprintf(cell_y_char,"%d",cell_y);
		sprintf(cell_t_char,"%d",cell_t);
		sprintf(hdiff_char,"%d",hdiff);
		sprintf(pNo_char,"%d",pNo);
		sprintf(if_3D_joint_char, "%d", if_3D_joint_position);

		descFold = descPath + "HON4DDesc-" + cell_x_char + "_" + cell_y_char + "_" + cell_t_char + "_" + hdiff_char + "_" + pNo_char + "_if3d_"+if_3D_joint_char +"\\";

		string cmd = "mkdir " + descFold;
		system(cmd.c_str());

		int processed_frames = 0;
		int count = 0;
		clock_t start, end;
		start = clock();

		while ( openFile.good())
		{
			clock_t t0, t1, t2, t3, t4, t5;
			t0 = clock();
			getline (openFile,line);
			if (line.length()<2)
				continue;
			cout << line << endl;

			wstring tmp(line.length(), L' ');
			copy(line.begin(), line.end(), tmp.begin());

			//if (count < 250)
			//{
			//	count ++;
			//	continue;
			//}


			t5 = clock();
			local_HON4D = new LocalHON4D(mode, if_3D_joint_position, cellwidth, cellheight, window_w, window_h, patch_w, patch_h, patch_depth,
				tmp,cell_t,hdiff,p,projDim, skeleton_info[count], joint_realworld_coords[count]);
			HON4DFeatLen=local_HON4D->get_feature_length();
			pfHON4DFeat = new float[HON4DFeatLen];
			local_HON4D->Read_Image();
			t4 = clock();
			cout<<"time for reading pic: "<<(t4-t5)/1000.0<<endl;

			x = line.find_last_of("\\");
			fileNameSub = line.substr(x+1,line.length()-x-MSR_FILE_LEN);
			descFileName = descFold + fileNameSub + ".txt";
			savefile.open(descFileName);
			if(local_HON4D->GetFeature(pfHON4DFeat)!= HON4DFeatLen){
				printf("get hon4d feature error!\n");
				return 0;
			}
			t1 = clock();
			cout<<"stage1: "<<(t1-t0)/1000.0<<endl;
			savefile << pfHON4DFeat[0]; 
			for (int i=1;i<HON4DFeatLen;i++){ 
				savefile << "," << pfHON4DFeat[i] ;
			}
			         savefile <<  "\n";				
			         savefile.close();
			t2 = clock();
			cout<<"time for store data: "<<(t2-t1)/1000.0<<endl;
			processed_frames = processed_frames + (local_HON4D->get_frames());
			delete local_HON4D;
			delete [] pfHON4DFeat;

			count ++;
			if(count % 10 == 0)
			{	
				end = clock();
				std::cout<<"have processed "<<count <<" videos, ";
				std::cout<<"have processed "<<count <<" videos, "<<processed_frames<<" frames. ";
				std::cout<<"the running time is "<<(end-start)/1000.0<<std::endl; 
			}

			t3 = clock();
			cout<<"total time for this video: "<<(t3-t0)/1000.0<<endl;
		}

		openFile.close();
		mySVM.free_train_model();

	}

	else if(atoi(argv[1]) == 55)
	{
		int total_frames = 0;
		clock_t read_s, read_f;
		read_s = clock();

		//	system("dir /a-d /s /b  ..\\..\\..\\MSRDailyAct3D\\*.bin* >..\\..\\..\\allfiles.txt");

		ifstream input;
		input.open("..\\..\\Data\\MSRDailyAct3D\\allfiles.txt");
		if (input.fail())
		{
			cout<<"error: cannot open file"<<"..\\..\\..\\allfiles.txt"<<endl;
			exit(1);
		}
		vector<string> path_vec;
		string str;
		int index = 0;
		while (getline(input, str))
		{
			index ++;
			path_vec.push_back(str);
		}

		cout<<"the size of path_vec: "<<path_vec.size()<<endl;

		for (int i=0; i<(int)path_vec.size(); i++)
		{
			const char* depthFileName = path_vec[i].c_str();

			FILE * fp = fopen(depthFileName, "rb");

			if(fp == NULL)
			{
				cout<<"error: cannot open the file:" << depthFileName<<endl;
				return 1;
			}


			int nofs = 0; //number of frames conatined in the file (each file is a video sequence of depth maps)
			int ncols = 0;
			int nrows = 0;
			ReadDepthMapSktBinFileHeader(fp, nofs, ncols, nrows);

			total_frames += nofs;
			cout<<i<<": "<<depthFileName<<endl;
			printf("number of frames=%i\n", nofs);
			cout<<"column: "<<ncols<<"  row: "<<nrows<<endl;

			//read each frame
			int f; //frame index
			for(f=0; f<nofs; f++)
			{
				CDepthMapSkt depthMap;
				depthMap.SetSize(ncols, nrows); //it allocates space
				//the data will be stored in <depthMap>
				int max_v = ReadDepthMapSktBinFileNextFrame(fp, ncols, nrows, depthMap);

				//check to see what has been loaded for DEBUG purpose:
				int nNonZeroPoints = depthMap.NumberOfNonZeroPoints();
				float avg = depthMap.AvgNonZeroDepth();
				vector<pair<int, int> > skeleton;

			}

			fclose(fp);
			fp=NULL;

			cout<<"finish "<<i<<endl;

		}

		read_f = clock();

		cout<<"total frames is: "<<total_frames<<endl;
		cout<<"total time is "<<(read_f - read_s)/1000.0<<"  average for each frame is: "<<(read_f-read_s)/(double)total_frames<<endl;

	}


	return 0;	
}

*/








