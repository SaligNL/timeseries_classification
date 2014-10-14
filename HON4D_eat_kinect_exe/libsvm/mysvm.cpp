
#include "mysvm.h"


using namespace  std;

void print_null(const char *s) {}

static int (*info)(const char *fmt,...) = &printf;

void exit_input_error(int line_num)
{
	fprintf(stderr,"Wrong input format at line %d\n", line_num);
	exit(1);
}


void exit_with_help()
{
	printf(
		"Usage: svm-train [options] training_set_file [model_file]\n"
		"options:\n"
		"-s svm_type : set type of SVM (default 0)\n"
		"	0 -- C-SVC		(multi-class classification)\n"
		"	1 -- nu-SVC		(multi-class classification)\n"
		"	2 -- one-class SVM\n"
		"	3 -- epsilon-SVR	(regression)\n"
		"	4 -- nu-SVR		(regression)\n"
		"-t kernel_type : set type of kernel function (default 2)\n"
		"	0 -- linear: u'*v\n"
		"	1 -- polynomial: (gamma*u'*v + coef0)^degree\n"
		"	2 -- radial basis function: exp(-gamma*|u-v|^2)\n"
		"	3 -- sigmoid: tanh(gamma*u'*v + coef0)\n"
		"	4 -- precomputed kernel (kernel values in training_set_file)\n"
		"-d degree : set degree in kernel function (default 3)\n"
		"-g gamma : set gamma in kernel function (default 1/num_features)\n"
		"-r coef0 : set coef0 in kernel function (default 0)\n"
		"-c cost : set the parameter C of C-SVC, epsilon-SVR, and nu-SVR (default 1)\n"
		"-n nu : set the parameter nu of nu-SVC, one-class SVM, and nu-SVR (default 0.5)\n"
		"-p epsilon : set the epsilon in loss function of epsilon-SVR (default 0.1)\n"
		"-m cachesize : set cache memory size in MB (default 100)\n"
		"-e epsilon : set tolerance of termination criterion (default 0.001)\n"
		"-h shrinking : whether to use the shrinking heuristics, 0 or 1 (default 1)\n"
		"-b probability_estimates : whether to train a SVC or SVR model for probability estimates, 0 or 1 (default 0)\n"
		"-wi weight : set the parameter C of class i to weight*C, for C-SVC (default 1)\n"
		"-v n: n-fold cross validation mode\n"
		"-q : quiet mode (no outputs)\n"
		);
	exit(1);
}

char* MySVM::readline(FILE *input, char *line)
{
	int len;

	if(fgets(line,max_line_len,input) == NULL)
		return NULL;

	while(strrchr(line,'\n') == NULL)
	{
		max_line_len *= 2;	
		line = (char *) realloc(line,max_line_len);
		len = (int) strlen(line);
		if(fgets(line+len,max_line_len-len,input) == NULL)
			break;
	}
	return line;
}



