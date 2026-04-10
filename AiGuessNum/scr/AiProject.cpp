#include <iostream>
#include <fstream>
#include <vector>
#include<cmath>
#include<filesystem>
#include<cstdlib>
#include<string>
#include <algorithm>
#include <random>
#include <SFML/Graphics.hpp>
#include "lodepng.h"
using namespace std;


class VectorD {
public:
	VectorD(){
		length = 1;
		vectorHead = new double[1];
		initialValue(0);
		
	}
	VectorD(int llength,double value=0) {
		length = llength;
		vectorHead = new double[length];
		initialValue(value);
	}
	VectorD(const VectorD& other) {
		length = other.length;
		vectorHead = new double[length];
		for (int i = 0; i < length; i++) {
			vectorHead[i] = other.vectorHead[i];
		}
	}
	~VectorD() {
		delete[] vectorHead;
	}
	void initialValue(double value) {
		for (int i = 0; i < length; i++) {
			vectorHead[i] = 0;
		}
	}
	void setValue(int i,double num) {
		if (i <= 0 || i > length) return;
		else {
			vectorHead[i - 1] = num;
		}
	}
	int getLength() { return length; }

	VectorD& operator=(const VectorD& other) {
		if (this != &other) {
			delete[] vectorHead;
			length = other.length;
			vectorHead = new double[length];
			for (int i = 0; i < length; i++) {
				vectorHead[i] = other.vectorHead[i];
			}
		}
		return *this;
	}
	double &operator[](int i) {
		return vectorHead[i];
	}

	void show(const char testname[] ="testname\0") {
		cout << endl;
		cout << *testname;
		cout << endl;
		for (int i = 0; i < length; i++) {
			cout<<vectorHead[i]<<' ';
		}
		cout << endl;
	}

private:
	double* vectorHead;
	int length;
};
class Matrix {
public:
	Matrix() {
		columnNum = 1;
		rowNum = 1;
		// 分配内存
		columnVector = static_cast<VectorD*>(operator new[](sizeof(VectorD) * 1));
		// 直接构造对象
		new (&columnVector[0]) VectorD(1);
	}
	Matrix(int rrowNum,int ccolumnNum,double value=0) {
		if (rrowNum <= 0) rrowNum = 1;
		if (ccolumnNum <= 0) ccolumnNum = 1;
		rowNum = rrowNum;
		columnNum = ccolumnNum;
		// 分配内存
		columnVector = static_cast<VectorD*>(operator new[](sizeof(VectorD)* columnNum));
		// 直接构造对象
		for (int i = 0; i < columnNum; i++) {
			new (&columnVector[i]) VectorD(rowNum,value);
		}
	}
	Matrix(const Matrix& other) {
		rowNum = other.rowNum;
		columnNum = other.columnNum;
		// 分配内存
		columnVector = static_cast<VectorD*>(operator new[](sizeof(VectorD)* columnNum));
		// 直接构造对象
		for (int i = 0; i < columnNum; i++) {
			new (&columnVector[i]) VectorD(other.columnVector[i]);
		}
	}
	Matrix& operator=(const Matrix& other) {
		if (this != &other) {
			// 手动调用析构函数
			for (int i = 0; i < columnNum; i++) {
				columnVector[i].~VectorD();
			}
			// 释放内存
			operator delete[](columnVector);

			rowNum = other.rowNum;
			columnNum = other.columnNum;
			// 分配内存
			columnVector = static_cast<VectorD*>(operator new[](sizeof(VectorD)* columnNum));
			// 直接构造对象
			for (int i = 0; i < columnNum; i++) {
				new (&columnVector[i]) VectorD(other.columnVector[i]);
			}
		}
		return *this;
	}
	~Matrix() {
		// 手动调用析构函数
		for (int i = 0; i < columnNum; i++) {
			columnVector[i].~VectorD();
		}
		// 释放内存
		operator delete[](columnVector);
	}
	

