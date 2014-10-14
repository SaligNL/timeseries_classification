
#include "read_from_kinect.h"

int Read_Kinect::control()
{
	int rtn = 0;
	control_initialize();

	while (1)
	{
		control_running();

		//exit
		if (_if_shutdown)
		{
			break;
		}
		int c = cvWaitKey(1);
		if (c == 27 || c == 'q' || c == 'Q')
		{
			_if_shutdown = true;
			break;
		}
	}

	control_finish();

	return rtn;
}

int Read_Kinect::control_initialize()
{
	int rtn = 0;


	//visualization_setup();

	HRESULT hr = NuiInitialize(
		NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX
		| NUI_INITIALIZE_FLAG_USES_COLOR
		| NUI_INITIALIZE_FLAG_USES_SKELETON);

	if (hr != S_OK)
	{
		cout << "NuiInitialize failed" << endl;
		return hr;
	}

	h1 = CreateEvent(NULL, TRUE, FALSE, NULL);
	h2 = NULL;
	hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480,
		0, 2, h1, &h2);
	if (FAILED(hr))
	{
		cout << "Could not open image stream video" << endl;
		return hr;
	}

	h3 = CreateEvent(NULL, TRUE, FALSE, NULL);
	h4 = NULL;	hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX,
		NUI_IMAGE_RESOLUTION_320x240, 0, 2, h3, &h4);

	if (FAILED(hr))
	{
		cout << "Could not open depth stream video" << endl;
		return hr;
	}

	h5 = CreateEvent(NULL, TRUE, FALSE, NULL);
	hr = NuiSkeletonTrackingEnable(h5, 0);
	if (FAILED(hr))
	{
		cout << "Could not open skeleton stream video" << endl;
		return hr;
	}

	return rtn;
}

int Read_Kinect::control_running()
{
	int rtn = 0;
	WaitForSingleObject(h1, INFINITE);
	//drawColor(h2);
	WaitForSingleObject(h3, INFINITE);
	drawDepth(h4);
	WaitForSingleObject(h5, INFINITE);
	//_depth.copyTo(_skeleton);
	//drawSkeleton(_skeleton);
	drawSkeleton();
	if (_remove_num > 0)
	{
		remove_cache();
	}

	fall_detection.falldetectiondistance(_avg_dis);
	_if_fall_down = fall_detection.get_fall_output_dis();

	//visualization_update();
	//imshow("Activity Recognition", _dispImg); 

	return rtn;
}

int Read_Kinect::control_finish()
{
	int rtn = 0;

	//destroyWindow("depth image");
	//cvDestroyWindow("color image");
	//cvDestroyWindow("skeleton image");

	//destroyWindow("Activity Recognition");

	NuiShutdown();

	return rtn;
}

int Read_Kinect::drawColor(HANDLE h)
{
	const NUI_IMAGE_FRAME * pImageFrame = NULL;
	HRESULT hr = NuiImageStreamGetNextFrame(h, 0, &pImageFrame);
	if (FAILED(hr)) 
	{
		cout << "Get Image Frame Failed" << endl;
		return -1;
	}
	INuiFrameTexture * pTexture = pImageFrame->pFrameTexture;
	NUI_LOCKED_RECT LockedRect;
	pTexture->LockRect(0, &LockedRect, NULL, 0);
	if (LockedRect.Pitch != 0)
	{
		BYTE * pBuffer = (BYTE*) LockedRect.pBits;

		_color = Mat(COLOR_HIGHT,COLOR_WIDTH,CV_8UC4,pBuffer);
		for (int i=0; i<COLOR_HIGHT; i++)
			for(int j=0; j<COLOR_WIDTH; j++)
		{
			_bottom_left_img.at<Vec3b>(i,j).val[0] = _color.at<Vec4b>(i,j).val[0];
			_bottom_left_img.at<Vec3b>(i,j).val[1] = _color.at<Vec4b>(i,j).val[1];
			_bottom_left_img.at<Vec3b>(i,j).val[2] = _color.at<Vec4b>(i,j).val[2];

		}
		
//		imshow("color image", _color);
	}
	NuiImageStreamReleaseFrame(h, pImageFrame);
	return 0;
}