void MySVM::parse_command_line(int argc, char **argv)
{
	int i;
	void (*print_func)(const char*) = NULL;	// default printing to stdout

	// default values
	param.svm_type = C_SVC;
	param.kernel_type = RBF;
	param.degree = 3;
	param.gamma = 0;	// 1/num_features
	param.coef0 = 0;
	param.nu = 0.5;
	param.cache_size = 100;
	param.C = 1;
	param.eps = 1e-3;
	param.p = 0.1;
	param.shrinking = 1;
	param.probability = 0;
	param.nr_weight = 0;
	param.weight_label = NULL;
	param.weight = NULL;
	cross_validation = 0;

	// parse options
	for(i=1;i<argc;i++)
	{
		if(argv[i][0] != '-') break;
		i++;
		//if(++i>=argc)
		//	exit_with_help();
		switch(argv[i-1][1])
		{
		case 's':
			param.svm_type = atoi(argv[i]);
			break;
		case 't':
			param.kernel_type = atoi(argv[i]);
			break;
		case 'd':
			param.degree = atoi(argv[i]);
			break;
		case 'g':
			param.gamma = atof(argv[i]);
			break;
		case 'r':
			param.coef0 = atof(argv[i]);
			break;
		case 'n':
			param.nu = atof(argv[i]);
			break;
		case 'm':
			param.cache_size = atof(argv[i]);
			break;
		case 'c':
			param.C = atof(argv[i]);
			break;
		case 'e':
			param.eps = atof(argv[i]);
			break;
		case 'p':
			param.p = atof(argv[i]);
			break;
		case 'h':
			param.shrinking = atoi(argv[i]);
			break;
		case 'b':
			param.probability = atoi(argv[i]);
			break;
		case 'q':
			print_func = &print_null;
			i--;
			break;
		case 'v':
			cross_validation = 1;
			nr_fold = atoi(argv[i]);
			if(nr_fold < 2)
			{
				fprintf(stderr,"n-fold cross validation: n must >= 2\n");
				exit_with_help();
			}
			break;
		case 'w':
			++param.nr_weight;
			param.weight_label = (int *)realloc(param.weight_label,sizeof(int)*param.nr_weight);
			param.weight = (double *)realloc(param.weight,sizeof(double)*param.nr_weight);
			param.weight_label[param.nr_weight-1] = atoi(&argv[i-1][2]);
			param.weight[param.nr_weight-1] = atof(argv[i]);
			break;
		default:
			fprintf(stderr,"Unknown option: -%c\n", argv[i-1][1]);
			exit_with_help();
		}
	}

	svm_set_print_string_function(print_func);

	cout<<"t: "<<param.kernel_type<<" g: "<<param.gamma<<" d: "<<param.degree<<" b: "<<param.probability<<" cross validation: "<<cross_validation<<endl;
}


  /// read in a problem (in svmlight format)
void MySVM::read_problem(const char *filename)
{
	int elements, max_index, inst_max_index, i, j;
//	FILE *fp = fopen(filename,"r");
	ifstream infile;
	infile.open(filename);
	char *endptr;
	char *idx, *val, *label;

	if(infile.fail())
	{
		fprintf(stderr,"can't open input file %s\n",filename);
		exit(1);
	}

	prob.l = 0;
	elements = 0;

	max_line_len = 1024;
//	char * line;
//	line = (char*)malloc(sizeof(char)*max_line_len);
//	while(readline(fp, line)!=NULL)
	string str;
	while(getline(infile, str))
	{
		char* line = new char[str.length()+1];
		strcpy(line, str.c_str());
		char *p = strtok(line," \t"); // label

		// features
		while(1)
		{
			p = strtok(NULL," \t");
			if(p == NULL || *p == '\n') // check '\n' as ' ' may be after the last feature
				break;
			++elements;
		}
		++elements;
		++prob.l;

		delete line;
	}

	cout<<"prob.l: "<<prob.l<<endl;
	cout<<"elements: "<<elements<<endl;

	infile.clear();
	infile.seekg(0);   

	prob.y = Malloc(double,prob.l);
	prob.x = Malloc(struct svm_node *,prob.l);
	x_train_space = Malloc(struct svm_node,elements);


	max_index = 0;
	j=0;
	for(i=0;i<prob.l;i++)
//	for(i = 0; i<1; i++)
	{
		inst_max_index = -1; // strtol gives 0 if wrong format, and precomputed kernel has <index> start from 0
		getline(infile, str);
		char* line = new char[str.length()+1];
		strcpy(line, str.c_str());

		prob.x[i] = &x_train_space[j];
		label = strtok(line," \t\n");
	//	label = strtok(line, " ");
		if(label == NULL) // empty line
			exit_input_error(i+1);

		prob.y[i] = strtod(label,&endptr);
		if(endptr == label || *endptr != '\0')
			exit_input_error(i+1);

		while(1)
		{
			idx = strtok(NULL,":");
			val = strtok(NULL," \t");
		//	val = strtok(NULL," ");


			if(val == NULL)
				break;

			errno = 0;
			x_train_space[j].index = (int) strtol(idx,&endptr,10);
			if(endptr == idx || errno != 0 || *endptr != '\0' || x_train_space[j].index <= inst_max_index)
				exit_input_error(i+1);
			else
				inst_max_index = x_train_space[j].index;

			errno = 0;
			x_train_space[j].value = strtod(val,&endptr);
			if(endptr == val || errno != 0 || (*endptr != '\0' && !isspace(*endptr)))
				exit_input_error(i+1);

			


			++j;
		}

		if(inst_max_index > max_index)
			max_index = inst_max_index;
		x_train_space[j++].index = -1;

		/// for test
		//if (i==0)
		//{
		//	for ( int mm = 0; mm<= j; mm++)
		//	{
		//		cout<<mm<<" "<<prob.x[i][mm].index<<":"<<prob.x[i][mm].value<<endl;
		//	}
		//	exit(1);
		//}
		

		delete line;
	}

	if(param.gamma == 0 && max_index > 0)
		param.gamma = 1.0/max_index;

	if(param.kernel_type == PRECOMPUTED)
		for(i=0;i<prob.l;i++)
		{
			if (prob.x[i][0].index != 0)
			{
				fprintf(stderr,"Wrong input format: first column must be 0:sample_serial_number\n");
				exit(1);
			}
			if ((int)prob.x[i][0].value <= 0 || (int)prob.x[i][0].value > max_index)
			{
				fprintf(stderr,"Wrong input format: sample_serial_number out of range\n");
				exit(1);
			}
		}

	//	fclose(fp);
		infile.close();
//		free(line);
}


