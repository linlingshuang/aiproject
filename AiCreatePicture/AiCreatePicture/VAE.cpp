#include "VAE.h"
#include "NeuralNetwork.h"


VAE::VAE(int input_dim, int hidden_dim1, int hidden_dim2, int latent_dim) {
	this->input_dim = input_dim;
	this->latent_dim = latent_dim;
	init_weights(W_e1, input_dim, hidden_dim1);
	init_bias(b_e1, hidden_dim1);
	init_weights(W_e2, hidden_dim1, hidden_dim2);
	init_bias(b_e2, hidden_dim2);
	init_weights(W_mu, hidden_dim2, latent_dim);
	init_bias(b_mu, latent_dim);
	init_weights(W_logvar, hidden_dim2, latent_dim);
	init_bias(b_logvar, latent_dim);

	init_weights(W_d1, latent_dim, hidden_dim2);
	init_bias(b_d1, hidden_dim2);
	init_weights(W_d2, hidden_dim2, hidden_dim1);
	init_bias(b_d2, hidden_dim1);
	init_weights(W_d3, hidden_dim1, input_dim);
	init_bias(b_d3, input_dim);
	layerNeuron.resize(7);
}

VAE::~VAE() {
	// 析构函数，Matrix 会自动管理内存
}

void VAE::init_weights(Matrix& W, int n_in, int n_out) {
    double limit = sqrt(6.0 / (n_in + n_out));
    W = Matrix(n_out, n_in, 0.0);
    for (int r = 0; r < n_out; ++r) {
        for (int c = 0; c < n_in; ++c) {
            double val = (rand() / (double)RAND_MAX) * 2 * limit - limit;
            W.setValue(r + 1, c + 1, val);
        }
    }
}
void VAE::init_bias(Matrix& b, int n_out) {
        b = Matrix(n_out, 1, 0.0);   // 全 0
        // 或者给一个很小的正数，比如 0.01，避免 ReLU 死亡
        // b = Matrix(n_out, 1, 0.01);
}
void VAE::forward(const Matrix& x, Matrix& recon, Matrix& mu, Matrix& logvar, Matrix& epsilon) {
    // ---------- 编码器 ----------
	encode(x, mu, logvar);
    // ---------- 重参数化 ----------
    Matrix z = reparameterize(mu, logvar, epsilon);
	layerNeuron[3] = z;
    // ---------- 解码器 ----------
    recon = decode(z);
}
void VAE::encode(const Matrix& x, Matrix& mu, Matrix& logvar) {
    Matrix h1 = addition(multiplication(W_e1, x), b_e1);
    h1 = ReLU(h1);   // 使用你已有的 ReLU
	layerNeuron[1] = h1;

    Matrix h2 = addition(multiplication(W_e2, h1), b_e2);
    h2 = ReLU(h2);
	layerNeuron[2] = h2;

    mu = addition(multiplication(W_mu, h2), b_mu);
    logvar = addition(multiplication(W_logvar, h2), b_logvar);
}
Matrix VAE::reparameterize(const Matrix& mu, const Matrix& logvar, Matrix &epsilon) {
    // 计算标准差 sigma = exp(0.5 * logvar)
    Matrix sigma(mu.getrowNum(), 1);
    for (int i = 0; i < mu.getrowNum(); ++i) {
        double lv = logvar.getVectorD()[0][i];
        double s = exp(0.5 * lv);
        sigma.setValue(i + 1, 1, s);
    }
	epsilon= Matrix(mu.getrowNum(), 1);
    // 生成标准正态随机数（使用静态随机数生成器）
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::normal_distribution<> dist(0.0, 1.0);
   
    for (int i = 0; i < mu.getrowNum(); ++i) {
        epsilon.setValue(i + 1, 1, dist(gen));
    }

    // z = mu + sigma * epsilon
    Matrix z = addition(mu, multiplicationOneByOne(sigma, epsilon));
    return z;
}
Matrix VAE::decode(const Matrix& z) {
    Matrix h2 = addition(multiplication(W_d1, z), b_d1);
    h2 = ReLU(h2);
	layerNeuron[4] = h2;

    Matrix h1 = addition(multiplication(W_d2, h2), b_d2);
    h1 = ReLU(h1);
	layerNeuron[5] = h1;

    Matrix out = addition(multiplication(W_d3, h1), b_d3);
	layerNeuron[6] = out;

    out = sigmoid(out);  // 输出范围 (0,1)
    return out;
}
double VAE::loss(const Matrix& x, const Matrix& recon, const Matrix& mu, const Matrix& logvar) {
    // 重构损失（BCE）
    double recon_loss = 0.0;
    for (int i = 0; i < input_dim; ++i) {
        double xi = x.getVectorD()[0][i];
        double xhat = recon.getVectorD()[0][i];
        // 确保 xhat 在合理范围内
        xhat = max(1e-10, min(1.0 - 1e-10, xhat));
        recon_loss -= xi * log(xhat) + (1 - xi) * log(1 - xhat);
    }

    // KL 散度
    double kl = 0.0;
    for (int j = 0; j < latent_dim; ++j) {
        double mu_j = mu.getVectorD()[0][j];
        double logvar_j = logvar.getVectorD()[0][j];
        // 防止 exp 溢出
        double exp_logvar = exp(min(logvar_j, 20.0));
        kl += 1 + logvar_j - mu_j * mu_j - exp_logvar;
    }
    kl *= -0.5;

    // 确保损失值不是 NaN 或 Inf
    if (isnan(recon_loss) || isinf(recon_loss)) {
        recon_loss = 1e10;
    }
    if (isnan(kl) || isinf(kl)) {
        kl = 1e10;
    }

    return recon_loss + kl;
}
double VAE::train_step(const pair<int, string> xOrigin, double learning_rate) {
	// 1. 前向：得到 recon, mu, logvar
	int label = xOrigin.first;
	string filepath = xOrigin.second;
	// 读取图片数据
	Matrix x = readTxtToMatrix(filepath);
	layerNeuron[0] = x;

	Matrix recon, mu, logvar, epsilon;
	forward(x, recon, mu, logvar, epsilon);
	double L = loss(x, recon, mu, logvar);

	// 2. 反向传播
	Matrix z = layerNeuron[3];
	backward(x, recon, mu, logvar, z, epsilon, learning_rate);

	return L;
}
static void save_matrix(const Matrix& M, const std::string& num) {
	string fileName = num;
	ofstream outWeightsFile;
	double w;
	outWeightsFile.open(fileName);
	for (int k = 0; k < M.getrowNum(); k++) {
		for (int j = 0; j < M.getcolumnNum(); j++) {
			w = M.getVectorD()[j][k];
			outWeightsFile << w << ' ';
		}
		outWeightsFile << endl;
	}
	outWeightsFile.close();
}