int Read_Kinect::drawDepth(HANDLE h)
{
	const NUI_IMAGE_FRAME * pImageFrame = NULL;
	HRESULT hr = NuiImageStreamGetNextFrame(h, 0, &pImageFrame);
	if (FAILED(hr))
	{
		cout << "Get Image Frame Failed" << endl;
		return -1;
	}

	INuiFrameTexture * pTexture = pImageFrame->pFrameTexture;	
	NUI_LOCKED_RECT LockedRect;
	pTexture->LockRect(0, &LockedRect, NULL, 0);
	
	//Mat real_depth_mat(DEPTH_HIGHT, DEPTH_WIDTH, CV_8UC3);
	//int depth_max = 0;
	if (LockedRect.Pitch != 0)
	{
		USHORT * pBuff = (USHORT*) LockedRect.pBits;
		for (int i = 0; i < DEPTH_WIDTH * DEPTH_HIGHT; i++)
		{
			BYTE index = pBuff[i] & 0x07;
			USHORT realDepth = (pBuff[i] & 0xfff8) >> 3;
			int row = i / DEPTH_WIDTH;
			int col = i % DEPTH_WIDTH;
			_current_real_depth.data[row][col] = (int)realDepth;
			//real_depth_mat.at<Vec3b>(row, col) = Vec3b(realDepth * 255 / 3975, realDepth * 255 / 3975,realDepth * 255 / 3975);
		
			BYTE scale = 255 - static_cast<BYTE>(256 * realDepth / 0x0fff);
			buf[CHANNEL * i] = buf[CHANNEL * i + 1] = buf[CHANNEL * i + 2] = 0;

			switch (index)
			{
			case 0:
				buf[CHANNEL * i] = scale / 2;
				buf[CHANNEL * i + 1] = scale / 2;
				buf[CHANNEL * i + 2] = scale / 2;
				break;
			case 1:
				buf[CHANNEL * i+2] = scale;
				break;
			case 2:
				buf[CHANNEL * i + 1] = scale;
				break;
			case 3:
				buf[CHANNEL * i + 0] = scale;
				buf[CHANNEL * i + 1] = scale;
				buf[CHANNEL * i + 2] = scale / 4;
				break;
			case 4:
				buf[CHANNEL * i] = scale / 4;
				buf[CHANNEL * i + 1] = scale;
				buf[CHANNEL * i + 2] = scale;
				break;
			case 5:
				buf[CHANNEL * i] = scale;
				buf[CHANNEL * i + 1] = scale / 4;
				buf[CHANNEL * i + 2] = scale;
				break;
			case 6:
				buf[CHANNEL * i + 0] = scale;
				buf[CHANNEL * i + 1] = scale / 2;
				buf[CHANNEL * i + 2] = scale / 2;
				break;
			case 7:
				buf[CHANNEL * i] = 255 - scale / 2;
				buf[CHANNEL * i + 1] = 255 - scale / 2;
				buf[CHANNEL * i + 2] = 255 - scale / 2;
				break;
			default:
				buf[CHANNEL * i] = 0;
				buf[CHANNEL * i + 1] = 0;
				buf[CHANNEL * i + 2] = 0;
				break;
			}
		}

		_depth = Mat(DEPTH_HIGHT, DEPTH_WIDTH, CV_8UC3,buf);

		//cvSetData(depth, buf, DEPTH_WIDTH * CHANNEL);
		//		cout<<depth_max<<endl;
	}
	
	NuiImageStreamReleaseFrame(h, pImageFrame);
//	imshow("depth image", real_depth_mat);
	return 0;
}