int find_label(const string& name)
{
	int rtn = 0;
	string ss = "";
	int label;

	for ( int j=name.length()-1;j>=0; j--)
	{
		if (name[j]=='a' && name[j+1]<58 && name[j+1]>47)
		{
			ss += name[j+1];
			ss += name[j+2];
			label = atoi(ss.c_str());
			if (label < 0)
			{
				cout<<"error: s_ind < 0"<<endl;
				exit(1);
			}

			break;
		}
	}

	rtn = label;
	return rtn;
}

int MySVM::file_format_convertion(const wstring& path, const char* out_file_path)
{
	int rtn = 0;

	//wstring path = L"..\\..\\Data\\MSRAction\\Desc\\training_set\\";
	
	vector<wstring> vecFiles;
	File_Operations::SearchDirectory(vecFiles,path, L"txt");

	ifstream infile; 
	ofstream outfile;
	outfile.open(out_file_path);

	int total = 0;
	for (int i=0; i<(int)vecFiles.size(); i++)
	{
		wcout<<i<<" "<<vecFiles[i]<<endl;

		string filep(vecFiles[i].begin(), vecFiles[i].end() );
		infile.open(filep.c_str());
		if (infile.fail())
		{
			cout<<"error: the file opening is wrong!"<<endl;
			exit(1);
		}

		int label = find_label(filep);
		outfile<<"+"<<label<<" ";

		string line;
		getline(infile, line);
		
		char* cstr = new char[line.length()+1];
		strcpy(cstr, line.c_str());
		char* par = ",";
		char* p = strtok(cstr, par);
		int index = 0;
		int flag = 0;
		while (p)
		{
			index ++;
			string ss = (string)p;
			
			//if (ss=="0")
			//{
			//	p = strtok(NULL, par);
			//	continue;
			//}
			//else
			{
				total ++;
				if(flag == 0)
				{
					outfile<<index<<":"<<p<<" ";
					flag = 1;
				}
				else
					outfile<<index<<":"<<p<<" ";

				p = strtok(NULL, par);
			}
			
		}
		if (i < (int)vecFiles.size())
		{
			outfile<<endl;
		}

		delete cstr;
		infile.close();
	}
	
	cout<<"total: "<<total<<endl;

	
	
	
	outfile.close();

	return rtn;
}

void MySVM::do_cross_validation()
{
	int i;
	int total_correct = 0;
	double total_error = 0;
	double sumv = 0, sumy = 0, sumvv = 0, sumyy = 0, sumvy = 0;
	double *target = Malloc(double,prob.l);

	svm_cross_validation(&prob,&param,nr_fold,target);
	if(param.svm_type == EPSILON_SVR ||
		param.svm_type == NU_SVR)
	{
		for(i=0;i<prob.l;i++)
		{
			double y = prob.y[i];
			double v = target[i];
			total_error += (v-y)*(v-y);
			sumv += v;
			sumy += y;
			sumvv += v*v;
			sumyy += y*y;
			sumvy += v*y;
		}
		printf("Cross Validation Mean squared error = %g\n",total_error/prob.l);
		printf("Cross Validation Squared correlation coefficient = %g\n",
			((prob.l*sumvy-sumv*sumy)*(prob.l*sumvy-sumv*sumy))/
			((prob.l*sumvv-sumv*sumv)*(prob.l*sumyy-sumy*sumy))
			);
	}
	else
	{
		for(i=0;i<prob.l;i++)
			if(target[i] == prob.y[i])
				++total_correct;
		printf("Cross Validation Accuracy = %g%%\n",100.0*total_correct/prob.l);
	}
	free(target);
}

