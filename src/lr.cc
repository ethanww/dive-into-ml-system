#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <boost/format.hpp>
#include "lr.h"
#include "utils.h"

using namespace std;
using namespace Eigen;

LR::LR(){
    LR(100,0.01,0.01,0.001);
}

LR::LR(int max_iter,double alpha,double lambda,double tolerance){
	this->lambda = lambda; 
	this->max_iter = max_iter;
	this->tolerance = tolerance;
	this->alpha = alpha;
}

LR::~LR(){}



void LR::fit(MatrixXd X,VectorXi y){
	//learn VectorXd W, consider reg,max_iter,tol.   

	W = VectorXd::Random(X.cols()+1);  //the last column of weight represent bias
	MatrixXd X_new(X.rows(),X.cols()+1);
	X_new<<X,MatrixXd::Ones(X.rows(),1);  //last column is 1.0

	for(int iter=0;iter<max_iter;iter++){
		VectorXd y_pred = predict_prob(X);
        VectorXd y_d = y.cast<double>();  //cast type first
		VectorXd E = y_pred - y_d;

        //W:= (1-lambda/n_samples)W-alpha*X^T*E
		//reference : http://blog.csdn.net/pakko/article/details/37878837
		W = (1.0-lambda/y.size())*W - alpha*X_new.transpose()*E;
		
		double loss = Utils::crossEntropyLoss(y,predict_prob(X));
        cout<<boost::format("Iteration: %d, logloss:%.5f") %iter %loss << endl;
		
        //when loss<tolerance, break
		if(loss<=tolerance) break;
	}

}


VectorXd LR::predict_prob(MatrixXd X){
	//predict the probability (of label 1) for given data X
	MatrixXd X_new(X.rows(),X.cols()+1);
	X_new<<X,MatrixXd::Ones(X.rows(),1);
	int num_samples = X_new.rows();
	VectorXd y_pred_prob = VectorXd::Zero(num_samples);
	for(int num=0;num<num_samples;num++){
		y_pred_prob(num) = Utils::sigmod(X_new.row(num).dot(W));
	}

	return y_pred_prob;
}


VectorXi LR::predict(MatrixXd X){
	//predict the label for given data X
	VectorXd y_pred_prob = predict_prob(X);
	VectorXi y_pred(y_pred_prob.size());
	for(int num=0;num<y_pred_prob.size();num++){
		y_pred(num) = y_pred_prob(num)>0.5?1:0;
	}
	return y_pred;
}


Eigen::VectorXd LR::getW(){
	return W;
}

void LR::saveWeights(std::string fpath){
	//save the model (save the weight ). 
	std::ofstream ofile;
	ofile.open(fpath.c_str());
	if (!ofile.is_open()){
		std::cerr<<"Can not open the file when call LR::saveWeights"<<std::endl;
		return;
    }

    //W write into the file
	for(int i=0;i<W.size()-1;i++){
		ofile<<W(i)<<" ";
	}
	ofile<<W(W.size()-1);
	ofile.close();
}


void LR::loadWeights(std::string fpath){
	//load the model (load the weight ) from filename.
	std::ifstream ifile;
	ifile.open(fpath.c_str());
	if (!ifile.is_open()){
		std::cerr<<"Can not open the file when call LR::loadWeights"<<std::endl;
		return;
    }

    //read the weights into vector<double>
	std::string line;
	std::vector<double> weights;
	getline(ifile,line);    //only one line
	std::stringstream ss(line); 
	double tmp;
	while(!ss.eof()){
		ss>>tmp;
		weights.push_back(tmp);
	}

	//initialize VectorXd with std::vector
	W = VectorXd::Map(weights.data(),weights.size());

	ifile.close();
}