int Read_Kinect::drawSkeleton()
{
	NUI_SKELETON_FRAME SkeletonFrame = {0};
	CvPoint pt[20];
	HRESULT hr = NuiSkeletonGetNextFrame(0, &SkeletonFrame);
	bool bFoundSkeleton = false;
	_if_skeleton = false;
	for (int i = 0; i < NUI_SKELETON_COUNT; i++)
	{
		if (SkeletonFrame.SkeletonData[i].eTrackingState
			== NUI_SKELETON_TRACKED)
		{
			_if_skeleton = true;
			bFoundSkeleton = true;
		}
	}
	// Has skeletons!
	//

	Vector4 floorplane_vector = SkeletonFrame.vFloorClipPlane; 
	float	FA = floorplane_vector.x; 
	float   FB = floorplane_vector.y; 
	float	FC = floorplane_vector.z; 
	float	FD = floorplane_vector.w; 
	float	F_norm = sqrt(FA*FA + FB*FB + FC*FC);
	//	cout<<"F_norm: "<<F_norm<<endl;
	if (bFoundSkeleton)
	{
		NuiTransformSmooth(&SkeletonFrame, NULL);

		//		memset(skeleton->imageData, 0, skeleton->imageSize);

		for (int i = 0; i < NUI_SKELETON_COUNT; i++)
		{
			//		cout<<NUI_SKELETON_COUNT<<endl;
			vector<double> distances(20); 
			double sum_dis = 0; 

			if (SkeletonFrame.SkeletonData[i].eTrackingState
				== NUI_SKELETON_TRACKED)
			{
				for (int j = 0; j < NUI_SKELETON_POSITION_COUNT; j++)
				{
					float fx, fy;
					NuiTransformSkeletonToDepthImage(
						SkeletonFrame.SkeletonData[i].SkeletonPositions[j],
						&fx, &fy, NUI_IMAGE_RESOLUTION_320x240);

					//pt[j].x = (int) (fx * SKELETON_WIDTH + 0.5f);
					//pt[j].y = (int) (fy * SKELETON_HIGHT + 0.5f);
					pt[j].x = (int) (fx + 0.5f);
					pt[j].y = (int) (fy + 0.5f);

					if ( _if_start_cache == 1)
					{
						int x_v = My_Max(1, pt[j].x);
						int y_v = My_Max(1, pt[j].y);
						x_v = My_Min(x_v, DEPTH_WIDTH);
						y_v = My_Min(y_v, DEPTH_HIGHT);
						_current_skeleton_pixel_coords[j] = make_pair(x_v, y_v);
						_current_skeleton_realworld_coords[j].x = SkeletonFrame.SkeletonData[i].SkeletonPositions[j].x;
						_current_skeleton_realworld_coords[j].y = SkeletonFrame.SkeletonData[i].SkeletonPositions[j].y;
						_current_skeleton_realworld_coords[j].z = SkeletonFrame.SkeletonData[i].SkeletonPositions[j].z;
						_if_start_cache ++;
						_pre_skeleton_pixel_coords = _current_skeleton_pixel_coords;
						_pre_skeleton_realworld_coords = _current_skeleton_realworld_coords;
					}
					else if(_if_start_cache == 2)
					{
						_pre_skeleton_pixel_coords = _current_skeleton_pixel_coords;
						_pre_skeleton_realworld_coords = _current_skeleton_realworld_coords;
						int x_v = My_Max(1, pt[j].x);
						int y_v = My_Max(1, pt[j].y);
						x_v = My_Min(x_v, DEPTH_WIDTH);
						y_v = My_Min(y_v, DEPTH_HIGHT);
						_current_skeleton_pixel_coords[j] = make_pair(x_v, y_v);
						_current_skeleton_realworld_coords[j].x = SkeletonFrame.SkeletonData[i].SkeletonPositions[j].x;
						_current_skeleton_realworld_coords[j].y = SkeletonFrame.SkeletonData[i].SkeletonPositions[j].y;
						_current_skeleton_realworld_coords[j].z = SkeletonFrame.SkeletonData[i].SkeletonPositions[j].z;
					}

					// calculate distance of joint positions to ground plane  
					Vector4 joint3D = SkeletonFrame.SkeletonData[i].SkeletonPositions[j]; 
					distances.at(j) = (FA*joint3D.x + FB*joint3D.y + FC*joint3D.z + FD)/F_norm; 
					sum_dis = sum_dis + distances.at(j); 
				}

				// average distance of joint positions to ground plane   
				_avg_dis = sum_dis/20; 
				//			cout<<"average dist: "<<_avg_dis<<endl;

			}
			//		cout<<fx_min<<" "<<fx_max<<"     "<<fy_min<<" "<<fy_max<<endl;
		}

		if(_if_start_cache == 2)
			sample();

	}

	synchronize_remove_test();
	_cache_size = (int)_real_depth_cache.size();
	//	cout<<"size of cache: "<<_skeleton_pixel_coords_cache.size()<<endl;

	//	imshow("skeleton image", skeleton);
	return 0;
}

