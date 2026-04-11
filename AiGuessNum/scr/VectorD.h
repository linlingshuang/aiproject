#pragma once
#include <iostream>
using namespace std;
class VectorD {
public:
	VectorD() {
		length = 1;
		vectorHead = new double[1];
		initialValue(0);

	}
	VectorD(int llength, double value = 0) {
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
	void setValue(int i, double num) {
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
	double& operator[](int i) {
		return vectorHead[i];
	}

	void show(const char testname[] = "testname\0") {
		cout << endl;
		cout << *testname;
		cout << endl;
		for (int i = 0; i < length; i++) {
			cout << vectorHead[i] << ' ';
		}
		cout << endl;
	}

private:
	double* vectorHead;
	int length;
};
