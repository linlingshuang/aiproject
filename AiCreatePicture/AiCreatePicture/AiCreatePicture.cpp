#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include "lodepng.h"
#include "VAE.h"
#include <SFML/Graphics.hpp>


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
	const int hidden1 = 256;
	const int hidden2 = 128;
	const int latent_dim = 20;
	const double lr = 0.001;
	const int epochs = 5;

	int myNum;
	cout << "输入数字：";
	cin >> myNum;

	string mode;
	cout << "输入模式：";
	cin >> mode;


	if (argc > 1) {
		mode = argv[1];
	}

	VAE vae(input_dim, hidden1, hidden2, latent_dim);

	if (mode == "train") {
		cout << "===== 开始训练 =====" << endl;
		vae.loadTrainingData("E:/Code/Github/AiProject/AiCreatePicture/AiCreatePicture/trainingDigits/" + to_string(myNum));
		for (int epoch = 0; epoch < epochs; epoch++) {
			double total_loss = 0.0;
			shuffle(vae.train_samples.begin(), vae.train_samples.end(), default_random_engine(random_device()()));
			for (size_t i = 0; i < vae.train_samples.size(); i++) {
				if (i % 50 == 0) {
					cout << "#";
				}
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
		vae.save("vae_final" + to_string(myNum));
		cout << "===== 训练完成 =====" << endl;
	}
	else if (mode == "generate") {
		cout << "===== 开始生成图片 =====" << endl;
		vae.load("vae_final" + to_string(myNum));

		random_device rd;
		mt19937 gen(rd());
		normal_distribution<> dist(0.0, 1.0);
		Matrix z(latent_dim, 1);
		for (int j = 0; j < latent_dim; j++) {
			z.setValue(j + 1, 1, dist(gen));
		}

		Matrix generated = vae.decode(z);
		save_image(generated, "generated_" + to_string(myNum) + ".png");

		cout << "===== 生成完成 =====" << endl;
	}
	else if (mode == "DIY") {
		srand(time(0));

		sf::RenderWindow window(sf::VideoMode(640, 640), "AI generated number");
		sf::Texture text;
		text.loadFromFile("window.png");
		sf::Sprite myWindow(text);
		myWindow.setTextureRect(sf::IntRect(0, 0, 640, 640));

		// 文本对象
		sf::Text text1inform, text2Clear, text3trainTime;
		sf::Font font;
		font.loadFromFile("arial.ttf");
		text1inform.setFont(font);
		text2Clear.setFont(font);
		text3trainTime.setFont(font);

		text1inform.setString("draw in the \nwhite area\n20times");
		text1inform.setCharacterSize(48);
		text1inform.setFillColor(sf::Color::Black);
		text1inform.setStyle(sf::Text::Bold);
		text1inform.setPosition(40, 40);

		text2Clear.setString("clear \nthe area");
		text2Clear.setCharacterSize(48);
		text2Clear.setFillColor(sf::Color::Black);
		text2Clear.setStyle(sf::Text::Bold);
		text2Clear.setPosition(380, 480);

		int trainTime = 0;
		text3trainTime.setString(std::string("trainTime\n") + std::to_string(myNum));
		text3trainTime.setCharacterSize(48);
		text3trainTime.setFillColor(sf::Color::Black);
		text3trainTime.setStyle(sf::Text::Bold);
		text3trainTime.setPosition(380, 300);

		sf::Texture text4AI;
		text4AI.loadFromFile("generated_0.png");
		sf::Sprite DIYphoto(text4AI);
		DIYphoto.setTextureRect(sf::IntRect(0, 0, 32, 32));
		DIYphoto.setPosition(380, 40);

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
						trainTime++;
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
						pair<int, string>temp;
						temp.first = 0;
						temp.second = "DIY";
						vae.train_step(temp, lr, input);

						if (trainTime % 20 == 0) {
							vae.save("vae_final_DIY");
							cout << "===== 训练完成 =====" << endl;
							trainTime = 0;
							cout << "===== 开始生成图片 =====" << endl;
							vae.load("vae_final_DIY");

							random_device rd;
							mt19937 gen(rd());
							normal_distribution<> dist(0.0, 1.0);
							Matrix z(latent_dim, 1);
							for (int j = 0; j < latent_dim; j++) {
								z.setValue(j + 1, 1, dist(gen));
							}

							Matrix generated = vae.decode(z);
							save_image(generated, "generated_DIY.png");

							text4AI.loadFromFile("generated_DIY.png");
							DIYphoto = sf::Sprite(text4AI);
							DIYphoto.setTextureRect(sf::IntRect(0, 0, 32, 32));
							DIYphoto.setPosition(380, 40);
						}
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
			window.draw(text3trainTime);
			window.draw(drawSprite);
			window.draw(DIYphoto);
			window.display();
		}
	}
	return 0;
}
