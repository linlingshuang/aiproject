#pragma once
#include <vector>
#include <fstream>
#include "VectorD.h"
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
	Matrix(int rrowNum, int ccolumnNum, double value = 0) {
		if (rrowNum <= 0) rrowNum = 1;
		if (ccolumnNum <= 0) ccolumnNum = 1;
		rowNum = rrowNum;
		columnNum = ccolumnNum;
		// 分配内存
		columnVector = static_cast<VectorD*>(operator new[](sizeof(VectorD)* columnNum));
		// 直接构造对象
		for (int i = 0; i < columnNum; i++) {
			new (&columnVector[i]) VectorD(rowNum, value);
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


	void setValue(int i, int j, double num) { //注意这里i、j范围要求>0
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
	VectorD* getVectorD() { return columnVector; }
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
			C.setValue(i + 1, j + 1, c);
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
	if (A.getcolumnNum() != B.getcolumnNum() || A.getrowNum() != B.getrowNum()) {
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