int Read_Kinect::drawSkeleton(Mat & skeleton)
{
	NUI_SKELETON_FRAME SkeletonFrame = {0};
	CvPoint pt[20];
	HRESULT hr = NuiSkeletonGetNextFrame(0, &SkeletonFrame);
	bool bFoundSkeleton = false;
	_if_skeleton = false;
	for (int i = 0; i < NUI_SKELETON_COUNT; i++)
	{
		if (SkeletonFrame.SkeletonData[i].eTrackingState
			== NUI_SKELETON_TRACKED)
		{
			_if_skeleton = true;
			bFoundSkeleton = true;
		}
	}
	// Has skeletons!
	//

	Vector4 floorplane_vector = SkeletonFrame.vFloorClipPlane; 
	float	FA = floorplane_vector.x; 
	float   FB = floorplane_vector.y; 
	float	FC = floorplane_vector.z; 
	float	FD = floorplane_vector.w; 
	float	F_norm = sqrt(FA*FA + FB*FB + FC*FC);
//	cout<<"F_norm: "<<F_norm<<endl;
	if (bFoundSkeleton)
	{
		NuiTransformSmooth(&SkeletonFrame, NULL);

		//		memset(skeleton->imageData, 0, skeleton->imageSize);

		for (int i = 0; i < NUI_SKELETON_COUNT; i++)
		{
	//		cout<<NUI_SKELETON_COUNT<<endl;
			vector<double> distances(20); 
			double sum_dis = 0; 

			if (SkeletonFrame.SkeletonData[i].eTrackingState
				== NUI_SKELETON_TRACKED)
			{
				for (int j = 0; j < NUI_SKELETON_POSITION_COUNT; j++)
				{
					float fx, fy;
					NuiTransformSkeletonToDepthImage(
						SkeletonFrame.SkeletonData[i].SkeletonPositions[j],
						&fx, &fy, NUI_IMAGE_RESOLUTION_320x240);

					//pt[j].x = (int) (fx * SKELETON_WIDTH + 0.5f);
					//pt[j].y = (int) (fy * SKELETON_HIGHT + 0.5f);
					pt[j].x = (int) (fx + 0.5f);
					pt[j].y = (int) (fy + 0.5f);

					if ( _if_start_cache == 1)
					{
						int x_v = My_Max(1, pt[j].x);
						int y_v = My_Max(1, pt[j].y);
						x_v = My_Min(x_v, DEPTH_WIDTH);
						y_v = My_Min(y_v, DEPTH_HIGHT);
						_current_skeleton_pixel_coords[j] = make_pair(x_v, y_v);
						_current_skeleton_realworld_coords[j].x = SkeletonFrame.SkeletonData[i].SkeletonPositions[j].x;
						_current_skeleton_realworld_coords[j].y = SkeletonFrame.SkeletonData[i].SkeletonPositions[j].y;
						_current_skeleton_realworld_coords[j].z = SkeletonFrame.SkeletonData[i].SkeletonPositions[j].z;
						_if_start_cache ++;
						_pre_skeleton_pixel_coords = _current_skeleton_pixel_coords;
						_pre_skeleton_realworld_coords = _current_skeleton_realworld_coords;
					}
					else if(_if_start_cache == 2)
					{
						_pre_skeleton_pixel_coords = _current_skeleton_pixel_coords;
						_pre_skeleton_realworld_coords = _current_skeleton_realworld_coords;
						int x_v = My_Max(1, pt[j].x);
						int y_v = My_Max(1, pt[j].y);
						x_v = My_Min(x_v, DEPTH_WIDTH);
						y_v = My_Min(y_v, DEPTH_HIGHT);
						_current_skeleton_pixel_coords[j] = make_pair(x_v, y_v);
						_current_skeleton_realworld_coords[j].x = SkeletonFrame.SkeletonData[i].SkeletonPositions[j].x;
						_current_skeleton_realworld_coords[j].y = SkeletonFrame.SkeletonData[i].SkeletonPositions[j].y;
						_current_skeleton_realworld_coords[j].z = SkeletonFrame.SkeletonData[i].SkeletonPositions[j].z;
					}

					circle(skeleton, pt[j], 5, CV_RGB(255, 0, 0), -1);
					// calculate distance of joint positions to ground plane  
					Vector4 joint3D = SkeletonFrame.SkeletonData[i].SkeletonPositions[j]; 
					distances.at(j) = (FA*joint3D.x + FB*joint3D.y + FC*joint3D.z + FD)/F_norm; 
					sum_dis = sum_dis + distances.at(j); 
				}

				// average distance of joint positions to ground plane   
				_avg_dis = sum_dis/20; 
	//			cout<<"average dist: "<<_avg_dis<<endl;

				line(skeleton, pt[NUI_SKELETON_POSITION_HEAD],
					pt[NUI_SKELETON_POSITION_SHOULDER_CENTER],
					CV_RGB(0, 255, 0));

				line(skeleton, pt[NUI_SKELETON_POSITION_SHOULDER_CENTER],
					pt[NUI_SKELETON_POSITION_SPINE], CV_RGB(0, 255, 0));

				line(skeleton, pt[NUI_SKELETON_POSITION_SPINE],
					pt[NUI_SKELETON_POSITION_HIP_CENTER],
					CV_RGB(0, 255, 0));

				line(skeleton, pt[NUI_SKELETON_POSITION_HAND_RIGHT],
					pt[NUI_SKELETON_POSITION_WRIST_RIGHT],
					CV_RGB(0, 255, 0));

				line(skeleton, pt[NUI_SKELETON_POSITION_WRIST_RIGHT],
					pt[NUI_SKELETON_POSITION_ELBOW_RIGHT],
					CV_RGB(0, 255, 0));

				line(skeleton, pt[NUI_SKELETON_POSITION_ELBOW_RIGHT],
					pt[NUI_SKELETON_POSITION_SHOULDER_RIGHT],
					CV_RGB(0, 255, 0));

				line(skeleton, pt[NUI_SKELETON_POSITION_SHOULDER_RIGHT],
					pt[NUI_SKELETON_POSITION_SHOULDER_CENTER],
					CV_RGB(0, 255, 0));

				line(skeleton, pt[NUI_SKELETON_POSITION_SHOULDER_CENTER],
					pt[NUI_SKELETON_POSITION_SHOULDER_LEFT],
					CV_RGB(0, 255, 0));

				line(skeleton, pt[NUI_SKELETON_POSITION_SHOULDER_LEFT],
					pt[NUI_SKELETON_POSITION_ELBOW_LEFT],
					CV_RGB(0, 255, 0));

				line(skeleton, pt[NUI_SKELETON_POSITION_ELBOW_LEFT],
					pt[NUI_SKELETON_POSITION_WRIST_LEFT],
					CV_RGB(0, 255, 0));

				line(skeleton, pt[NUI_SKELETON_POSITION_WRIST_LEFT],
					pt[NUI_SKELETON_POSITION_HAND_LEFT], CV_RGB(0, 255, 0));

				line(skeleton, pt[NUI_SKELETON_POSITION_HIP_CENTER],
					pt[NUI_SKELETON_POSITION_HIP_RIGHT], CV_RGB(0, 255, 0));

				line(skeleton, pt[NUI_SKELETON_POSITION_HIP_RIGHT],
					pt[NUI_SKELETON_POSITION_KNEE_RIGHT],
					CV_RGB(0, 255, 0));

				line(skeleton, pt[NUI_SKELETON_POSITION_KNEE_RIGHT],
					pt[NUI_SKELETON_POSITION_ANKLE_RIGHT],
					CV_RGB(0, 255, 0));

				line(skeleton, pt[NUI_SKELETON_POSITION_ANKLE_RIGHT],
					pt[NUI_SKELETON_POSITION_FOOT_RIGHT],
					CV_RGB(0, 255, 0));

				line(skeleton, pt[NUI_SKELETON_POSITION_HIP_CENTER],
					pt[NUI_SKELETON_POSITION_HIP_LEFT], CV_RGB(0, 255, 0));

				line(skeleton, pt[NUI_SKELETON_POSITION_HIP_LEFT],
					pt[NUI_SKELETON_POSITION_KNEE_LEFT], CV_RGB(0, 255, 0));

				line(skeleton, pt[NUI_SKELETON_POSITION_KNEE_LEFT],
					pt[NUI_SKELETON_POSITION_ANKLE_LEFT],
					CV_RGB(0, 255, 0));

				line(skeleton, pt[NUI_SKELETON_POSITION_ANKLE_LEFT],
					pt[NUI_SKELETON_POSITION_FOOT_LEFT], CV_RGB(0, 255, 0));
			}
			//		cout<<fx_min<<" "<<fx_max<<"     "<<fy_min<<" "<<fy_max<<endl;
		}

		if(_if_start_cache == 2)
			sample();
		
	}

	synchronize_remove_test();
	_cache_size = (int)_real_depth_cache.size();
//	cout<<"size of cache: "<<_skeleton_pixel_coords_cache.size()<<endl;

//	imshow("skeleton image", skeleton);
	return 0;
}

