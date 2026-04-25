#include "VAE.h"
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include "lodepng.h"

using namespace std;

vector<Matrix> load_training_data(const string& folder_path) {
    vector<Matrix> data;
    int count = 0;
    cout << "开始加载训练数据，路径: " << folder_path << endl;
    try {
        for (const auto& entry : filesystem::directory_iterator(folder_path)) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt" && count < 10) {
                cout << "加载文件: " << entry.path().string() << endl;
                Matrix img = readTxtToMatrix(entry.path().string());
                data.push_back(img);
                count++;
                cout << "已加载 " << count << " 张图片" << endl;
            }
        }
    } catch (const filesystem::filesystem_error& e) {
        cerr << "文件系统错误: " << e.what() << endl;
    }
    cout << "加载完成，共加载了 " << data.size() << " 张训练图片" << endl;
    return data;
}

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
    const double lr = 0.0001;
    const int epochs = 1;

    string mode = "generate";
    if (argc > 1) {
        mode = argv[1];
    }

    VAE vae(input_dim, hidden1, hidden2, latent_dim);

    if (mode == "train") {
        cout << "===== 开始训练 =====" << endl;
        cout << "当前工作目录: " << filesystem::current_path() << endl;
        vector<Matrix> train_images = load_training_data("trainingDigits");
        
        if (train_images.empty()) {
            cerr << "没有找到训练数据！" << endl;
            return 1;
        }

        for (int epoch = 0; epoch < epochs; epoch++) {
            double total_loss = 0.0;
            for (size_t i = 0; i < train_images.size(); i++) {
                double loss = vae.train_step(train_images[i], lr);
                total_loss += loss;
            }
            double avg_loss = total_loss / train_images.size();
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
    else {
        cout << "使用方法：" << endl;
        cout << "  训练: AiCreatePicture.exe train" << endl;
        cout << "  生成: AiCreatePicture.exe generate" << endl;
    }

    return 0;
}
