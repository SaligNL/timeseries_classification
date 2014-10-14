#ifndef HON4D_H_
#define HON4D_H_
#include <cv.h>
#include <highgui.h>
#include <string>
#include <windows.h>
#include <vector>
#include <fftw3.h>


using namespace std;

#define PI 3.1415926
#define MAX_DIFF_16	 	1500	//Assume that the three channel have the same difference value.
								// If they are not the same, it could be tuned to be the same by structure CBDiscColor.
//#define D_TAB_SIZE_16		(2*MAX_DIFF_16+1)

#define PROPORTION_OF_DIVIDING_PHI_BINS 0.7
//#define CUT_BIN_RATE 0.1

//#define HON4D_difference 5

inline void SafeImgDel(IplImage * imgRelease)
{
	if(imgRelease!=NULL) cvReleaseImage( &imgRelease);
	imgRelease = NULL;
}

inline void SafeMatDel(CvMat * matRelease)
{
	if(matRelease!=NULL) cvReleaseMat( &matRelease);
	matRelease = NULL;
}

typedef struct _tHON4DCell{
	int width;	//width used to build cell,default 8
	int height;	//height used to build cell,default 8
	int depth;
	int numR;	//cell number of each row
	int numC;	//cell number of each column
	int numD;	//cell number of each column
	int	bin_n_HON4D;//Bin number of theta in HON4D
}HON4D_cell;

typedef struct _tHON4DWindow{
	int width;	//width used for detection window, default 64
	int height;	//height used for detection window, default 128
	int	depth;//overall feature length for detection window
	int	featLen;//overall feature length for detection window
}HON4D_win;


class HON4D{

public:
	HON4D_cell cell;
	HON4D_win DetWin;
	//IplImage *pImg;
	IplImage **images;
	double golden; 
	double binThreshold;
	double goldeninv;
	

	float *HON4D_dx_image;
	float *HON4D_dy_image;
	float *HON4D_dz_image;

	float *feature_all_cells_of_whole_image;
	//int feature_length_for_all_cells_of_whole_image;


	int HON4D_difference;
	float theta_max;
	float phi_max;
	float si_max;
	int PixBegin;//the first pixel to calculate HON4D
	//float *HON4D_bin;
	bool need_compute_dxdydz_bin_index_image;
	float *total_hist_of_cells;
	int rotate_theta_bin, rotate_phi_bin;

	HON4D(int nCellWidth, int nCellHeight, int nWinWidth, int nWinHeight,wstring vidPath,int cell_t, int hdiff,CvMat* proj,int binsz);

	/// for sequence test
	HON4D::HON4D(int nCellWidth, int nCellHeight, int nWinWidth, int nWinHeight,int cell_t,int hdiff,CvMat* proj,int binsz, const std::vector<std::wstring>& ivecFiles);

	~HON4D();
	void UpdateImage(IplImage *pImg,int indexImg);
	void ReadAllImages();
	int GetFeature(int nPointx, int nPointy, float *pFeatOut);
	//int compute_theta_bin(int dx, int dy,int dz);
	//int compute_phi_bin(int dx, int dy,int dz);
	void compute_HON4D_hist(int dx, int dy,int dz,float* hist, vector<float>& p_d);
	int normalize_feature(float *feature_vector, float total_n);


	/// added by Wenjie Pei
	void set_start_end_frames(int start, int end) {_start_frame = start; _end_frame = end;}
	void set_pre_start_end_frames(int start, int end) {_pre_start_frame = start; _pre_end_frame = end;}
	int get_start_frame(){return _start_frame;}
	int get_end_frame(){return _end_frame;}
	void set_pre_dist_bin(const vector<vector<float> >& pre) {_pre_dist_bin = pre;}

	void get_point_dist_bin(vector<vector<float> >& pre_dist_bin) {pre_dist_bin = _point_dist_bin;} 
	const vector<vector<float> >& get_point_dist_bin() const {return _point_dist_bin;} 

	void compute_overlap_area();


private:
	std::vector<std::wstring> vecFiles;
	CvMat* p; // holds all vectors of orientation
	int noImages;
	void SetDefaultParam();
	void UpdateEnviorment();
	void preparePMat();
	//void set_histogram_bins();
	void compute_dxdydz_bin_index_image();
	int GetFeatInner(int nPointx, int nPointy, float *pFeatOut);
	int SearchDirectory(std::vector<std::wstring> &refvecFiles,
                    const std::wstring        &refcstrRootDirectory,
                    const std::wstring        &refcstrExtension,
                    bool                     bSearchSubdirectories = true);


	/// added by Wenjie Pei
	/// for optimization of sequential process
	vector<vector<float> > _point_dist_bin;
	vector<vector<float> > _pre_dist_bin;

	int _start_frame;
	int _end_frame;
	int _pre_start_frame;
	int _pre_end_frame;
	int _overlap_start;
	int _overlap_end;

	int _if_one_forth;
};

#endif /* HON4D_H_ */