	void setValue(int i,int j, double num) { //注意这里i、j范围要求>0
		if (i < 1 || i > rowNum) return;  // i 从1开始
		if (j < 1 || j > columnNum) return;
		else {
			columnVector[j - 1][i - 1] = num;
		}
	}
	void show(const string testname = "testname") {
		cout << endl;
		cout << testname;
		cout << endl;
		for (int i = 0; i < rowNum; i++) {
			for (int j = 0; j < columnNum; j++) {
				cout << columnVector[j][i] << ' ';
			}
			cout << endl;
		}
		cout << endl;
	}

	int getrowNum() { return rowNum; }
	int getcolumnNum() { return columnNum; }
	VectorD* getVectorD() {return columnVector;}
private:
	VectorD* columnVector;
	int rowNum;
	int columnNum;
};
// Matrix calculations //
Matrix multiplication(Matrix A, Matrix B) {
	if (A.getcolumnNum() != B.getrowNum()) {
		Matrix C = Matrix();
		return C;
	}
	Matrix C = Matrix(A.getrowNum(), B.getcolumnNum());
	for (int i = 0; i < A.getrowNum(); i++) {
		for (int j = 0; j < B.getcolumnNum(); j++) {
			double c = 0;
			for (int k = 0; k < A.getcolumnNum(); k++) {
				c += (A.getVectorD()[k][i] * B.getVectorD()[j][k]);
			}
			C.setValue(i+1, j+1, c);
		}
	}
	return C;
}
Matrix multiplicationOneByOne(Matrix A, Matrix B) {
	if (A.getcolumnNum() != B.getcolumnNum() || A.getrowNum() != B.getrowNum()) {
		Matrix C = Matrix();
		return C;
	}
	Matrix C = Matrix(A.getrowNum(), B.getcolumnNum());
	for (int i = 0; i < A.getrowNum(); i++) {
		for (int j = 0; j < B.getcolumnNum(); j++) {
			double c = 0;	
			c += (A.getVectorD()[j][i] * B.getVectorD()[j][i]);
			C.setValue(i + 1, j + 1, c);
		}
	}
	return C;
}
Matrix addition(Matrix A, Matrix B) {
	if (A.getcolumnNum() != B.getcolumnNum()|| A.getrowNum() != B.getrowNum()) {
		Matrix C = Matrix();
		return C;
	}
	Matrix C = Matrix(A.getrowNum(), B.getcolumnNum());
	for (int i = 0; i < A.getrowNum(); i++) {
		for (int j = 0; j < B.getcolumnNum(); j++) {
			double c = 0;	
			c += (A.getVectorD()[j][i] + B.getVectorD()[j][i]);
			C.setValue(i + 1, j + 1, c);
		}
	}
	return C;
}
Matrix subtraction(Matrix A, Matrix B) {
	if (A.getcolumnNum() != B.getcolumnNum() || A.getrowNum() != B.getrowNum()) {
		Matrix C = Matrix();
		return C;
	}
	Matrix C = Matrix(A.getrowNum(), B.getcolumnNum());
	for (int i = 0; i < A.getrowNum(); i++) {
		for (int j = 0; j < B.getcolumnNum(); j++) {
			double c = 0;
			c += (A.getVectorD()[j][i] - B.getVectorD()[j][i]);
			C.setValue(i + 1, j + 1, c);
		}
	}
	return C;
}
Matrix transpose(Matrix A) {
	Matrix C = Matrix(A.getcolumnNum(), A.getrowNum());
	for (int i = 0; i < A.getrowNum(); i++) {
		for (int j = 0; j < A.getcolumnNum(); j++) {
			C.setValue(j + 1, i + 1, A.getVectorD()[j][i]);
		}
	}
	return C;
}
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
			C.setValue(i + 1, j + 1, (1 - exp(-2 * X.getVectorD()[j][i]))/(1 + exp(-2*X.getVectorD()[j][i])));
		}
	}
	return C;
}
Matrix ReLU(Matrix X) {
	Matrix C = Matrix(X.getrowNum(), X.getcolumnNum());
	for (int i = 0; i < C.getrowNum(); i++) {
		for (int j = 0; j < C.getcolumnNum(); j++) {
			if(X.getVectorD()[j][i]>=0)	{
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
/*
double Loss(Matrix& rightValue, Matrix& prediectValue) {
	Matrix lossMatrix = subtraction(rightValue, prediectValue);
	double loss = 0;
	for (int i = 0; i < lossMatrix.getrowNum(); i++) {
		for (int j = 0; j < lossMatrix.getcolumnNum(); j++) {
			loss += pow(lossMatrix.getVectorD()[j][i], 2) / (lossMatrix.getrowNum() * lossMatrix.getcolumnNum());
		}
	}
	return loss;
}
*/
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
/*
Matrix datlaSoftmax(Matrix &frontLayer, Matrix &weights, Matrix &target, Matrix &prediectValue) {
	Matrix error = subtraction(prediectValue, target);

	Matrix I = Matrix(prediectValue.getrowNum(), prediectValue.getcolumnNum());
	for (int i = 0; i < I.getrowNum(); i++) {
		for (int j = 0; j < I.getcolumnNum(); j++) {
			I.setValue(i + 1, j + 1, 1);
		}
	}
	Matrix softmaxDerivative = multiplicationOneByOne(prediectValue, subtraction(I, prediectValue));

	// 计算梯度
	Matrix gradient = multiplicationOneByOne(error, softmaxDerivative);

	// 计算 dW = gradient * frontLayer^T
	Matrix frontLayerTranspose = transpose(frontLayer);
	Matrix datla = multiplication(gradient, frontLayerTranspose);

	return datla;
}
Matrix datlaSigmoid(Matrix &thisLayer, Matrix &weightsfront, Matrix& weightsback, Matrix &backdatla, Matrix &frontLayer) {
	// 计算 Wback^T * dW
	Matrix WbackTranspose = transpose(weightsback);
	Matrix hiddenError = multiplication(WbackTranspose, backdatla);

	// 计算 sigmoid 导数
	Matrix sigmoidDerivative = Matrix(thisLayer.getrowNum(), thisLayer.getcolumnNum());
	for (int i = 0; i < sigmoidDerivative.getrowNum(); i++) {
		for (int j = 0; j < sigmoidDerivative.getcolumnNum(); j++) {
			double sigmoidVal = (1 + exp(-multiplication(weightsfront, frontLayer).getVectorD()[j][i]));
			sigmoidDerivative.setValue(i + 1, j + 1, sigmoidVal * (1 - sigmoidVal));
		}
	}

	// 计算隐藏层误差
	Matrix hiddenGradient = multiplicationOneByOne(hiddenError, sigmoidDerivative);

	// 计算 dWfront = hiddenGradient * X^T
	Matrix frontLayerTranspose = transpose(frontLayer);
	Matrix datla = multiplication(hiddenGradient, frontLayerTranspose);

	return datla;
}
Matrix update(Matrix& weights, Matrix& datla, double learnRate) {
	Matrix I = Matrix(datla.getrowNum(), datla.getcolumnNum());
	for (int i = 0; i < I.getrowNum(); i++) {
		for (int j = 0; j < I.getcolumnNum(); j++) {
			I.setValue(i + 1, j + 1, learnRate);
		}
	}
	return subtraction(weights, multiplicationOneByOne(datla, I));
}
*/
Matrix readTxtToMatrix(const string& filepath) {
	ifstream file(filepath);
	if (!file.is_open()) {
		cerr << "无法打开文件: " << filepath << endl;
		return Matrix(32 * 32, 1, 0.0);
	}
	Matrix X(32 * 32, 1, 0.0);
	string line;
	int row = 0;
	while (getline(file, line) && row < 32) {
		// 假设每行32个字符，无空格
		for (int col = 0; col < 32 && col < line.length(); col++) {
			char ch = line[col];
			double val = (ch == '1') ? 1.0 : 0.0;
			//cout << val;
			// 计算在X中的索引：行优先
			int idx = row * 32 + col;  // 0-based
			X.setValue(idx + 1, 1, val);  // setValue要求索引从1开始
		}
		//cout << endl;
		row++;
	}
	return X;
}

class NeuralNetwork {
public:
	NeuralNetwork(){}
	~NeuralNetwork(){}
	
	void train(const string& data_folder) {
		// 1. 加载所有训练样本
		loadTrainingData(data_folder);  // 填充 train_samples

		int epochs = 30;
		int num_samples = train_samples.size();
		cout << "共加载 " << num_samples << " 个训练样本" << endl;
		//loadModel(3);

		for (int epoch = 0; epoch < epochs; epoch++) {
			// 打乱顺序
			shuffle(train_samples.begin(), train_samples.end(), default_random_engine(random_device()()));

			double total_loss = 0.0;
			int correct = 0;

			for (int i = 0; i < num_samples; i++) {
				int label = train_samples[i].first;
				string filepath = train_samples[i].second;

				// 读取图片数据
				Matrix X = readTxtToMatrix(filepath);

				// 设置网络输入和目标
				layerNeuron[0] = X;
				setTarget(label);

				// 前向传播
				forward();
				total_loss += loss;

				// 计算准确率（可选）
				Matrix& output = layerNeuron[layerNum - 1];
				int predict = 0;
				double max_val = output.getVectorD()[0][0];
				for (int j = 1; j < 10; j++) {
					if (output.getVectorD()[0][j] > max_val) {
						max_val = output.getVectorD()[0][j];
						predict = j;
					}
				}
				if (predict == label) correct++;
				output.show("output is");

				// 反向传播
				backward();
				

				// 可选：每一定步数打印进度
				if ((i + 1) % 500 == 0) {
					cout << "Epoch " << epoch + 1 << ", iter " << i + 1 << "/" << num_samples
						<< ", loss: " << loss << endl;
				}
			}

			double avg_loss = total_loss / num_samples;
			double acc = 100.0 * correct / num_samples;
			cout << "Epoch " << epoch + 1 << " 完成, 平均 loss: " << avg_loss << ", 准确率: " << acc << "%" << endl;
		}
		saveModel(3);
	}
	void train() {
		// 假设已调用 loadTrainingData("path/to/txt/folder");
		int epochs = 30;
		int num_samples = train_samples.size();
		for (int epoch = 0; epoch < epochs; epoch++) {
			double total_loss = 0;
			int correct = 0;
			// 每个epoch打乱顺序（可选）
			shuffle(train_samples.begin(), train_samples.end(), default_random_engine(random_device()()));
			for (int idx = 0; idx < num_samples; idx++) {
				int label = train_samples[idx].first;
				string filepath = train_samples[idx].second;

				// 读取txt文件，构造输入矩阵 X (1024,1)
				Matrix X = readTxtToMatrix(filepath);  // 需要实现该函数

				// 设置输入层
				layerNeuron[0] = X;
				// 设置目标
				setTarget(label);

				// 前向传播
				forward();
				total_loss += loss;

				// 计算预测正确性（可选）
				Matrix& output = layerNeuron[layerNum - 1];
				int predict = 0;
				double max_prob = output.getVectorD()[0][0];
				for (int j = 1; j < 10; j++) {
					if (output.getVectorD()[0][j] > max_prob) {
						max_prob = output.getVectorD()[0][j];
						predict = j;
					}
				}
				if (predict == label) correct++;

				// 反向传播
				backward();
			}
			cout << "Epoch " << epoch + 1 << ", loss: " << total_loss / num_samples
				<< ", accuracy: " << 100.0 * correct / num_samples << "%" << endl;
		}
	}
	void test() {
		int right;
		cout << "你想要哪个数字结果:";
		cin >> right;
		setTarget(right);
		const char* filename = "testnum.png";
		// 加载PNG文件
		std::vector<unsigned char> image;
		unsigned width, height;
		unsigned error = lodepng::decode(image, width, height, filename, LCT_GREY, 8);
		if (error) {
			std::cerr << "解码错误 " << error << ": " << lodepng_error_text(error) << std::endl;
			return ;
		}
		Matrix png = Matrix(height, width);
		cout << "数字图像为：" << endl;
		// 输出二值化像素
		for (unsigned y = 0; y < height; ++y) {
			for (unsigned x = 0; x < width; ++x) {
				// 获取灰度值 (0-255)
				unsigned char pixel = image[y * width + x];
				// 二值化：大于128为1，否则为0
				int binary = (pixel > 128) ? 0 : 1;
				png.setValue(y, x, binary);
				cout << binary;
			}
			cout << std::endl;
		}
		double loss = 0;
		Matrix X(32 * 32, 1);
		for (int i = 1; i <= 32; i++) {
			for (int j = 1; j <= 32; j++) {
				int input = png.getVectorD()[j - 1][i - 1];
				X.setValue((i - 1) * 32 + j, 1, input);
			}
		}
		layerNeuron[0] = X;
		loadModel(3);

		forward();
		layerNeuron[2].show("predict");
		double maxPredict = layerNeuron[2].getVectorD()[0][0];
		int maxPredictNum = 0;
		for (int i = 0; i < layerNeuron[2].getrowNum(); i++) {
			if (layerNeuron[2].getVectorD()[0][i] >= maxPredict) {
				maxPredict = layerNeuron[2].getVectorD()[0][i];
				maxPredictNum = i;
			}
		}
		cout << endl<<"我认为这是" << maxPredictNum << endl;
	}
	void test(const string filename) {
		cout << "输入label：";
		int label;
		cin >> label;
		// 读取图片数据
		Matrix X = readTxtToMatrix(filename);

		// 设置网络输入和目标
		layerNeuron[0] = X;
		setTarget(label);
		loadModel(3);

		forward();
		layerNeuron[2].show("predict");
	}
	// 在 NeuralNetwork 类中添加 predict 方法
	int predict(const Matrix& input) {
		layerNeuron[0] = input;
		for (int i = 0; i < layerNum - 1; i++) {
			Matrix z = multiplication(weights[i], layerNeuron[i]);
			z = addition(z, bias[i]);
			if (i == layerNum - 2) {
				layerNeuron[i + 1] = softmax(z);
			}
			else {
				layerNeuron[i + 1] = sigmoid(z);
			}
		}
		// 找到概率最大的类别
		Matrix& output = layerNeuron[layerNum - 1];
		int best = 0;
		double maxval = output.getVectorD()[0][0];
		for (int i = 1; i < 10; ++i) {
			if (output.getVectorD()[0][i] > maxval) {
				maxval = output.getVectorD()[0][i];
				best = i;
			}
		}
		return best;
	}
	void forward() {
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
		loss = Loss(target, layerNeuron[layerNum - 1]);
		cout <<endl<< "loss is "<<loss << endl;
	}
	void backward() {
		// 输出层误差 (10,1)
		Matrix delta_out = subtraction(layerNeuron[layerNum - 1], target);

		// 输出层权重梯度 dW2 = delta_out * a_hidden^T
		Matrix a_hidden = layerNeuron[layerNum - 2];  // (128,1)
		Matrix dW2 = multiplication(delta_out, transpose(a_hidden));  // (10,128)
		double testdW2sum = 0;
		for (int i = 0; i < dW2.getrowNum(); i++) {
			for (int j = 0; j < dW2.getcolumnNum(); j++) {
				testdW2sum += pow(dW2.getVectorD()[j][i],2);
			}
		}
		//cout << endl << "dw2: "<<testdW2sum << endl;
		//dW2.show("梯度");

		// 输出层偏置梯度 db2 = delta_out
		Matrix db2 = delta_out;

		// 隐藏层误差 delta_hidden = (W2^T * delta_out) ⊙ sigmoid'(z_hidden)
		Matrix W2T = transpose(weights[layerNum - 2]);  // (128,10)
		Matrix delta_hidden_temp = multiplication(W2T, delta_out);  // (128,1)

		// 计算 sigmoid 导数：a_hidden * (1 - a_hidden)
		Matrix sigmoid_deriv(a_hidden.getrowNum(), 1);
		for (int i = 0; i < a_hidden.getrowNum(); i++) {
			double val = a_hidden.getVectorD()[0][i];
			sigmoid_deriv.setValue(i + 1, 1, val * (1 - val));
		}
		Matrix delta_hidden = multiplicationOneByOne(delta_hidden_temp, sigmoid_deriv);  // (128,1)

		// 隐藏层权重梯度 dW1 = delta_hidden * X^T
		Matrix X = layerNeuron[0];  // (1024,1)
		Matrix dW1 = multiplication(delta_hidden, transpose(X));  // (128,1024)
		double testdW1sum = 0;
		for (int i = 0; i < dW1.getrowNum(); i++) {
			for (int j = 0; j < dW1.getcolumnNum(); j++) {
				testdW1sum += pow(dW1.getVectorD()[j][i],2);
			}
		}
		//cout << endl <<"dw1: "<< testdW1sum << endl;
		//dW1.show("梯度");

		// 隐藏层偏置梯度 db1 = delta_hidden
		Matrix db1 = delta_hidden;

		// 更新权重和偏置（使用标量学习率）
		double lr = learningRate;
		// 更新输出层
		for (int r = 0; r < dW2.getrowNum(); r++) {
			for (int c = 0; c < dW2.getcolumnNum(); c++) {
				double grad = dW2.getVectorD()[c][r];  // 注意列优先索引
				double new_w = weights[layerNum - 2].getVectorD()[c][r] - lr * grad;
				weights[layerNum - 2].setValue(r + 1, c + 1, new_w);
			}
		}
		// 更新输出层偏置
		for (int r = 0; r < db2.getrowNum(); r++) {
			double new_b = bias[layerNum - 2].getVectorD()[0][r] - lr * db2.getVectorD()[0][r];
			bias[layerNum - 2].setValue(r + 1, 1, new_b);
		}

		// 更新隐藏层
		for (int r = 0; r < dW1.getrowNum(); r++) {
			for (int c = 0; c < dW1.getcolumnNum(); c++) {
				double grad = dW1.getVectorD()[c][r];
				double new_w = weights[0].getVectorD()[c][r] - lr * grad;
				weights[0].setValue(r + 1, c + 1, new_w);
			}
		}
		// 更新隐藏层偏置
		for (int r = 0; r < db1.getrowNum(); r++) {
			double new_b = bias[0].getVectorD()[0][r] - lr * db1.getVectorD()[0][r];
			bias[0].setValue(r + 1, 1, new_b);
		}
	}
	void initNet(int llayerNum, double llearningRate, double llossThreshold) {
		layerNum = llayerNum;
		learningRate = llearningRate;
		lossThreshold = llossThreshold;
		layerNeuron.resize(layerNum);
		weights.resize(layerNum - 1);
		bias.resize(layerNum - 1);

		// 假设网络结构：输入层(1024) -> 隐藏层(128) -> 输出层(10)
		layerNeuron[0] = Matrix(32 * 32, 1, 0.0);        // 输入层
		layerNeuron[1] = Matrix(128, 1, 0.0);          // 隐藏层（可调整）
		layerNeuron[2] = Matrix(10, 1, 0.0);            // 输出层

		// 初始化权重（使用 Xavier 初始化）
		for (int i = 0; i < layerNum - 1; i++) {
			initWeights(i);
			initBias(i);
		}
		initTarget();
	}
	void initlayerNeuron(int llayerNumLabel,int layerNeuronNum) {
		if (llayerNumLabel == 1) {
			layerNeuron[0] = Matrix(32 * 32, 1, 0);
		}
		else if(llayerNumLabel == layerNum) {
			layerNeuron[layerNum - 1] = Matrix(10, 1, 0);
		}
		else {
			layerNeuron[llayerNumLabel] = Matrix(layerNeuronNum, layerNeuron[llayerNumLabel - 1].getcolumnNum(), 0);
		}
	}
	void initWeights(int layerIdx, bool random = true) {
		int n_in = layerNeuron[layerIdx].getrowNum();
		int n_out = layerNeuron[layerIdx + 1].getrowNum();
		double limit = sqrt(6.0 / (n_in + n_out))/10;  // Xavier均匀分布范围
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
	void initTarget() {
		target = Matrix(10, 1, 0);
	}
	void setTarget(int label){
		cout << endl<<"target label is "<<label << endl;
		initTarget();
		target.setValue(label+1, 1, 1);
		target.show("target is:");
	}
	void saveModel(int llayerNum) {
		for (int i = 0; i <= llayerNum - 2; i++) {
			string fileName = "weight" + to_string(i) + ".txt";
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
	void loadModel(int llayerNum) {
		for(int i=0;i<= llayerNum-2;i++){
			string fileName = "weight" + to_string(i) + ".txt";
			ifstream inWeightsFile;
			double w;
			inWeightsFile.open(fileName);
			for (int k = 0; k < weights[i].getrowNum(); k++) {
				for (int j = 0; j < weights[i].getcolumnNum(); j++) {
					inWeightsFile >> w;
					weights[i].setValue(k+1, j+1, w);
				}
			}
			inWeightsFile.close();
		}
	}
	// 在 NeuralNetwork 类中添加成员：
	vector<pair<int, string>> train_samples;  // 存储 (label, filepath)
	int current_epoch_sample_index;  // 用于顺序读取（可选）

	void loadTrainingData(const string& folder_path) {
		train_samples.clear();
		for (const auto& entry : filesystem::directory_iterator(folder_path)) {
			if (entry.is_regular_file() && entry.path().extension() == ".txt") {
				string filename = entry.path().filename().string();
			//	cout << endl << "文件名：" << filename << endl;
				// 解析文件名：假设格式为 "数字_编号.txt"
				size_t underscore_pos = filename.find('_');
				if (underscore_pos == string::npos) continue;
				int label = stoi(filename.substr(0, underscore_pos));
			//	cout << endl << "label：" << label << endl;
				if (label < 0 || label > 9) continue;  // 确保标签有效
				train_samples.push_back({ label, entry.path().string() });
			}
		}
		// 打乱样本顺序（可选，用于epoch）
		shuffle(train_samples.begin(), train_samples.end(), default_random_engine(random_device()()));
	}

public:
	int layerNum; //包括输入层X，隐藏层Hi，输出层Y，取>=2
	vector<int>layerNumLabel;//数值取取>=1
	vector<Matrix>layerNeuron;
	vector<Matrix>weights;
	vector<Matrix>bias;//偏置即Y=WX+b里的b
	Matrix target;
	double learningRate;
	double loss;
	double lossThreshold;
};


int main(){
	// 创建神经网络并加载训练好的权重
	NeuralNetwork nn;
	nn.initNet(3, 0.01, 0.01);
	nn.loadModel(3);

	srand(time(0));

	sf::RenderWindow window(sf::VideoMode(640, 640), "AI guess number");
	sf::Texture text;
	text.loadFromFile("window.png");
	sf::Sprite myWindow(text);
	myWindow.setTextureRect(sf::IntRect(0, 0, 640, 640));

	// 文本对象
	sf::Text text1inform, text2Clear, text3Submit, text4AI;
	sf::Font font;
	font.loadFromFile("arial.ttf");
	text1inform.setFont(font);
	text2Clear.setFont(font);
	text3Submit.setFont(font);
	text4AI.setFont(font);

	text1inform.setString("draw the num\nin the \nwhite area");
	text1inform.setCharacterSize(48);
	text1inform.setFillColor(sf::Color::Black);
	text1inform.setStyle(sf::Text::Bold);
	text1inform.setPosition(40, 40);

	text2Clear.setString("clear \nthe area");
	text2Clear.setCharacterSize(48);
	text2Clear.setFillColor(sf::Color::Red);
	text2Clear.setStyle(sf::Text::Bold);
	text2Clear.setPosition(380, 480);

	text3Submit.setString("submit \nthe num");
	text3Submit.setCharacterSize(48);
	text3Submit.setFillColor(sf::Color::Green);
	text3Submit.setStyle(sf::Text::Bold);
	text3Submit.setPosition(380, 300);

	text4AI.setString("I guess:\nis ?");   // 初始显示
	text4AI.setCharacterSize(48);
	text4AI.setFillColor(sf::Color::Black);
	text4AI.setStyle(sf::Text::Bold);
	text4AI.setPosition(380, 40);

	// 创建绘制区域 (320x320)
	sf::RenderTexture drawTexture;
	drawTexture.create(320, 320);
	drawTexture.clear(sf::Color::White);
	drawTexture.display();
	sf::Sprite drawSprite(drawTexture.getTexture());
	drawSprite.setPosition(20, 300);

	// 32x32 绘制网格，0=空白，1=笔迹
	int drawingGrid[32][32] = { 0 };

	bool isDrawing = false;   // 鼠标左键是否按下

	while (window.isOpen()) {
		sf::Event gameEvent;
		while (window.pollEvent(gameEvent)) {
			if (gameEvent.type == sf::Event::Closed) {
				window.close();
			}

			// 鼠标左键按下
			if (gameEvent.type == sf::Event::MouseButtonPressed &&
				gameEvent.mouseButton.button == sf::Mouse::Left) {
				sf::Vector2i pos = sf::Mouse::getPosition(window);
				int x = pos.x;
				int y = pos.y;

				// 检查按钮区域
				if (x >= 380 && x <= 620 && y >= 300 && y <= 460) {
					// Submit 按钮：进行预测
					// 将 drawingGrid 转换为输入矩阵
					Matrix input(32 * 32, 1, 0.0);
					for (int row = 0; row < 32; ++row) {
						for (int col = 0; col < 32; ++col) {
							double val = drawingGrid[row][col] ? 1.0 : 0.0;
							int idx = row * 32 + col; // 行优先
							input.setValue(idx + 1, 1, val);
						}
					}
					int result = nn.predict(input);
					text4AI.setString("I guess:\nis " + std::to_string(result));
				}
				else if (x >= 380 && x <= 620 && y >= 480 && y <= 620) {
					// Clear 按钮：清空绘制区域
					drawTexture.clear(sf::Color::White);
					drawTexture.display();
					memset(drawingGrid, 0, sizeof(drawingGrid));
					// 不清除 AI 结果，可保留上次预测
				}
				else if (x >= 20 && x <= 340 && y >= 300 && y <= 620) {
					// 开始绘制
					isDrawing = true;
					// 立即绘制当前点
					int localX = x - 20;
					int localY = y - 300;
					// 在 drawTexture 上画圆
					sf::CircleShape circle(25);
					circle.setFillColor(sf::Color::Black);
					circle.setPosition(localX - 5, localY - 5);
					drawTexture.draw(circle);
					drawTexture.display();

					// 更新 drawingGrid：标记鼠标所在格子及周围 8 邻域
					int gx = localX / 10;
					int gy = localY / 10;
					for (int dy = -1; dy <= 1; ++dy) {
						for (int dx = -1; dx <= 1; ++dx) {
							int nx = gx + dx;
							int ny = gy + dy;
							if (nx >= 0 && nx < 32 && ny >= 0 && ny < 32) {
								drawingGrid[ny][nx] = 1;
							}
						}
					}
				}
			}

			// 鼠标左键释放
			if (gameEvent.type == sf::Event::MouseButtonReleased &&
				gameEvent.mouseButton.button == sf::Mouse::Left) {
				isDrawing = false;
			}

			// 鼠标移动
			if (gameEvent.type == sf::Event::MouseMoved && isDrawing) {
				sf::Vector2i pos = sf::Mouse::getPosition(window);
				int x = pos.x;
				int y = pos.y;
				if (x >= 20 && x <= 340 && y >= 300 && y <= 620) {
					int localX = x - 20;
					int localY = y - 300;
					// 画圆
					sf::CircleShape circle(25);
					circle.setFillColor(sf::Color::Black);
					circle.setPosition(localX - 5, localY - 5);
					drawTexture.draw(circle);
					drawTexture.display();

					// 更新 drawingGrid
					int gx = localX / 10;
					int gy = localY / 10;
					for (int dy = -1; dy <= 1; ++dy) {
						for (int dx = -1; dx <= 1; ++dx) {
							int nx = gx + dx;
							int ny = gy + dy;
							if (nx >= 0 && nx < 32 && ny >= 0 && ny < 32) {
								drawingGrid[ny][nx] = 1;
							}
						}
					}
				}
			}
		}

		// 绘制界面
		window.clear(sf::Color::White);
		window.draw(myWindow);
		window.draw(text1inform);
		window.draw(text2Clear);
		window.draw(text3Submit);
		window.draw(drawSprite);
		window.draw(text4AI);
		window.display();
	}

	return 0;

	/*
	while (1) {
		nn.test();
	}
	//nn.train("trainingDigits");
	while (1) {
		cout << "输入文件名：";
		cin >> input;
		cout << endl;
		nn.test(input);
	}
	*/
}

