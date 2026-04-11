#pragma once
#pragma execution_character_set("utf-8")
#include<filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <random>
#include "VectorD.h"
#include "Matrix.h"
using namespace std;
// Function //
Matrix sigmoid(Matrix X) {
	Matrix C = Matrix(X.getrowNum(), X.getcolumnNum());
	for (int i = 0; i < C.getrowNum(); i++) {
		for (int j = 0; j < C.getcolumnNum(); j++) {
			C.setValue(i + 1, j + 1, 1.0 / (1.0 + exp(-X.getVectorD()[j][i])));
		}
	}
	return C;
}
Matrix tanh(Matrix X) {
	Matrix C = Matrix(X.getrowNum(), X.getcolumnNum());
	for (int i = 0; i < C.getrowNum(); i++) {
		for (int j = 0; j < C.getcolumnNum(); j++) {
			C.setValue(i + 1, j + 1, (1 - exp(-2 * X.getVectorD()[j][i])) / (1 + exp(-2 * X.getVectorD()[j][i])));
		}
	}
	return C;
}
Matrix ReLU(Matrix X) {
	Matrix C = Matrix(X.getrowNum(), X.getcolumnNum());
	for (int i = 0; i < C.getrowNum(); i++) {
		for (int j = 0; j < C.getcolumnNum(); j++) {
			if (X.getVectorD()[j][i] >= 0) {
				C.setValue(i + 1, j + 1, X.getVectorD()[j][i]);
			}
			else {
				C.setValue(i + 1, j + 1, 0);
			}
		}
	}
	return C;
}
Matrix softmax(Matrix Out) {
	double sum = 0;
	for (int i = 0; i < Out.getrowNum(); i++) {
		sum += exp(Out.getVectorD()[0][i]);
	}
	Matrix Y(Out.getrowNum(), Out.getcolumnNum());
	for (int i = 0; i < Out.getrowNum(); i++) {
		Y.setValue(i + 1, 1, (exp(Out.getVectorD()[0][i]) / sum));
	}
	return Y;
}
double Loss(Matrix& target, Matrix& predict) {
	double loss = 0;
	for (int i = 0; i < predict.getrowNum(); i++) {
		double p = predict.getVectorD()[0][i];
		double t = target.getVectorD()[0][i];
		if (t > 0.5) {  // one-hot 中只有正确类别为1
			loss -= log(p + 1e-10);  // 避免 log(0)
		}
	}
	return loss;
}