void Read_Kinect::sample()
{
	_total_frames ++;
	if (_sample_index % SAMPLE_RATE == 0)
	{
		_skeleton_pixel_coords_cache.push_back(_current_skeleton_pixel_coords);
		_skeleton_realworld_coords_cache.push_back(_current_skeleton_realworld_coords);
		_real_depth_cache.push_back(_current_real_depth);
		_cache_size = (int)_real_depth_cache.size();

		//Mat temp;
		//_bottom_left_img.copyTo(temp);
		//_color_cache.push_back(temp);
		if (_cache_size % 10 == 0)
		{
			cout<<"_cache_size: "<<_cache_size<<endl;

		}
	}
	_sample_index = (_sample_index+1) % SAMPLE_RATE;

	//record_video();
}

void Read_Kinect::visualization_setup()
{
	namedWindow("Activity Recognition");
	_dispImg.create(Size(COLOR_WIDTH*2, COLOR_HIGHT+FALL_TEXT_HIGHT), CV_8UC3);

	_bottom_left_img = _dispImg(Rect(0, FALL_TEXT_HIGHT, COLOR_WIDTH, COLOR_HIGHT));	
	_skeleton = _dispImg(Rect(COLOR_WIDTH-SKELETON_WIDTH, FALL_TEXT_HIGHT, SKELETON_WIDTH, SKELETON_HIGHT));
	_top_right_img = _dispImg(Rect(FALL_TEXT_WIDTH, 0, EAT_TEXT_WIDTH, EAT_TEXT_HIGHT));
	_bottom_right_img = _dispImg(Rect(FALL_TEXT_WIDTH, FALL_TEXT_HIGHT, COLOR_WIDTH, COLOR_HIGHT));
	
	string output_video = "..\\data\\Activity_Recognition.avi";
	_vwr.open(output_video.c_str(), CV_FOURCC('M','J','P','G'), 30, Size(_dispImg.size().width, _dispImg.size().height) );

	//namedWindow("color image", CV_WINDOW_AUTOSIZE);
//	cvNamedWindow("depth image", CV_WINDOW_AUTOSIZE);
	//cvNamedWindow("skeleton image", CV_WINDOW_AUTOSIZE);
}

