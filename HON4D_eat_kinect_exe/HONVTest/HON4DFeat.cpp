#include "stdafx.h"
#include "HON4D.h"
#include "math.h"
#include<stdio.h>
#include<stdlib.h>
#include<cxcore.h>
#include<highgui.h>
#include<typeinfo>
#include<string>
#include<iostream>
#include <vector>
#include <conio.h>
#include <windef.h>
#include <windows.h>

using namespace std;

int HON4D::SearchDirectory(std::vector<wstring> &refvecFiles,
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

  /*for(int i=0; i<10; i++)
	  std::wcout << refvecFiles[i]<<std::endl;*/

  return 0;
}


void HON4D::ReadAllImages()
{
	images = new IplImage*[noImages];
	IplImage *imgDepth;//=NULL;
	int counter = 0;
	
	for(vector<std::wstring>::iterator iterFiles = vecFiles.begin();iterFiles != vecFiles.end();++iterFiles)
	{
		wstring ws = *iterFiles;
		string FileName(ws.begin(), ws.end());
		FileName.assign(ws.begin(), ws.end());

		imgDepth = NULL;
		imgDepth = cvLoadImage(FileName.c_str(),CV_LOAD_IMAGE_ANYDEPTH);
		UpdateImage(imgDepth,counter);
		
	
		counter++;
	}

	need_compute_dxdydz_bin_index_image = true;
	//feature_length_for_all_cells_of_whole_image=(images[0]->width/cell.width)*(images[0]->height/cell.height)*cell.bin_n_theta*cell.bin_n_phi;
}

void HON4D::UpdateImage(IplImage *pInImg,int indexImg)
{

	IplImage *pImg;
	pImg = NULL;
//	median filter
	cvSmooth(pInImg, pInImg, CV_MEDIAN, 5, 5);

//	allocate pImg a float image
	pImg = cvCreateImage(cvSize(pInImg->width,pInImg->height),IPL_DEPTH_32F,1);
	cvConvert(pInImg,pImg);
	SafeImgDel(pInImg);

//	bilateral filter
	IplImage *pImg_bilateral=NULL;
	pImg_bilateral = cvCreateImage(cvSize(pImg->width,pImg->height),IPL_DEPTH_32F,1);
	cvSmooth(pImg, pImg_bilateral, CV_BILATERAL,11,11,50.0f,50.0f);
	SafeImgDel(pImg);
	images[indexImg]=cvCloneImage(pImg_bilateral);
	SafeImgDel(pImg_bilateral);
}


void HON4D::compute_HON4D_hist(int dx, int dy,int dz,float* hist, vector<float>& p_d)
{
	float fTan, fi, fj,fk, f_value = 0.0;
	int which_bin;

	fi=float(dx);
	fj=float(dy);
	fk=float(dz);
	float gmag = sqrt(fi*fi + fj*fj + fk*fk);
	
	//if (gmag==0){
	//	return;
	//}

	CvScalar s;
	CvMat* d;
	CvMat* mulres;
	CvMat* magMat;

	d =  cvCreateMat(4,1,CV_32FC1);

	cvSet2D(d,0,0,cvScalar(fi));
	cvSet2D(d,1,0,cvScalar(fj));
	cvSet2D(d,2,0,cvScalar(fk));
	cvSet2D(d,3,0,cvScalar(-1));

	mulres = cvCreateMat(cell.bin_n_HON4D,1,CV_32FC1);
	cvMatMul(p,d,mulres);

	//s = cvGet2D(mulres,0,0);
	//cout<<s.val[0];


	// prepare gmag matrix
	magMat = cvCreateMat(cell.bin_n_HON4D,1,CV_32FC1);
	cvSetZero(magMat);
	cvAddS(magMat,cvScalar(gmag),magMat);

	//divide result over gmag
	cvDiv(mulres,magMat,mulres);

	// subtract threshold
	cvSubS(mulres,cvScalar(binThreshold),mulres);

	// remove all negative values
	for (int i=0;i<cell.bin_n_HON4D;i++) {
		s = cvGet2D(mulres,i,0);

		if (s.val[0]<0){ 
			 cvSet2D(mulres,i,0,cvScalar(0));
		}
	}


	//multiply by magnitude
	//cvMul(mulres,magMat,mulres);

	// compute vec magnitude
	float vecmag = 0;
	for (int i=0;i<cell.bin_n_HON4D;i++) {
		s = cvGet2D(mulres,i,0);
		vecmag  = vecmag +  (s.val[0]*s.val[0]);
	}
	vecmag = sqrt(vecmag);

	if (vecmag!=0){
		//prepare vecmagmat 
		cvSetZero(magMat);
		cvAddS(magMat,cvScalar(vecmag),magMat);

		//divide result over vecmag
		cvDiv(mulres,magMat,mulres);
	

		// copy result to histogram
		for (int i=0;i<cell.bin_n_HON4D;i++) {
		s = cvGet2D(mulres,i,0);
		hist[i]  = hist[i]  + s.val[0];
		p_d[i]  = p_d[i] + s.val[0];

		}
	}

	SafeMatDel(mulres);
	SafeMatDel(magMat);
	SafeMatDel(d);
}

