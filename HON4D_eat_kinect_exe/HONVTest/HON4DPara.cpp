#include "stdafx.h"
#include "HON4D.h"
#include<stdio.h>
#include <iostream>


HON4D::HON4D(int nCellWidth, int nCellHeight, int nWinWidth, int nWinHeight,wstring vidPath,int cell_t,int hdiff,CvMat* proj,int binsz)
{
	
	_if_one_forth = 1;
	int iRC = 0;
	iRC = SearchDirectory(vecFiles,vidPath, L"png");
	noImages = vecFiles.size();

	
	SetDefaultParam();

	cell.width = nCellWidth;
	cell.height = nCellHeight;
	cell.depth = (noImages-1)/cell_t;
	DetWin.width = nWinWidth;
	DetWin.height = nWinHeight;
	DetWin.depth = cell.depth*cell_t;

	HON4D_difference = hdiff;

	

	//preparePMat();
	cell.bin_n_HON4D = binsz;
	p =  cvCreateMat(cell.bin_n_HON4D,4,CV_32FC1);
	cvCopy(proj,p);
	binThreshold =1.3090;// 3.5674 for 600 // 0.5 for 24//  1.3090 for 120 

	UpdateEnviorment();
}

HON4D::HON4D(int nCellWidth, int nCellHeight, int nWinWidth, int nWinHeight,int cell_t,int hdiff,CvMat* proj,int binsz, const std::vector<std::wstring>& ivecFiles)
{

	_if_one_forth = 1;
	vecFiles = ivecFiles;
	noImages = vecFiles.size();
//	cout<<"noImages: "<<noImages<<endl;



	SetDefaultParam();

	cell.width = nCellWidth;
	cell.height = nCellHeight;
	cell.depth = (noImages-1)/cell_t;
//	cout<<"cell.depth: "<<cell.depth<<endl;
	DetWin.width = nWinWidth;
	DetWin.height = nWinHeight;
	DetWin.depth = cell.depth*cell_t;

	HON4D_difference = hdiff;


	//preparePMat();
	cell.bin_n_HON4D = binsz;
	p =  cvCreateMat(cell.bin_n_HON4D,4,CV_32FC1);
	cvCopy(proj,p);
	binThreshold =1.3090;// 3.5674 for 600 // 0.5 for 24//  1.3090 for 120 

	UpdateEnviorment();
}


HON4D::~HON4D()
{

	// delete the images
	for (int i=0;i<noImages;i++)
		SafeImgDel(images[i]);

	SafeMatDel(p);


	if(NULL != HON4D_dx_image){
		delete [] HON4D_dx_image;
		need_compute_dxdydz_bin_index_image = true;
	}
	if(NULL != HON4D_dy_image){
		delete [] HON4D_dy_image;
		need_compute_dxdydz_bin_index_image = true;
	}

	if(NULL != HON4D_dz_image){
		delete [] HON4D_dz_image;
		need_compute_dxdydz_bin_index_image = true;
	}


	if(NULL != feature_all_cells_of_whole_image){
		delete [] feature_all_cells_of_whole_image;
		need_compute_dxdydz_bin_index_image = true;
	}
	if(NULL != total_hist_of_cells){
		delete [] total_hist_of_cells;
	}
}

void HON4D::SetDefaultParam()
{
	cell.width = 16;
	cell.height = 16;
	cell.depth = 17;
	cell.bin_n_HON4D = 300;

	DetWin.width = 128;
	DetWin.height = 128;

	images = NULL;

	HON4D_dx_image=NULL;
	HON4D_dy_image=NULL;
	HON4D_dz_image=NULL;
	feature_all_cells_of_whole_image=NULL;

	HON4D_difference = 5;
}

void HON4D::UpdateEnviorment()
{
	need_compute_dxdydz_bin_index_image = true;
	total_hist_of_cells = new float [cell.bin_n_HON4D];

	cell.numR = DetWin.width/cell.width;
	cell.numC = DetWin.height/cell.height;
	cell.numD = DetWin.depth/cell.depth;

	DetWin.featLen = cell.numD*cell.numR * cell.numC * cell.bin_n_HON4D;

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

	if(NULL != feature_all_cells_of_whole_image){
		delete [] feature_all_cells_of_whole_image;
		feature_all_cells_of_whole_image = NULL;
	}
}
