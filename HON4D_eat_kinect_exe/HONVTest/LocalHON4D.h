#ifndef LocalHON4D_H_
#define LOcalHON4D_H_

#include "HON4D.h"
#include "..\\DepthMapSktIO\DepthMapSktBinFileIO.h"
#include "myMatrix.h"
#include "sequential_version.h"
#include "read_from_kinect.h"

#include <cv.h>
#include <highgui.h>
#include <string>
#include <windows.h>
#include <vector>
#include <fftw3.h>
#include <time.h>
#include <cmath>
#include <algorithm>



using namespace std;

#define PI 3.1415926
#define MAX_DIFF_16	 	1500	//Assume that the three channel have the same difference value.
								// If they are not the same, it could be tuned to be the same by structure CBDiscColor.
//#define D_TAB_SIZE_16		(2*MAX_DIFF_16+1)

#define PROPORTION_OF_DIVIDING_PHI_BINS 0.7
//#define CUT_BIN_RATE 0.1 

//#define HON4D_difference 5

typedef struct _tLocalHON4DPatch{
	int width;	//width used for detection window, default 64
	int height;	//height used for detection window, default 128
	int	depth;//overall feature length for detection window
	int	featLen;//overall feature length for detection window
}LocalHON4D_Patch;

inline int nextPow2(int x)
{
    if (x < 0)
        return 0;
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x+1;
}


class LocalHON4D{

public:

	LocalHON4D(){}
	LocalHON4D(int mode, int if_3D_joint_position, int nCellWidth, int nCellHeight, int nWinWidth, int nWinHeight, int patch_w, int patch_h, int patch_depth, 
		wstring vidPath,int cell_t, int hdiff,CvMat* proj,int binsz, const vector<vector<pair<int, int> > >& skeleton_info, const vector<vector<MyCoords> >& joint_realworld);
	LocalHON4D(int mode, int if_3D_joint_position, int nCellWidth, int nCellHeight, int nWinWidth, int nWinHeight, int patch_w, int patch_h, int patch_depth, 
		int cell_t, int hdiff,CvMat* proj,int binsz, const vector<vector<pair<int, int> > >& skeleton_info, 
		const vector<vector<MyCoords> >& joint_realworld, const Sequential& sequ, int pre_start_frame, int pre_end_frame);
	/// for kinect version
	LocalHON4D::LocalHON4D(int if_3D_joint_position, int nCellWidth, int nCellHeight, int nWinWidth, int nWinHeight, int patch_w, int patch_h, int patch_depth,
		int cell_t, int hdiff,CvMat* proj,int binsz,  const Sequential& sequ, const Read_Kinect& read_kinect, int pre_start_frame, int pre_end_frame);

	~LocalHON4D();
	void UpdateImage(IplImage *pImg,int indexImg);
	void compute_overlap_area();
    void Read_Image();
	void ReadAllImages();
    void ReadAllImages_txt();
	void ReadAllImagesBin(FILE * fp);
	int medfilt2();
	int GetFeature(float* pFeatOut);
	int GetFeature(float* pFeatOut, const vector<vector<float> >& pre_HON4D_feature );
	int GetFeature_one_joint_one_frame(int joint_ind, int frame_ind, vector<float>& pFeatOut);
	void compute_3DJointPosition_one_joint_one_frame(int joint_ind, int frame_ind, vector<float>& pFeatOut);
	void compute_HON4D_hist(int dx, int dy,int dz,float* hist);
	int normalize_feature(float *feature_vector, float total_n);

	static int read_skeleton_information(vector<vector<vector<pair<int, int> > > >& skeleton_info, 
		vector<vector<vector<MyCoords> > > & joint_realworld_coord);

	int get_feature_length(){return DetWin.featLen;}
	int get_frames() {return noImages;}

	float* myFFT(float *pFeatOut, int layer_index);
	void set_start_end_frames(int start, int end) 
	{
		_start_frame = start; _end_frame = end;
		//cout<<"start_frame: "<<_start_frame<<"  end_frame: "<<_end_frame<<endl;
	}
	void set_pre_vector_feature(vector<vector<float> >& pre_HON4D_feature) {pre_HON4D_feature = _HON4D_feature;}
	int get_start_frame(){return _start_frame;}
	int get_end_frame(){return _end_frame;}


private:
	void SetDefaultParam();
	void UpdateEnviorment();
	void preparePMat();
	//void set_histogram_bins();
	void compute_dxdydz_bin_index_image(int nPointw, int nPointh, int nPointt);
	int GetFeatInner(int nPointx, int nPointy, int nPointt, vector<float>& pFeatOut);
	int SearchDirectory(std::vector<std::wstring> &refvecFiles,
		const std::wstring        &refcstrRootDirectory,
		const std::wstring        &refcstrExtension,
		bool                     bSearchSubdirectories = true);


private:
	std::vector<std::wstring> vecFiles;
	CvMat* p; // holds all vectors of orientation
	int noImages;


	vector<vector<pair<int, int> > >  _skeleton_info;
	vector<vector<MyCoords> > _joints_realworld_coords;
	/// store the HON4D result, but not the final feature(fft result)
	vector<vector<float> > _HON4D_feature;

	HON4D_cell cell;
	HON4D_win DetWin;
	LocalHON4D_Patch _patch;
	//IplImage *pImg;
	IplImage **images;
    /// another data structure of it for bin or kinect
    vector<myMatrix> _images_value;
    
	double golden; 
	double binThreshold;
	int HON4D_difference;
	float *total_hist_of_cells;

	float *HON4D_dx_image;
	float *HON4D_dy_image;
	float *HON4D_dz_image;
	int PixBegin;

	int _joint_number;

	/// get _top_n coefficients from fft
	int _top_n;

    /// _mode == 1 means adopt ReadAllImages
    /// _mode == 2 means adopt ReadAllImages_txt
    int _mode;

	/*******
	   _if_3D_joint_position == 0 means only localHON4D
	   _if_3D_joint_position == 1 means only 3D joint position
	   _if_3D_joint_position == 2 means both 
	*/
	int _if_3D_joint_position;

	int _layer_number;

	int _pre_start_frame;
	int _pre_end_frame;
	int _start_frame;
	int _end_frame;
	int _overlap_start;
	int _overlap_end;

	bool _if_sequential;

};

#endif /* LocalHON4D_H_ */
