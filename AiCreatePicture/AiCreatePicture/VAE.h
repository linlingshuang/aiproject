#pragma once
#include "Matrix.h"
#include <vector>
#include <cmath>
#include <random>
#include <filesystem>

class VAE {
public:
    // 构造函数：指定输入维度、隐藏层维度、隐变量维度
    VAE(int input_dim, int hidden_dim1, int hidden_dim2, int latent_dim);
    ~VAE();

    // 前向传播：输入 x，输出重构图像，同时返回 mu, logvar
    void forward(const Matrix& x, Matrix& recon, Matrix& mu, Matrix& logvar, Matrix& epsilon);

    void encode(const Matrix& x, Matrix& mu, Matrix& logvar);
    // 重参数化采样：给定 mu, logvar，返回采样 z
    Matrix reparameterize(const Matrix& mu, const Matrix& logvar, Matrix& epsilon);

    // 解码器单独前向：从 z 生成图像（用于生成阶段）
    Matrix decode(const Matrix& z);

    // 计算损失（重构损失 + KL 散度）
    double loss(const Matrix& x, const Matrix& recon, const Matrix& mu, const Matrix& logvar);

    // 训练一步（单样本或小批量），返回损失值
    double train_step(const pair<int, string> xOrigin, double learning_rate, Matrix DIY = Matrix(1,1,0));

    // 保存/加载模型参数
    void save(const std::string& prefix) const;
    void load(const std::string& prefix);


    vector<pair<int, string>> train_samples;  // 存储 (label, filepath)
    void loadTrainingData(const string& folder_path) {
        train_samples.clear();
        for (const auto& entry : filesystem::directory_iterator(folder_path)) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                string filename = entry.path().filename().string();
                //	cout << endl << "文件名：" << filename << endl;
                // 解析文件名：格式为 "数字_编号.txt"
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


private:
    int input_dim;
    int latent_dim;

    vector<Matrix>layerNeuron;

    // 编码器权重和偏置（三层： input -> hidden1 -> hidden2 -> mu/logvar）
    Matrix W_e1, b_e1;   // input -> hidden1
    Matrix W_e2, b_e2;   // hidden1 -> hidden2
    Matrix W_mu, b_mu;   // hidden2 -> mu
    Matrix W_logvar, b_logvar; // hidden2 -> logvar

    // 解码器权重和偏置（三层： latent -> hidden2 -> hidden1 -> output）
    Matrix W_d1, b_d1;   // latent -> hidden2
    Matrix W_d2, b_d2;   // hidden2 -> hidden1
    Matrix W_d3, b_d3;   // hidden1 -> output

    // 辅助函数：Xavier 初始化
    void init_weights(Matrix& W, int n_in, int n_out);
    void init_bias(Matrix& b, int n_out);
    
    // 反向传播函数
    void backward(const Matrix& x, const Matrix& recon, const Matrix& mu, const Matrix& logvar, const Matrix& z, const Matrix& epsilon, double learning_rate);
    
    // 辅助函数
    void update_weights(Matrix& W, const Matrix& dW, double lr);
    void update_biases(Matrix& b, const Matrix& db, double lr);
    Matrix multiplicationScalar(const Matrix& A, double scalar);
};