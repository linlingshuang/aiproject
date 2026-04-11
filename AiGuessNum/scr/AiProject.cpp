#include<filesystem>
#include "VectorD.h"
#include "Matrix.h"
#include "NeuralNetwork.h"
#include "lodepng.h"
#include <SFML/Graphics.hpp>
using namespace std;

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
}

