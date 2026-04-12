#pragma once
#include <filesystem>
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
		layerNeuron[2] = Matrix(1, 1, 0.0);            // 输出层
		// 初始化权重（使用 Xavier 初始化）
		for (int i = 0; i < layerNum - 1; i++) {
			initWeights(i);
			initBias(i);
		}
	}
	void initWeights(int layerIdx, bool random = true) {
		int n_in = layerNeuron[layerIdx].getrowNum();
		int n_out = layerNeuron[layerIdx + 1].getrowNum();
		double limit = sqrt(6.0 / (n_in + n_out));  // Xavier均匀分布范围
		weights[layerIdx] = Matrix(n_out, n_in, 0.0);
		if (random) {
			srand(time(0) + layerIdx + rand());  // 不同层不同种子，增加随机性
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
			//layerNeuron[i].show();
			Matrix z = multiplication(weights[i], layerNeuron[i]);
			z = addition(z, bias[i]);
			//z.show();
			if (i == layerNum - 2) {
				// 输出层使用 sigmoid，只输出一个值
				layerNeuron[i + 1] = sigmoid(z);
			}
			else {
				layerNeuron[i + 1] = sigmoid(z);
			}
		}
		double output = layerNeuron[2].getVectorD()[0][0];
		cout << "output: " << output << endl;
		// 概率大于 0.5 则跳跃
		return (output > 0.5) ? 1 : 0;
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
			for (int k = 0; k < weights[i].getrowNum(); k++) {
				for (int j = 0; j < weights[i].getcolumnNum(); j++) {
					inWeightsFile >> w;
					weights[i].setValue(k + 1, j + 1, w);
				}
			}
			inWeightsFile.close();
		}
	}
	// 均匀交叉函数
	void uniformCrossover(int llayerNum, int birdnum1, int birdnum2) {
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
					// 均匀交叉：50%概率从父代1取值，否则从父代2取值
					double crossedValue = (rand() / (double)RAND_MAX < 0.5) ? w1 : w2;
					// 变异：添加高斯噪声
					double mutationRate = 0.1;      // 每个权重变异概率
					double mutationStrength = 0.1;  // 变异强度
					if (rand() / (double)RAND_MAX < mutationRate) {
						// 生成高斯噪声
						double u1 = rand() / (double)RAND_MAX;
						double u2 = rand() / (double)RAND_MAX;
						double z = sqrt(-2.0 * log(u1)) * cos(2.0 * 3.1415926535 * u2);
						crossedValue += z * mutationStrength;
					}
					weights[i].setValue(k + 1, j + 1, crossedValue);
				}
			}
			inWeightsFile1.close();
			inWeightsFile2.close();
		}
	}

	// 算术交叉函数
	void arithmeticCrossover(int llayerNum, int birdnum1, int birdnum2, double alpha = 0.5) {
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
					// 算术交叉：alpha * w1 + (1 - alpha) * w2
					double crossedValue = alpha * w1 + (1 - alpha) * w2;
					// 变异：添加高斯噪声
					double mutationRate = 0.1;      // 每个权重变异概率
					double mutationStrength = 0.1;  // 变异强度
					if (rand() / (double)RAND_MAX < mutationRate) {
						// 生成高斯噪声
						double u1 = rand() / (double)RAND_MAX;
						double u2 = rand() / (double)RAND_MAX;
						double z = sqrt(-2.0 * log(u1)) * cos(2.0 * 3.1415926535 * u2);
						crossedValue += z * mutationStrength;
					}
					weights[i].setValue(k + 1, j + 1, crossedValue);
				}
			}
			inWeightsFile1.close();
			inWeightsFile2.close();
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
					// 交叉：取两个父代的平均值
					double crossedValue = 0.5 * w1 + 0.5 * w2;
					// 变异：添加高斯噪声
					double mutationRate = 0.4;      // 每个权重变异概率
					double mutationStrength = 0.8;  // 变异强度
					if (rand() / (double)RAND_MAX < mutationRate) {
						// 生成高斯噪声
						double u1 = rand() / (double)RAND_MAX;
						double u2 = rand() / (double)RAND_MAX;
						double z = sqrt(-2.0 * log(u1)) * cos(2.0 * 3.1415926535 * u2);
						crossedValue += z * mutationStrength;
					}
					weights[i].setValue(k + 1, j + 1, crossedValue);
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
