
#include "FFT_function_old.h"
#include <fftw3.h>

using namespace std;



void bitrp (vector<double> & xreal, vector<double>& ximag, int n)
{
	// 位反转置换 Bit-reversal Permutation
	int i, j, a, b, p;

	for (i = 1, p = 0; i < n; i *= 2)
	{
		p ++;
	}
	for (i = 0; i < n; i ++)
	{
		a = i;
		b = 0;
		for (j = 0; j < p; j ++)
		{
			b = (b << 1) + (a & 1);    // b = b * 2 + a % 2;
			a >>= 1;        // a = a / 2;
		}
		if ( b > i)
		{
			swap (xreal [i], xreal [b]);
			swap (ximag [i], ximag [b]);
		}
	}
}

void FFT(vector<double>& xreal, vector<double>& ximag, int n)
{
	// 快速傅立叶变换，将复数 x 变换后仍保存在 x 中，xreal, ximag 分别是 x 的实部和虚部
	double wreal [N / 2], wimag [N / 2], treal, timag, ureal, uimag, arg;
	int m, k, j, t, index1, index2;

	bitrp (xreal, ximag, n);

	// 计算 1 的前 n / 2 个 n 次方根的共轭复数 W'j = wreal [j] + i * wimag [j] , j = 0, 1, ... , n / 2 - 1
	arg = - 2 * PI / n;
	treal = cos (arg);
	timag = sin (arg);
	wreal [0] = 1.0;
	wimag [0] = 0.0;
	for (j = 1; j < n / 2; j ++)
	{
		wreal [j] = wreal [j - 1] * treal - wimag [j - 1] * timag;
		wimag [j] = wreal [j - 1] * timag + wimag [j - 1] * treal;
	}

	for (m = 2; m <= n; m *= 2)
	{
		for (k = 0; k < n; k += m)
		{
			for (j = 0; j < m / 2; j ++)
			{
				index1 = k + j;
				index2 = index1 + m / 2;
				t = n * j / m;    // 旋转因子 w 的实部在 wreal [] 中的下标为 t
				treal = wreal [t] * xreal [index2] - wimag [t] * ximag [index2];
				timag = wreal [t] * ximag [index2] + wimag [t] * xreal [index2];
				ureal = xreal [index1];
				uimag = ximag [index1];
				xreal [index1] = ureal + treal;
				ximag [index1] = uimag + timag;
				xreal [index2] = ureal - treal;
				ximag [index2] = uimag - timag;
			}
		}
	}
}


void get_amplitude(const vector<double>& xreal, const vector<double>& ximag, vector<double>& rtn)
{
	if (xreal.size() != ximag.size())
	{
		cout<<"error: the size of real != size of imag!"<<endl;
		exit(1);
	}
	rtn.resize(xreal.size());
	for (int i=0; i<xreal.size(); i++)
	{
		rtn[i] = xreal[i]*xreal[i] + ximag[i]*ximag[i];
		rtn[i] /= (double)rtn.size();
	}
}

void get_amplitude(const vector<double>& xreal, const vector<double>& ximag, vector<double>& rtn, int top_n)
{
	if (xreal.size() != ximag.size())
	{
		cout<<"error: the size of real != size of imag!"<<endl;
		exit(1);
	}
	if (top_n > (int)xreal.size())
	{
		rtn.resize(xreal.size());
	}
	else
		rtn.resize(top_n);

	for (int i=0; i<(int)rtn.size(); i++)
	{
		rtn[i] = xreal[i]*xreal[i] + ximag[i]*ximag[i];
		rtn[i] /= (double)rtn.size();
	}
}

void FFT_amplitude(vector<double>& xreal, vector<double>& rtn)
{
	int size = (int)xreal.size(); 
	if (!isPower2(size))
	{
		size = nextPow2(size);
	}
	xreal.resize(size, 0);
	vector<double> ximag;
	ximag.resize(size, 0);
	FFT(xreal, ximag, size);
	get_amplitude(xreal, ximag, rtn);
}