void Read_Kinect::visualization_update()
{	
	_top_left_img = _dispImg(Rect(0, 0, FALL_TEXT_WIDTH, FALL_TEXT_HIGHT));
	_top_left_img.setTo(Scalar(100,139,33));
	putText (_top_left_img,"Fall Down Detection",Point(100,50), CV_FONT_HERSHEY_DUPLEX|CV_FONT_ITALIC, 1, Scalar(109, 55, 5),2);
	if(_if_fall_down)
		putText (_top_left_img,"Fall down!",Point(50,150), CV_FONT_HERSHEY_DUPLEX|CV_FONT_ITALIC, 2, Scalar(0, 10, 128),2);
	else
		putText (_top_left_img,"Normal",Point(50,150), CV_FONT_HERSHEY_DUPLEX|CV_FONT_ITALIC, 2, Scalar(0, 10, 128),2);

	_top_right_img.setTo(Scalar(128,128,0));
	putText (_top_right_img,"Eating Detection",Point(100,50), CV_FONT_HERSHEY_DUPLEX|CV_FONT_ITALIC, 1, Scalar(70, 30, 0),2);
	if(_if_eat)
	{
		if (_time_index == 0)
		{
			_start_time_display = clock();
			_time_index ++;
		}
		else
		{
			_end_time_display = clock();
			if (_end_time_display - _start_time_display > 5000)
			{
				_if_eat = false;
				_time_index = 0;
			}
		}
		putText (_top_right_img,"Eating!!!",Point(50,150), CV_FONT_HERSHEY_DUPLEX|CV_FONT_ITALIC, 2.5, Scalar(0, 10, 128),2);
		

	}
	else
		putText (_top_right_img,"No eating",Point(50,150), CV_FONT_HERSHEY_DUPLEX|CV_FONT_ITALIC, 1.5, Scalar(0, 10, 128),2);
	
	putText (_top_right_img, std::to_string(_current_decision_value),Point(250,240), 
			CV_FONT_HERSHEY_DUPLEX|CV_FONT_ITALIC, 2.0, Scalar(0, 10, 128),2);


//	resize(_real_depth_cache[_current_process_frame], _bottom_right_img, Size(COLOR_WIDTH, COLOR_HIGHT));
	if(_cache_size > _current_process_frame && _if_bottom_right_change)
	{
		_color_cache[_current_process_frame].copyTo(_bottom_right_img);

		_if_bottom_right_change = false;
		//cout<<"current_process frame: "<<_current_process_frame<<endl;
		//cout<<"color cache size: "<<_color_cache.size()<<endl;
	}
}	