void MySVM::free_train_model()
{
	svm_free_and_destroy_model(&model);

}

void MySVM::free_test_model()
{
	svm_free_and_destroy_model(&model);
}

int MySVM::control_train()
{
	int rtn;

//	char input_file_name[1024];
	const char* input_file_name = train_data_path.c_str();
	const char *error_msg;

	int argc = 9;
	char* argv[9];
	argv[0] = " svm-train";
	argv[1] = "-t";
	argv[2] = "1";
	argv[3] = "-g";
	argv[4] = "0.125";
	argv[5] = "-d";
	argv[6] = "3";
	argv[7] = "-b";
	argv[8] = "0";

	parse_command_line(argc, argv);
	read_problem(input_file_name);

	error_msg = svm_check_parameter(&prob,&param);

	if(error_msg)
	{
		fprintf(stderr,"ERROR: %s\n",error_msg);
		exit(1);
	}

	if(cross_validation)
	{
		do_cross_validation();
	}
	else
	{
		model = svm_train(&prob,&param);
		if(svm_save_model(model_file_name.c_str(),model))
		{
			fprintf(stderr, "can't save model to file %s\n", model_file_name);
			exit(1);
		}
		svm_free_and_destroy_model(&model);
	}


	svm_destroy_param(&param);

	free(prob.y);
	free(prob.x);
	free(x_train_space);

	cout<<"training svm is completed!"<<endl;

	return rtn;
}


int MySVM::control_predict()
{
	int rtn = 0;
	void (*print_func)(const char*) = NULL;	// default printing to stdout

	FILE *output;
	int i;
	int argc = 3;
	char * argv[3];
	argv[0] = " svm-test";
	argv[1] = "-b";
	argv[2] = "0";


	// parse options
	for(i=1;i<argc;i++)
	{
		if(argv[i][0] != '-') break;
		++i;
		switch(argv[i-1][1])
		{
		case 'b':
			predict_probability = atoi(argv[i]);
			break;
		case 'q':
			print_func = &print_null;
			i--;
			break;
		default:
			fprintf(stderr,"Unknown option: -%c\n", argv[i-1][1]);
			exit_with_help();
		}
	}


		output = fopen(predict_output.c_str(),"w");
		if(output == NULL)
		{
			fprintf(stderr,"can't open output file %s\n",predict_output);
			exit(1);
		}
	
	//if((model=svm_load_model(model_file_name.c_str()))==0)
	//{
	//	fprintf(stderr,"can't open model file %s\n",model_file_name);
	//	exit(1);
	//}

	x_test = (struct svm_node *) malloc(max_nr_attr*sizeof(struct svm_node));
	if(predict_probability)
	{
		if(svm_check_probability_model(model)==0)
		{
			fprintf(stderr,"Model does not support probabiliy estimates\n");
			exit(1);
		}
	}
	else
	{
		if(svm_check_probability_model(model)!=0)
			info("Model supports probability estimates, but disabled in prediction.\n");
	}

	predict_from_file(output);
	
//	svm_free_and_destroy_model(&model);
	free(x_test);
	fclose(output);

	return rtn; 

}



