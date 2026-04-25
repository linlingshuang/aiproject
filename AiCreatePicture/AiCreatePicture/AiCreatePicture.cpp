﻿﻿﻿#include "VAE.h"
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include "lodepng.h"

using namespace std;


void save_image(const Matrix& img, const string& filename) {
    vector<unsigned char> image_data(32 * 32);
    for (int i = 0; i < 32 * 32; i++) {
        double val = img.getVectorD()[0][i];
        val = max(0.0, min(1.0, val));
        image_data[i] = static_cast<unsigned char>(val * 255);
    }
    lodepng::encode(filename, image_data, 32, 32, LCT_GREY, 8);
    cout << "图片已保存为 " << filename << endl;
}

int main(int argc, char* argv[]) {
    const int input_dim = 32 * 32;
    const int hidden1 = 16;
    const int hidden2 = 8;
    const int latent_dim = 2;
    const double lr = 0.001;
    const int epochs = 1;

    string mode = "generate";
    if (argc > 1) {
        mode = argv[1];
    }

    VAE vae(input_dim, hidden1, hidden2, latent_dim);

    if (mode == "train") {
        cout << "===== 开始训练 =====" << endl;
        vae.loadTrainingData("E:/Code/Github/AiProject/AiCreatePicture/AiCreatePicture/trainingDigits");
        for (int epoch = 0; epoch < epochs; epoch++) {
            double total_loss = 0.0;
            for (size_t i = 0; i < vae.train_samples.size(); i++) {
                double loss = vae.train_step(vae.train_samples[i], lr);
                total_loss += loss;
            }
            double avg_loss = total_loss / vae.train_samples.size();
            cout << "Epoch " << epoch + 1 << "/" << epochs << ", 平均损失 = " << avg_loss << endl;
            
            if ((epoch + 1) % 10 == 0) {
                vae.save("vae_epoch" + to_string(epoch + 1));
                cout << "模型已保存" << endl;
            }
        }
        vae.save("vae_final");
        cout << "===== 训练完成 =====" << endl;
    }
    else if (mode == "generate") {
        cout << "===== 开始生成图片 =====" << endl;
        vae.load("vae_final");
        random_device rd;
        mt19937 gen(rd());
        normal_distribution<> dist(0.0, 1.0);
        for (int i = 0; i < 10; i++) {
            Matrix z(latent_dim, 1);
            for (int j = 0; j < latent_dim; j++) {
                z.setValue(j + 1, 1, dist(gen));
            }

            Matrix generated = vae.decode(z);
            save_image(generated, "generated_" + to_string(i) + ".png");
        }
        cout << "===== 生成完成 =====" << endl;
    }
    return 0;
}
