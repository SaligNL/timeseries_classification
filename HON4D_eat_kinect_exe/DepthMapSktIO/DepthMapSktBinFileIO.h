#ifndef _DEPTH_MAP_SKT_BIN_FILE_IO_H
#define _DEPTH_MAP_SKT_BIN_FILE_IO_H

#include "DepthMapSkt.h"

#include<highgui.h>
#include <vector>
#include <opencv2/core/core.hpp>

using namespace std;

int ReadDepthMapSktBinFileHeader(FILE * fp, int &retNumFrames, int &retNCols, int &retNRows);

//the caller needs to allocate space for <retDepthMap>
int ReadDepthMapSktBinFileNextFrame(FILE * fp, int numCols, int numRows, CDepthMapSkt & retDepthMap);

//<fp> must be opened with "wb"
int WriteDepthMapSktBinFileHeader(FILE * fp, int nFrames, int nCols, int nRows);

//<fp> must be opened with "wb"
int WriteDepthMapSktBinFileNextFrame(FILE * fp, const CDepthMapSkt & depthMap);


/// add by Wenjie
int SaveImagetoPngFile(int flag, const std::vector<std::pair<int, int> >& skeleton, const CDepthMapSkt& depthMap, int max_v, int frameindexm, const char* filename);
int SaveImagetoTxtFile(int flag, const std::vector<pair<int, int> >& skeleton, const CDepthMapSkt& depthMap, int max_v, int frameindex, const char* filename);


#endif