void Read_Kinect::remove_cache()
{
	_skeleton_pixel_coords_cache.erase(_skeleton_pixel_coords_cache.begin(), _skeleton_pixel_coords_cache.begin()+_remove_num);
	_skeleton_realworld_coords_cache.erase(_skeleton_realworld_coords_cache.begin(), _skeleton_realworld_coords_cache.begin()+_remove_num);
	_real_depth_cache.erase(_real_depth_cache.begin(), _real_depth_cache.begin()+_remove_num);
	//_color_cache.erase(_color_cache.begin(), _color_cache.begin() + _remove_num);
	_cache_size -= _remove_num;
	cout<<"remove cache success! current cache size: "<<_cache_size<<endl;
	_remove_num = 0;
}

bool Read_Kinect::synchronize_remove_test()
{
	bool rtn = true;

	if (_skeleton_pixel_coords_cache.size() != _real_depth_cache.size())
	{
		cout<<"error: _skeleton_pixel_coords_cache.size() != _real_depth_cache.size():   "<<
			_skeleton_pixel_coords_cache.size()<<"  "<< _real_depth_cache.size()<<endl;
		rtn = false;
		Sleep(1000);
		//if(abs((int)_skeleton_pixel_coords_cache.size()-(int)_real_depth_cache.size()) % 60 != 0)
		//	exit(1);
	}
	return rtn;
}

void Read_Kinect::record_video()
{

	if (_total_frames < _record_video_length)
	{
		_vwr << _dispImg;
	}
	else if (_total_frames == _record_video_length)
	{
		_vwr.release();
	}
}