void FFT_amplitude(vector<double>& xreal, vector<double>& rtn, int top_n)
{
	int size = (int)xreal.size(); 
	if (!isPower2(size))
	{
		size = nextPow2(size);
	}
	xreal.resize(size, 0);
	vector<double> ximag;
	ximag.resize(size, 0);
	FFT(xreal, ximag, size);
	get_amplitude(xreal, ximag, rtn, top_n);
}

void  IFFT (vector<double>& xreal, vector<double>& ximag, int n)
{
	// 快速傅立叶逆变换
	double wreal [N / 2], wimag [N / 2], treal, timag, ureal, uimag, arg;
	int m, k, j, t, index1, index2;

	bitrp (xreal, ximag, n);

	// 计算 1 的前 n / 2 个 n 次方根 Wj = wreal [j] + i * wimag [j] , j = 0, 1, ... , n / 2 - 1
	arg = 2 * PI / n;
	treal = cos (arg);
	timag = sin (arg);
	wreal [0] = 1.0;
	wimag [0] = 0.0;
	for (j = 1; j < n / 2; j ++)
	{
		wreal [j] = wreal [j - 1] * treal - wimag [j - 1] * timag;
		wimag [j] = wreal [j - 1] * timag + wimag [j - 1] * treal;
	}

	for (m = 2; m <= n; m *= 2)
	{
		for (k = 0; k < n; k += m)
		{
			for (j = 0; j < m / 2; j ++)
			{
				index1 = k + j;
				index2 = index1 + m / 2;
				t = n * j / m;    // 旋转因子 w 的实部在 wreal [] 中的下标为 t
				treal = wreal [t] * xreal [index2] - wimag [t] * ximag [index2];
				timag = wreal [t] * ximag [index2] + wimag [t] * xreal [index2];
				ureal = xreal [index1];
				uimag = ximag [index1];
				xreal [index1] = ureal + treal;
				ximag [index1] = uimag + timag;
				xreal [index2] = ureal - treal;
				ximag [index2] = uimag - timag;
			}
		}
	}

	for (j=0; j < n; j ++)
	{
		xreal [j] /= n;
		ximag [j] /= n;
	}
}



void FFT_test()
{
	string inputfile = "..\\pwjTools\\fft_test_data.txt";
	vector<double> xreal;
	vector<double> ximag;

	ifstream openfile;
	openfile.open(inputfile.c_str());
	if (!openfile.is_open())
	{
		cout<<"cannot open file!"<<endl;
		exit(1);
	}

	int n=0;
	double realp, imagp;
	while (openfile >> realp >> imagp)
	{
		xreal.push_back(realp);
		ximag.push_back(imagp);
		n++;
	}
	cout<<n<<endl;
	int m = n;

	if (!isPower2(m))
	{
		cout<<m<<" is not a power of 2!"<<endl;
		exit (1);
	}
	//while (m > 1) // 要求 n 为 2 的整数幂
	//{
	//	if (m % 2)
	//	{
	//		cout<<m<<" is not a power of 2!"<<endl;
	//		exit (1);
	//	}
	//	m /= 2;
	//}

	FFT (xreal, ximag, n);
	cout<<"FFT:    i	    real	imag "<<endl;
	for (int i = 0; i < n; i ++)
	{
		cout<<i<<": "<<xreal[i]<<" "<<ximag [i]<<endl;
	}

	IFFT (xreal, ximag, n);
	cout<<"FFT:    i	    real	imag "<<endl;
	for (int i = 0; i < n; i ++)
	{
		cout<<i<<": "<<xreal[i]<<" "<<ximag [i]<<endl;
	}
}

