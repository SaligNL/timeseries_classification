#include "LocalHON4D.h"
#include <fstream>
using namespace std;

void LocalHON4D::compute_overlap_area()
{
	//	cout<<"pre: "<<_pre_start_frame<<" "<<_pre_end_frame<<"    "<<_start_frame<<" "<<_end_frame<<endl;

	if (_pre_start_frame > _pre_end_frame)
	{
		_overlap_start = -1;
		_overlap_end = -1;
		cout<<"error: pre_start_frame > pre_end_frame"<<endl;
		return;
	}

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
		//cout<<"_pre_start_frame > _start_frame!  pre_start_frame: "<<_pre_start_frame<<" start_frame: "<<_start_frame<<endl;
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

void LocalHON4D::Read_Image()
{
    if (_mode == 1)
    {
        ReadAllImages();
    }
    else if (_mode == 2)
    {
		/// do nothing since the reading work is done in Constructor
    }
	else
        ReadAllImages_txt();
    
}

void LocalHON4D::ReadAllImages()
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

}

void LocalHON4D::ReadAllImages_txt()
{

    int counter = 0;

    for(vector<std::wstring>::iterator iterFiles = vecFiles.begin();iterFiles != vecFiles.end();++iterFiles)
    {
        wstring ws = *iterFiles;
        string FileName(ws.begin(), ws.end());
        FileName.assign(ws.begin(), ws.end());

        /// read depth information
        fstream infile;
        infile.open(FileName.c_str());
        if (infile.fail())
        {
            cout<<"error: cannot open file: "<<FileName<<endl;
            exit(1);
        }
        
        myMatrix matd(DetWin.height, DetWin.width);
        for (int i=0; i<DetWin.height; i++)
            for (int j=0; j<DetWin.width; j++)
            {
                infile >> matd.data[i][j];
            }
        _images_value[counter] = matd;
        counter++;
    }

}

void LocalHON4D::ReadAllImagesBin(FILE * fp)
{
	//read each frame
	int f; //frame index
	for(f=0; f<noImages; f++)
	{
		CDepthMapSkt depthMap;
		depthMap.SetSize(DetWin.width, DetWin.height); //it allocates space
		//the data will be stored in <depthMap>
		int max_v = ReadDepthMapSktBinFileNextFrame(fp, DetWin.width, DetWin.height, depthMap);

		myMatrix matd(DetWin.height, DetWin.width);
		for (int i=0; i<DetWin.height; i++)
		//for (int i=DetWin.height-1; i>-1; i--)
			for(int j=0; j<DetWin.width; j++)
			{
				int val = depthMap.GetItem(i,j);
				matd.data[i][j] = val;
			}
		_images_value[f] = matd;

	}
	

	clock_t st, ft;
	st = clock();

//	medfilt2();

	ft = clock();
	cout<<"time for medfilt2: "<<(ft-st)/1000.0<<endl;

	fclose(fp);
	fp=NULL;

	
}

void LocalHON4D::UpdateImage(IplImage *pInImg,int indexImg)
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

int LocalHON4D::medfilt2()
{
	/// 5*5
	int wid = 5;
	int hei = 5;

	int rtn = 0;
	vector<int> can(wid*hei);

	for (int f=0; f<noImages; f++)
	{
		myMatrix temp_image = _images_value[f];
		for (int i=hei/2; i<DetWin.height-hei/2; i++)
			for (int j=wid/2; j<DetWin.width-wid/2; j++)
			{
				for (int h=0; h< hei; h++)
					for (int w=0; w<wid; w++)
					{
						can[h*w+w] = _images_value[f].data[h+i-hei/2][w+j-wid/2];
					}
				sort(can.begin(), can.end());
				int median = can[wid*hei/2];
				temp_image.data[i][j] = median;	
			}
		_images_value[f] = temp_image;
	
	}
	return rtn;
}