class NeuralNetwork {
public:
	NeuralNetwork() {}
	~NeuralNetwork() {}
	void initNet(int llayerNum) {
		layerNum = llayerNum;
		layerNeuron.resize(layerNum);
		weights.resize(layerNum - 1);
		bias.resize(layerNum - 1);
		// 假设网络结构：输入层 -> 隐藏层 -> 输出层
		layerNeuron[0] = Matrix(5, 1, 0.0);        // 输入层
		layerNeuron[1] = Matrix(30, 1, 0.0);          // 隐藏层（可调整）
		layerNeuron[2] = Matrix(2, 1, 0.0);            // 输出层
		// 初始化权重（使用 Xavier 初始化）
		for (int i = 0; i < layerNum - 1; i++) {
			initWeights(i);
			initBias(i);
		}
	}
	void initWeights(int layerIdx, bool random = true) {
		int n_in = layerNeuron[layerIdx].getrowNum();
		int n_out = layerNeuron[layerIdx + 1].getrowNum();
		double limit = sqrt(6.0 / (n_in + n_out)) - 10;  // Xavier均匀分布范围
		weights[layerIdx] = Matrix(n_out, n_in, 0.0);
		if (random) {
			srand(time(0) + layerIdx);  // 不同层不同种子
			for (int r = 0; r < n_out; r++) {
				for (int c = 0; c < n_in; c++) {
					double val = (rand() / (double)RAND_MAX) * 2 * limit - limit;  // [-limit, limit]
					weights[layerIdx].setValue(r + 1, c + 1, val);
				}
			}
		}
		else {
			// 固定值（可用于调试）
			for (int r = 0; r < n_out; r++)
				for (int c = 0; c < n_in; c++)
					weights[layerIdx].setValue(r + 1, c + 1, 0.01);
		}
	}
	void initBias(int layerIdx, double val = 0.0) {
		int n_out = layerNeuron[layerIdx + 1].getrowNum();
		bias[layerIdx] = Matrix(n_out, 1, val);
	}
	int play(Matrix birdState) {
		layerNeuron[0] = birdState;
		for (int i = 0; i < layerNum - 1; i++) {
			Matrix z = multiplication(weights[i], layerNeuron[i]);
			z = addition(z, bias[i]);  // 加上偏置
			if (i == layerNum - 2) {
				layerNeuron[i + 1] = softmax(z);
			}
			else {
				layerNeuron[i + 1] = sigmoid(z);
			}
		}
		cout << endl << layerNeuron[2].getVectorD()[0][0] << ' ' << layerNeuron[2].getVectorD()[0][1] << endl;
		if (layerNeuron[2].getVectorD()[0][0] <= layerNeuron[2].getVectorD()[0][1]) {
			return 1;
		}
		else {
			return 0;
		}
	}
	void saveModel(int llayerNum, int birdnum) {
		for (int i = 0; i <= llayerNum - 2; i++) {
			string fileName = "bird" + to_string(birdnum) + "weight" + to_string(i) + ".txt";
			filesystem::remove(fileName);
			ofstream outWeightsFile;
			double w;
			outWeightsFile.open(fileName);
			for (int k = 0; k < weights[i].getrowNum(); k++) {
				for (int j = 0; j < weights[i].getcolumnNum(); j++) {
					w = weights[i].getVectorD()[j][k];
					outWeightsFile << w << ' ';
				}
				outWeightsFile << endl;
			}
			outWeightsFile.close();

		}
	}
	void loadModel(int llayerNum, int birdnum) {

		for (int i = 0; i <= llayerNum - 2; i++) {
			string fileName = "bird" + to_string(birdnum) + "weight" + to_string(i) + ".txt";
			ifstream inWeightsFile;
			double w;
			inWeightsFile.open(fileName);
			srand(time(0));  
			for (int k = 0; k < weights[i].getrowNum(); k++) {
				for (int j = 0; j < weights[i].getcolumnNum(); j++) {
					inWeightsFile >> w;
					weights[i].setValue(k + 1, j + 1, w);
				}
			}
			inWeightsFile.close();
		}

	}
	void crossoverAndmutation(int llayerNum, int birdnum1, int birdnum2) {
		for (int i = 0; i <= llayerNum - 2; i++) {
			string fileName1 = "bird" + to_string(birdnum1) + "weight" + to_string(i) + ".txt";
			ifstream inWeightsFile1;
			string fileName2 = "bird" + to_string(birdnum2) + "weight" + to_string(i) + ".txt";
			ifstream inWeightsFile2;
			double w1;
			double w2;
			inWeightsFile1.open(fileName1);
			inWeightsFile2.open(fileName2);
			for (int k = 0; k < weights[i].getrowNum(); k++) {
				for (int j = 0; j < weights[i].getcolumnNum(); j++) {

					inWeightsFile1 >> w1;
					inWeightsFile2 >> w2;
					weights[i].setValue(k + 1, j + 1, (0.5 * w1 + 0.5 * w2) + 0.8 * (rand() % 2));
				}
			}
			inWeightsFile1.close();
			inWeightsFile2.close();
		}
	}

public:
	int layerNum; //包括输入层X，隐藏层Hi，输出层Y，取>=2
	vector<int>layerNumLabel;//数值取取>=1
	vector<Matrix>layerNeuron;
	vector<Matrix>weights;
	vector<Matrix>bias;//偏置即Y=WX+b里的b
	double loss;
};
