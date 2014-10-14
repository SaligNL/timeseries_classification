
#ifndef myMatrix_H
#define myMatrix_H

#include <vector>
#include <iostream>
#include <string>

using namespace std;

template<typename T>
inline T My_Min(T x, T y)
{
	return (x < y) ? x:y;
}

template<typename T>
inline T My_Max(T x, T y)
{
	return (x > y) ? x:y;
}

inline string wint2string(int o)
{
	string rtn;
	_Longlong number = o;
	rtn = std::to_string(number);
	return rtn;
}

class myMatrix
{
public:
	myMatrix(){}
	myMatrix(int row, int col):_row(row),_col(col) 
	{
		data.resize(row);
		for (int i=0; i<row; i++)
		{
			data[i].resize(col);
		}

	}

	int get_data(int row, int col) {return data[row][col];}
	void resize(int row, int col) 
	{
		_row = row;
		_col = col;
		data.resize(row);
		for (int i=0; i<row; i++)
		{
			data[i].resize(col);
		}
	}

public:
	vector<vector<int> > data;
	int _row;
	int _col;

};


class MyCoords{
public:
	MyCoords(){}
	MyCoords(double x_v, double y_v, double z_v)
	{
		x = x_v;
		y = y_v;
		z = z_v;
	}

	MyCoords operator - (const MyCoords& C) const
	{
		MyCoords newone;
		newone.x = x - C.x;
		newone.y = y - C.y;
		newone.z = z - C.z;
		return newone;
	};

public:
	double x;
	double y;
	double z;

};

#endif