static void load_matrix(Matrix& M, const std::string& num) {
	string fileName = num;
	ifstream inWeightsFile;
	double w;
	inWeightsFile.open(fileName);
	if (!inWeightsFile.is_open()) {
		cerr << "无法打开文件: " << fileName << endl;
		return;
	}
	for (int k = 0; k < M.getrowNum(); k++) {
		for (int j = 0; j < M.getcolumnNum(); j++) {
			inWeightsFile >> w;
			M.setValue(k + 1, j + 1, w);
		}
	}
	inWeightsFile.close();
}

void VAE::save(const std::string& prefix)const {
	save_matrix(W_e1, prefix + "_We1.txt");
	save_matrix(b_e1, prefix + "_be1.txt");
	save_matrix(W_e2, prefix + "_We2.txt");
	save_matrix(b_e2, prefix + "_be2.txt");
	save_matrix(W_mu, prefix + "_Wmu.txt");
	save_matrix(b_mu, prefix + "_bmu.txt");
	save_matrix(W_logvar, prefix + "_Wlogvar.txt");
	save_matrix(b_logvar, prefix + "_blogvar.txt");

	save_matrix(W_d1, prefix + "_Wd1.txt");
	save_matrix(b_d1, prefix + "_bd1.txt");
	save_matrix(W_d2, prefix + "_Wd2.txt");
	save_matrix(b_d2, prefix + "_bd2.txt");
	save_matrix(W_d3, prefix + "_Wd3.txt");
	save_matrix(b_d3, prefix + "_bd3.txt");
}