float* LocalHON4D::myFFT(float *ppointer, int layer_index)
{
	int temp_top_n;
	int partition;
	if (layer_index == 1)
	{
		temp_top_n = 10;
		partition = 1;
	}
	else if (layer_index == 2)
	{
		temp_top_n = 5;
		partition = 2;
	}
	else if (layer_index == 3)
	{
		temp_top_n = 2;
		partition = 4;
	}
	else
	{
		cout<<"layer_index is too big!"<<endl;
		exit(1);
	}
	
	
	
	int frame_len = (int)_HON4D_feature.size(); 
	int feature_len = (int)_HON4D_feature[0].size();
	int one_len = frame_len / partition;
	if (temp_top_n > one_len)
	{
		cout<<"error: temp_top_n > one_len"<<endl;
		exit(1);
	}

	int valid_num = nextPow2(one_len);

	int num = valid_num;
	fftw_complex *in, *out;
	fftw_plan pff;

	in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * num);
	out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * num);
	pff = fftw_plan_dft_1d(num, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

	for (int p=0; p<partition; p++)
	{
		/// transpose matrix
		for (int i=0; i<feature_len; i++)
		{
			for(int j=0; j<one_len; j++)
			{
				in[j][0] = (double)_HON4D_feature[j+p*one_len][i];
				in[j][1] = 0;
			}
			for (int j=one_len; j<num; j++)
			{
				in[j][0] = 0;
				in[j][1] = 0;
			}

			fftw_execute(pff); /* repeat as needed */

			vector<double> fft_feature(temp_top_n);

			/// calculate the magnitude
			for (int j=0; j<temp_top_n; j++)
			{
				fft_feature[j] = (out[j][0]*out[j][0] + out[j][1]*out[j][1])/ (double)valid_num;
			}

			for (int k=0; k<temp_top_n; k++)
			{
				*(ppointer+k) = (float)fft_feature[k];
				//cout<<"fft_feature: "<<fft_feature[k]<<endl;
			}
			ppointer += temp_top_n;

		}
	}
	
	

	fftw_destroy_plan(pff);
	fftw_free(in);
	fftw_free(out);


	return ppointer;

}

int LocalHON4D::GetFeature(float* pFeatOut)
{
    clock_t t0, t1, t2;
    t0 = clock();

	_HON4D_feature.resize(noImages- _patch.depth/2 - _patch.depth/2);
	
	for (int f=_patch.depth/2; f<noImages-_patch.depth/2; f++)
	{			

		for (int j=0; j<_joint_number; j++)
		{
			GetFeature_one_joint_one_frame(j,f, _HON4D_feature[f-_patch.depth/2]);
		}        
		
	}

	t1 = clock();
    //cout<<"time for localHON4D: "<<(t1-t0)/1000.0<<endl;
	/// next step: fft \\\

	float* ppointer = pFeatOut;
	for (int i=0; i<_layer_number; i++)
	{
		ppointer = myFFT(ppointer, i+1);
	}
	
	ppointer = NULL;
		
	t2 = clock();
//	cout<<"time for fft: "<<(t2-t1)/1000.0<<endl;

	return DetWin.featLen;
}

/// for sequential version 
int LocalHON4D::GetFeature(float* pFeatOut, const vector<vector<float> >& pre_HON4D_feature)
{
	clock_t t0, t1, t2;
	t0 = clock();

	_HON4D_feature.resize(noImages- _patch.depth/2 - _patch.depth/2);

	for (int f=_patch.depth/2; f<noImages-_patch.depth/2; f++)
	{	
		if ((f - _patch.depth/2 +_start_frame >= _overlap_start) && (f + _start_frame <=_overlap_end - _patch.depth/2))
		{

			int ind = f-_patch.depth/2 + _start_frame - _pre_start_frame;
			_HON4D_feature[f-_patch.depth/2] = pre_HON4D_feature[ind];
			continue;
		}

		for (int j=0; j<_joint_number; j++)
		{
			GetFeature_one_joint_one_frame(j,f, _HON4D_feature[f-_patch.depth/2]);
		}        

	}

	t1 = clock();
	//cout<<"time for localHON4D: "<<(t1-t0)/1000.0<<endl;
	/// next step: fft \\\

	float* ppointer = pFeatOut;
	for (int i=0; i<_layer_number; i++)
	{
		ppointer = myFFT(ppointer, i+1);
	}

	ppointer = NULL;

	t2 = clock();
	//cout<<"time for fft: "<<(t2-t1)/1000.0<<endl;

	return DetWin.featLen;
}