void MySVM::predict_from_file(FILE *output)
{
	int correct = 0;
	int total = 0;
	double error = 0;
	double sump = 0, sumt = 0, sumpp = 0, sumtt = 0, sumpt = 0;

	int svm_type=svm_get_svm_type(model);
	int nr_class=svm_get_nr_class(model);

	double *prob_estimates=NULL;
	double *decision_values = NULL;
	int j;

	if(predict_probability)
	{
		if (svm_type==NU_SVR || svm_type==EPSILON_SVR)
			info("Prob. model for test data: target value = predicted value + z,\nz: Laplace distribution e^(-|z|/sigma)/(2sigma),sigma=%g\n",svm_get_svr_probability(model));
		else
		{
			int *labels=(int *) malloc(nr_class*sizeof(int));
			svm_get_labels(model,labels);
			prob_estimates = (double *) malloc(nr_class*sizeof(double));

			fprintf(output,"labels");		
			for(j=0;j<nr_class;j++)
				fprintf(output," %d",labels[j]);
			fprintf(output,"\n");
			free(labels);
		}
	}


//	max_line_len = 1024;
//	line = (char *)malloc(max_line_len*sizeof(char));
//	while(readline(input) != NULL)
	ifstream infile;
	infile.open(test_data_path.c_str());
	if (infile.fail())
	{
		cout<<"error: cannot open "<<test_data_path<<endl;
		exit(1);
	}
	
	string str;
	int nn=0;
	while(getline(infile, str))
	{
		nn ++;
		char* line = new char[str.length()+1];
		strcpy(line, str.c_str());
		int i = 0;
		double target_label, predict_label;
		char *idx, *val, *label, *endptr;
		int inst_max_index = -1; // strtol gives 0 if wrong format, and precomputed kernel has <index> start from 0

		label = strtok(line," \t\n");
	//	label = strtok(line," ");

		if(label == NULL) // empty line
			exit_input_error(total+1);

		target_label = strtod(label,&endptr);
		if(endptr == label || *endptr != '\0')
			exit_input_error(total+1);

		while(1)
		{
			if(i>=max_nr_attr-1)	// need one more for index = -1
			{
				max_nr_attr *= 2;
				x_test = (struct svm_node *) realloc(x_test,max_nr_attr*sizeof(struct svm_node));
			}

			idx = strtok(NULL,":");
			val = strtok(NULL," \t");
	//		val = strtok(NULL," ");

			if(val == NULL)
				break;
			errno = 0;
			x_test[i].index = (int) strtol(idx,&endptr,10);
			if(endptr == idx || errno != 0 || *endptr != '\0' || x_test[i].index <= inst_max_index)
				exit_input_error(total+1);
			else
				inst_max_index = x_test[i].index;

			errno = 0;
			x_test[i].value = strtod(val,&endptr);
			if(endptr == val || errno != 0 || (*endptr != '\0' && !isspace(*endptr)))
				exit_input_error(total+1);
			
			//if(nn==2)
			//cout<<x_test[i].index<<":"<<x_test[i].value<<endl;
			++i;
		}
		x_test[i].index = -1;
		

		if (predict_probability && (svm_type==C_SVC || svm_type==NU_SVC))
		{
			predict_label = svm_predict_probability(model,x_test,prob_estimates);
			fprintf(output,"%g",predict_label);
			for(j=0;j<nr_class;j++)
				fprintf(output," %g",prob_estimates[j]);
			fprintf(output,"\n");
		}
		else
		{
			decision_values = (double *) malloc(nr_class*(nr_class-1)/2 * sizeof(double));
			predict_label = svm_predict_values(model,x_test, decision_values);
			if (decision_values[0] > _eat_threshold)
			{
				predict_label = +1;
			}
			else
				predict_label = -1;
			
			fprintf(output,"%g",predict_label);
			for(j=0;j<nr_class*(nr_class-1)/2;j++)
				fprintf(output," %g",decision_values[j]);
			fprintf(output,"\n");
		}

		if(predict_label == target_label)
		{
			++correct;
			fprintf(output, "correct\n");
		}
		else
			fprintf(output, "wrong\n");

		error += (predict_label-target_label)*(predict_label-target_label);
		sump += predict_label;
		sumt += target_label;
		sumpp += predict_label*predict_label;
		sumtt += target_label*target_label;
		sumpt += predict_label*target_label;
		++total;

		delete line;
	}
	if (svm_type==NU_SVR || svm_type==EPSILON_SVR)
	{
		info("Mean squared error = %g (regression)\n",error/total);
		info("Squared correlation coefficient = %g (regression)\n",
			((total*sumpt-sump*sumt)*(total*sumpt-sump*sumt))/
			((total*sumpp-sump*sump)*(total*sumtt-sumt*sumt))
			);
	}
	else
		info("Accuracy = %g%% (%d/%d) (classification)\n",
		(double)correct/total*100,correct,total);
	if(predict_probability)
		free(prob_estimates);
	free(decision_values);
}