void HON4D::compute_overlap_area()
{
//	cout<<"pre: "<<_pre_start_frame<<" "<<_pre_end_frame<<"    "<<_start_frame<<" "<<_end_frame<<endl;

	if (_pre_start_frame > _pre_end_frame)
	{
		_overlap_start = -1;
		_overlap_end = -1;
		return;
	}
	

	int valid_frames = (int)_pre_dist_bin.size() / (cell.numC * cell.numR);
	if (_pre_end_frame < _pre_start_frame + valid_frames-1)
	{
		cout<<"error: _pre_end_frame < _pre_start_frame + valid_frames-1" << endl;
		exit(1);
	}
	
	cout<<"valid: "<<valid_frames<<endl;

	_pre_end_frame = _pre_start_frame + valid_frames -1;
	cout<<"pre_start_frame: "<<_pre_start_frame<<" pre_end_frame: "<<_pre_end_frame<<endl;

	if (_pre_start_frame > _end_frame)
	{
		_overlap_start = -1;
		_overlap_end = -1;
	}
	else if (_pre_end_frame < _start_frame)
	{
		_overlap_start = -1;
		_overlap_end = -1;
	}
	else if (_pre_start_frame > _start_frame && _pre_end_frame > _end_frame)
	{
		_overlap_start = _pre_start_frame;
		_overlap_end = _end_frame;
	}
	else if (_pre_start_frame > _start_frame && _pre_end_frame <= _end_frame)
	{
		_overlap_start = _pre_start_frame;
		_overlap_end = _pre_end_frame;
	}
	else if (_pre_start_frame <= _start_frame && _pre_end_frame > _end_frame)
	{
		_overlap_start = _start_frame;
		_overlap_end = _end_frame;
	}
	else if (_pre_start_frame <= _start_frame && _pre_end_frame <= _end_frame)
	{
		_overlap_start = _start_frame;
		_overlap_end = _pre_end_frame;
	}
	else
	{
		cout<<"error: there are still some situations that we haven't considered!"<<endl;
		exit(1);
	}

	if (_overlap_end < _overlap_start)
	{
		cout<<"error: _overlap_end < _overlap_start"<<endl;
		exit(1);
	}
	
	//cout<<"overlap: "<<_overlap_start<<" "<<_overlap_end<<endl;
	
	
	
	
}