void VAE::backward(const Matrix& x, const Matrix& recon, const Matrix& mu, const Matrix& logvar, const Matrix& z, const Matrix& epsilon, double learning_rate) {
	// 1. 计算损失对解码器输出的导数（在sigmoid之前）
	// dL/dlogit = p - x
	Matrix dL_dlogit = subtraction(recon, x);
	
	// 2. 解码器反向传播
	// 解码器输出层权重梯度 dW_d3 = dL_dlogit * h1^T
	Matrix h1 = layerNeuron[5];
	Matrix dW_d3 = multiplication(dL_dlogit, transpose(h1));
	Matrix db_d3 = dL_dlogit;
	
	// 解码器隐藏层1误差 delta_h1 = (W_d3^T * dL_dlogit) ⊙ ReLU'(h1)
	Matrix W_d3T = transpose(W_d3);
	Matrix delta_h1_temp = multiplication(W_d3T, dL_dlogit);
	// 计算 ReLU 导数
	Matrix ReLU_deriv_h1(h1.getrowNum(), 1);
	for (int i = 0; i < h1.getrowNum(); i++) {
		double val = h1.getVectorD()[0][i];
		ReLU_deriv_h1.setValue(i + 1, 1, val > 0 ? 1 : 0);
	}
	Matrix delta_h1 = multiplicationOneByOne(delta_h1_temp, ReLU_deriv_h1);
	
	// 解码器隐藏层1权重梯度 dW_d2 = delta_h1 * h2^T
	Matrix h2 = layerNeuron[4];
	Matrix dW_d2 = multiplication(delta_h1, transpose(h2));
	Matrix db_d2 = delta_h1;
	
	// 解码器隐藏层2误差 delta_h2 = (W_d2^T * delta_h1) ⊙ ReLU'(h2)
	Matrix W_d2T = transpose(W_d2);
	Matrix delta_h2_temp = multiplication(W_d2T, delta_h1);
	// 计算 ReLU 导数
	Matrix ReLU_deriv_h2(h2.getrowNum(), 1);
	for (int i = 0; i < h2.getrowNum(); i++) {
		double val = h2.getVectorD()[0][i];
		ReLU_deriv_h2.setValue(i + 1, 1, val > 0 ? 1 : 0);
	}
	Matrix delta_h2 = multiplicationOneByOne(delta_h2_temp, ReLU_deriv_h2);
	
	// 解码器隐藏层2权重梯度 dW_d1 = delta_h2 * z^T
	Matrix dW_d1 = multiplication(delta_h2, transpose(z));
	Matrix db_d1 = delta_h2;
	
	// 3. 损失对隐变量 z 的导数
	Matrix dL_dz = delta_h2;
	
	// 4. 计算损失对编码器输出的导数
	// 计算 sigma = exp(0.5 * logvar)
	Matrix sigma(mu.getrowNum(), 1);
	Matrix sigma_squared(mu.getrowNum(), 1);
	for (int i = 0; i < mu.getrowNum(); ++i) {
		double lv = logvar.getVectorD()[0][i];
		double s = exp(0.5 * lv);
		sigma.setValue(i + 1, 1, s);
		sigma_squared.setValue(i + 1, 1, s * s);
	}
	
	// 计算梯度 mu: g_mu = dL/dz + mu
	Matrix grad_mu = addition(dL_dz, mu);
	
	// 计算梯度 logvar: g_logvar = 0.5 * (dL/dz * epsilon * sigma - (1 - sigma^2))
	Matrix temp1 = multiplicationOneByOne(dL_dz, epsilon);
	Matrix temp2 = multiplicationOneByOne(temp1, sigma);
	Matrix temp3 = subtraction(1, sigma_squared);
	Matrix temp4 = subtraction(temp2, temp3);
	Matrix grad_logvar = multiplicationScalar(temp4, 0.5);
	
	// 5. 编码器反向传播
	// 编码器输出层（mu和logvar）的误差
	// 合并mu和logvar的梯度
	Matrix encoder_output_error = addition(grad_mu, grad_logvar);
	
	// 编码器隐藏层2误差 delta_enc_h2 = (W_mu^T * grad_mu + W_logvar^T * grad_logvar) ⊙ ReLU'(h2_enc)
	Matrix h2_enc = layerNeuron[2];
	Matrix W_muT = transpose(W_mu);
	Matrix W_logvarT = transpose(W_logvar);
	Matrix delta_enc_h2_temp1 = multiplication(W_muT, grad_mu);
	Matrix delta_enc_h2_temp2 = multiplication(W_logvarT, grad_logvar);
	Matrix delta_enc_h2_temp = addition(delta_enc_h2_temp1, delta_enc_h2_temp2);
	// 计算 ReLU 导数
	Matrix ReLU_deriv_enc_h2(h2_enc.getrowNum(), 1);
	for (int i = 0; i < h2_enc.getrowNum(); i++) {
		double val = h2_enc.getVectorD()[0][i];
		ReLU_deriv_enc_h2.setValue(i + 1, 1, val > 0 ? 1 : 0);
	}
	Matrix delta_enc_h2 = multiplicationOneByOne(delta_enc_h2_temp, ReLU_deriv_enc_h2);
	
	// 编码器隐藏层2权重梯度 dW_e2 = delta_enc_h2 * h1_enc^T
	Matrix h1_enc = layerNeuron[1];
	Matrix dW_e2 = multiplication(delta_enc_h2, transpose(h1_enc));
	Matrix db_e2 = delta_enc_h2;
	
	// 编码器隐藏层1误差 delta_enc_h1 = (W_e2^T * delta_enc_h2) ⊙ ReLU'(h1_enc)
	Matrix W_e2T = transpose(W_e2);
	Matrix delta_enc_h1_temp = multiplication(W_e2T, delta_enc_h2);
	// 计算 ReLU 导数
	Matrix ReLU_deriv_enc_h1(h1_enc.getrowNum(), 1);
	for (int i = 0; i < h1_enc.getrowNum(); i++) {
		double val = h1_enc.getVectorD()[0][i];
		ReLU_deriv_enc_h1.setValue(i + 1, 1, val > 0 ? 1 : 0);
	}
	Matrix delta_enc_h1 = multiplicationOneByOne(delta_enc_h1_temp, ReLU_deriv_enc_h1);
	
	// 编码器隐藏层1权重梯度 dW_e1 = delta_enc_h1 * x^T
	Matrix dW_e1 = multiplication(delta_enc_h1, transpose(x));
	Matrix db_e1 = delta_enc_h1;
	
	// 编码器输出层权重梯度
	Matrix dW_mu = multiplication(grad_mu, transpose(h2_enc));
	Matrix db_mu = grad_mu;
	Matrix dW_logvar = multiplication(grad_logvar, transpose(h2_enc));
	Matrix db_logvar = grad_logvar;
	
	// 6. 更新权重和偏置
	double lr = learning_rate;
	
	// 更新解码器权重和偏置
	update_weights(W_d3, dW_d3, lr);
	update_biases(b_d3, db_d3, lr);
	update_weights(W_d2, dW_d2, lr);
	update_biases(b_d2, db_d2, lr);
	update_weights(W_d1, dW_d1, lr);
	update_biases(b_d1, db_d1, lr);
	
	// 更新编码器权重和偏置
	update_weights(W_e1, dW_e1, lr);
	update_biases(b_e1, db_e1, lr);
	update_weights(W_e2, dW_e2, lr);
	update_biases(b_e2, db_e2, lr);
	update_weights(W_mu, dW_mu, lr);
	update_biases(b_mu, db_mu, lr);
	update_weights(W_logvar, dW_logvar, lr);
	update_biases(b_logvar, db_logvar, lr);
}