void MySVM::load_training_model()
{
	cout<<"Loading training model...."<<endl;
	if((model=svm_load_model(model_file_name.c_str()))==0)
	{
		fprintf(stderr,"can't open model file %s\n",model_file_name);
		exit(1);
	}
	cout<<"load training model success!"<<endl;
}

void MySVM::predict_one_sample(const pair<int, vector<double> >& hon4d_feature, int& final_predict_label, double & decision_value)
{
	int rtn = 0;
	void (*print_func)(const char*) = NULL;	// default printing to stdout

	int i;
	int argc = 3;
	char * argv[3];
	argv[0] = " svm-test";
	argv[1] = "-b";
	argv[2] = "0";

	// parse options
	for(i=1;i<argc;i++)
	{
		if(argv[i][0] != '-') break;
		++i;
		switch(argv[i-1][1])
		{
		case 'b':
			predict_probability = atoi(argv[i]);
			break;
		case 'q':
			print_func = &print_null;
			i--;
			break;
		default:
			fprintf(stderr,"Unknown option: -%c\n", argv[i-1][1]);
			exit_with_help();
		}
	}
	

	//if((model=svm_load_model(model_file_name.c_str()))==0)
	//{
	//	fprintf(stderr,"can't open model file %s\n",model_file_name);
	//	exit(1);
	//}
	
	if(predict_probability)
	{
		if(svm_check_probability_model(model)==0)
		{
			fprintf(stderr,"Model does not support probabiliy estimates\n");
			exit(1);
		}
	}
	else
	{
		if(svm_check_probability_model(model)!=0)
			info("Model supports probability estimates, but disabled in prediction.\n");
	}
	int svm_type=svm_get_svm_type(model);
	int nr_class=svm_get_nr_class(model);

	double *prob_estimates=NULL;
	double *decision_values;
	int j;
	if(predict_probability)
	{
		if (svm_type==NU_SVR || svm_type==EPSILON_SVR)
			info("Prob. model for test data: target value = predicted value + z,\nz: Laplace distribution e^(-|z|/sigma)/(2sigma),sigma=%g\n",svm_get_svr_probability(model));
		else
		{
			//int *labels=(int *) malloc(nr_class*sizeof(int));
			//svm_get_labels(model,labels);
			prob_estimates = (double *) malloc(nr_class*sizeof(double));
			//cout<<"labels:"<<endl;		
			//for(j=0;j<nr_class;j++)
			//	printf(" %d",labels[j]);
			//printf("\n");
			//free(labels);
		}
	}

		double target_label, predict_label;


		target_label = (double)hon4d_feature.first;
		int feature_num = (int)hon4d_feature.second.size();
		cout<<"true label: "<<target_label<<endl;

		x_test = (struct svm_node *) malloc((feature_num+1)*sizeof(struct svm_node));


		i=0;
		for(; i<feature_num; i++)
		{
			x_test[i].index = i+1;
			
			x_test[i].value = hon4d_feature.second[i];
			
		}
		x_test[i].index = -1;
//		cout<<"test3"<<endl;



		decision_values = (double *) malloc(nr_class*(nr_class-1)/2 * sizeof(double));
		predict_label = svm_predict_values(model,x_test, decision_values);
		if (decision_values[0] > _eat_threshold)
		{
			predict_label = +1;
		}
		else
			predict_label = -1;

		final_predict_label = predict_label;
		decision_value = decision_values[0];
		cout<<"predicted label: "<<final_predict_label<<" decision value: "<<decision_value<<endl;



		if(predict_label == target_label)
		{
			printf("predicted correct\n");
		}
		else
			printf("predicted wrong\n");

	if(predict_probability)
		free(prob_estimates);

	free(x_test);
//	svm_free_and_destroy_model(&model);

}