void HON4D::compute_dxdydz_bin_index_image()
{
	int nImg_w= images[0]->width;
	int nImg_h= images[0]->height;

	float *dxLine= HON4D_dx_image;
	float *dyLine= HON4D_dy_image;
	float *dzLine= HON4D_dz_image;

	int dx,dy,dz;
	CvScalar left,right,up,down,current,next;


	memset(HON4D_dx_image,0, nImg_h*nImg_w*(noImages-1)*sizeof(float));
	memset(HON4D_dy_image,0, nImg_h*nImg_w*(noImages-1)*sizeof(float));
	memset(HON4D_dz_image,0, nImg_h*nImg_w*(noImages-1)*sizeof(float));


	bool if_start = true;
	if (_end_frame - _start_frame == 12)
	{
		if_start = false;
	}

	for (int indImg=0;indImg<noImages-1;indImg++)
	{	
		int tmin = cell.numD * cell.depth-1;
		for (int i=PixBegin; i<nImg_h-PixBegin; i++)
		{
			dxLine+= nImg_w;
			dyLine+= nImg_w;
			dzLine+= nImg_w;
			
			if (((indImg+_start_frame) >= _overlap_start) && ((indImg+_start_frame) <= _overlap_end) && indImg < tmin && if_start)
			{
				continue;
			}

			for ( int j=PixBegin; j < nImg_w-PixBegin; j++)
			{
				
				if(i<HON4D_difference || i>=nImg_h-HON4D_difference || j<HON4D_difference || j>=nImg_w-HON4D_difference)
				{
					*(dxLine+j)= 0;
					*(dyLine+j)= 0;
					*(dzLine+j)= 0;
					continue;
				}

	//			get 4 pixel values around this pixel
				left = cvGet2D(images[indImg],i,j-HON4D_difference);
				right = cvGet2D(images[indImg],i,j+HON4D_difference);
				up = cvGet2D(images[indImg],i-HON4D_difference,j);
				down = cvGet2D(images[indImg],i+HON4D_difference,j);
				current = cvGet2D(images[indImg],i,j);
				next = cvGet2D(images[indImg+1],i,j);


	//			compute dx dy
				dx= int(right.val[0]-left.val[0]);
				dy= int(down.val[0]-up.val[0]);
				dz= int(current.val[0]-next.val[0]);

				*(dxLine+j)= dx;
				*(dyLine+j)= dy;
				*(dzLine+j)= dz;

				//if (_if_one_forth == 1 && j%2 == 1)
				//{
				//	j++;
				//}
			}
			//if (_if_one_forth == 1 && i%2 == 1)
			//{
			//	i++;
			//}
		}
	}
}

int HON4D::GetFeatInner(int nPointx, int nPointy, float *pFeatOut)
{
	int total_n=0;
	//int bin_n_theta = cell.bin_n_theta;
	//int bin_n_phi = cell.bin_n_phi;
	//int bin_n_si = cell.bin_n_si;
	int bin_n_HON4D = cell.bin_n_HON4D;
	float *temp_hist = new float [bin_n_HON4D];

	int image_w= images[0]->width;
	int image_h= images[0]->height;

	int win_w = DetWin.width;
	int win_h = DetWin.height;

	int win_end_point_x = nPointx+win_w-1;
	int win_end_point_y = nPointy+win_h-1;

	if(win_end_point_x>image_w || win_end_point_y>image_h)
	{
		printf("HON4D error! Detection window gets out of image!!!\n");
		exit(0);
	}

	int ci,cj,ct,xmin,ymin,xmax,ymax,x,y,dx,dy,dz,t,tmin,tmax;
	float *pFeatOut_pointer=pFeatOut;


	//	compute histogram for each cell
	int count = 0;
	for(ct=0;ct<cell.numD;ct++)
		for(ci=0;ci<cell.numR;ci++)
			for(cj=0;cj<cell.numC;cj++)
			{
				total_n=0;

				memset(temp_hist,0.0, bin_n_HON4D*sizeof(float));


				xmin = nPointx + cell.width * ci;
				ymin = nPointy + cell.height * cj;
				xmax = xmin + cell.width -1;
				ymax = ymin + cell.height -1;

				tmin = cell.depth*ct;
				tmax = tmin + cell.depth-1;

	//			cout<<"test: "<<" tmin "<<tmin<<" tmax: "<<tmax<<endl;

				//			for each pixel in this cell
				for(t=tmin;t<=tmax;t++) //-1 because of image difference
				{
					//		cout<<"tmin: "<<tmin<<" tmax: "<<tmax<<" ct: "<<ct<<" ci: "<<ci<<" cj: "<<cj<<"  t: "<<t<<endl;
					
					bool if_start = true;
					if (_end_frame - _start_frame == 12)
					{
						if_start = false;
					}
					
					/// if in the overlap interval
		//			if (((t+_start_frame) >= _overlap_start) && ((t+_start_frame) <= _overlap_end) && (!(ct ==2 && ci == 2 && cj ==3)) )
					if (((t+_start_frame) >= _overlap_start) && ((t+_start_frame) <= _overlap_end) && (!(t == cell.numD * cell.depth-1)) && if_start)

					{
			//			cout<<"overlap: "<<ct<<" "<<ci<<" "<<cj<<" "<<t+_start_frame<<"  "<<_overlap_start<<" "<<_overlap_end<<endl;
						int temp_index = t+_start_frame - _pre_start_frame;

						const vector<float>& temp_v = _pre_dist_bin[ci+cj*cell.numR + temp_index*cell.numC*cell.numR];
			//			const vector<float>& temp_v = _pre_dist_bin[temp_index+(cj+ci*cell.numC)*cell.depth];

						// copy result to histogram
						for (int i=0;i<cell.bin_n_HON4D;i++) {
							temp_hist[i]  = temp_hist[i]  + temp_v[i];
						}
						_point_dist_bin[ci+cj*cell.numR + t*cell.numC*cell.numR] = temp_v;
		//				_point_dist_bin[temp_index+(cj+ci*cell.numC)*cell.depth] = temp_v;

						count++;
					}
					else
					{
						vector<float>& p_d = _point_dist_bin[ci+cj*cell.numR + t*cell.numC*cell.numR];
		//				vector<float>& p_d = _point_dist_bin[t+(cj+ci*cell.numC)*cell.depth];

						p_d.resize(cell.bin_n_HON4D, 0.0);
						count ++;

						for(x=xmin;x<=xmax;x++)
						{
							for(y=ymin;y<=ymax;y++)
							{
								
								//					find out which bin to vote
								dx = HON4D_dx_image[x+y*image_w + t*image_w*image_h];
								dy = HON4D_dy_image[x+y*image_w + t*image_w*image_h];
								dz = HON4D_dz_image[x+y*image_w + t*image_w*image_h];

								
								if(dx == 0 || dy == 0 || dz == 0)
								{
									continue;
								}


								compute_HON4D_hist(dx,dy,dz,temp_hist, p_d);

								if (_if_one_forth == 1 && y%2 == 1)
								{
									y++;
								}
								

								// comment if not vote for 1
								//total_n++;
							}
							if (_if_one_forth == 1 && x%2 == 1)
							{
								x++;
							}
						}

					}

				}

				// comment if vote for 1
				// compute total_n
				for (int indHist=0;indHist<bin_n_HON4D;indHist++){
					total_n = total_n + temp_hist[indHist];
				}

	//			cout<<"xmin: "<<xmin<<" xmax: "<<xmax<<" ymin: "<<ymin<<" ymax: "<<ymax<<endl;
	//		cout<<"tmin: "<<tmin<<" tmax: "<<tmax<<" ct: "<<ct<<" ci: "<<ci<<" cj: "<<cj<<"  total_n: "<<total_n<<endl;

				if (total_n!= 0){
					normalize_feature(temp_hist, (float) total_n);
				}
				//			concatenate histogram in cells
				memcpy(pFeatOut_pointer, temp_hist, bin_n_HON4D*sizeof(float));

				// if not last iteration
				pFeatOut_pointer = pFeatOut_pointer + bin_n_HON4D;
			}

	//		cout<<"count: "<<count<<" size of point_dist " << _point_dist_bin.size()<<endl;
	//		_point_dist_bin.resize(count);

			delete [] temp_hist;
			pFeatOut_pointer=NULL;

			return DetWin.featLen;
}