void fft_Test2()
{
	cout << "The original signal is: " << endl;
	int index;
	vector<double> xreal(32);
	vector<double> ximag(32);

	for( int i=0; i<8; i++ )
	{
		cout << endl;
		for( int j=0; j<3; j++ )
		{
			index = 3*i+j;
			xreal[index] = i+j;
			cout << setiosflags(ios::fixed) << setprecision(6);
			cout << "\t" << xreal[index];
		}
	}
	cout<<endl;

	FFT(xreal, ximag, 32);
	cout<<"FFT: i real	imag "<<endl;
	for (int i = 0; i < 24; i ++)
	{
		cout << setiosflags(ios::fixed) << setprecision(6);
		cout<<i<<": \t"<<xreal[i]<<" \t "<<ximag [i]<<endl;
	}
}

/// work load test
void fft_Test3 ()
{
	int size = 256;
	vector<double> xreal(size);
	vector<double> ximag(size);

	for (int i=0; i<size; i++)
	{
		xreal[i] = i;
	}
	vector<vector<double> > xr(100000);
	vector<vector<double> > xi(100000);

	for (int i=0; i<100000; i++)
	{
		xr[i] = xreal;
		xi[i] = ximag;
	}

	clock_t start, end;
	start = clock();

	for(int i = 0; i < 20000; i++)
	{
		FFT(xr[i], xi[i], size);

		if ((i+1)%20000 == 0)
		{
			end = clock();
			cout<<"elapsed time for "<<i<<": "<<(end-start)/1000.0<<endl;
		}
		
	}

	//for (int i=0; i<256; i++)
	//{
	//	cout << setiosflags(ios::fixed) << setprecision(6);
	//	cout<<i<<": \t"<<xr[1][i]<<" \t "<<xi[1][i]<<endl;

	//}

	vector<double> rtn;
	get_amplitude(xr[1], xi[1], rtn);

	for (int i=0; i<256; i++)
	{
		cout << setiosflags(ios::fixed) << setprecision(6);
		cout<<i<<": \t"<<rtn[i]<<endl;

	}
}

void test4()
{
	int size = 200;
	vector<double> xreal(size);
	vector<double> rtn;

	for (int i=0; i<size; i++)
	{
		xreal[i] = i;
	}

	FFT_amplitude(xreal, rtn, 10);
	cout<<rtn.size()<<endl;

	for (int i=0; i<10; i++)
	{
		cout << setiosflags(ios::fixed) << setprecision(6);
		cout<<i<<": \t"<<rtn[i]<<endl;

	}
}


/// test the fftw
void test5()
{
    int bnum = 200;
    fftw_complex *in, *out;
    fftw_plan p;

    int num = nextPow2(bnum);
    num = bnum;
 //   cout<<num<<endl;

    in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * num);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * num);
    p = fftw_plan_dft_1d(num, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    for (int i=0; i<20000; i++)
    {
        for (int idx = 0; idx < bnum; idx++)
        {
            in[idx][0] = idx;
            in[idx][1] = 0;
        }
        for (int idx = bnum; idx < num; idx++)
        {
            in[idx][0] = 0;
            in[idx][1] = 0;
        }

        fftw_execute(p); /* repeat as needed */

        vector<double> fft_feature(num);
        /// calculate the magnitude
        for (int j=0; j<num; j++)
        {
            fft_feature[j] = (out[j][0]*out[j][0] + out[j][1]*out[j][1])/ (double)num;
        }
        //for (int i=0; i<num; i++)
        //{
        //    cout << setiosflags(ios::fixed) << setprecision(6);
        //    cout<<out[i][0]<<" "<<out[i][1]<<" "<<fft_feature[i]<<endl;
        //}
    }
    

    fftw_destroy_plan(p);
    fftw_free(in);
    fftw_free(out);
}

//int main ()
//{
//    //fft_test ();
//    //fft_test3();
//    clock_t start, end;
//    start = clock();
//        test5();
//            end = clock();
//            cout<<"elapsed time: "<<(end-start)/1000.0<<endl;
//
//    return 0;
//}