int LocalHON4D::GetFeature_one_joint_one_frame(int joint_ind, int frame_ind, vector<float>& pFeatOut)
{
	if (_if_3D_joint_position != 0)
	{
		/// 3D joint position feature;
		compute_3DJointPosition_one_joint_one_frame(joint_ind, frame_ind, pFeatOut);
	}

	if (_if_3D_joint_position != 1)
	{
		if(NULL !=HON4D_dx_image) delete []HON4D_dx_image;
		if(NULL != HON4D_dy_image) delete []HON4D_dy_image;
		if(NULL != HON4D_dz_image) delete []HON4D_dz_image;

		if((HON4D_dx_image = new float[_patch.width*_patch.height*_patch.depth])== NULL)
		{
			printf("theta_bin_index_image OUT OF MEMORY!!!\n");
			exit(1);
		}

		if((HON4D_dy_image = new float[_patch.width*_patch.height*_patch.depth])== NULL)
		{
			printf("phi_bin_index_image OUT OF MEMORY!!!\n");
			exit(1);
		}


		if((HON4D_dz_image = new float[_patch.width*_patch.height*_patch.depth])== NULL)
		{
			printf("si_bin_index_image OUT OF MEMORY!!!\n");
			exit(1);
		}

		int center_w;
		int center_h;

		if (_skeleton_info[frame_ind].size() == 0 && frame_ind > 0)
		{
			center_w = _skeleton_info[frame_ind-1][joint_ind].first-1;
			center_h = _skeleton_info[frame_ind-1][joint_ind].second-1;			
		}
		else
		{
			center_w = _skeleton_info[frame_ind][joint_ind].first-1;
			center_h = _skeleton_info[frame_ind][joint_ind].second-1;
		}
	
		//if (frame_ind == 3)
		//{
		//	cout<<joint_ind<<"  center_w: "<<(double)(center_w/320.0)<<" "<<center_w<<" center_h: "<<(double)(center_h/240.0)<<" "<<center_h<<endl;
		//	cout<<"depth: "<<_images_value[frame_ind].data[center_h][center_w]<<endl;
		//}



		int nPointw = center_w - _patch.width/2;
		int nPointh = center_h - _patch.height/2;
		int nPointt = frame_ind - _patch.depth/2;

		//cout<<joint_ind<<" "<<frame_ind<<": "<<nPointw<<" "<<nPointh<<" "<<nPointt<<endl;
		compute_dxdydz_bin_index_image(nPointw, nPointh, nPointt);
		GetFeatInner(nPointw, nPointh, nPointt, pFeatOut);
	}
	

	return 0;


}

