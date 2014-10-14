
#ifndef MYSUM_H_
#define MYSVM_H_

#include "svm.h"
#include "..\HONVTest\HON4D.h"
#include "..\pwjTools\file_operations.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <ctype.h>
#include <errno.h>
#include <fstream>
#include <vector>


#define Malloc(type,n) (type *)malloc((n)*sizeof(type))


class MySVM{

public:
	MySVM () {}
	MySVM(const std::string& train_data, const std::string& test_data )
	{
		train_data_path = train_data;
		test_data_path = test_data;
		//model_file_name = "..\\..\\Data\\MSRAction\\Desc\\svm_model.dat";
		//predict_probability = 0;
		//predict_output = "..\\..\\Data\\MSRAction\\Desc\\svm_output.dat";
		//_training_min_max_file = "..\\..\\Data\\MSRAction\\Desc\\train_min_max.dat";
		model_file_name = "..\\..\\Data\\MSRDailyAct3D\\Desc_local_HON4D_bin\\svm_eat_model.dat";
		predict_probability = 0;
		predict_output = "..\\..\\Data\\MSRDailyAct3D\\Desc_local_HON4D_bin\\svm_eat_output.dat";
		_training_min_max_file = "..\\..\\Data\\MSRDailyAct3D\\Desc_local_HON4D_bin\\train_min_max_eat.dat";

		max_nr_attr = 64;

		_eat_threshold = -0.6677;
		//_eat_threshold = -0.66;

	}
	MySVM(const std::string& train_data, int if_half)
	{
		if (if_half == 0)
		{
			train_data_path = train_data;
			model_file_name = "..\\data\\svm_eat_model.dat";
			predict_probability = 0;
			predict_output = "..\\data\\svm_eat_output.dat";
			_training_min_max_file = "..\\data\\train_min_max_eat.dat";
			test_data_path = "..\\data\\test_eat_svm.dat";
			max_nr_attr = 64;
			_eat_threshold = -0.6677;
	//		_eat_threshold = -0.70;

		}
		else if (if_half == 1)
		{
			train_data_path = train_data;
			model_file_name = "..\\data\\svm_eat_model_half.dat";
			predict_probability = 0;
			predict_output = "..\\data\\svm_eat_output_half.dat";
			_training_min_max_file = "..\\data\\train_min_max_eat_half.dat";
			test_data_path = "..\\data\\test_eat_svm_half.dat";
			max_nr_attr = 64;
			_eat_threshold = -0.6677;
	//		_eat_threshold = -0.70;

		}
		


	}
	~MySVM() {}

	int control_train();
	int control_predict();

	void parse_command_line(int argc, char **argv);
	void read_problem(const char *filename);
	void do_cross_validation();

	int file_format_convertion(const wstring& path, const char* out_file_path); /// convert the dataset file to svmlight format
	void free_train_model();
	void free_test_model();
	void predict_from_file(FILE *output);
	void predict_one_sample(const pair<int, vector<double> >& hon4d_feature, int& final_predict_label, double & cprobility);
	void MySVM::predict_one_sample(float *pfHON4DFeat,int feature_length, int& final_predict_label, double & decision_value);

	/// scale the data into [-1, 1]
	/// flag ==1 means for training data
	/// flag == 2 means for test data
	int scale_format_convertion_data(const wstring& path, const char* out_file_path, int flag=1); 

	int scale_one_sample(pair<int, vector<double> >& hon4d_feature);
	int MySVM::scale_one_sample(int feature_len, float *hon4d_feature);
	int save_training_min_max();
	void load_training_min_max();
	void load_training_model();

private:
	char* readline(FILE *input, char *line);

private:
	struct svm_parameter param;		// set by parse_command_line
	struct svm_problem prob;		// set by read_problem
	struct svm_model *model;
	struct svm_node *x_train_space;

	int cross_validation;
	int nr_fold;

	int max_line_len;

	std::string train_data_path;
	std::string test_data_path;

	std::string model_file_name;

	/// about predict
	int predict_probability;
	std::string predict_output;
	struct svm_node *x_test;
	int max_nr_attr;

	vector<pair<double, double> > _training_min_max;
	std::string _training_min_max_file;

	/// the decision threshold for eating or not-eating
	double _eat_threshold;


};





#endif