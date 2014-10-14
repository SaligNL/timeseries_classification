#include "StdAfx.h"
#include <stdlib.h>
#include <stdio.h>
#include "DepthMapSktBinFileIO.h"
#include <string>
#include <iostream>
#include <direct.h>
#include <io.h>
#include <fstream>

using namespace std;
using namespace cv;


int ReadDepthMapSktBinFileHeader(FILE * fp, int &retNumFrames, int &retNCols, int &retNRows)
{
	if(fp == NULL)
		return 0;


	fread(&retNumFrames, 4, 1, fp); //read 4 bytes 
	fread(&retNCols, 4, 1, fp);
	fread(&retNRows, 4, 1, fp);
	//fscanf(fp, "%i", &retNumFrames);
	//fscanf(fp, "%i", &retWidth);
	//fscanf(fp, "%i", &retHeight);

	return 1;
}

//the caller needs to allocate space for <retDepthMap>
int ReadDepthMapSktBinFileNextFrame(FILE * fp, int numCols, int numRows, CDepthMapSkt & retDepthMap)
{
	int max_v=0;
	int r,c;
	//for(h=0; h<height; h++) //for each row
	int * tempRow = new int[numCols];
	uint8_t* tempRowID = new uint8_t[numCols];
	for(r=0;r<numRows;r++) //one row at a time
	{
		fread(tempRow, 4, numCols, fp);
		fread(tempRowID, 1,numCols,fp);
		for(c=0; c<numCols; c++) //for each colume	
		{
			//int temp=0;
			//fread(&temp, 4, 1, fp);
			//retDepthMap.SetItem(r,c,temp);
			int temp = tempRow[c];

			if (temp > max_v)
			{
				max_v = temp;
			}
			retDepthMap.SetItem(r,c,(float) temp);
			retDepthMap.SetSkeletonID(r,c,tempRowID[c]);
		}
	}
	delete[] tempRow;
	tempRow = NULL;
	delete[] tempRowID;
	tempRowID = NULL;
	return max_v;	
}

//<fp> must be opened with "wb"
int WriteDepthMapSktBinFileHeader(FILE * fp, int nFrames, int nCols, int nRows)
{
	if(fp == NULL)
		return 0;


	fwrite(&nFrames, 4, 1, fp); //read 4 bytes 
	fwrite(&nCols, 4, 1, fp);
	fwrite(&nRows, 4, 1, fp);
	//fscanf(fp, "%i", &retNumFrames);
	//fscanf(fp, "%i", &retWidth);
	//fscanf(fp, "%i", &retHeight);

	return 1;
}

//<fp> must be opened with "wb"
int WriteDepthMapSktBinFileNextFrame(FILE * fp, const CDepthMapSkt & depthMap)
{
	int numCols = depthMap.GetNCols();
	int numRows = depthMap.GetNRows();

	int r,c;
	//for(h=0; h<height; h++) //for each row
	int * tempRow = new int[numCols];
	uint8_t* tempRowID = new uint8_t[numCols];
	for(r=0;r<numRows;r++) //one row at a time
	{
		for(c=0; c<numCols; c++) //for each colume
		{
			int temp = (int) (depthMap.GetItem(r,c));
			tempRow[c] = temp;
			tempRowID[c] = depthMap.GetSkeletonID(r,c);
		}
		fwrite(tempRow, 4, numCols, fp);
		fwrite(tempRowID,1,numCols,fp);
	}
	delete[] tempRow;
	tempRow = NULL;
	delete[] tempRowID;
	tempRowID = NULL;
	return 1;	
}

/// added by Wenjie
int SaveImagetoTxtFile(int flag, const vector<pair<int, int> >& skeleton, const CDepthMapSkt& depthMap, int max_v, int frameindex, const char* filename)
{
	int rtn = 0;
	int aimed_width;
	int x_from, x_to;

	aimed_width = 320;
	x_from = 0;
	x_to = 319;

	int nCols = depthMap.GetNCols();
	int nRows = depthMap.GetNRows();
	//	cout<<"ncols: "<<nCols<<"nrow: "<<nRows<<endl;
	string ss = "";
	if (frameindex < 10)
	{
		ss = "000"+std::to_string(_Longlong(frameindex));
	}
	else if (frameindex < 100)
	{
		ss = "00"+std::to_string(_Longlong(frameindex));
	}
	else if (frameindex < 1000)
	{
		ss = "0"+std::to_string(_Longlong(frameindex));
	}
	else if (frameindex < 10000)
	{
		ss = std::to_string(_Longlong(frameindex));
	}
	else
	{
		cout<<"error: frameindex > 100000"<<endl;
		exit(1);
	}

	/// parse filename
	string name(filename);
	for (int i=name.length()-1; i>-1; i--)
	{
		if (name[i] == '\\')
		{
			int l= name.length()-1-i-4;
			name = name.substr(i+1, l );
			break;
		}
	}
	string path = "D:\\SALIG_project\\MSR_activity_dataset\\depth_txt\\"+name;
	if(access(path.c_str(),6)==-1)
		mkdir(path.c_str());

	string outputfilename = path+"\\"+ss+".txt";

	fstream outfile1;
	outfile1.open(outputfilename.c_str(), ios::out);
	if (outfile1.fail())
	{
		cout<<"error: cannot open "<<outputfilename<<endl;
		exit(1);
	}

	for (int i=0; i<nRows; i++)
		for(int j=0; j<aimed_width; j++)
		{
			float val = depthMap.GetItem(i,j+x_from);
			outfile1 << val <<" ";            
		}

		outfile1.close();

		return rtn; 

}


int SaveImagetoPngFile(const vector<pair<int, int> >& skeleton, const CDepthMapSkt& depthMap, int max_v, int frameindex, const char* filename)
{
	int rtn = 0;
	int aimed_width;
	int x_from, x_to;

	aimed_width = 320;
	x_from = 0;
	x_to = 319;

	int nCols = depthMap.GetNCols();
	int nRows = depthMap.GetNRows();
	//	cout<<"ncols: "<<nCols<<"nrow: "<<nRows<<endl;

	IplImage* imgDepth = cvCreateImage(cvSize(aimed_width,nRows),IPL_DEPTH_8U,1);
	for (int i=0; i<nRows; i++)
		for(int j=0; j<aimed_width; j++)
		{
			float val = depthMap.GetItem(i,j+x_from);
			val = val / max_v;
			imgDepth->imageData[i*aimed_width+j] = int(val*255+0.5);

		}

		string ss = "";
		if (frameindex < 10)
		{
			ss = "000"+std::to_string(_Longlong(frameindex));
		}
		else if (frameindex < 100)
		{
			ss = "00"+std::to_string(_Longlong(frameindex));
		}
		else if (frameindex < 1000)
		{
			ss = "0"+std::to_string(_Longlong(frameindex));
		}
		else if (frameindex < 10000)
		{
			ss = std::to_string(_Longlong(frameindex));
		}
		else
		{
			cout<<"error: frameindex > 100000"<<endl;
			exit(1);
		}

		/// parse filename
		string name(filename);
		for (int i=name.length()-1; i>-1; i--)
		{
			if (name[i] == '\\')
			{
				int l= name.length()-1-i-4;
				name = name.substr(i+1, l );
				break;
			}
		}
		string path = "D:\\SALIG_project\\MSR_activity_dataset\\depth_png\\"+name;
		if(access(path.c_str(),6)==-1)
			mkdir(path.c_str());

		string outputfilename = path+"\\"+ss+".png";


		cvSaveImage(outputfilename.c_str(), imgDepth);
		cvReleaseImage(&imgDepth);

		return rtn; 
}