void LocalHON4D::compute_dxdydz_bin_index_image(int nPointw, int nPointh, int nPointt)
{
	

	if (nPointt < 0)
	{
		nPointt = 0;
	}


	int nImg_w= _patch.width;
	int nImg_h= _patch.height;

	float *dxLine= HON4D_dx_image;
	float *dyLine= HON4D_dy_image;
	float *dzLine= HON4D_dz_image;

	int dx,dy,dz;
    //float dx,dy,dz;
	CvScalar left,right,up,down,current,next;


	memset(HON4D_dx_image,0, nImg_h*nImg_w*(_patch.depth)*sizeof(float));
	memset(HON4D_dy_image,0, nImg_h*nImg_w*(_patch.depth)*sizeof(float));
	memset(HON4D_dz_image,0, nImg_h*nImg_w*(_patch.depth)*sizeof(float));

	for (int indImg=nPointt;indImg<nPointt+_patch.depth;indImg++)
	{	
		for (int i=nPointh; i<nPointh+nImg_h; i++)
		{
			for ( int j=nPointw; j < nPointw+nImg_w; j++)
			{

				if(i<HON4D_difference || i>=DetWin.height-HON4D_difference || j<HON4D_difference || j>=DetWin.width-HON4D_difference)
				{
					*(dxLine+j-nPointw)= 0;
					*(dyLine+j-nPointw)= 0;
					*(dzLine+j-nPointw)= 0;
					continue;
				}


				//			get 4 pixel values around this pixel
                if (_mode == 1)
                {
                    left = cvGet2D(images[indImg],i,j-HON4D_difference);
                    right = cvGet2D(images[indImg],i,j+HON4D_difference);
                    up = cvGet2D(images[indImg],i-HON4D_difference,j);
                    down = cvGet2D(images[indImg],i+HON4D_difference,j);
                    current = cvGet2D(images[indImg],i,j);
                    next = cvGet2D(images[indImg+1],i,j);


                    //			compute dx dy
                    //dx= int((right.val[0]-left.val[0]));
                    //dy= int((down.val[0]-up.val[0]));
                    //dz= int((current.val[0]-next.val[0]));

					dx= int((current.val[0]-left.val[0]));
					dy= int((current.val[0]-up.val[0]));
					dz= int((next.val[0]-current.val[0]));
                }
                else
                {
                    int leftv = _images_value[indImg].data[i][j-HON4D_difference];
//                    int rightv = _images_value[indImg].data[i][j+HON4D_difference];
                    int upv = _images_value[indImg].data[i-HON4D_difference][j];
//                    int downv = _images_value[indImg].data[i+HON4D_difference][j];
                    int currentv = _images_value[indImg].data[i][j];
                    int nextv = _images_value[indImg+1].data[i][j];

                    //dx= rightv-leftv;
                    //dy= downv-upv;
                    //dz= currentv-nextv;

					dx = currentv - leftv;
					dy = currentv - upv;
					dz = nextv - currentv;
                }

				*(dxLine+j-nPointw)= dx;
				*(dyLine+j-nPointw)= dy;
				*(dzLine+j-nPointw)= dz;

			
			}
			
			dxLine+= nImg_w;
			dyLine+= nImg_w;
			dzLine+= nImg_w;
		}
	}
}