void MySVM::predict_one_sample(float *pfHON4DFeat, int feature_length, int& final_predict_label, double & decision_value)
{
	int rtn = 0;
	void (*print_func)(const char*) = NULL;	// default printing to stdout

	int i;
	int argc = 3;
	char * argv[3];
	argv[0] = " svm-test";
	argv[1] = "-b";
	argv[2] = "0";

	// parse options
	for(i=1;i<argc;i++)
	{
		if(argv[i][0] != '-') break;
		++i;
		switch(argv[i-1][1])
		{
		case 'b':
			predict_probability = atoi(argv[i]);
			break;
		case 'q':
			print_func = &print_null;
			i--;
			break;
		default:
			fprintf(stderr,"Unknown option: -%c\n", argv[i-1][1]);
			exit_with_help();
		}
	}


	//if((model=svm_load_model(model_file_name.c_str()))==0)
	//{
	//	fprintf(stderr,"can't open model file %s\n",model_file_name);
	//	exit(1);
	//}

	if(predict_probability)
	{
		if(svm_check_probability_model(model)==0)
		{
			fprintf(stderr,"Model does not support probabiliy estimates\n");
			exit(1);
		}
	}
	else
	{
		if(svm_check_probability_model(model)!=0)
			info("Model supports probability estimates, but disabled in prediction.\n");
	}
	int svm_type=svm_get_svm_type(model);
	int nr_class=svm_get_nr_class(model);

	double *prob_estimates=NULL;
	double *decision_values;
	int j;
	if(predict_probability)
	{
		if (svm_type==NU_SVR || svm_type==EPSILON_SVR)
			info("Prob. model for test data: target value = predicted value + z,\nz: Laplace distribution e^(-|z|/sigma)/(2sigma),sigma=%g\n",svm_get_svr_probability(model));
		else
		{
			//int *labels=(int *) malloc(nr_class*sizeof(int));
			//svm_get_labels(model,labels);
			prob_estimates = (double *) malloc(nr_class*sizeof(double));
			//cout<<"labels:"<<endl;		
			//for(j=0;j<nr_class;j++)
			//	printf(" %d",labels[j]);
			//printf("\n");
			//free(labels);
		}
	}

	double predict_label;


	int feature_num = feature_length;

	x_test = (struct svm_node *) malloc((feature_num+1)*sizeof(struct svm_node));


	i=0;
	for(; i<feature_num; i++)
	{
		x_test[i].index = i+1;

		x_test[i].value = pfHON4DFeat[i];

	}
	x_test[i].index = -1;
	//		cout<<"test3"<<endl;



	decision_values = (double *) malloc(nr_class*(nr_class-1)/2 * sizeof(double));
	predict_label = svm_predict_values(model,x_test, decision_values);
	if (decision_values[0] > _eat_threshold)
	{
		predict_label = +1;
	}
	else
		predict_label = -1;

	final_predict_label = predict_label;
	decision_value = decision_values[0];
	cout<<"eating predicted label: "<<final_predict_label<<" decision value: "<<decision_value<<endl;


	if(predict_probability)
		free(prob_estimates);

	free(x_test);
	//	svm_free_and_destroy_model(&model);

}


int MySVM::scale_format_convertion_data(const wstring& path, const char* out_file_path, int flag)
{
	int rtn = 0;

	vector<vector<double> > data;
	vector<pair<double, double> > min_max;

	vector<wstring> vecFiles;
	File_Operations::SearchDirectory(vecFiles,path, L"txt");

	ifstream infile; 
	ofstream outfile;
	outfile.open(out_file_path);

	int feature_num = 0;

	for (int i=0; i<(int)vecFiles.size(); i++)
	{
		vector<double> one_line;

//		wcout<<i<<" "<<vecFiles[i]<<endl;

		string filep(vecFiles[i].begin(), vecFiles[i].end() );
		infile.open(filep.c_str());
		if (infile.fail())
		{
			cout<<"error: the file opening is wrong!"<<endl;
			exit(1);
		}

		
		string line;
		getline(infile, line);

		char* cstr = new char[line.length()+1];
		strcpy(cstr, line.c_str());
		char* par = ",";
		char* p = strtok(cstr, par);
		int flag = 0;
		int total = 0;
		while (p)
		{
			if (i == 0)
			{
				feature_num ++;
			}

			total ++;

			one_line.push_back(atof(p) );

			p = strtok(NULL, par);


		}
		delete cstr;
		data.push_back(one_line);
		infile.close();

		if (total != feature_num)
		{
			cout<<"error: the total != feature_num: "<<total<<" "<<feature_num<<endl;
			exit(1);
		}
		if (feature_num != (int)one_line.size())
		{
			cout<<"error: feature_num != one_Line.size(): "<<feature_num<<" "<<one_line.size()<<endl;
			exit(1);
		}
		for (int k=0; k<feature_num; k++)
		{
			if (i==0)
			{
				pair<double, double> mm_t;
				mm_t.first = one_line[k];
				mm_t.second = one_line[k];
				min_max.push_back(mm_t);
			}
			else
			{
				if (min_max[k].first > one_line[k])
				{
					min_max[k].first = one_line[k];
				}
				if (min_max[k].second < one_line[k])
				{
					min_max[k].second = one_line[k];
				}
			}
		}
	}

	for (int i=0; i<data.size(); i++)
	{
		for(int j=0; j<feature_num; j++)
		{
			if (min_max[j].second - min_max[j].first !=0)
				data[i][j] = (data[i][j] - min_max[j].first ) / (min_max[j].second - min_max[j].first) *2 -1.0;
		}
	}
	

	for (int i=0; i<(int)data.size(); i++)
	{
		wcout<<i<<" "<<vecFiles[i]<<endl;

		string filep(vecFiles[i].begin(), vecFiles[i].end() );
		infile.open(filep.c_str());
		if (infile.fail())
		{
			cout<<"error: the file opening is wrong!"<<endl;
			exit(1);
		}

		int label = find_label(filep);

		if (label == 2)
		{
			label = 1;

		}
		else
			label = -1;

		outfile<<label<<" ";

		for (int k=0; k<data[i].size(); k++)
		{
			outfile<<k+1<<":"<<data[i][k]<<" ";
		}

		outfile<<endl;
		
		infile.close();
	}

	if (flag == 1)
	{
		_training_min_max = min_max;
		save_training_min_max();

	}
	

	outfile.close();

	return rtn;
}

