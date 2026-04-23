#include "VAE.h"
#include <iostream>
#include <vector>
#include <string>
#include "lodepng.h"  // 加载 PNG 图片

// 读取 32x32 手写数字图片（与你之前格式相同）
Matrix load_image(const std::string& filename) {
    std::vector<unsigned char> image;
    unsigned width, height;
    lodepng::decode(image, width, height, filename, LCT_GREY, 8);
    Matrix mat(32 * 32, 1);
    for (int y = 0; y < 32; ++y) {
        for (int x = 0; x < 32; ++x) {
            unsigned char pixel = image[y * width + x];
            double val = (pixel > 128) ? 1.0 : 0.0; // 二值化
            mat.setValue(y * 32 + x + 1, 1, val);
        }
    }
    return mat;
}

int main() {
    // 超参数
    const int input_dim = 32 * 32;  // 1024
    const int hidden1 = 256;
    const int hidden2 = 128;
    const int latent_dim = 20;
    const double lr = 0.001;
    const int epochs = 100;

    VAE vae(input_dim, hidden1, hidden2, latent_dim);

    // 加载训练数据（假设你有一堆 txt 或 png 文件）
    std::vector<Matrix> train_images;
    // 伪代码：遍历文件夹，用 load_image 读取，存入 train_images

    // 训练循环
    for (int epoch = 0; epoch < epochs; ++epoch) {
        double total_loss = 0.0;
        for (size_t i = 0; i < train_images.size(); ++i) {
            double loss = vae.train_step(train_images[i], lr);
            total_loss += loss;
        }
        std::cout << "Epoch " << epoch << ", avg loss = " << total_loss / train_images.size() << std::endl;
        // 每 10 个 epoch 保存一次模型
        if (epoch % 10 == 0) {
            vae.save("vae_epoch" + std::to_string(epoch));
        }
    }

    vae.save("vae_final");
    return 0;
}
/*
#include "VAE.h"
#include "lodepng.h"
#include <iostream>

int main() {
    VAE vae(1024, 256, 128, 20);
    vae.load("vae_final");  // 加载训练好的参数

    // 从标准正态分布采样隐变量
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> dist(0.0, 1.0);
    Matrix z(latent_dim, 1);
    for (int i = 0; i < latent_dim; ++i) {
        z.setValue(i+1, 1, dist(gen));
    }

    // 解码
    Matrix generated = vae.decode(z);  // 1024 x 1

    // 保存为 PNG 图片
    std::vector<unsigned char> image(32*32);
    for (int i = 0; i < 32*32; ++i) {
        double val = generated.getVectorD()[0][i];
        unsigned char pixel = static_cast<unsigned char>(val * 255);
        image[i] = pixel;
    }
    lodepng::encode("generated.png", image, 32, 32, LCT_GREY, 8);
    std::cout << "生成图片已保存为 generated.png\n";
    return 0;
}
*/