int HON4D::GetFeature(int nPointx, int nPointy, float *pFeatOut)
{

			if(NULL !=HON4D_dx_image) delete []HON4D_dx_image;
			if(NULL != HON4D_dy_image) delete []HON4D_dy_image;
			if(NULL != HON4D_dz_image) delete []HON4D_dz_image;

			if((HON4D_dx_image = new float[images[0]->width*images[0]->height*(noImages-1)])== NULL)
			{
				printf("theta_bin_index_image OUT OF MEMORY!!!\n");
				exit(1);
			}

			if((HON4D_dy_image = new float[images[0]->width*images[0]->height*(noImages-1)])== NULL)
			{
				printf("phi_bin_index_image OUT OF MEMORY!!!\n");
				exit(1);
			}


			if((HON4D_dz_image = new float[images[0]->width*images[0]->height*(noImages-1)])== NULL)
			{
				printf("si_bin_index_image OUT OF MEMORY!!!\n");
				exit(1);
			}


			/// added by Wenjie Pei
	//		_point_dist_bin.resize(images[0]->width*images[0]->height*(noImages-1));
			_point_dist_bin.resize(cell.numC*cell.numR*cell.depth*cell.numD);

			compute_overlap_area();

			compute_dxdydz_bin_index_image();
		return GetFeatInner(nPointx, nPointy, (float *)pFeatOut);
	

}

int HON4D::normalize_feature(float *feature_vector, float total_n)
{
	int feature_dim = cell.bin_n_HON4D;

	int i;

	if(total_n>0)
		for(i=0;i<feature_dim;i++)
			feature_vector[i]=feature_vector[i]/total_n;

	return 1;
}