int MySVM::save_training_min_max()
{
	int rtn = 0;

	ofstream outfile;
	outfile.open(_training_min_max_file.c_str());
	if (outfile.fail())
	{
		cout<<"error: cannot open training_min_max!"<<endl;
		exit(1);
	}

	for (int i=0; i<(int)_training_min_max.size(); i++)
	{
		outfile<<i<<" "<< _training_min_max[i].first<<" "<<_training_min_max[i].second;
		if (i<(int)_training_min_max.size()-1)
		{
			outfile<<endl;
		}
	}
	
	outfile.close();

	return rtn;
}

void MySVM::load_training_min_max()
{
	ifstream infile;
	infile.open(_training_min_max_file.c_str());
	if (infile.fail())
	{
		cout<<"error: cannot open training_min_max!"<<endl;
		exit(1);
	}
	int index = 0;
	double min;
	double max;
	while (infile >> index >> min >> max)
	{
		pair<double, double> p;
		p.first = min;
		p.second = max;
		_training_min_max.push_back(p);
	}
	cout<<"size of training_min_max: "<<_training_min_max.size()<<endl;
	cout<<"load training_min_max success!"<<endl;
}

int MySVM::scale_one_sample(pair<int, vector<double> >& hon4d_feature)
{
	int rtn =0; 

	int feature_num = (int)hon4d_feature.second.size();
	for (int i=0; i<feature_num; i++)
	{
		if (hon4d_feature.second[i] < _training_min_max[i].first)
		{
			hon4d_feature.second[i] = -1.0;
		}
		else if (hon4d_feature.second[i] > _training_min_max[i].second)
		{
			hon4d_feature.second[i] = 1.0;
		}
		else
		{
			double diff = _training_min_max[i].second - _training_min_max[i].first;
			if ( diff != 0)
				hon4d_feature.second[i] = (hon4d_feature.second[i] - _training_min_max[i].first ) / diff *2 -1.0;
		}
		
	}


	return rtn;
}

int MySVM::scale_one_sample(int feature_len, float *hon4d_feature)
{
	int rtn =0; 

	int feature_num = feature_len;
	for (int i=0; i<feature_num; i++)
	{
		if (hon4d_feature[i] < _training_min_max[i].first)
		{
			hon4d_feature[i] = -1.0;
		}
		else if (hon4d_feature[i] > _training_min_max[i].second)
		{
			hon4d_feature[i] = 1.0;
		}
		else
		{
			double diff = _training_min_max[i].second - _training_min_max[i].first;
			if ( diff != 0)
				hon4d_feature[i] = (hon4d_feature[i] - _training_min_max[i].first ) / diff *2 -1.0;
		}

	}


	return rtn;
}