int LocalHON4D::GetFeatInner(int nPointx, int nPointy, int nPointt, vector<float>& pFeatOut)
{
	int total_n=0;

	int bin_n_HON4D = cell.bin_n_HON4D;
	float *temp_hist = new float [bin_n_HON4D];

	int image_w= _patch.width;
	int image_h= _patch.height;

	int win_w = DetWin.width;
	int win_h = DetWin.height;

	int win_end_point_x = nPointx+image_w-1;
	int win_end_point_y = nPointy+image_h-1;

	//if(win_end_point_x>win_w || win_end_point_y>win_h)
	//{
	//	printf("HON4D error! Detection window gets out of image!!!\n");
	//	cout<<win_end_point_x<<" "<<win_w<<"  "<<win_end_point_y<<" "<<win_h<<endl;
	//	cout<<"npointx: "<<nPointx<<"  npointy: "<<nPointy<<endl;
	//	exit(0);
	//}

	int ci,cj,ct,xmin,ymin,xmax,ymax,x,y,dx,dy,dz,t,tmin,tmax;


	//	compute histogram for each cell
	int count = 0;
	for(ct=0;ct<cell.numD;ct++)
		for(ci=0;ci<cell.numR;ci++)
			for(cj=0;cj<cell.numC;cj++)
			{
				total_n=0;

				memset(temp_hist,0.0, bin_n_HON4D*sizeof(float));
				int valid_n = 0;


				xmin = nPointx + cell.width * ci;
				ymin = nPointy + cell.height * cj;
				xmax = xmin + cell.width -1;
				ymax = ymin + cell.height -1;

				tmin = nPointt + cell.depth*ct;
				tmax = tmin + cell.depth-1;

				//	for each pixel in this cell
				for(t=tmin;t<=tmax;t++) //-1 because of image difference
				{
						count ++;

						for(x=xmin;x<=xmax;x++)
						{
							for(y=ymin;y<=ymax;y++)
							{


								//	find out which bin to vote
								int relative_x = x-nPointx;
								int relative_y = y-nPointy;
								int relative_t = t-nPointt;
								dx = HON4D_dx_image[relative_x+relative_y*image_w + relative_t*image_w*image_h];
								dy = HON4D_dy_image[relative_x+relative_y*image_w + relative_t*image_w*image_h];
								dz = HON4D_dz_image[relative_x+relative_y*image_w + relative_t*image_w*image_h];


								if(dx == 0 || dy == 0 || dz == 0)
								{
									continue;
								}

								valid_n ++;

								compute_HON4D_hist(dx,dy,dz,temp_hist);
							}
						}
				}

				// //comment if vote for 1
				// //compute total_n
				//for (int indHist=0;indHist<bin_n_HON4D;indHist++){
				//	total_n = total_n + temp_hist[indHist];
				//}

				//if (total_n!= 0){
				//	normalize_feature(temp_hist, (float) total_n);
				//}

				if (valid_n != 0)
				{
					normalize_feature(temp_hist, (float)valid_n);
				}
				

				//			concatenate histogram in cells
				int ss = (int)pFeatOut.size();
				pFeatOut.resize(ss + bin_n_HON4D);
				for (int indHist=0;indHist<bin_n_HON4D;indHist++){
					pFeatOut[ss+indHist] = temp_hist[indHist];
				}


			}

			delete [] temp_hist;

			return DetWin.featLen;
}

void LocalHON4D::compute_HON4D_hist(int dx, int dy,int dz,float* hist)
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

	for (int i=0;i<cell.bin_n_HON4D;i++) {
		s = cvGet2D(mulres,i,0);
		if (_isnan(s.val[0]) || (!_finite(s.val[0])))
		{
			cvSet2D(mulres,i,0,cvScalar(0));
		}
	}

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
			if (_isnan(s.val[0]) || (!_finite(s.val[0])))
			{
				s.val[0] = 0;
			}
			hist[i]  = hist[i]  + s.val[0];

		}
	}

	SafeMatDel(mulres);
	SafeMatDel(magMat);
	SafeMatDel(d);
}

int LocalHON4D::normalize_feature(float *feature_vector, float total_n)
{
	int feature_dim = cell.bin_n_HON4D;

	int i;

	if(total_n>0)
		for(i=0;i<feature_dim;i++)
			feature_vector[i]=feature_vector[i]/total_n;

	return 1;
}

void LocalHON4D::compute_3DJointPosition_one_joint_one_frame(int joint_ind, int frame_ind, vector<float>& pFeatOut)
{
	const MyCoords& centerCoord = _joints_realworld_coords[frame_ind][joint_ind];
	int ss = (int)pFeatOut.size();
	pFeatOut.resize(ss + 3*_joint_number);

	for (int i = 0; i<_joint_number; i++)
	{
		const MyCoords& c = _joints_realworld_coords[frame_ind][i];
		MyCoords subtraction;
		subtraction = centerCoord - c;
		pFeatOut[ss+3*i+0] = subtraction.x;
		pFeatOut[ss+3*i+1] = subtraction.y;
		pFeatOut[ss+3*i+2] = subtraction.z;
	}
	
}