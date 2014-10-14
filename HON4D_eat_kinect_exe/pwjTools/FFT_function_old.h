//
// 快速傅立叶变换 Fast Fourier Transform
// By rappizit@yahoo.com.cn
// 2007-07-20
// 版本 2.0
// 改进了《算法导论》的算法，旋转因子取 ωn-kj  (ωnkj 的共轭复数)
// 且只计算 n / 2 次，而未改进前需要计算 (n * lg n) / 2 次。
// 
////////////////////////////////////////////////

// no use anymore

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <time.h>


using namespace std;

#define  N 1024
#define  PI  3.1415927

inline bool isPower2( int n )
{
	int bits = 0;
	while( n )
	{
		bits += n & 1;
		n >>= 1;
	}

	return ( bits == 1 );
}

/// return the next smallest power 2 which is bigger than x 
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

inline void swap (double &a, double &b)
{
	double t;
	t = a;
	a = b;
	b = t;
}

void bitrp (vector<double>& xreal, vector<double>& ximag, int n);

void FFT(vector<double>& xreal, vector<double>& ximag, int n);

/// only real, return the amplitude
void FFT_amplitude(vector<double>& xreal, vector<double>& rtn_amplitude);
/// get top n amplitude(from lower frequency)
void FFT_amplitude(vector<double>& xreal, vector<double>& rtn_amplitude, int top_n);


void  IFFT (vector<double>& xreal, vector<double>& ximag, int n);

void get_amplitude(const vector<double>& xreal, const vector<double>& ximag, vector<double>& rtn);
void get_amplitude(const vector<double>& xreal, const vector<double>& ximag, vector<double>& rtn, int top_n);


void FFT_test ();