// 辅助函数：更新权重
void VAE::update_weights(Matrix& W, const Matrix& dW, double lr) {
	for (int r = 0; r < W.getrowNum(); r++) {
		for (int c = 0; c < W.getcolumnNum(); c++) {
			double grad = dW.getVectorD()[c][r];
			double new_w = W.getVectorD()[c][r] - lr * grad;
			W.setValue(r + 1, c + 1, new_w);
		}
	}
}

// 辅助函数：更新偏置
void VAE::update_biases(Matrix& b, const Matrix& db, double lr) {
	for (int r = 0; r < b.getrowNum(); r++) {
		double grad = db.getVectorD()[0][r];
		double new_b = b.getVectorD()[0][r] - lr * grad;
		b.setValue(r + 1, 1, new_b);
	}
}

// 辅助函数：矩阵标量乘法
Matrix VAE::multiplicationScalar(const Matrix& A, double scalar) {
	Matrix result(A.getrowNum(), A.getcolumnNum());
	for (int i = 0; i < A.getrowNum(); i++) {
		for (int j = 0; j < A.getcolumnNum(); j++) {
			double val = A.getVectorD()[j][i] * scalar;
			result.setValue(i + 1, j + 1, val);
		}
	}
	return result;
}

void VAE::load(const std::string& prefix) {
	load_matrix(W_e1, prefix + "_We1.txt");
	load_matrix(b_e1, prefix + "_be1.txt");
	load_matrix(W_e2, prefix + "_We2.txt");
	load_matrix(b_e2, prefix + "_be2.txt");
	load_matrix(W_mu, prefix + "_Wmu.txt");
	load_matrix(b_mu, prefix + "_bmu.txt");
	load_matrix(W_logvar, prefix + "_Wlogvar.txt");
	load_matrix(b_logvar, prefix + "_blogvar.txt");

	load_matrix(W_d1, prefix + "_Wd1.txt");
	load_matrix(b_d1, prefix + "_bd1.txt");
	load_matrix(W_d2, prefix + "_Wd2.txt");
	load_matrix(b_d2, prefix + "_bd2.txt");
	load_matrix(W_d3, prefix + "_Wd3.txt");
	load_matrix(b_d3, prefix + "_bd3.txt");
}


