#include "VAE.h"
//#include "NeuralNetwork.h"

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

VAE::VAE(int input_dim, int hidden_dim1, int hidden_dim2, int latent_dim) {
    // ... 存储维度
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
void VAE::forward(const Matrix& x, Matrix& recon, Matrix& mu, Matrix& logvar) {
    // ---------- 编码器 ----------
    // 第一层：x -> h1 (ReLU)
    Matrix h1 = addition(multiplication(W_e1, x), b_e1);
    h1 = ReLU(h1);

    // 第二层：h1 -> h2 (ReLU)
    Matrix h2 = addition(multiplication(W_e2, h1), b_e2);
    h2 = ReLU(h2);

    // 输出 mu 和 logvar（线性，无激活）
    mu = addition(multiplication(W_mu, h2), b_mu);
    logvar = addition(multiplication(W_logvar, h2), b_logvar);

    // ---------- 重参数化 ----------
    Matrix z = reparameterize(mu, logvar);

    // ---------- 解码器 ----------
    // 第一层：z -> d1 (ReLU)
    Matrix d1 = addition(multiplication(W_d1, z), b_d1);
    d1 = ReLU(d1);

    // 第二层：d1 -> d2 (ReLU)
    Matrix d2 = addition(multiplication(W_d2, d1), b_d2);
    d2 = ReLU(d2);

    // 输出层：d2 -> recon (Sigmoid)
    Matrix out = addition(multiplication(W_d3, d2), b_d3);
    recon = sigmoid(out);
}
void VAE::encode(const Matrix& x, Matrix& mu, Matrix& logvar) {
    Matrix h1 = addition(multiplication(W_e1, x), b_e1);
    h1 = ReLU(h1);   // 使用你已有的 ReLU

    Matrix h2 = addition(multiplication(W_e2, h1), b_e2);
    h2 = ReLU(h2);

    mu = addition(multiplication(W_mu, h2), b_mu);
    logvar = addition(multiplication(W_logvar, h2), b_logvar);
}
Matrix VAE::reparameterize(const Matrix& mu, const Matrix& logvar) {
    // 计算标准差 sigma = exp(0.5 * logvar)
    Matrix sigma(mu.getrowNum(), 1);
    for (int i = 0; i < mu.getrowNum(); ++i) {
        double lv = logvar.getVectorD()[0][i];
        double s = exp(0.5 * lv);
        sigma.setValue(i + 1, 1, s);
    }

    // 生成标准正态随机数
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> dist(0.0, 1.0);
    Matrix epsilon(mu.getrowNum(), 1);
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

    Matrix h1 = addition(multiplication(W_d2, h2), b_d2);
    h1 = ReLU(h1);

    Matrix out = addition(multiplication(W_d3, h1), b_d3);
    out = sigmoid(out);  // 输出范围 (0,1)
    return out;
}
double VAE::loss(const Matrix& x, const Matrix& recon, const Matrix& mu, const Matrix& logvar) {
    // 重构损失（BCE）
    double recon_loss = 0.0;
    for (int i = 0; i < input_dim; ++i) {
        double xi = x.getVectorD()[0][i];
        double xhat = recon.getVectorD()[0][i];
        recon_loss -= xi * log(xhat + 1e-10) + (1 - xi) * log(1 - xhat + 1e-10);
    }

    // KL 散度
    double kl = 0.0;
    for (int j = 0; j < latent_dim; ++j) {
        double mu_j = mu.getVectorD()[0][j];
        double logvar_j = logvar.getVectorD()[0][j];
        kl += 1 + logvar_j - mu_j * mu_j - exp(logvar_j);
    }
    kl *= -0.5;

    return recon_loss + kl;
}
double VAE::train_step(const Matrix& x, double learning_rate) {
    // 1. 前向：得到 recon, mu, logvar
    Matrix recon, mu, logvar;
    forward(x, recon, mu, logvar);
    double L = loss(x, recon, mu, logvar);

    // 2. 数值梯度（对所有权重和偏置逐一扰动）
    // 以 W_e1 为例：
    Matrix grad_W_e1(W_e1.getrowNum(), W_e1.getcolumnNum(), 0.0);
    double eps = 1e-5;
    for (int r = 0; r < W_e1.getrowNum(); ++r) {
        for (int c = 0; c < W_e1.getcolumnNum(); ++c) {
            double old = W_e1.getVectorD()[c][r]; // 注意存储顺序
            W_e1.setValue(r + 1, c + 1, old + eps);
            Matrix recon2, mu2, logvar2;
            forward(x, recon2, mu2, logvar2);
            double L2 = loss(x, recon2, mu2, logvar2);
            grad_W_e1.setValue(r + 1, c + 1, (L2 - L) / eps);
            W_e1.setValue(r + 1, c + 1, old);
        }
    }
    // 更新参数
    for (int r = 0; r < W_e1.getrowNum(); ++r)
        for (int c = 0; c < W_e1.getcolumnNum(); ++c) {
            double grad = grad_W_e1.getVectorD()[c][r];
            double new_val = W_e1.getVectorD()[c][r] - learning_rate * grad;
            W_e1.setValue(r + 1, c + 1, new_val);
        }
    Matrix grad_b_e1(b_e1.getrowNum(), 1, 0.0);
    for (int r = 0; r < b_e1.getrowNum(); ++r) {
        double old = b_e1.getVectorD()[0][r];
        b_e1.setValue(r + 1, 1, old + eps);
        Matrix recon2, mu2, logvar2;
        forward(x, recon2, mu2, logvar2);
        double L2 = loss(x, recon2, mu2, logvar2);
        grad_b_e1.setValue(r + 1, 1, (L2 - L) / eps);
        b_e1.setValue(r + 1, 1, old);
    }
    // 更新 b_e1
    for (int r = 0; r < b_e1.getrowNum(); ++r) {
        double grad = grad_b_e1.getVectorD()[0][r];
        double new_val = b_e1.getVectorD()[0][r] - learning_rate * grad;
        b_e1.setValue(r + 1, 1, new_val);
    }

    Matrix grad_W_e2(W_e2.getrowNum(), W_e2.getcolumnNum(), 0.0);
    for (int r = 0; r < W_e2.getrowNum(); ++r) {
        for (int c = 0; c < W_e2.getcolumnNum(); ++c) {
            double old = W_e2.getVectorD()[c][r]; // 注意存储顺序
            W_e2.setValue(r + 1, c + 1, old + eps);
            Matrix recon2, mu2, logvar2;
            forward(x, recon2, mu2, logvar2);
            double L2 = loss(x, recon2, mu2, logvar2);
            grad_W_e2.setValue(r + 1, c + 1, (L2 - L) / eps);
            W_e2.setValue(r + 1, c + 1, old);
        }
    }
    // 更新参数
    for (int r = 0; r < W_e2.getrowNum(); ++r)
        for (int c = 0; c < W_e2.getcolumnNum(); ++c) {
            double grad = grad_W_e2.getVectorD()[c][r];
            double new_val = W_e2.getVectorD()[c][r] - learning_rate * grad;
            W_e2.setValue(r + 1, c + 1, new_val);
        }
    Matrix grad_b_e2(b_e2.getrowNum(), 1, 0.0);
    for (int r = 0; r < b_e2.getrowNum(); ++r) {
        double old = b_e2.getVectorD()[0][r];
        b_e2.setValue(r + 1, 1, old + eps);
        Matrix recon2, mu2, logvar2;
        forward(x, recon2, mu2, logvar2);
        double L2 = loss(x, recon2, mu2, logvar2);
        grad_b_e2.setValue(r + 1, 1, (L2 - L) / eps);
        b_e2.setValue(r + 1, 1, old);
    }
    // 更新 b_e2
    for (int r = 0; r < b_e2.getrowNum(); ++r) {
        double grad = grad_b_e2.getVectorD()[0][r];
        double new_val = b_e2.getVectorD()[0][r] - learning_rate * grad;
        b_e2.setValue(r + 1, 1, new_val);
    }

    Matrix grad_W_mu(W_mu.getrowNum(), W_mu.getcolumnNum(), 0.0);
    for (int r = 0; r < W_mu.getrowNum(); ++r) {
        for (int c = 0; c < W_mu.getcolumnNum(); ++c) {
            double old = W_mu.getVectorD()[c][r]; // 注意存储顺序
            W_mu.setValue(r + 1, c + 1, old + eps);
            Matrix recon2, mu2, logvar2;
            forward(x, recon2, mu2, logvar2);
            double L2 = loss(x, recon2, mu2, logvar2);
            grad_W_mu.setValue(r + 1, c + 1, (L2 - L) / eps);
            W_mu.setValue(r + 1, c + 1, old);
        }
    }
    // 更新参数
    for (int r = 0; r < W_mu.getrowNum(); ++r)
        for (int c = 0; c < W_mu.getcolumnNum(); ++c) {
            double grad = grad_W_mu.getVectorD()[c][r];
            double new_val = W_mu.getVectorD()[c][r] - learning_rate * grad;
            W_mu.setValue(r + 1, c + 1, new_val);
        }
    Matrix grad_b_mu(b_mu.getrowNum(), 1, 0.0);
    for (int r = 0; r < b_mu.getrowNum(); ++r) {
        double old = b_mu.getVectorD()[0][r];
        b_mu.setValue(r + 1, 1, old + eps);
        Matrix recon2, mu2, logvar2;
        forward(x, recon2, mu2, logvar2);
        double L2 = loss(x, recon2, mu2, logvar2);
        grad_b_mu.setValue(r + 1, 1, (L2 - L) / eps);
        b_mu.setValue(r + 1, 1, old);
    }
    // 更新 b_mu
    for (int r = 0; r < b_mu.getrowNum(); ++r) {
        double grad = grad_b_mu.getVectorD()[0][r];
        double new_val = b_mu.getVectorD()[0][r] - learning_rate * grad;
        b_mu.setValue(r + 1, 1, new_val);
    }

    Matrix grad_W_logvar(W_logvar.getrowNum(), W_logvar.getcolumnNum(), 0.0);
    for (int r = 0; r < W_logvar.getrowNum(); ++r) {
        for (int c = 0; c < W_logvar.getcolumnNum(); ++c) {
            double old = W_logvar.getVectorD()[c][r]; // 注意存储顺序
            W_logvar.setValue(r + 1, c + 1, old + eps);
            Matrix recon2, mu2, logvar2;
            forward(x, recon2, mu2, logvar2);
            double L2 = loss(x, recon2, mu2, logvar2);
            grad_W_logvar.setValue(r + 1, c + 1, (L2 - L) / eps);
            W_logvar.setValue(r + 1, c + 1, old);
        }
    }
    // 更新参数
    for (int r = 0; r < W_logvar.getrowNum(); ++r)
        for (int c = 0; c < W_logvar.getcolumnNum(); ++c) {
            double grad = grad_W_logvar.getVectorD()[c][r];
            double new_val = W_logvar.getVectorD()[c][r] - learning_rate * grad;
            W_logvar.setValue(r + 1, c + 1, new_val);
        }
    Matrix grad_b_logvar(b_logvar.getrowNum(), 1, 0.0);
    for (int r = 0; r < b_logvar.getrowNum(); ++r) {
        double old = b_logvar.getVectorD()[0][r];
        b_logvar.setValue(r + 1, 1, old + eps);
        Matrix recon2, mu2, logvar2;
        forward(x, recon2, mu2, logvar2);
        double L2 = loss(x, recon2, mu2, logvar2);
        grad_b_logvar.setValue(r + 1, 1, (L2 - L) / eps);
        b_logvar.setValue(r + 1, 1, old);
    }
    // 更新 b_logvar
    for (int r = 0; r < b_logvar.getrowNum(); ++r) {
        double grad = grad_b_logvar.getVectorD()[0][r];
        double new_val = b_logvar.getVectorD()[0][r] - learning_rate * grad;
        b_logvar.setValue(r + 1, 1, new_val);
    }
    
    Matrix grad_W_d1(W_d1.getrowNum(), W_d1.getcolumnNum(), 0.0);
    for (int r = 0; r < W_d1.getrowNum(); ++r) {
        for (int c = 0; c < W_d1.getcolumnNum(); ++c) {
            double old = W_d1.getVectorD()[c][r]; // 注意存储顺序
            W_d1.setValue(r + 1, c + 1, old + eps);
            Matrix recon2, mu2, logvar2;
            forward(x, recon2, mu2, logvar2);
            double L2 = loss(x, recon2, mu2, logvar2);
            grad_W_d1.setValue(r + 1, c + 1, (L2 - L) / eps);
            W_d1.setValue(r + 1, c + 1, old);
        }
    }
    // 更新参数
    for (int r = 0; r < W_d1.getrowNum(); ++r)
        for (int c = 0; c < W_d1.getcolumnNum(); ++c) {
            double grad = grad_W_d1.getVectorD()[c][r];
            double new_val = W_d1.getVectorD()[c][r] - learning_rate * grad;
            W_d1.setValue(r + 1, c + 1, new_val);
        }
    Matrix grad_b_d1(b_d1.getrowNum(), 1, 0.0);
    for (int r = 0; r < b_d1.getrowNum(); ++r) {
        double old = b_d1.getVectorD()[0][r];
        b_d1.setValue(r + 1, 1, old + eps);
        Matrix recon2, mu2, logvar2;
        forward(x, recon2, mu2, logvar2);
        double L2 = loss(x, recon2, mu2, logvar2);
        grad_b_d1.setValue(r + 1, 1, (L2 - L) / eps);
        b_d1.setValue(r + 1, 1, old);
    }
    // 更新 b_d1
    for (int r = 0; r < b_d1.getrowNum(); ++r) {
        double grad = grad_b_d1.getVectorD()[0][r];
        double new_val = b_d1.getVectorD()[0][r] - learning_rate * grad;
        b_d1.setValue(r + 1, 1, new_val);
    }

    Matrix grad_W_d2(W_d2.getrowNum(), W_d2.getcolumnNum(), 0.0);
    for (int r = 0; r < W_d2.getrowNum(); ++r) {
        for (int c = 0; c < W_d2.getcolumnNum(); ++c) {
            double old = W_d2.getVectorD()[c][r]; // 注意存储顺序
            W_d2.setValue(r + 1, c + 1, old + eps);
            Matrix recon2, mu2, logvar2;
            forward(x, recon2, mu2, logvar2);
            double L2 = loss(x, recon2, mu2, logvar2);
            grad_W_d2.setValue(r + 1, c + 1, (L2 - L) / eps);
            W_d2.setValue(r + 1, c + 1, old);
        }
    }
    // 更新参数
    for (int r = 0; r < W_d2.getrowNum(); ++r)
        for (int c = 0; c < W_d2.getcolumnNum(); ++c) {
            double grad = grad_W_d2.getVectorD()[c][r];
            double new_val = W_d2.getVectorD()[c][r] - learning_rate * grad;
            W_d2.setValue(r + 1, c + 1, new_val);
        }
    Matrix grad_b_d2(b_d2.getrowNum(), 1, 0.0);
    for (int r = 0; r < b_d2.getrowNum(); ++r) {
        double old = b_d2.getVectorD()[0][r];
        b_d2.setValue(r + 1, 1, old + eps);
        Matrix recon2, mu2, logvar2;
        forward(x, recon2, mu2, logvar2);
        double L2 = loss(x, recon2, mu2, logvar2);
        grad_b_d2.setValue(r + 1, 1, (L2 - L) / eps);
        b_d2.setValue(r + 1, 1, old);
    }
    // 更新 b_d2
    for (int r = 0; r < b_d2.getrowNum(); ++r) {
        double grad = grad_b_d2.getVectorD()[0][r];
        double new_val = b_d2.getVectorD()[0][r] - learning_rate * grad;
        b_d2.setValue(r + 1, 1, new_val);
    }

    Matrix grad_W_d3(W_d3.getrowNum(), W_d3.getcolumnNum(), 0.0);
    for (int r = 0; r < W_d3.getrowNum(); ++r) {
        for (int c = 0; c < W_d3.getcolumnNum(); ++c) {
            double old = W_d3.getVectorD()[c][r]; // 注意存储顺序
            W_d3.setValue(r + 1, c + 1, old + eps);
            Matrix recon2, mu2, logvar2;
            forward(x, recon2, mu2, logvar2);
            double L2 = loss(x, recon2, mu2, logvar2);
            grad_W_d3.setValue(r + 1, c + 1, (L2 - L) / eps);
            W_d3.setValue(r + 1, c + 1, old);
        }
    }
    // 更新参数
    for (int r = 0; r < W_d3.getrowNum(); ++r)
        for (int c = 0; c < W_d3.getcolumnNum(); ++c) {
            double grad = grad_W_d3.getVectorD()[c][r];
            double new_val = W_d3.getVectorD()[c][r] - learning_rate * grad;
            W_d3.setValue(r + 1, c + 1, new_val);
        }
    Matrix grad_b_d3(b_d3.getrowNum(), 1, 0.0);
    for (int r = 0; r < b_d3.getrowNum(); ++r) {
        double old = b_d3.getVectorD()[0][r];
        b_d3.setValue(r + 1, 1, old + eps);
        Matrix recon2, mu2, logvar2;
        forward(x, recon2, mu2, logvar2);
        double L2 = loss(x, recon2, mu2, logvar2);
        grad_b_d3.setValue(r + 1, 1, (L2 - L) / eps);
        b_d3.setValue(r + 1, 1, old);
    }
    // 更新 b_d3
    for (int r = 0; r < b_d3.getrowNum(); ++r) {
        double grad = grad_b_d3.getVectorD()[0][r];
        double new_val = b_d3.getVectorD()[0][r] - learning_rate * grad;
        b_d3.setValue(r + 1, 1, new_val);
    }

    return L;
}
void save_matrix(const Matrix& M, const std::string& num) {
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
void VAE::save(const std::string& prefix)const {

    save_matrix(W_e1, prefix + "_We1.bin");
    save_matrix(b_e1, prefix + "_be1.bin");
    save_matrix(W_e2, prefix + "_We1.bin");
    save_matrix(b_e2, prefix + "_be1.bin");
    save_matrix(W_mu, prefix + "_We1.bin");
    save_matrix(b_mu, prefix + "_be1.bin");
    save_matrix(W_logvar, prefix + "_We1.bin");
    save_matrix(b_logvar, prefix + "_be1.bin");

    save_matrix(W_d1, prefix + "_We1.bin");
    save_matrix(b_d1, prefix + "_be1.bin");
    save_matrix(W_d2, prefix + "_We1.bin");
    save_matrix(b_d2, prefix + "_be1.bin");
    save_matrix(W_d3, prefix + "_We1.bin");
    save_matrix(b_d3, prefix + "_be1.bin");
}
void VAE::load(const std::string& prefix) {
   
}


